//
// BSP_tool - botman's Half-Life BSP utilities
//
// (http://planethalflife.com/botman/)
//
// font.h
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

#ifndef TEXTURE_H
#include "texture.h"
#endif

#ifndef FONT_H
#define FONT_H

typedef struct
{
   float c00[2];
   float c10[2];
   float c11[2];
   float c01[2];
} coords_t;

class Font
{
   public:

   coords_t coords[256];
   Texture  *m_texture;

   Font(const char *filename);
   ~Font(void);

   void print(int x, int y, const char *s);
};

#endif

