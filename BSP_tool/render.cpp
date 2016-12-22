//
// BSP_tool - botman's Half-Life BSP utilities
//
// (http://planethalflife.com/botman/)
//
// render.cpp
//
// Copyright (C) 2001 - Jeffrey "botman" Broome
//

/* 
 * HL rendering engine
 * Copyright (c) 2000,2001 Bart Sekura
 *
 * Permission to use, copy, modify and distribute this software
 * is hereby granted, provided that both the copyright notice and 
 * this permission notice appear in all copies of the software, 
 * derivative works or modified versions.
 *
 * THE AUTHOR ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION AND DISCLAIMS ANY LIABILITY OF ANY KIND FOR ANY DAMAGES 
 * WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 *
 * OpenGL window
 */

//
// Parts of this code is from:
//
//                  POLY ENGINE version 0.79
//                Written by Alexey Goloshubin
//

//
// Parts of this code is from:
//
// WinQuake, GLQuake, QuakeWorld, and GLQuakeWorld
// Copyright (C) 1996-1997 Id Software, Inc.
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

#include "render.h"
#include "matrix.h"

#include "config.h"
#include "bspfile.h"
#include "camera.h"
#include "sys_time.h"
#include "entity.h"
#include "world.h"
#include "trace.h"

#include "fastmath.h"

#ifndef VK_UP
#define VK_UP    GLUT_KEY_UP
#define VK_DOWN  GLUT_KEY_DOWN
#define VK_LEFT  GLUT_KEY_LEFT
#define VK_RIGHT GLUT_KEY_RIGHT
#endif

Camera       m_camera;

bool draw_traceline_pt = FALSE;
vec3_t traceline_pt[2];

// studio model stuff..
vec3_t g_vright;  // needs to be set to viewer's right in order for chrome to work
float  g_lambert = 1.5;

extern int num_textures;
extern Texture *textures[];
extern Texture *sky_textures[];

extern Config config;

extern float gametime;
extern float frames_per_second;

extern bool switchable_texture_off;

extern vec3_t spawn_point;
extern float spawn_point_yaw;

#ifndef MAX_KEYS
#define MAX_KEYS 256
#endif

bool keys[MAX_KEYS];
bool left_button;
bool right_button;
int  xpos;  // for mouse
int  ypos;  // for mouse
int  m_width;
int  m_height;
bool m_center_window;
bool m_fullscreen;

////////////////////////////////////////////////////////////


template<const int elem_size>
class array_t {
public:
#define CHUNK   (4096)
    array_t() {
        size = CHUNK*elem_size;
        data = new float[size];
        count = 0;
    }

    ~array_t() {
        if(data) {
            delete data;
            data = 0;
        }
    }

    void add(float* src) {
        if(count >= size/elem_size) {
            //resize
            int s = size;
            size += CHUNK*elem_size;
            float* n = new float[size];
            for(int i = 0; i < s; i++) {
                n[i] = data[i];
            }
            delete data;
            data = n;
        }

        float* dst = data + count*elem_size;
        for(int i = 0; i < elem_size; i++) {
            *dst++ = *src++;
        }
        count++;
    }

    operator float*(void) const { return data; }
    const float* operator[](int i) const { return data + i*elem_size; }

    int current() const { return count; }

private:
    int size;
    int count;
    float* data;
};

static array_t<3> vt_array;
static array_t<2> st_array;
static array_t<2> lst_array;


// visible faces by texture index
face_t **visible_faces = NULL;  // pointer to array of face_t pointers
int max_visible_faces;

unsigned char *marked_faces = NULL;
face_t **faces = NULL;  // pointer to array of face_t pointers
face_t *skyfaces = NULL;


struct frustum_plane_t {
    vec3_t normal;
    float dist;
};

#define Z_DEPTH (8000.0f)

// frustum clipping planes
frustum_plane_t frustum_planes[5];

// frustum params
double projection;
double angle_horiz;
double angle_vert;
double sin_horiz;
double cos_horiz;
double sin_vert;
double cos_vert;

// statistics
int rendered;

void projection_setup(int width, int height);

// collision stuff
void check_collisions(const vec3_t& src, vec3_t dir, vec3_t out);

void frustum_setup(const Camera& cam);
bool frustum_cull(const short* mins, const short* maxs);
int find_leaf(const vec3_t& coords);


static double znear = 1.0;

struct frustum_t {
    double left;
    double right;
    double bottom;
    double top;
    double znear;
    double zfar;
};

static frustum_t frustum;


void projection_setup(int width, int height)
{
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);

    double aspect = (double) height/ (double) width;
    double fovx   = 80;
    double rprojz = tan(deg2rad(fovx)*0.5);

    glLoadIdentity();
    glFrustum(-rprojz, +rprojz, -aspect*rprojz, +aspect*rprojz, znear, Z_DEPTH);

    frustum.left = -rprojz;
    frustum.right = +rprojz;
    frustum.bottom = -aspect*rprojz;
    frustum.top = +aspect*rprojz;
    frustum.znear = 1.0;
    frustum.zfar = Z_DEPTH;

    double s = sin(deg2rad(fovx)*0.5);
    double c = cos(deg2rad(fovx)*0.5);

    // precompute stuff for frustum culling
    projection = (c*(double)(width>>1))/s;
    angle_horiz = atan2((double)(width>>1),projection);
    angle_vert = atan2((double)(height>>1),projection);
    sin_horiz = sin(angle_horiz);
    sin_vert = sin(angle_vert);
    cos_horiz = cos(angle_horiz);
    cos_vert = cos(angle_vert);

    // back to model view
    glMatrixMode(GL_MODELVIEW);
}

void frustum_setup(const Camera& cam)
{
    int i = 0;
    vec3_t v;
    matrix_t m = cam.matrix();

    // znear plane
    v[0] = 0;  v[1] = 0;  v[2] = 1.0f;
    m.multiply(v, frustum_planes[i].normal);
    frustum_planes[i].dist = DotProduct(cam.eye(), frustum_planes[i].normal);
    i++;

    // left plane
    v[0] = cos_horiz;  v[1] = 0;  v[2] = sin_horiz;
    m.multiply(v, frustum_planes[i].normal);
    frustum_planes[i].dist = DotProduct(cam.eye(), frustum_planes[i].normal);
    i++;

    // right plane
    v[0] = -cos_horiz;  v[1] = 0;  v[2] = sin_horiz;
    m.multiply(v, frustum_planes[i].normal);
    frustum_planes[i].dist = DotProduct(cam.eye(), frustum_planes[i].normal);
    i++;

    // top plane
    v[0] = 0;  v[1] = cos_vert;  v[2] = sin_vert;
    m.multiply(v, frustum_planes[i].normal);
    frustum_planes[i].dist = DotProduct(cam.eye(), frustum_planes[i].normal);
    i++;

    // bottom plane
    v[0] = 0,  v[1] = -cos_vert;  v[2] = sin_vert;
    m.multiply(v, frustum_planes[i].normal);
    frustum_planes[i].dist = DotProduct(cam.eye(), frustum_planes[i].normal);
    i++;
}

