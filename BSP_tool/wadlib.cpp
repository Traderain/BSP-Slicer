//
// BSP_tool - botman's Half-Life BSP utilities
//
// (http://planethalflife.com/botman/)
//
// wadlib.cpp
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
#include "scriplib.h"
#include "config.h"
#include "wadlib.h"

extern Config config;


/*
====================
W_OpenWad
====================
*/
FILE *W_OpenWad (char *filename)
{
   FILE *wadhandle;

   wadhandle = SafeOpenRead(filename);

   return wadhandle;
}


/*
====================
W_ReadWadHeader
====================
*/
int W_ReadWadHeader(FILE *wadhandle, wadinfo_t *wadinfo)
{
   SafeRead(wadhandle, wadinfo, sizeof(wadinfo_t));

   if (strncmp(wadinfo->identification, "WAD3", 4))
      return 0;

   wadinfo->numlumps = LittleLong(wadinfo->numlumps);
   wadinfo->infotableofs = LittleLong(wadinfo->infotableofs);

   return 1;
}


/*
====================
W_ReadWadLumps
====================
*/
void W_ReadWadLumps(FILE *wadhandle, long offset, int num_lumps, lumpinfo_t *lumpinfo)
{
   int index;

   fseek(wadhandle, offset, SEEK_SET);

   for (index=0; index < num_lumps; index++)
   {
      SafeRead(wadhandle, &lumpinfo[index], sizeof(lumpinfo_t));

      lumpinfo[index].filepos = LittleLong(lumpinfo[index].filepos);
      lumpinfo[index].disksize = LittleLong(lumpinfo[index].disksize);
      lumpinfo[index].size = LittleLong(lumpinfo[index].size);
   }
}


/*
====================
W_ReadWadMip
====================
*/
void W_ReadWadMip(FILE *wadhandle, lumpinfo_t *lumpinfo, void *buffer)
{
   fseek(wadhandle, lumpinfo->filepos, SEEK_SET);

   SafeRead(wadhandle, buffer, lumpinfo->disksize);
}


/*
====================
LoadWadFile - Add WAD file to config info and load the lump table
====================
*/
bool LoadWadFile(char *wadfile)
{
   int mod_index;
   char wadpath[256];

   // is the MOD directory known from the BSP file?
   if (config.bsp_mod_index >= 0)
   {
      strcpy(wadpath, config.mods[config.bsp_mod_index].dir);
      strcat(wadpath, wadfile);

      if (FileTime(wadpath) != -1)  // does this wad file exist?
      {
         config.AddWadFile(config.bsp_mod_index, wadpath);

         return TRUE;
      }
   }

   // use the valve directory by default...

   strcpy(wadpath, config.mods[config.valve_mod_index].dir);
   strcat(wadpath, wadfile);

   if (FileTime(wadpath) != -1)  // does this wad file exist?
   {
      config.AddWadFile(config.valve_mod_index, wadpath);

      return TRUE;
   }

   // if a MOD directory is known and WAD not found so far, return error
   if (config.bsp_mod_index >= 0)
      return FALSE;

   // search ALL of the MOD directories for this wad file...
   for (mod_index = 0; mod_index < config.num_mods; mod_index++)
   {
      strcpy(wadpath, config.mods[mod_index].dir);
      strcat(wadpath, wadfile);

      if (FileTime(wadpath) != -1)  // does this wad file exist?
      {
         config.AddWadFile(mod_index, wadpath);

         return TRUE;
      }
   }

   // not found, return bad status
   return FALSE;
}


/*
====================
SearchWadMipname - Search WAD files info for mipname
====================
*/
FILE *SearchWadMipname(char *mipname, lumpinfo_t **lumpinfo)
{
   wadconfig_t *pWadConfig;
   int index, mod_index;

   // is the MOD directory known from the BSP file?
   if (config.bsp_mod_index >= 0)
   {
      pWadConfig = config.mods[config.bsp_mod_index].pWad;

      while (pWadConfig)
      {
         for (index=0; index < pWadConfig->num_lumps; index++)
         {
            if (stricmp(pWadConfig->lumpinfo[index].name, mipname) == 0)
            {
               *lumpinfo = &pWadConfig->lumpinfo[index];

               return pWadConfig->wadhandle;
            }
         }

         pWadConfig = pWadConfig->next_config;
      }
   }

   // use the valve directory by default...

   pWadConfig = config.mods[config.valve_mod_index].pWad;

   while (pWadConfig)
   {
      for (index=0; index < pWadConfig->num_lumps; index++)
      {
         if (stricmp(pWadConfig->lumpinfo[index].name, mipname) == 0)
         {
            *lumpinfo = &pWadConfig->lumpinfo[index];

            return pWadConfig->wadhandle;
         }
      }

      pWadConfig = pWadConfig->next_config;
   }

   // if a MOD directory is known and MIP not found so far, return error
   if (config.bsp_mod_index >= 0)
      return FALSE;

   // search ALL of the MOD directories for this MIP...

   for (mod_index = 0; mod_index < config.num_mods; mod_index++)
   {
      pWadConfig = config.mods[mod_index].pWad;

      while (pWadConfig)
      {
         for (index=0; index < pWadConfig->num_lumps; index++)
         {
            if (stricmp(pWadConfig->lumpinfo[index].name, mipname) == 0)
            {
               *lumpinfo = &pWadConfig->lumpinfo[index];

               return pWadConfig->wadhandle;
            }
         }

         pWadConfig = pWadConfig->next_config;
      }
   }

   return NULL;  // not found!
}

