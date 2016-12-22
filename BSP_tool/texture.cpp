//
// BSP_tool - botman's Half-Life BSP utilities
//
// (http://planethalflife.com/botman/)
//
// texture.cpp
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

#include <GL/gl.h>
#include <GL/glu.h>

#include "cmdlib.h"
#include "bspfile.h"
#include "config.h"
#include "entity.h"
#include "wadlib.h"
#include "texture.h"
#include "graphics.h"

#define MAX_TEXTURES 1024

int num_textures;
Texture *textures[MAX_TEXTURES];

Texture *sky_textures[6];
char skys[6][3]={"ft","bk","lf","rt","up","dn"};

#define MAX_TEX_WIDTH 512
#define MAX_TEX_HEIGHT 512
#define MAX_TEX_SIZE (MAX_TEX_WIDTH * MAX_TEX_HEIGHT)
#define MAX_MIP_SIZE (MAX_TEX_SIZE+(MAX_TEX_SIZE/4)+(MAX_TEX_SIZE/16)+(MAX_TEX_SIZE/64))
#define MAX_LUMP_SIZE (MAX_MIP_SIZE + sizeof(miptex_t) + 2 + (256*3) + 2)

// static buffer for mip lumps (max mip size=256x256)
unsigned char mipdata[MAX_LUMP_SIZE];

// static buffer for RGB or RGBA data to build OpenGL texture from
unsigned char rgba[MAX_TEX_SIZE * 4];

bool switchable_texture_off = FALSE;

extern Config config;
extern float gametime;


Texture::Texture(const unsigned char *rgba_data, int t_width, int t_height, int format, Quality quality)
{
   int internal_format;

   if (quality == unspecified)
      quality = linear_mipmap_linear;

   if (format == GL_RGBA)
      internal_format = 4;  // 4 color components per element
   else
      internal_format = format;

   width = t_width;
   height = t_height;

   flags = 0;  // no texture flags enabled by default

   current_frame = -1;
   next_frame = -1;
   off_frame = -1;

   glGenTextures(1, &m_id);
   glBindTexture(GL_TEXTURE_2D, m_id);

   if (quality == nearest)
   {
      min = GL_NEAREST;
      mag = GL_NEAREST;
   }
   else if (quality == linear)
   {
      min = GL_LINEAR;
      mag = GL_LINEAR;
   }
   else if (quality == nearest_mipmap_nearest)
   {
      min = GL_NEAREST_MIPMAP_NEAREST;
      mag = GL_LINEAR;
   }
   else if (quality == linear_mipmap_nearest)
   {
      min = GL_LINEAR_MIPMAP_NEAREST;
      mag = GL_LINEAR;
   }
   else if (quality == nearest_mipmap_linear)
   {
      min = GL_NEAREST_MIPMAP_LINEAR;
      mag = GL_LINEAR;
   }
   else if (quality == linear_mipmap_linear)
   {
      min = GL_LINEAR_MIPMAP_LINEAR;
      mag = GL_LINEAR;
   }

   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

   if ((quality == nearest_mipmap_nearest) ||
       (quality == linear_mipmap_nearest) ||
       (quality == nearest_mipmap_linear) ||
       (quality == linear_mipmap_linear))
   {
      gluBuild2DMipmaps(GL_TEXTURE_2D, internal_format, 
                        width, height, format, GL_UNSIGNED_BYTE, rgba_data);
   }
   else
   {
      glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0,
                   format, GL_UNSIGNED_BYTE, rgba_data);
   }
}


Texture::~Texture()
{
   if (m_id)
      glDeleteTextures(1, &m_id);
}


void Texture::bind(void)
{
   if (this->flags & TEXTURE_ANIMATED)
   {
      if ((switchable_texture_off) && (this->off_frame != -1))
         glBindTexture(GL_TEXTURE_2D, textures[this->off_frame]->m_id);
      else
         glBindTexture(GL_TEXTURE_2D, textures[this->current_frame]->m_id);
   }
   else
      glBindTexture(GL_TEXTURE_2D, m_id);

   if (this->flags & TEXTURE_SKY)
   {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   }
   else
   {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag);
   }
}


