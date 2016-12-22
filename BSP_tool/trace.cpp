//
// BSP_tool - botman's Half-Life BSP utilities
//
// (http://planethalflife.com/botman/)
//
// trace.cpp
//
// Copyright (C) 2001 - Jeffrey "botman" Broome
//
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
//
// See the GNU General Public License for more details at:
// http://www.gnu.org/copyleft/gpl.html
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "math.h"

#include "bspfile.h"
#include "mathlib.h"
#include "trace.h"

#define MAX(a,b) (a > b ? a : b)
#define MIN(a,b) (a < b ? a : b)

dnode_t *gNode;

// return the leaf node for a point in 3D space...
dleaf_t *TracePointInLeaf(vec3_t point)
{
   int      nodenum;
   vec_t    d;
   dnode_t  *node;
   dplane_t *plane;

   nodenum = 0;  // start at the root of the BSP tree

   // walk through the tree to find the leaf for the point...
   while (nodenum >= 0)
   {
      node = &dnodes[nodenum];
      plane = &dplanes[node->planenum];
      d = DotProduct(point, plane->normal) - plane->dist;
      if (d > 0)
         nodenum = node->children[0];
      else
         nodenum = node->children[1];
   }

   gNode = node;
   return &dleafs[-nodenum - 1];
}

// find the contents of a coordinate in 3D space...
int PointContents(vec3_t coord)
{
   dleaf_t  *leaf;

   leaf = TracePointInLeaf(coord);

   // return contents (CONTENTS_EMPTY, CONTENTS_SOLID, CONTENTS_WATER, etc.)
   return leaf->contents;
}


// trace a line from start to end, fill in trace_t structure with result...
void TraceLine(vec3_t start, vec3_t end, trace_t *tr)
{
   dleaf_t  *startleaf, *endleaf;
   int      numsteps, totalsteps;
   vec3_t   move, step, position;
   float    dist, trace_dist;

   memset(tr, 0, sizeof(trace_t));

   if ((start[0] < -4095) || (start[0] > 4095) ||
       (start[1] < -4095) || (start[1] > 4095) ||
       (start[2] < -4095) || (start[2] > 4095))
   {
      // start beyond edge of world is INVALID!!!
      Error("TraceLine: start point beyond edge of world!\n");
   }

   if (end[0] > 4095.0f)
   {
      float percent = 4095.0f / end[0];
      end[1] = end[1] * percent;
      end[2] = end[2] * percent;
      end[0] = 4095.0f;
   }

   if (end[1] > 4095.0f)
   {
      float percent = 4095.0f / end[1];
      end[0] = end[0] * percent;
      end[2] = end[2] * percent;
      end[1] = 4095.0f;
   }

   if (end[2] > 4095.0f)
   {
      float percent = 4095.0f / end[2];
      end[0] = end[0] * percent;
      end[1] = end[1] * percent;
      end[2] = 4095.0f;
   }

   if (end[0] < -4095.0f)
   {
      float percent = 4095.0f / end[0];
      end[1] = end[1] * percent;
      end[2] = end[2] * percent;
      end[0] = -4095.0f;
   }

   if (end[1] < -4095.0f)
   {
      float percent = 4095.0f / end[1];
      end[0] = end[0] * percent;
      end[2] = end[2] * percent;
      end[1] = -4095.0f;
   }

   if (end[2] < -4095.0f)
   {
      float percent = 4095.0f / end[2];
      end[0] = end[0] * percent;
      end[1] = end[1] * percent;
      end[2] = -4095.0f;
   }

   // find the starting and ending leafs...
   startleaf = TracePointInLeaf(start);
   endleaf = TracePointInLeaf(end);

   // set endpos, fraction and contents to the default (trace completed)
   VectorCopy(end, tr->endpos);
   tr->fraction = 1.0f;
   tr->contents = endleaf->contents;

   if (startleaf->contents == CONTENTS_SOLID)
   {
      tr->startsolid = TRUE;

      if (endleaf->contents == CONTENTS_SOLID)
         tr->allsolid = TRUE;
   }

   // is start and end leaf the same (couldn't possibly hit the world)...
   if (startleaf == endleaf)
      return;

   // get the length of each interation of the loop...
   VectorSubtract(end, start, move);
   dist = (float)VectorLength(move);

   // determine the number of steps from start to end...
   if (dist > 1.0f)
      numsteps = totalsteps = (int)dist + 1;
   else
      numsteps = totalsteps = 1;

   // calculate the length of the step vector...
   VectorScale(move, (float)2/numsteps, step);

   VectorCopy(start, position);

   while (numsteps)
   {
      VectorAdd(position, step, position);

      endleaf = TracePointInLeaf(position);

      if ((endleaf->contents == CONTENTS_SOLID) ||  // we hit something solid...
          (endleaf->contents == CONTENTS_SKY))      // we hit the sky
      {
         vec3_t hitpos;

         VectorCopy(position, hitpos);

         // store the hit position
         VectorCopy(position, tr->hitpos);

         // back off one step before solid
         VectorSubtract(position, step, position);

         // store the end position and end position contents
         VectorCopy(position, tr->endpos);
         tr->contents = endleaf->contents;

         VectorSubtract(position, start, move);
         trace_dist = (float)VectorLength(move);
         tr->fraction = trace_dist / dist;

         break;  // break out of while loop
      }

      numsteps--;
   }
}