bool frustum_cull(const short* mins, const short* maxs)
{
    for (int i = 0; i < 5; i++)
    {
        const short* min = mins, *max = maxs;
        const float* n = frustum_planes[i].normal;
        float d = -frustum_planes[i].dist;

        d += (*n>0 ? (*n*(float)*min) : (*n*(float)*max)); min++, max++; n++;
        d += (*n>0 ? (*n*(float)*min) : (*n*(float)*max)); min++, max++; n++;
        d += (*n>0 ? (*n*(float)*min) : (*n*(float)*max)); min++, max++; n++;

        if (d > 0)
            return true;
    }

    return false;
}

int find_leaf(const vec3_t& coords)
{
    int i = dmodels[0].headnode[0];

    while(i>=0)
    {
        dnode_t* n = &dnodes[i];
        dplane_t* p = &dplanes[n->planenum];

        float d;

        if(p->type <= PLANE_Z)  // easier for axial planes
        {
            d = coords[p->type] - p->dist;
        }
        else
        {
            // f(x,y,z) = Ax+Ay+Az-D
            d = p->normal[0]*coords[0]
               +p->normal[1]*coords[1]
               +p->normal[2]*coords[2]
               -p->dist;
        }

        if(d >= 0)  // in front or on the plane
        {
            i = n->children[0];
        }
        else  // behind the plane
        {
            i = n->children[1];
        }
    }

    return -(i+1);
}


////////////////////////////////////////////////////////////

class collision_data {
public:
    collision_data() : found(false), nearest(-1.0f), dir_len(0.0f) {}

    vec3_t src;
    vec3_t dir;
    vec3_t ndir;
    float  dir_len;

    bool found;
    float nearest;
    vec3_t nearest_poly;
};

bool point_in_poly(const vec3_t& p, const face_t* f)
{
    int i;
    vec3_t v[32];

    for (i = 0; i < f->count; i++)
    {
        int c = f->first + i;
        VectorSubtract(p, vt_array[c], v[i]);
        VectorNormalize(v[i]);
    }

    float total = 0.0f;

    for (i = 0; i < f->count-1; i++)
        total += (float) acos(DotProduct(v[i], v[i+1]));

    total += (float) acos(DotProduct(v[f->count-1], v[0]));

    if (fabsf(total-6.28f)<0.01f)
        return true;

    return false;
}

void closest_on_line(const vec3_t& a, const vec3_t& b, const vec3_t& p, vec3_t out)
{
    vec3_t c;
    vec3_t v;

    VectorSubtract(p, a, c);
    VectorSubtract(b, a, v);
    float d = VectorLength(v);

    if (d)
        VectorScale(v, (1.0f/d), v);

    float t = DotProduct(v, c);

    if (t < 0.0f)
    {
        VectorCopy(a, out);
        return;
    }

    if (t > d)
    {
        VectorCopy(b, out);
        return;
    }

    vec3_t vec_scale;
    VectorScale(v, t, vec_scale);
    VectorAdd(a, vec_scale, out);
}

void closest_on_poly(const vec3_t& p, const face_t* f, vec3_t out)
{
    int i;
    vec3_t v[32];
    float d[32];
    vec3_t temp1;
    vec3_t temp2;

    for (i = 0; i < f->count-1; i++)
    {
        int c = f->first + i;
        VectorCopy(vt_array[c], temp1);
        VectorCopy(vt_array[c+1], temp2);

        closest_on_line(temp1, temp2, p, v[i]);

        vec3_t t;
        VectorSubtract(p, v[i], t);
        d[i] = VectorLength(t);
    }

    i = f->count-1;

    VectorCopy(vt_array[f->first + i], temp1);
    VectorCopy(vt_array[f->first], temp2);

    closest_on_line(temp1, temp2, p, v[i]);

    vec3_t t;
    VectorSubtract(p, v[i], t);
    d[i] = VectorLength(t);

    float min = d[0];
    vec3_t r;
    VectorCopy(v[0], r);

    for (i = 1; i < f->count; i++)
    {
        if(d[i] < min)
        {
            min = d[i];
            VectorCopy(v[i], r);
        }
    }   

    VectorCopy(r, out);
}

float intersect(const vec3_t& r0,
          const vec3_t& rn,
          const vec3_t& p0,
          const vec3_t& pn)
{
    float d = -DotProduct(pn, p0);

    float numer = DotProduct(pn, r0) + d;
    float denom = DotProduct(pn, rn);
  
    if(denom <= -0.001f || denom >= 0.001f)
        return -(numer/denom);

    return -1.0f;
}

float intersect_sphere(const vec3_t& r, 
                 const vec3_t& rv, 
                 const vec3_t& s, 
                 float sr)
{
    vec3_t q;
    VectorSubtract(s, r, q);
    float c = VectorLength(q);
    float v = DotProduct(q, rv);
    float d = sr*sr - (c*c-v*v);

    if(d < 0.0f) return -1.0f;
    return v-sqrt(d);
}


void check_collision(const face_t* f, collision_data& coldat)
{
    vec3_t r, pt, n;
    vec3_t inverse;

    VectorCopy(vt_array[f->first], pt);
    VectorCopy(f->p.normal(), n);

    float radius = 15.0f;
    vec3_t vec_scale;
    VectorScale(n, radius, vec_scale);

    vec3_t s;
    VectorSubtract(coldat.src, vec_scale, s);

    float t = f->p.dist_to_point(s);

    if(t > coldat.dir_len)
        return;

    if(t < -2*radius)
        return;

    if(t < 0.0f)
    {
        vec3_t vec_scale;
        VectorScale(n, radius, vec_scale);

        if(!f->p.intersect(s,vec_scale,pt,r))
            return;
    }
    else
    {
        if(!f->p.intersect(s,coldat.dir,pt,r))
            return;
    }

    if (!point_in_poly(r,f))
        closest_on_poly(r, f, r);

    VectorCopy(coldat.ndir, inverse);
    VectorInverse(inverse);

    t = intersect_sphere(r, inverse, coldat.src, radius);

    if (t >= 0.0f && t <= coldat.dir_len)
    {
        if (!coldat.found || t < coldat.nearest)
        {
            coldat.found = true;
            coldat.nearest = t;
            VectorCopy(r, coldat.nearest_poly);
        }
    }
}

