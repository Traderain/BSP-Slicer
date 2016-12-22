//
// BSP_tool - botman's Half-Life BSP utilities
//
// (http://planethalflife.com/botman/)
//
// texture.h
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

#ifndef GLunit
#include <GL/gl.h>
#endif

#ifndef TEXTURE_H
#define TEXTURE_H

enum Quality {
   unspecified,
   nearest,
   linear,
   nearest_mipmap_nearest,
   linear_mipmap_nearest,
   nearest_mipmap_linear,
   linear_mipmap_linear
};

// define texture flags for rendering (tranparency, animated, etc.)
#define TEXTURE_ANIMATED     (1<<0)
#define TEXTURE_RANDOM       (1<<1)
#define TEXTURE_LIQUID       (1<<2)
#define TEXTURE_SKY          (1<<3)

#define MAX_FRAMES 10

class Texture
{
   public:

   GLuint m_id;
   int width;
   int height;
   int flags;
   int min, mag;  // GL_TEXTURE_MIN_FILTER and GL_TEXTURE_MAG_FILTER

   // animated texture stuff...
   int current_frame;
   int next_frame;
   int off_frame;  // index of switchable "off" texture

   Texture(const unsigned char *data, int width, int height, int format,
           Quality quality = unspecified);
   ~Texture(void);

   void bind(void);
};


bool Load_Textures(void);
void Free_Textures(void);
void Animate_Textures(void);

#endif