#define TWO_PI 6.2831853f
#define DELTA 0.001f

// find the face where the traceline hit...
dface_t *TraceLineFindFace(vec3_t start, trace_t *tr)
{
   vec3_t v_intersect, v_normalized, v_temp;
   dface_t *return_face = NULL;
   float min_diff = 9999.9f;

   VectorSubtract(tr->endpos, start, v_normalized);
   VectorNormalize(v_normalized);

   dleaf_t *endleaf = TracePointInLeaf(tr->endpos);

   unsigned short *p = dmarksurfaces + endleaf->firstmarksurface;

   // find a plane with endpos on one side and hitpos on the other side...
   for (int i = 0; i < endleaf->nummarksurfaces; i++)
   {
      int face_idx = *p++;

      dface_t *face = &dfaces[face_idx];

      dplane_t *plane = &dplanes[face->planenum];

      float d1 = DotProduct(tr->endpos, plane->normal) - plane->dist;
      float d2 = DotProduct(tr->hitpos, plane->normal) - plane->dist;

      if ((d1 > 0 && d2 <= 0)||(d1 <= 0 && d2 > 0))
      {
         // found a plane, find the intersection point in the plane...

         vec3_t plane_origin, v_angle1, v_angle2;

         VectorScale(plane->normal, plane->dist, plane_origin);

         float dist = DistanceToIntersection(start, v_normalized, plane_origin, plane->normal);

         if (dist < 0)
            return NULL;  // can't find intersection

         VectorScale(v_normalized, dist, v_temp);
         VectorAdd(start, v_temp, v_intersect);

         // loop through all of the vertexes of all the edges of this face and
         // find the angle between vertex-n, v_intersect and vertex-n+1, then add
         // all these angles together.  if the sum of these angles is 360 degrees
         // (or 2 PI radians), then the intersect point lies within that polygon.

         float angle_sum = 0.0f;

         // loop though all of the edges, getting the vertexes...
         for (int edge_index = 0; edge_index < face->numedges; edge_index++)
         {
             vec3_t vertex1, vertex2;

            // get the coordinates of the vertex of this edge...
            int edge = dsurfedges[face->firstedge + edge_index];

            if (edge < 0)
            {
               edge = -edge;
               dedge_t* e = &dedges[edge];
               VectorCopy(dvertexes[e->v[1]].point, vertex1);
               VectorCopy(dvertexes[e->v[0]].point, vertex2);
            }
            else
            {
               dedge_t* e = &dedges[edge];
               VectorCopy(dvertexes[e->v[0]].point, vertex1);
               VectorCopy(dvertexes[e->v[1]].point, vertex2);
            }

            // now create vectors from the vertexes to the plane intersect point...
            VectorSubtract(vertex1, v_intersect, v_angle1);
            VectorSubtract(vertex2, v_intersect, v_angle2);

            VectorNormalize(v_angle1);
            VectorNormalize(v_angle2);

            // find the angle between these vectors...
            float angle = DotProduct(v_angle1, v_angle2);

            angle = (float)acos(angle);

            angle_sum += angle;

            edge++;
         }

         // is the sum of the angles 360 degrees (2 PI)?...
         if ((angle_sum >= (TWO_PI - DELTA)) && (angle_sum <= (TWO_PI + DELTA)))
         {
            // find the difference between the sum and 2 PI...
            float diff = (float)fabs(angle_sum - TWO_PI);

            if (diff < min_diff)  // is this the BEST so far?...
            {
               min_diff = diff;
               return_face = face;
            }
         }
      }
   }

   return return_face;
}