static int cutoff = 0;

void check_collisions(const vec3_t& src, vec3_t dir, vec3_t out)
{
    if (++cutoff>5)
    {
        VectorCopy(src, out);
        return;
    }

    collision_data coldat;
    VectorCopy(src, coldat.src);
    VectorCopy(dir, coldat.dir);

    coldat.dir_len = VectorLength(dir);

    if (coldat.dir_len < 0.001f)
    {
        VectorCopy(src, out);
        return;
    }

    VectorScale(coldat.dir, (1.0f/coldat.dir_len), coldat.ndir);

    if (!config.enable_noclip)
    {
        for (int i = 0; i < max_visible_faces; i++)
        {
            face_t* f = visible_faces[i];
            while(f)
            {
                check_collision(f, coldat);
                f = f->next;
            }
        }
    }

    if(coldat.found)
    {
        vec3_t s;
        VectorCopy(src, s);

        if (coldat.nearest >= 0.001f)
        {
            vec3_t vec_scale;
            VectorScale(coldat.ndir, (coldat.nearest-0.001f), vec_scale);
            VectorAdd(s, vec_scale, s);
        }

        vec3_t dst;
        vec3_t n;

        VectorAdd(src, dir, dst);
        VectorSubtract(s, coldat.nearest_poly, n);
        VectorNormalize(n);

        float t = intersect(dst, n, coldat.nearest_poly, n);

        vec3_t vec_scale;
        vec3_t newdst;
        vec3_t newdir;

        VectorScale(n, t, vec_scale);
        VectorAdd(dst, vec_scale, newdst);
        VectorSubtract(newdst, coldat.nearest_poly, newdir);

        check_collisions(s, newdir, out);

        return;
    }

    vec3_t vec_scale;
    VectorScale(coldat.ndir, (coldat.dir_len-0.001f), vec_scale);
    VectorAdd(src, vec_scale, out);
}


//////////////////////////////////////////////////////////////

static int current_style = 0;

void Render::process_visible_faces(void)
{
    rendered = 0;

    // setup for frustum culling
    frustum_setup(m_camera);

    vec3_t cam;
    VectorCopy(m_camera.eye(), cam);

    // clear stuff
    memset(marked_faces, 0, (numfaces+7)/8);
    memset(visible_faces, 0, max_visible_faces * sizeof(face_t *));

    // find leaf we're in
    int idx = find_leaf(cam);
    vis_leaf_t &vis_leaf = pVisibility->VisLeaf(idx);
 
    // go thru leaf visibility list
    for (int i = 0; i < vis_leaf.num_leafs; i++)
    {
        const dleaf_t &leaf = dleafs[vis_leaf.leafs[i]];

        // discard leafs outside frustum
        if (!frustum_cull(leaf.mins, leaf.maxs))
        {
            unsigned short* p = dmarksurfaces + leaf.firstmarksurface;

            for(int x = 0; x < leaf.nummarksurfaces; x++)
            {
                // don't render those already rendered
                int face_idx = *p++;

                if (!(marked_faces[face_idx>>3] & (1<<(face_idx & 7))))
                {
                       // back face culling
                    face_t* f = faces[face_idx];

                    if (f == NULL)
                        continue;

                    float d = DotProduct(cam, f->plane.normal) - f->plane.dist;

                    if (f->side)
                    {
                        if (d > 0)
                            continue;
                    }
                    else
                    {
                        if (d < 0)
                            continue;
                    }

                    // mark face as visible
                    marked_faces[face_idx>>3] |= (1<<(face_idx & 7));

                    int idx = f->texture;

                    if (textures[idx] && (textures[idx]->flags & TEXTURE_SKY))
                    {
                        f->next = skyfaces;
                        skyfaces = f;
                    }
                    else
                    {
                        f->next = visible_faces[idx];
                        visible_faces[idx] = f;
                    }

                    rendered++;

                    // chose lightmap style
                    f->lightmap = f->lightmaps[0];
                    if(current_style != 0) {
                        for(int c = 0; c < MAXLIGHTMAPS; c++) {
                            if(f->styles[c] == current_style) {
                                f->lightmap = f->lightmaps[c];
                                break;
                            }
                        }
                    }
                }
            }
        }        
    }
}

void Render::render_edges(const face_t *face)
{
   int i, v1, v2;

   glBegin (GL_LINES);

   for (i = 0; i < face->count; i++)
   {
      v1 = face->first + i;
      v2 = face->first + ((i + 1) % face->count);

      glVertex3f (vt_array[v1][0], vt_array[v1][1], vt_array[v1][2]);
      glVertex3f (vt_array[v2][0], vt_array[v2][1], vt_array[v2][2]);
   }

   glEnd ();
}

void Render::render_visible_edges(void)
{
   if (config.show_edges == 0)
      return;

   if (config.show_edges == 2)
   {
      glEnable(GL_LINE_SMOOTH);  // more visible, but a lot slower
      glLineWidth(1.0f);  // glEnable(GL_LINE_SMOOTH) is already enabled
   }
   else
      glLineWidth(2.0f);  // faster, but not as visible

   glColor3f (0, 0, 0);  // draw in black

   for (int i = 0; i < max_visible_faces; i++)
   {
      face_t* f = visible_faces[i];

      while (f)
      {
         render_edges(f);
         f = f->next;
      }
   }

   glColor3f (1.0f, 1.0f, 1.0f);  // back to white

   if (config.show_edges == 2)
      glDisable(GL_LINE_SMOOTH);
}

void Render::render_face(const face_t* face)
{
    if (textures[face->texture] == NULL)
       return;

    textures[face->texture]->bind();

    glTexCoordPointer(2, GL_FLOAT, 0, st_array);
    glDrawArrays(face->type, face->first, face->count);
}

void Render::render_visible_faces()
{
    for (int i = 0; i < max_visible_faces; i++)
    {
        face_t* f = visible_faces[i];
        while (f)
        {
            // is this NOT a special texture?
            if (!(f->flags & TEX_SPECIAL))
                render_face(f);
            f = f->next;
        }
    }
}

void Render::render_lightmap(const face_t* face)
{
    if ((face->lightmap) && (config.enable_lighting))
    {
        glBlendFunc(GL_ZERO, GL_SRC_COLOR);
        glEnable(GL_BLEND);
        glDepthMask(GL_FALSE);
        glDepthFunc(GL_EQUAL);

        face->lightmap->bind();

        glTexCoordPointer(2, GL_FLOAT, 0, lst_array);
        glDrawArrays(face->type, face->first, face->count);
        
        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);
    }
}

