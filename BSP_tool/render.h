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
 * core render class
 */

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

#ifndef __render_h__
#define __render_h__

#ifndef __linux__
#include "glwin.h"
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif

#ifndef BSPFILE_H
#include "bspfile.h"
#endif

#ifndef FONT_H
#include "font.h"
#endif

#ifndef __MATHLIB__
#include "mathlib.h"
#endif

#ifndef VIS_H
#include "vis.h"
#endif

#include "plane.h"
#include "matrix.h"

class DispList {
public:
    DispList() { m_id = glGenLists(1); }
    ~DispList() { glDeleteLists(m_id,1); }

    void begin() const { glNewList(m_id, GL_COMPILE); }
    void end() const { glEndList(); }
    void render() const { glCallList(m_id); }
    GLuint id() const { return m_id; }

private:
    GLuint    m_id;
};

///////////////////////////////////////////////////


class face_t {
public:
    face_t() : texture(-1), lightmap(0), next(0) {
        for(int i = 0; i < MAXLIGHTMAPS;i++) {
            styles[i] = -1;
            lightmaps[i] = 0;
        }
    }
    ~face_t() { 
        for(int i = 0; i < MAXLIGHTMAPS; i++) {
            if(lightmaps[i]) {
                delete lightmaps[i];
                lightmaps[i] = 0;
                styles[i] = -1;
            }
        }
    }

    int type;
    int flags;
    int texture; // index into texture
    Texture* lightmap; // current lightmap
    Texture* lightmaps[MAXLIGHTMAPS];
    int         styles[MAXLIGHTMAPS];

    dplane_t plane;
    plane_t p;
    int side;

    int first;
    int count;

    face_t* next;
};


//////////////////////////////////////////////////////
//
#ifdef WIN32
class Render : public GLWindow {
#else
class Render {
#endif

public:
    Render();
    ~Render();

    void InitRender();
    void frame();

    virtual void resize();
    virtual void init();
    virtual void shutdown();

    void begin_orto();
    void end_orto();

    void handle_input();
    void reset_mouse();
    void print_text(int x, int y, char *text);
    void overlay();

    void display_screen();

    void process_visible_faces();

    void render_face(const face_t* f);
    void render_visible_faces();

    void render_lightmap(const face_t* face);
    void render_visible_lightmaps();

    void render_special_textures();
    void render_skyfaces();

    void render_edges(const face_t *face);
    void render_visible_edges();

    void render_entvars();

    void traceline_draw(void);
    void draw_crosshairs(void);

private:
    Font    *pFont;
    Visibility *pVisibility;

    // screen mid coords
    float    m_xcenter;
    float    m_ycenter;

    // movement and rotation vectors
    vec3_t dir;
    vec3_t rotation;
};

#endif // __render_h__

