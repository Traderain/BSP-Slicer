//
// BSP_tool - botman's Half-Life BSP utilities
//
// (http://planethalflife.com/botman/)
//
// wadlib.h
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

#ifndef WADLIB_H
#define WADLIB_H

/*
   WAD3 Header
   {
      char identification[4];   // should be WAD3 or 3DAW
      int  numlumps;
      int  infotableofs;        // offset to lump table
   }

   Mip section
   {
      First mip
      {
         char name[16];
         unsigned width, height;
         unsigned offsets[4];  // mip offsets relative to start of this mip
         byte first_mip[width*height];
         byte second_mip[width*height/4];
         byte third_mip[width*height/16];
         byte fourth_mip[width*height/64];
         short int palette_size;
         byte palette[palette_size*3];
         short int padding;
      }
      Next mip {}
      Next mip {}
      .
      .
      .
      Last mip {}
   }

   Lump table
   {
      First lump entry     // 32 bytes in size
      {
         int  filepos;     // file offset of mip
         int  disksize;    // size of mip in bytes
         int  size;        // uncompressed
         char type;        // 0x43 = WAD3 mip (Half-Life)
         char compression; // not used?
         char pad1, pad2;  // not used?
         char name[16];    // null terminated mip name
      }
      Next lump {}
      Next lump {}
      .
      .
      .
      Last lump {}
   }
*/

//
// wad reading
//

#define CMP_NONE  0
#define CMP_LZSS  1

#define TYP_NONE  0
#define TYP_LABEL 1
#define TYP_LUMPY 64    // 64 + grab command number

typedef struct
{
   char  identification[4];   // should be WAD2 or 2DAW
   int   numlumps;
   int   infotableofs;
} wadinfo_t;

typedef struct
{
   int   filepos;          // file offset of mip
   int   disksize;         // mip size
   int   size;             // uncompressed
   char  type;             // 0x43 = WAD3 mip (Half-Life)
   char  compression;      // not used?
   char  pad1, pad2;       // not used?
   char  name[16];         // must be null terminated
} lumpinfo_t;

typedef struct wadconfig_s
{
   FILE        *wadhandle;
   wadinfo_t    wadinfo;
   lumpinfo_t  *lumpinfo;
   int          num_lumps;
   struct wadconfig_s *next_config;
} wadconfig_t;


FILE *W_OpenWad (char *filename);
int W_ReadWadHeader(FILE *wadhandle, wadinfo_t *wadinfo);
void W_ReadWadLumps(FILE *wadhandle, long offset, int num_lumps, lumpinfo_t *lumpinfo);
void W_ReadWadMip(FILE *pakhandle, lumpinfo_t *lumpinfo, void *buffer);
bool LoadWadFile(char *wadfile);
FILE *SearchWadMipname(char *mipname, lumpinfo_t **lumpinfo);

#endif