void Render::render_visible_lightmaps()
{
    for (int i = 0; i < max_visible_faces; i++)
    {
        face_t* f = visible_faces[i];
        while (f)
        {
            // is this NOT a special texture?
            if (!(f->flags & TEX_SPECIAL))
                render_lightmap(f);
            f = f->next;
        }
    }
}

void Render::render_special_textures()
{
   int ent_index;
   int model_index;

   if (!config.render_special_textures)
      return;

   glEnable(GL_BLEND);  // Turn Blending On
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   glColor4f(1.0f, 1.0f, 1.0f, 0.95f);

   for (int i = 0; i < max_visible_faces; i++)
   {
      face_t* f = visible_faces[i];
      while (f)
      {
         // is this a special texture?
         if (f->flags & TEX_SPECIAL)
            render_face(f);
         f = f->next;
      }
   }

   for (ent_index = 0; ent_index < num_entvars; ent_index++)
   {
      // is this a brush model?
      if (entvars[ent_index].brush_model_index > 0)
      {
         model_index = entvars[ent_index].brush_model_index;

         glPushMatrix();

         dmodel_t* m = &dmodels[model_index];

         glTranslatef(m->origin[0],m->origin[1],m->origin[2]);

         for (int c = 0; c < m->numfaces; c++)
         {
            face_t *face = faces[m->firstface + c];

            if (face == NULL)
                continue;

            // is this a special texture?
            if (face->flags & TEX_SPECIAL)
               render_face(face);
         }

         glPopMatrix();
      }
   }

   glColor4f(1.0f, 1.0f, 1.0f, 1.0f);  // back to white, full alpha

   glDisable(GL_BLEND);  // Turn Blending Off
}


void Render::render_entvars()
{
   int ent_index;
   int model_index;
   int pass;
   vec3_t v1, v2;
   float dot;

   // set up right vector for studio model chrome rendering...

   m_camera.right_vector(g_vright);

   // two passes, render solid objects first, then non-solid/masked objects
   for (pass = 0; pass < 2; pass++)
   {
      for (ent_index = 0; ent_index < num_entvars; ent_index++)
      {
         // is this a brush model?
         if (entvars[ent_index].brush_model_index > 0)
         {
            model_index = entvars[ent_index].brush_model_index;

            if (((pass == 0) && (entvars[ent_index].renderamt == 255)) ||
                ((pass == 1) && (entvars[ent_index].renderamt != 255)))
            {
               glPushMatrix();

               dmodel_t* m = &dmodels[model_index];

               glTranslatef(m->origin[0],m->origin[1],m->origin[2]);

               glEnable(GL_BLEND);  // Turn Blending On
               glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

//               glColor4f(entvars[ent_index].rendercolor[0],
//                         entvars[ent_index].rendercolor[1],
//                         entvars[ent_index].rendercolor[2],
//                         entvars[ent_index].renderamt);
               glColor4f(1.0, 1.0, 1.0, entvars[ent_index].renderamt);

               for (int c = 0; c < m->numfaces; c++)
               {
                  face_t *face = faces[m->firstface + c];

                  if (face == NULL)
                     continue;

                  // is this NOT a special texture?
                  if (!(face->flags & TEX_SPECIAL))
                      render_face(face);
               }

               glDisable(GL_BLEND);  // Turn Blending Off

               glPopMatrix();
            }
         }
         else if ((pass == 0) && (entvars[ent_index].studio_model))
         {
            m_camera.fwd_vec(v1);
            VectorSubtract(entvars[ent_index].origin, m_camera.eye(), v2);
            dot = DotProduct(v1, v2);

            // is this entity outside the view frustum?
            if (dot > 0.7660)  // greater than sin(50)?
               continue;

            glPushMatrix();

            glTranslatef(entvars[ent_index].origin[0],
                         entvars[ent_index].origin[1],
                         entvars[ent_index].origin[2]);

            // rotate about the Z axis (by yaw degrees)...
            glRotatef(entvars[ent_index].angles[1], 0, 0, 1);

            entvars[ent_index].studio_model->SetBlending(0, 0.0f);
            entvars[ent_index].studio_model->SetBlending(1, 0.0f);

            entvars[ent_index].studio_model->DrawModel();

            glPopMatrix();
         }
      }
   }

   glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}


#define SKY_DEPTH  4000.0f

#define SKY_FRONT  (1<<0)
#define SKY_BACK   (1<<1)
#define SKY_LEFT   (1<<2)
#define SKY_RIGHT  (1<<3)
#define SKY_UP     (1<<4)
#define SKY_DOWN   (1<<5)