bool Load_Textures(void)
{
   int width, height;
   dmiptexlump_t *pMipTexLump;
   miptex_t *pMipTex, *pMipTex2;
   miptex_t *pMipData;
   lumpinfo_t *lumpinfo = NULL;
   unsigned char *palette, *indices;
   int index, index2;
   int size, datasize, i, pos;
   FILE *wadfile;
   int animate_index[MAX_FRAMES+1];
   int frame_number;
   int total_frames;
   int texture_index;

   num_textures = 0;
   texture_index = 0;

   for (index = 0; index < MAX_TEXTURES; index++)
      textures[index] = NULL;

   for (index = 0; index < 6; index++)
      sky_textures[index] = NULL;

   pMipTexLump = (dmiptexlump_t *)dtexdata;

   if (pMipTexLump->nummiptex > MAX_TEXTURES)
      return FALSE;  // indicate failure

   for (index = 0; index < pMipTexLump->nummiptex; index++)
   {
      if (pMipTexLump->dataofs[index] == -1)  // unused mip lump
      {
         // let's just allocate a solid color texture that's 16 by 16 pixels...
         width = 16;
         height = 16;

         size = width * height;

         pos = 0;
         for (i = 0; i < size; i++)
         {
            rgba[pos++] = 255;  // solid red
            rgba[pos++] = 0;
            rgba[pos++] = 0;
         }

         textures[texture_index] = new Texture(rgba, width, height, GL_RGB, linear_mipmap_linear);

         num_textures++;
         texture_index++;
         continue;
      }

      pMipTex = (miptex_t*)(dtexdata + pMipTexLump->dataofs[index]);

      width = pMipTex->width;
      height = pMipTex->height;

      if ((width < 1) || (width > MAX_TEX_WIDTH) ||
          (height < 1) || (height > MAX_TEX_HEIGHT))
      {
          continue;  // ignore invalid textures
      }

      size = width * height;

      if (pMipTex->offsets[0] != 0)
      {
         datasize = size + (size/4) + (size/16) + (size/64);

         indices = (unsigned char *)pMipTex + pMipTex->offsets[0];

         palette = indices + datasize + 2;
      }
      else
      {
         wadfile = SearchWadMipname(pMipTex->name, &lumpinfo);

         if (wadfile != NULL)
         {
            W_ReadWadMip(wadfile, lumpinfo, mipdata);

            pMipData = (miptex_t *)mipdata;

            indices = (unsigned char *)pMipData + pMipData->offsets[0];

            palette = (unsigned char *)pMipData + pMipData->offsets[3] +
                      ((width/8) * (height/8)) + 2;
         }
         else
            Error("Texture not found: %s\n", pMipTex->name);
      }

      // convert mip name to lowercase for easy comparisons...
      for (i = 0; i < 16; i++)
         pMipTex->name[i] = tolower(pMipTex->name[i]);

      // does this mip contain alpha transparency?
      if (pMipTex->name[0] == '{')
      {
         unsigned char r, g, b, a;

         // convert the mip palette based bitmap to RGB format...
         pos = 0;
         for (i = 0; i < size; i++)
         {
            r = palette[indices[i]*3];
            g = palette[indices[i]*3 + 1];
            b = palette[indices[i]*3 + 2];
            a = 255;

            if ((r == 0) && (g == 0) && (b == 255))
            {
               r = 0; b = 0; g = 0; a = 0;
            }

            rgba[pos++] = r;
            rgba[pos++] = g;
            rgba[pos++] = b;
            rgba[pos++] = a;
         }

         textures[texture_index] = new Texture(rgba, width, height, GL_RGBA, linear_mipmap_linear);
      }
      else if (strcmp(pMipTex->name, "sky") == 0)  // sky texture?
      {
         char skyname[64];
         char skyfilename[256];
         char skypathname[256];
         unsigned char *tga_data;
         int sky_index;

         // convert the mip palette based bitmap to RGB format...
         pos = 0;
         for (i = 0; i < size; i++)
         {
            rgba[pos++] = palette[indices[i]*3];
            rgba[pos++] = palette[indices[i]*3 + 1];
            rgba[pos++] = palette[indices[i]*3 + 2];
         }

         textures[texture_index] = new Texture(rgba, width, height, GL_RGB, nearest);

         textures[texture_index]->flags = TEXTURE_SKY;

         bool skyname_found = FALSE;

         int ent_index = -1;

         if ((ent_index = FindEntityByClassname(ent_index, "worldspawn")) != -1)
         {
            char *value;

            // look for a worldspawn "skyname" key...
            value = ValueForKey(&entities[ent_index], "skyname");
            if (value[0])
            {
               strncpy(skyname, value, 64);
               skyname_found = TRUE;
            }

            // didn't find it?  then look for a "message" key and use that as skyname
            if (!skyname_found)
            {
               value = ValueForKey(&entities[ent_index], "message");
               if (value[0])
               {
                  strncpy(skyname, value, 64);
                  skyname_found = TRUE;
               }
            }
         }

         if (skyname_found)
         {
            for (sky_index = 0; sky_index < 6; sky_index++)
            {
               tga_data = NULL;

               strcpy(skyfilename, "gfx/env/");
               strcat(skyfilename, skyname);
               strcat(skyfilename, skys[sky_index]);
               strcat(skyfilename, ".tga");

               // see if sky texture is in a MOD directory first...
               for (int mod_index = 0; mod_index < config.num_mods; mod_index++)
               {
                  strcpy(skypathname, config.mods[mod_index].dir);
                  strcat(skypathname, skyfilename);

                  if (FileTime(skypathname) != -1)
                  {
                     tga_data = TGA_Load(skypathname, &width, &height);
                     break;  // break out of for loop
                  }
               }

               if (tga_data == NULL)
               {
                  // load the sky texture from PAK file...
                  tga_data = TGA_LoadPak(skyfilename, &width, &height);
               }

               if (!tga_data)
                  Error("Error loading sky texture: %s\n", skyfilename);

               sky_textures[sky_index] = new Texture(tga_data, width, height, GL_RGB, nearest);

               sky_textures[sky_index]->flags = TEXTURE_SKY;

               free(tga_data);
            }
         }
         else
         {
            for (sky_index = 0; sky_index < 6; sky_index++)
               sky_textures[sky_index] = textures[texture_index];
         }
      }
      else if ((strcmp(pMipTex->name, "clip") == 0) ||  // special textures?
               (strcmp(pMipTex->name, "origin") == 0) ||
               (strcmp(pMipTex->name, "aaatrigger") == 0))
      {
         unsigned char r, g, b;

         // convert the mip palette based bitmap to RGB format...
         pos = 0;
         for (i = 0; i < size; i++)
         {
            r = palette[indices[i]*3];
            g = palette[indices[i]*3 + 1];
            b = palette[indices[i]*3 + 2];

            rgba[pos++] = r;
            rgba[pos++] = g;
            rgba[pos++] = b;

            rgba[pos++] = config.special_texture_transparency;
         }

         textures[texture_index] = new Texture(rgba, width, height, GL_RGBA, linear_mipmap_linear);
      }
      else
      {
         float gamma = config.gamma_adjust;
         float f, inf;

         for (i = 0; i < 768; i++)
         {
            f = (float)pow((palette[i]+1)/256.0, gamma);
            inf = f * 255.0f + 0.5f;
            if (inf < 0)
               inf = 0;
            if (inf > 255)
               inf = 255;
            palette[i] = (int)inf;
         }

         // convert the mip palette based bitmap to RGB format...
         pos = 0;
         for (i = 0; i < size; i++)
         {
            rgba[pos++] = palette[indices[i]*3];
            rgba[pos++] = palette[indices[i]*3 + 1];
            rgba[pos++] = palette[indices[i]*3 + 2];
         }

         textures[texture_index] = new Texture(rgba, width, height, GL_RGB, linear_mipmap_linear);
      }

      num_textures++;
      texture_index++;
   }

   // sequence the animations...
   for (index = 0; index < pMipTexLump->nummiptex; index++)
   {
      pMipTex = (miptex_t*)(dtexdata + pMipTexLump->dataofs[index]);

      // is this the first texture in an animated or switchable sequence?
      if ((pMipTex->name[0] == '+') && (pMipTex->name[1] == '0'))
      {
         textures[index]->flags |= TEXTURE_ANIMATED;

         textures[index]->current_frame = index;
         textures[index]->next_frame = index;
         textures[index]->off_frame = -1;  // no "off" texture by default

         for (index2 = 0; index2 < MAX_FRAMES+1; index2++)
            animate_index[index2] = index;

         total_frames = 0;

         // search for the rest of the frames matching this texture...
         for (index2 = 0; index2 < pMipTexLump->nummiptex; index2++)
         {
            pMipTex2 = (miptex_t*)(dtexdata + pMipTexLump->dataofs[index2]);

            if (stricmp(&pMipTex2->name[2], &pMipTex->name[2]) == 0)
            {
               frame_number = pMipTex2->name[1];
               if ((frame_number >= '0') && (frame_number <= '9'))
               {
                  frame_number -= '0';  // convert from ASCII to integer

                  // save the texture index of this frame number...
                  animate_index[frame_number] = index2;

                  total_frames++;
               }
               else if ((frame_number == 'A') || (frame_number == 'a'))
               {
                  // save the index of the "off" texture...
                  textures[index]->off_frame = index2;
               }
               else
                  Error("Error! I don't know how to animate this texture: %s\n", pMipTex2->name);
            }
         }

         for (index2 = 0; index2 < total_frames; index2++)
            textures[animate_index[index2]]->next_frame = animate_index[index2+1];
      }
   }

   return TRUE;  // indicate success
}

void Free_Textures(void)
{
   int index;

   for (index=0; index < num_textures; index++)
   {
      if (textures[index])
         delete textures[index];
   }

   num_textures = 0;
}


void Animate_Textures(void)
{
   dmiptexlump_t *pMipTexLump;
   static float update_time = 0.0f;
   int index;
   int current_frame;
   int next_frame;

   pMipTexLump = (dmiptexlump_t *)dtexdata;

   if (update_time <= gametime)
   {
      update_time = gametime + 0.1f;  // update in 1/10th of a second

      for (index=0; index < pMipTexLump->nummiptex; index++)
      {
         if (textures[index] == NULL)
            continue;

         if (textures[index]->flags & TEXTURE_ANIMATED)
         {
            current_frame = textures[index]->current_frame;
            next_frame = textures[current_frame]->next_frame;
            textures[index]->current_frame = next_frame;
         }
      }
   }
}

