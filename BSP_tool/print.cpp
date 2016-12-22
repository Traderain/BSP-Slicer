//
// BSP_tool - botman's Half-Life BSP utilities
//
// (http://planethalflife.com/botman/)
//
// print.cpp
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

#include "cmdlib.h"
#include "bspfile.h"


void print_models(void)
{
   int i;

   printf("Printing %d Models...\n\n", nummodels);

   for (i=0; i < nummodels; i++)
   {
      printf("Model index           : %d\n", i);
      printf("Model boundbox min (X): %f\n", dmodels[i].mins[0]);
      printf("                   (Y): %f\n", dmodels[i].mins[1]);
      printf("                   (Z): %f\n", dmodels[i].mins[2]);
      printf("Model boundbox max (X): %f\n", dmodels[i].maxs[0]);
      printf("                   (Y): %f\n", dmodels[i].maxs[1]);
      printf("                   (Z): %f\n", dmodels[i].maxs[2]);
      printf("Model origin       (X): %f\n", dmodels[i].origin[0]);
      printf("                   (Y): %f\n", dmodels[i].origin[1]);
      printf("                   (Z): %f\n", dmodels[i].origin[2]);
      printf("Model headnode[0]     : %d\n", dmodels[i].headnode[0]);
      printf("      headnode[1]     : %d\n", dmodels[i].headnode[1]);
      printf("      headnode[2]     : %d\n", dmodels[i].headnode[2]);
      printf("      headnode[3]     : %d\n", dmodels[i].headnode[3]);
      printf("Model Visleafs        : %d\n", dmodels[i].visleafs);
      printf("Model first poly ID   : %d\n", dmodels[i].firstface);
      printf("Model num faces/polys : %d\n\n", dmodels[i].numfaces);
   }
}


void print_vertexes(void)
{
   int i;

   printf("Printing %d Vertexes...\n\n", numvertexes);

   for (i=0; i < numvertexes; i++)
   {
      printf("Vertex index : %d\n", i);
      printf("Vertex   (X) : %f\n",   dvertexes[i].point[0]);
      printf("         (Y) : %f\n",   dvertexes[i].point[1]);
      printf("         (Z) : %f\n\n", dvertexes[i].point[2]);
   }
}


void print_planes(void)
{
   int i;

   printf("Printing %d Planes...\n\n", numplanes);

   for (i=0; i < numplanes; i++)
   {
      printf("Plane index            : %d\n", i);
      printf("Plane normal  (X)      : %f\n", dplanes[i].normal[0]);
      printf("Plane normal  (Y)      : %f\n", dplanes[i].normal[1]);
      printf("Plane normal  (Z)      : %f\n", dplanes[i].normal[2]);
      printf("Plane dist from (0,0,0): %f\n", dplanes[i].dist);
      printf("Plane type             : %d\n\n", dplanes[i].type);
   }
}


void print_leafs(void)
{
   int i;

   printf("Printing %d Leafs...\n\n", numleafs);

   for (i=0; i < numleafs; i++)
   {
      printf("Leaf index            : %d\n", i);
      printf("Leaf type             : %d\n", dleafs[i].contents);
      printf("Leaf vislist          : %d\n", dleafs[i].visofs);
      printf("Leaf boundbox min (X) : %d\n", dleafs[i].mins[0]);
      printf("                  (Y) : %d\n", dleafs[i].mins[1]);
      printf("                  (Z) : %d\n", dleafs[i].mins[2]);
      printf("Leaf boundbox max (X) : %d\n", dleafs[i].maxs[0]);
      printf("                  (Y) : %d\n", dleafs[i].maxs[1]);
      printf("                  (Z) : %d\n", dleafs[i].maxs[2]);
      printf("Leaf idx of facelist  : %d\n", dleafs[i].firstmarksurface);
      printf("Leaf number of faces  : %d\n", dleafs[i].nummarksurfaces);
      printf("Ambient sounds: water : %d\n", dleafs[i].ambient_level[0]);
      printf("                sky   : %d\n", dleafs[i].ambient_level[1]);
      printf("                slime : %d\n", dleafs[i].ambient_level[2]);
      printf("                lava  : %d\n\n", dleafs[i].ambient_level[3]);
  }
}


void print_nodes(void)
{
   int i;

   printf("Printing %d Nodes...\n\n", numnodes);

   for (i=0; i < numnodes; i++)
   {
      printf("Node index           : %d\n", i);
      printf("Plane ID             : %d\n", dnodes[i].planenum);

      if ((dnodes[i].children[0] & 0x8000) == 0)  // is NOT negative?
         printf("Front child node     : %d\n", dnodes[i].children[0]);
      else
         printf("Front child leaf     : %d\n", ~dnodes[i].children[0]);

      if ((dnodes[i].children[1] & 0x8000) == 0)  // is NOT negative?
         printf("Back child node      : %d\n", dnodes[i].children[1]);
      else
         printf("Back child leaf      : %d\n", ~dnodes[i].children[0]);

      printf("Box minimum (X)      : %d\n", dnodes[i].mins[0]);
      printf("            (Y)      : %d\n", dnodes[i].mins[1]);
      printf("            (Z)      : %d\n", dnodes[i].mins[2]);
      printf("Box maximum (X)      : %d\n", dnodes[i].maxs[0]);
      printf("            (Y)      : %d\n", dnodes[i].maxs[1]);
      printf("            (Z)      : %d\n", dnodes[i].maxs[2]);
      printf("First polygon ID     : %d\n", dnodes[i].firstface);
      printf("Number of faces/polys: %d\n\n", dnodes[i].numfaces);
   }
}