void Render::render_skyfaces()
{
   int sky_flag;  // which skybox faces should be rendered
   vec3_t cam;

   if (sky_textures[0] == NULL)  // no sky textures loaded?
      return;

   VectorCopy(m_camera.eye(), cam);

   // I should REALLY be using the skyfaces linked list to walk through
   // the faces and only render the ones that are visible, but I seem
   // to be having some problems with that.  For now, just render ALL
   // of the skyfaces (even the ones that aren't possibly visible)...

   sky_flag = SKY_FRONT | SKY_BACK | SKY_LEFT | SKY_RIGHT | SKY_UP | SKY_DOWN;

   if (sky_flag & SKY_FRONT)
   {
     sky_textures[0]->bind();  // FRONT

      glBegin(GL_QUADS);

      glTexCoord2f(0,0);
      glVertex3f(cam[0]+SKY_DEPTH, cam[1]-SKY_DEPTH, cam[2]-SKY_DEPTH);
      glTexCoord2f(0,1);
      glVertex3f(cam[0]+SKY_DEPTH, cam[1]-SKY_DEPTH, cam[2]+SKY_DEPTH);
      glTexCoord2f(1,1);
      glVertex3f(cam[0]-SKY_DEPTH, cam[1]-SKY_DEPTH, cam[2]+SKY_DEPTH);
      glTexCoord2f(1,0);
      glVertex3f(cam[0]-SKY_DEPTH, cam[1]-SKY_DEPTH, cam[2]-SKY_DEPTH);

      glEnd();
   }

   if (sky_flag & SKY_BACK)
   {
      sky_textures[1]->bind();  // BACK

      glBegin(GL_QUADS);

      glTexCoord2f(1,0);
      glVertex3f(cam[0]+SKY_DEPTH, cam[1]+SKY_DEPTH, cam[2]-SKY_DEPTH);
      glTexCoord2f(0,0);
      glVertex3f(cam[0]-SKY_DEPTH, cam[1]+SKY_DEPTH, cam[2]-SKY_DEPTH);
      glTexCoord2f(0,1);
      glVertex3f(cam[0]-SKY_DEPTH, cam[1]+SKY_DEPTH, cam[2]+SKY_DEPTH);
      glTexCoord2f(1,1);
      glVertex3f(cam[0]+SKY_DEPTH, cam[1]+SKY_DEPTH, cam[2]+SKY_DEPTH);

      glEnd();
   }

   if (sky_flag & SKY_LEFT)
   {
      sky_textures[2]->bind();  // LEFT

      glBegin(GL_QUADS);

      glTexCoord2f(1,0);
      glVertex3f(cam[0]-SKY_DEPTH, cam[1]+SKY_DEPTH, cam[2]-SKY_DEPTH);
      glTexCoord2f(0,0);
      glVertex3f(cam[0]-SKY_DEPTH, cam[1]-SKY_DEPTH, cam[2]-SKY_DEPTH);
      glTexCoord2f(0,1);
      glVertex3f(cam[0]-SKY_DEPTH, cam[1]-SKY_DEPTH, cam[2]+SKY_DEPTH);
      glTexCoord2f(1,1);
      glVertex3f(cam[0]-SKY_DEPTH, cam[1]+SKY_DEPTH, cam[2]+SKY_DEPTH);

      glEnd();
   }

   if (sky_flag & SKY_RIGHT)
   {
      sky_textures[3]->bind();  // RIGHT

      glBegin(GL_QUADS);

      glTexCoord2f(1,0);
      glVertex3f(cam[0]+SKY_DEPTH, cam[1]-SKY_DEPTH, cam[2]-SKY_DEPTH);
      glTexCoord2f(0,0);
      glVertex3f(cam[0]+SKY_DEPTH, cam[1]+SKY_DEPTH, cam[2]-SKY_DEPTH);
      glTexCoord2f(0,1);
      glVertex3f(cam[0]+SKY_DEPTH, cam[1]+SKY_DEPTH, cam[2]+SKY_DEPTH);
      glTexCoord2f(1,1);
      glVertex3f(cam[0]+SKY_DEPTH, cam[1]-SKY_DEPTH, cam[2]+SKY_DEPTH);

      glEnd();
   }

   if (sky_flag & SKY_UP)
   {
      sky_textures[4]->bind();  // UP

      glBegin(GL_QUADS);

      glTexCoord2f(0,1);
      glVertex3f(cam[0]-SKY_DEPTH, cam[1]+SKY_DEPTH, cam[2]+SKY_DEPTH);
      glTexCoord2f(1,1);
      glVertex3f(cam[0]-SKY_DEPTH, cam[1]-SKY_DEPTH, cam[2]+SKY_DEPTH);
      glTexCoord2f(1,0);
      glVertex3f(cam[0]+SKY_DEPTH, cam[1]-SKY_DEPTH, cam[2]+SKY_DEPTH);
      glTexCoord2f(0,0);
      glVertex3f(cam[0]+SKY_DEPTH, cam[1]+SKY_DEPTH, cam[2]+SKY_DEPTH);

      glEnd();
   }

   if (sky_flag & SKY_DOWN)
   {
      sky_textures[5]->bind();  // DOWN

      glBegin(GL_QUADS);

      glTexCoord2f(0,1);
      glVertex3f(cam[0]+SKY_DEPTH, cam[1]+SKY_DEPTH, cam[2]-SKY_DEPTH);
      glTexCoord2f(1,1);
      glVertex3f(cam[0]+SKY_DEPTH, cam[1]-SKY_DEPTH, cam[2]-SKY_DEPTH);
      glTexCoord2f(1,0);
      glVertex3f(cam[0]-SKY_DEPTH, cam[1]-SKY_DEPTH, cam[2]-SKY_DEPTH);
      glTexCoord2f(0,0);
      glVertex3f(cam[0]-SKY_DEPTH, cam[1]+SKY_DEPTH, cam[2]-SKY_DEPTH);

      glEnd();
   }
}


Render::Render()
: m_xcenter(0.0f), m_ycenter(0.0f)
{
    pFont = NULL;
    pVisibility = NULL;
}


Render::~Render()
{
}

void Render::resize()
{
    projection_setup(m_width, m_height);
    m_xcenter = (float) m_width/2;
    m_ycenter = (float) m_height/2;
}

void Render::init()
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);    
    glClearDepth(1.0f);

    // misc
    glDisable(GL_POLYGON_SMOOTH);
    glDisable(GL_POLYGON_STIPPLE);
    glDisable(GL_DITHER);
    glDisable(GL_NORMALIZE);
    glDisable(GL_LOGIC_OP);

    // texture settings
    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    // depth buffer
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);

    // nice perspective
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    // culling
    glCullFace(GL_FRONT);
    glEnable(GL_CULL_FACE);


    pFont = new Font("font.tga");

    if (!pFont)
        Error("Error creating font!\n");

#ifdef WIN32
    PROC swp = wglGetProcAddress("wglSwapIntervalEXT");
    if(swp)
    {
        typedef BOOL (WINAPI *swp_t)(int);
        swp_t s = (swp_t) swp;

        if (!config.enable_fullscreen)
            s(0);
    }
#endif
}

void Render::shutdown()
{
    int i;

    if (faces)
    {
        for (i = 0; i < numfaces; i++)
        {
            if (faces[i])
                free(faces[i]);
        }
        free(faces);
        faces = NULL;
    }

    if (marked_faces)
    {
        free(marked_faces);
        marked_faces = NULL;
    }

    if (visible_faces)
    {
        free(visible_faces);
        visible_faces = NULL;
    }

    if (pVisibility)
    {
        delete pVisibility;
        pVisibility = NULL;
    }

    Free_Textures();

    if (pFont)
    {
        delete pFont;
        pFont = NULL;
    }
}

void CalcSurfaceExtents (dface_t *s, float *mins, float *maxs)
{
   float val;
   int   i,j, e;
   dvertex_t *v;
   texinfo_t *tex;
//   int bmins[2], bmaxs[2];

   mins[0] = mins[1] =  999999;
   maxs[0] = maxs[1] = -999999;

   tex = texinfo + s->texinfo;

   for (i=0 ; i<s->numedges ; i++)
   {
      e = dsurfedges[s->firstedge + i];
      if (e >= 0)
         v = &dvertexes[dedges[e].v[0]];
      else
         v = &dvertexes[dedges[-e].v[1]];
        
      for (j=0 ; j<2 ; j++)
      {
         val = v->point[0] * tex->vecs[j][0] + 
            v->point[1] * tex->vecs[j][1] +
            v->point[2] * tex->vecs[j][2] +
            tex->vecs[j][3];
         if (val < mins[j])
            mins[j] = val;
         if (val > maxs[j])
            maxs[j] = val;
      }
   }

//   for (i=0 ; i<2 ; i++)
//   {  
//      bmins[i] = floor(mins[i]/16);
//      bmaxs[i] = ceil(maxs[i]/16);
//      s->texturemins[i] = bmins[i] * 16;
//      s->extents[i] = (bmaxs[i] - bmins[i]) * 16;
//      if ( !(tex->flags & TEX_SPECIAL) && s->extents[i] > 512 /* 256 */ )
//         Sys_Error ("Bad surface extents");
//   }
}

