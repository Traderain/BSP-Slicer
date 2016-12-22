//
// BSP_tool - botman's Half-Life BSP utilities
//
// (http://planethalflife.com/botman/)
//
// font.cpp
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

#ifndef __linux__
#include <windows.h>
#endif

#include <stdio.h>

#include "cmdlib.h"
#include "config.h"
#include "graphics.h"
#include "font.h"
#include "texture.h"

extern Config config;

extern int m_width;
extern int m_height;


Font::Font(const char *filename)
{
   int width, height;
   unsigned char *tga_data;
   char fontfile[MAX_PATH];

   strcpy(fontfile, config.szPath);
   strcat(fontfile, filename);

   // load the font file...
   tga_data = TGA_Load(fontfile, &width, &height);

   if (!tga_data)
      Error("Error loading font file: font.tga\n");

   m_texture = new Texture(tga_data, width, height, GL_ALPHA, linear_mipmap_linear);

   free(tga_data);

   float inc = 1.0f/16.0f;
   int c = 0;

   for (float y = 1-inc; y >= 0; y -= inc)
   {
      for (float x = 0; x < 1.0f; x += inc)
      {
         coords[c].c00[0] = x; coords[c].c00[1] = y;
         coords[c].c10[0] = x+inc; coords[c].c10[1] = y;
         coords[c].c11[0] = x+inc, coords[c].c11[1] = y+inc;
         coords[c].c01[0] = x; coords[c].c01[1] = y+inc;
         c++;
      }
   }
}


Font::~Font(void)
{
   if (m_texture)
      delete m_texture;
}


void Font::print(int x, int y, const char *s)
{
   char c;
   coords_t *pCoord;
   float horz_scale = 1.0;
   float vert_scale = 1.0;

   if (m_width < 800)
      horz_scale = m_width / 800.0f;
   if (m_height < 600)
      vert_scale = m_height / 600.0f;

   m_texture->bind();

   glEnable(GL_TEXTURE_2D);
   glPushMatrix();
   glScalef(horz_scale, vert_scale, 1.0);
   glBegin(GL_QUADS);

   while (c = *s++)
   {
      pCoord = &coords[c];

      glTexCoord2fv(pCoord->c00); glVertex2s(x,y);
      glTexCoord2fv(pCoord->c10); glVertex2s(x+16,y);
      glTexCoord2fv(pCoord->c11); glVertex2s(x+16,y+16);
      glTexCoord2fv(pCoord->c01); glVertex2s(x,y+16);

      x += 16;
   }

   glEnd();
   glPopMatrix();
}