void print_texinfo(void)
{
   int i;

   printf("Printing %d Texinfos...\n\n", numtexinfo);

   for (i=0; i < numtexinfo; i++)
   {
      printf("Texinfo index : %d\n", i);
      printf("   vecs[0][0] : %5.2f\n", texinfo[i].vecs[0][0]);
      printf("   vecs[0][1] : %5.2f\n", texinfo[i].vecs[0][1]);
      printf("   vecs[0][2] : %5.2f\n", texinfo[i].vecs[0][2]);
      printf("   vecs[0][3] : %5.2f\n", texinfo[i].vecs[0][3]);
      printf("   vecs[1][0] : %5.2f\n", texinfo[i].vecs[1][0]);
      printf("   vecs[1][1] : %5.2f\n", texinfo[i].vecs[1][1]);
      printf("   vecs[1][2] : %5.2f\n", texinfo[i].vecs[1][2]);
      printf("   vecs[1][3] : %5.2f\n", texinfo[i].vecs[1][3]);
      printf("       miptex : %d\n", texinfo[i].miptex);
      printf("        flags : %d\n\n", texinfo[i].flags);
   }
}


void print_miptex(void)
{
   int i;
   dmiptexlump_t  *mtl;
   miptex_t* MipTex;
  
   mtl = (dmiptexlump_t *)dtexdata;

   printf("Printing %d Miptexes...\n\n", mtl->nummiptex);

   for (i=0; i < mtl->nummiptex; i++)
   {
      printf("Mip index       : %d\n", i);

      if (mtl->dataofs[i] != -1)
      {
         printf("Mip offset      : %ld\n", mtl->dataofs[i]);

         MipTex = (miptex_t*)(dtexdata + mtl->dataofs[i]);

         printf("Mip name        : %s\n", MipTex->name);
         printf("Mip width       : %d\n", MipTex->width);
         printf("Mip height      : %d\n", MipTex->height);
         printf("Mip offset0     : %ld\n", MipTex->offsets[0]);
         printf("Mip offset1     : %ld\n", MipTex->offsets[1]);
         printf("Mip offset2     : %ld\n", MipTex->offsets[2]);
         printf("Mip offset3     : %ld\n\n", MipTex->offsets[3]);
      }
      else
         printf("Mip offset      : %ld (mip is unused)\n\n", mtl->dataofs[i]);
   }
}


void print_faces(void)
{
   int i;

   printf("Printing %d Faces...\n\n", numfaces);

   for (i=0; i < numfaces; i++)
   {
      printf("Face index         : %d\n", i);
      printf("Face plane Id      : %d\n", dfaces[i].planenum);
      printf("Face plane side    : %d\n", dfaces[i].side);
      printf("Face First edge    : %d\n", dfaces[i].firstedge);
      printf("Face nr. of edges  : %d\n", dfaces[i].numedges);
      printf("Face Texinfo Id    : %d\n", dfaces[i].texinfo);
      printf("Face type of light : %d\n", dfaces[i].styles[0]);
      printf("Face baselight     : %d\n", dfaces[i].styles[1]);
      printf("Face light [1]     : %d\n", dfaces[i].styles[2]);
      printf("Face light [2]     : %d\n", dfaces[i].styles[3]);
      printf("Face lightmap      : %d\n\n", dfaces[i].lightofs);
   }
}


void print_edges(void)
{
   int i;

   printf("Printing %d Edges...\n\n", numedges);

   for (i=0; i < numedges; i++)
   {
      printf("Edge index            : %d\n", i);
      printf("Edge vertex index 0   : %d\n", dedges[i].v[0]);
      printf("Edge vertex index 1   : %d\n\n", dedges[i].v[1]);
   }
}


void print_entities(void)
{
   int i;
   epair_t *pEpair;

   printf("Printing %d Entities...\n\n", num_entities);

   for (i=0; i < num_entities; i++)
   {
      printf("Entity index      : %d\n", i);
      printf("Entity origin (X) : %f\n", entities[i].origin[0]);
      printf("              (Y) : %f\n", entities[i].origin[1]);
      printf("              (Z) : %f\n", entities[i].origin[2]);
      printf("First brush       : %d\n", entities[i].firstbrush);
      printf("Number of brushes : %d\n", entities[i].numbrushes);

      pEpair = entities[i].epairs;

      while (pEpair)
      {
         printf("Key/Value         : %s/%s\n", pEpair->key, pEpair->value);
         pEpair = pEpair->next;
      }
      printf("\n");
   }
}