void ComputeLightmap(dface_t *dface, face_t *face, float *mins, float *maxs, int *lh, int *lw)
{
    int c, i;
    int width, height;
    unsigned char *data;

    // compute lightmap size
    int size[2];
    for (c = 0; c < 2; c++)
    {
        float tmin = (float) floor(mins[c]/16.0f);
        float tmax = (float) ceil(maxs[c]/16.0f);

        size[c] = (int) (tmax-tmin);
    }

    width = size[0]+1;
    height = size[1]+1;

    *lw = width;
    *lh = height;

    int lsz = width * height * 3;  // RGB buffer size

    // generate lightmaps texture
//    for(c = 0; c < MAX_LIGHTMAPS; c++)
    for (c = 0; c < 1; c++)
    {
        if (dface->styles[c] == -1)
            break;

        face->styles[c] = dface->styles[c];

        data = (unsigned char *)malloc(lsz);

        memcpy(data, dlightdata + dface->lightofs + (lsz * c), lsz);

        float f, light_adjust;
        int inf;

        light_adjust = 1.0f - config.brightness;

        // scale lightmap value...
        for (i = 0; i < lsz; i++)
        {
            f = (float)pow((data[i]+1)/256.0, light_adjust);
            inf = f * 255.0f + 0.5f;
            if (inf < 0)
               inf = 0;
            if (inf > 255)
               inf = 255;
            data[i] = (int)inf;
        }

        face->lightmaps[c] = new Texture(data, width, height, GL_RGB,
                                         nearest_mipmap_nearest);

        free(data);
    }

    face->lightmap = face->lightmaps[0];
}

void Render::InitRender()
{
    int index;
    int ent_index;

    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

#ifdef WIN32
    flip();
#else
//    glutSwapBuffers();
#endif

    for (index = 0; index < 256; index++)
        keys[MAX_KEYS] = 0;

    left_button = 0;
    right_button = 0;
    xpos = 0;
    ypos = 0;
    m_width = config.width;
    m_height = config.height;

    ent_index = -1;

    if ((ent_index = FindEntityByClassname(ent_index, "worldspawn")) != -1)
    {
        char *key_value;
        char value[MAX_VALUE];
        char *pos, *term;
        char wadfilename[64];

        key_value = ValueForKey(&entities[ent_index], "wad");
        if (key_value[0])
        {
            strcpy(value, key_value);

            pos = value;
            while ((term = strstr(pos, ";")) != NULL)
            {
                *term = 0;  // change ; to null terminator
                ExtractFileBase(pos, wadfilename);
                strcat(wadfilename, ".wad");
                LoadWadFile(wadfilename);
                pos = term + 1;
            }

            ExtractFileBase(pos, wadfilename);
            strcat(wadfilename, ".wad");
            LoadWadFile(wadfilename);
        }
        else
            Error("Can't find WAD key for worldspawn entity!\n");
    }
    else
        Error("Can't find worldspawn entity!\n");


    InitSpawnPoint();

    float x, y, z;

    x = spawn_point[0];
    y = spawn_point[1];
    z = spawn_point[2];

    // setup camera
    m_camera.move(x, y, z);
    m_camera.rotate(-90.0, 0.0, spawn_point_yaw);


    // load the textures...
    Load_Textures();

    max_visible_faces = num_textures;


    // load the models...
    for (index = 0; index < num_entvars; index++)
    {
        // check if this classname needs a studio model...
        for (int model_index = 0; model_index < config.num_models; model_index++)
        {
            if (stricmp(entvars[index].classname,
                        config.model_classname[model_index]) == 0)
            {
                // has the studio model not be allocated for this classname yet?
                if (config.studio_model[model_index] == NULL)
                {
                    // load the studio model for this entity...
                    StudioModel *pStudioModel = new StudioModel;

                    if (pStudioModel->Init(config.model_filename[model_index]))
                    {
                        pStudioModel->SetController(0, 0.0f);
                        pStudioModel->SetController(1, 0.0f);
                        pStudioModel->SetController(2, 0.0f);
                        pStudioModel->SetController(3, 0.0f);

                        pStudioModel->SetMouth(0);
                        pStudioModel->SetBodygroup(0, 0);
                        pStudioModel->SetSkin(0);

                        pStudioModel->SetSequence(0);

                        config.studio_model[model_index] = pStudioModel;
                    }
                    else
                    {
                        delete pStudioModel;  // free the unused studio model
                    }
                }

                // all entities of the same classname share the same studio model...
                entvars[index].studio_model = config.studio_model[model_index];
            }
        }
    }

    visible_faces = (face_t **)malloc(max_visible_faces * sizeof(face_t *));
    if (visible_faces == NULL)
        Error("Error allocating memory for visible faces!\n");


    pVisibility = new Visibility();
    if (pVisibility == NULL)
        Error("Error allocating memory for visibility list!\n");

    pVisibility->Decompress_VIS();


    // setup...
    marked_faces = (unsigned char *)malloc((numfaces+7)/8);  // allocate enough bits
    if (marked_faces == NULL)
        Error("Error allocating memory for marked faces!\n");


    faces = (face_t **)malloc(numfaces * sizeof(face_t *));
    if (faces == NULL)
        Error("Error allocating memory for faces!\n");

    // setup textures, faces and lightmaps...
    for (int i = 0; i < numfaces; i++)
    {
        face_t *face = (face_t *)malloc(sizeof(face_t));
        if (face == NULL)
            Error("Error allocating memory for a face!\n");

        // get plane info
        memcpy(&face->plane, &dplanes[dfaces[i].planenum], sizeof(dplane_t));
        face->side = dfaces[i].side;

        // determine best primitive type
        switch(dfaces[i].numedges) {
        case 3: face->type = GL_TRIANGLES; break;
        case 4: face->type = GL_QUADS; break;
        default: face->type = GL_POLYGON;
        }

        // get texture index
        texinfo_t *ti = texinfo + dfaces[i].texinfo;
        face->texture = ti->miptex; // texture index
        face->flags = ti->flags;

        //
        // the following computations are based on:
        // PolyEngine (c) Alexey Goloshubin and Quake I source by id Software
        //
        
        int c;
        float min[2], max[2];
        int lw,lh;

        // compute s and t extents
        CalcSurfaceExtents(&dfaces[i], min, max);

        face->lightmap = NULL;
        for (int ndx = 0; ndx < MAXLIGHTMAPS; ndx++)
           face->lightmaps[ndx] = NULL;

        if (!(face->flags & TEX_SPECIAL))
           ComputeLightmap(&dfaces[i], face, min, max, &lh, &lw);


        //////////////////////////////////////////////////////

        if (textures[ti->miptex] == NULL)
        {
           faces[i] = NULL;
           continue;
        }

        float is = 1.0f/(float)textures[ti->miptex]->width;
        float it = 1.0f/(float)textures[ti->miptex]->height;

        face->first = vt_array.current();
        face->count = dfaces[i].numedges;

        for (c = 0; c < dfaces[i].numedges; c++)
        {
            float v[7];
            int eidx = *(dsurfedges + dfaces[i].firstedge + c);
            if(eidx < 0)
            {
                eidx = -eidx;
                dedge_t* e = &dedges[eidx];
                v[0] = dvertexes[e->v[1]].point[0];
                v[1] = dvertexes[e->v[1]].point[1];
                v[2] = dvertexes[e->v[1]].point[2];
            }
            else
            {
                dedge_t* e = &dedges[eidx];
                v[0] = dvertexes[e->v[0]].point[0];
                v[1] = dvertexes[e->v[0]].point[1];
                v[2] = dvertexes[e->v[0]].point[2];
            }

            float s = DotProduct(v, ti->vecs[0]) + ti->vecs[0][3];
            float t = DotProduct(v, ti->vecs[1]) + ti->vecs[1][3];

            v[3] = s*is;
            v[4] = t*it;

            if (!(face->flags & TEX_SPECIAL))
            {
                // compute lightmap coords
                float mid_poly_s = (min[0] + max[0])/2.0f;
                float mid_poly_t = (min[1] + max[1])/2.0f;
                float mid_tex_s = (float)lw / 2.0f;
                float mid_tex_t = (float)lh / 2.0f;
                float ls = mid_tex_s + (s - mid_poly_s) / 16.0f;
                float lt = mid_tex_t + (t - mid_poly_t) / 16.0f;
                ls /= (float) lw;
                lt /= (float) lh;

                v[5] = ls;
                v[6] = lt;
            }

            ////////////////////////
            vt_array.add(v);
            st_array.add(v+3);          
            lst_array.add(v+5);
        }

        ///////////////////////////////

        vec3_t n;
        VectorCopy(face->plane.normal, n);
        float d = face->plane.dist;

        if(face->side)
        {
            VectorInverse(n);
            face->p = plane_t(n, d);
        }
        else
            face->p = plane_t(n, -d);

        // add face to array of faces
        faces[i] = face;
    }

    // setup vertex arrays
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, vt_array);


#ifdef WIN32
    ShowCursor(FALSE);
#endif
}

void Render::begin_orto()
{
    // save projection matrix
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    glOrtho(0, m_width, 0, m_height, -1, 1);
 
    // save modelview
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_CULL_FACE);
}

void Render::end_orto()
{
    glEnable(GL_CULL_FACE);

    // restore projection and modelview
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();   
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void Render::reset_mouse()
{
#ifdef WIN32
    POINT p;
    p.x = m_xcenter, p.y = m_ycenter;

    if (!m_fullscreen)
        ClientToScreen(m_hwnd,&p);

    SetCursorPos(p.x,p.y);
#endif
}

static const float piover180 = 0.0174532925f;
static const float piover2 = 1.570796327f;
static float keybounce_time = 0.0f;
static float keybounce_delay = 0.25;

void Render::handle_input()
{
    float x=0,y=0,z=0;
    float yaw=0,pitch=0,roll=0;
    float speed, fps;
    vec3_t vec_start;
    vec3_t vec_end;
    trace_t trace;

    fps = frames_per_second;

    if ((fps > 999.9) || (fps < 0.001))
       fps = 30;  // prevent overflow/underflow for bad fps values

    if (fps > 30.0)
       fps = 30.0;  // clamp to 30 frames per second

    // movement speed is based on frame rate
    speed = 120.0f / fps;

    if (speed < 4.0f)
        speed = 4.0f;

    speed = speed * config.movement_speed;

    yaw = (xpos-m_xcenter) * config.mouse_sensitivity;

    if (config.enable_inverted_mouse)
        pitch = (m_ycenter-ypos) * config.mouse_sensitivity;
    else
        pitch = (ypos-m_ycenter) * config.mouse_sensitivity;

    reset_mouse();

    float pitch_radians = (m_camera.pitch() + pitch) + 90.0;
    float yaw_radians = m_camera.yaw() + yaw;

    // convert degrees to radians
    pitch_radians *= piover180;
    yaw_radians *= piover180;

    if(keys[VK_UP])
    {
        x = fsin(yaw_radians) * speed * (1.0 - fsin(fabs(pitch_radians)));
        y = fcos(yaw_radians) * speed * (1.0 - fsin(fabs(pitch_radians)));
        z = -fsin(pitch_radians) * speed;
    }
    if(keys[VK_DOWN])
    {
        x = -fsin(yaw_radians) * speed * (1.0 - fsin(fabs(pitch_radians)));
        y = -fcos(yaw_radians) * speed * (1.0 - fsin(fabs(pitch_radians)));
        z = fsin(pitch_radians) * speed;
    }
    if(keys[VK_LEFT])
    {
        x = fsin(yaw_radians-piover2) * speed;
        y = fcos(yaw_radians-piover2) * speed;
    }
    if(keys[VK_RIGHT])
    {
        x = -fsin(yaw_radians-piover2) * speed;
        y = -fcos(yaw_radians-piover2) * speed;
    }

    if (keys['S']) { z = -speed; }  // move straight up
    if (keys['W']) { z = +speed; }  // move straight down

    if (keys['C'] && (keybounce_time < gametime))  // clipping on/off
    {
        keybounce_time = gametime + keybounce_delay;
        config.enable_noclip = !config.enable_noclip;
    }

    if (keys['O'] && (keybounce_time < gametime))  // switchable texture on/off
    {
        keybounce_time = gametime + keybounce_delay;
        switchable_texture_off = !switchable_texture_off;
    }

    if (keys['L'] && (keybounce_time < gametime))  // lighting on/off
    {
        keybounce_time = gametime + keybounce_delay;
        config.enable_lighting = !config.enable_lighting;
    }

    if (keys['E'] && (keybounce_time < gametime))  // show edges on/off
    {
        keybounce_time = gametime + keybounce_delay;
        config.show_edges++;
        if (config.show_edges > 2)
           config.show_edges = 0;
    }

    if ((keys['N']) && (draw_traceline_pt))  // turn off traceline point (if on)
       draw_traceline_pt = !draw_traceline_pt;

    if (keys['T'])  // print the texture name
    {
        VectorCopy(m_camera.eye(), vec_start);

        m_camera.fwd_vec(vec_end);
        VectorNormalize(vec_end);

        VectorScale(vec_end, -4096.0, vec_end);
        VectorAdd(vec_start, vec_end, vec_end);

        TraceLine (vec_start, vec_end, &trace);

        if (trace.fraction < 1.0)
        {
            dface_t *face = TraceLineFindFace(vec_start, &trace);

            if (face != NULL)
            {
                texinfo_t *ti = texinfo + face->texinfo;

                dmiptexlump_t *pMipTexLump = (dmiptexlump_t *)dtexdata;

                if (pMipTexLump->dataofs[ti->miptex] != -1)  // not unused mip lump
                {
                    int x, y, len;

                    miptex_t *pMipTex = (miptex_t*)(dtexdata + pMipTexLump->dataofs[ti->miptex]);

                    len = strlen(pMipTex->name);

                    x = config.width / 2;
                    y = config.height / 2;

                    // rescale X and Y to keep text center regardless of screen size...
                    if (config.width < 800)
                        x = x * (800.0f / (float)config.width);
                    if (config.height < 600)
                        y = y * (600.0f / (float)config.height);

                    // center text on screen (down slightly from center)
                    x = x - len * 8;  // each char is 16 units wide
                    y = y - 32;

                    print_text(x, y, pMipTex->name);
                }
            }
        }
    }

    if (keys['B'])  // draw a point at then end of a traceline
    {
        draw_traceline_pt = TRUE;

        VectorCopy(m_camera.eye(), vec_start);

        VectorCopy(vec_start, traceline_pt[0]);  // save the starting point

        m_camera.fwd_vec(vec_end);
        VectorNormalize(vec_end);

        VectorScale(vec_end, -4096.0, vec_end);
        VectorAdd(vec_start, vec_end, vec_end);

        TraceLine (vec_start, vec_end, &trace);

        VectorCopy(trace.endpos, traceline_pt[1]);  // save the ending point
    }

    dir[0] = x;  dir[1] = y;  dir[2] = z;

    rotation[0] = pitch;  rotation[1] = roll;  rotation[2] = yaw;
}

void Render::print_text(int x, int y, char *text)
{
    begin_orto();
    glDisable(GL_DEPTH_FUNC);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    pFont->print(x, y, text);

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_FUNC);
    end_orto();
}

void Render::overlay()
{
    char c_clip[4];

    begin_orto();
    glDisable(GL_DEPTH_FUNC);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    if (config.enable_noclip)
       strcpy(c_clip, "OFF");
    else
       strcpy(c_clip, "ON ");

    char tmp[64];
    sprintf(tmp, "FPS: %4.1f  Polygons Rendered: %4d",
            frames_per_second, rendered);
    pFont->print(0, 0, tmp);

    sprintf(tmp, "camera: %7.2f %7.2f %7.2f  clip: %s",
            m_camera.eye()[0],m_camera.eye()[1],m_camera.eye()[2], c_clip);
    pFont->print(0, 16, tmp);

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_FUNC);
    end_orto();
}

void Render::traceline_draw(void)
{
    if (!draw_traceline_pt)
        return;

    glPointSize (10.0f);  // big dot
    glColor3f (0, 0, 1.0f);  // draw points in blue

    glBegin (GL_POINTS);
    glVertex3f (traceline_pt[0][0], traceline_pt[0][1], traceline_pt[0][2]);
    glVertex3f (traceline_pt[1][0], traceline_pt[1][1], traceline_pt[1][2]);
    glEnd ();

    glEnable(GL_LINE_SMOOTH);  // more visible, but a lot slower
    glLineWidth(1.0f);

    glColor3f (1.0f, 0, 0);  // draw line in red

    glBegin (GL_LINES);
    glVertex3f (traceline_pt[0][0], traceline_pt[0][1], traceline_pt[0][2]);
    glVertex3f (traceline_pt[1][0], traceline_pt[1][1], traceline_pt[1][2]);
    glEnd ();

    glDisable(GL_LINE_SMOOTH);

    glColor3f (1.0f, 1.0f, 1.0f);  // back to white
}

void Render::draw_crosshairs(void)
{
    begin_orto();
    glDisable(GL_DEPTH_FUNC);

    glEnable(GL_LINE_SMOOTH);  // more visible, but a lot slower
    glLineWidth(1.0f);

    glColor3f (1.0f, 0.7f, 0.01f);  // Half-Life orangish

    glBegin (GL_LINES);
    glVertex2i(config.width / 2, config.height / 2 - 10);
    glVertex2i(config.width / 2, config.height / 2 + 10);
    glEnd ();

    glBegin (GL_LINES);
    glVertex2i(config.width / 2 - 10, config.height / 2);
    glVertex2i(config.width / 2 + 10, config.height / 2);
    glEnd ();

    glDisable(GL_LINE_SMOOTH);

    glEnable(GL_DEPTH_FUNC);
    end_orto();

    glColor3f (1.0f, 1.0f, 1.0f);  // back to white
}

void Render::frame()
{
    static bool was_inactive = FALSE;

    SysTime_Update();

#ifndef __linux__
    if (!m_active)  // are we no longer the active window?
    {
        was_inactive = TRUE;
        Sleep(100);  // sleep 100 milliseconds to reduce CPU cycles
        return;
    }
#endif

    if (was_inactive)  // are we just waking up?
    {
        was_inactive = FALSE;
        // do we need to reset the mouse or do anything after waking up here?
    }

    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    handle_input();

    Animate_Textures();

    skyfaces = NULL;

    process_visible_faces();

    cutoff=0;

    vec3_t eye;
    vec3_t dst;
    VectorCopy(m_camera.eye(), eye);

    check_collisions(eye, dir, dst);

    m_camera.move(dst);
    m_camera.rotate_delta(rotation);
    m_camera.transform();

    m_camera.load();

    render_visible_faces();

    render_visible_lightmaps();

    render_visible_edges();

    render_skyfaces();  // render skybox

    render_entvars();  // render entities (brush models and models)

    render_special_textures();  // aaatrigger, face and origin textures

    traceline_draw();

    draw_crosshairs();

    overlay();

#ifdef WIN32
    flip();
#else
//    glutSwapBuffers();
#endif

}

