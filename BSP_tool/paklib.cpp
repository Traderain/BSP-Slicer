//
// BSP_tool - botman's Half-Life BSP utilities
//
// (http://planethalflife.com/botman/)
//
// paklib.cpp
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
#include "scriplib.h"
#include "paklib.h"
#include "config.h"

extern Config config;


/*
====================
P_OpenPak
====================
*/
FILE *P_OpenPak (char *filename)
{
   FILE *pakhandle;

   pakhandle = SafeOpenRead(filename);

   if (pakhandle == NULL)
   {
      Error ("Can't open PAK file %s\n", filename);
      return NULL;
   }

   return pakhandle;
}


/*
====================
P_ReadPakHeader
====================
*/
int P_ReadPakHeader(FILE *pakhandle, pakheader_t *pakheader)
{
   SafeRead(pakhandle, pakheader, sizeof(pakheader_t));

   if (strncmp(pakheader->identification, "PACK", 4))
      return 0;

   pakheader->dir_offset = LittleLong(pakheader->dir_offset);
   pakheader->dir_length = LittleLong(pakheader->dir_length);

   return 1;
}


/*
====================
P_ReadPakInfo
====================
*/
void P_ReadPakInfo(FILE *pakhandle, long offset, int num_entries, pakinfo_t *pakinfo)
{
   int index;

   fseek(pakhandle, offset, SEEK_SET);

   for (index=0; index < num_entries; index++)
   {
      SafeRead(pakhandle, &pakinfo[index], sizeof(pakinfo_t));

      pakinfo[index].file_pos = LittleLong(pakinfo[index].file_pos);
      pakinfo[index].file_length = LittleLong(pakinfo[index].file_length);
   }
}


/*
====================
P_ReadPakItem
====================
*/
void P_ReadPakItem(FILE *pakhandle, pakinfo_t *pakinfo, void *buffer)
{
   fseek(pakhandle, pakinfo->file_pos, SEEK_SET);

   SafeRead(pakhandle, buffer, pakinfo->file_length);
}




/*
====================
SearchPakFilename - Search PAK config info for filename
====================
*/
bool SearchPakFilename(char *filename, pakconfig_t **pakconfig, pakinfo_t **pakinfo)
{
   int mod_index;

   // is the MOD directory known from the BSP file?
   if (config.bsp_mod_index >= 0)
   {
      pakconfig_t *pPakConfig = config.mods[config.bsp_mod_index].pPak;
      int index;

      while (pPakConfig)
      {
         for (index=0; index < pPakConfig->num_entries; index++)
         {
            if (stricmp(pPakConfig->pakinfo[index].filename, filename) == 0)
            {
               *pakconfig = pPakConfig;
               *pakinfo = &pPakConfig->pakinfo[index];

               return TRUE;
            }
         }

         pPakConfig = pPakConfig->next_config;
      }

   }

   // use the valve directory by default...
   pakconfig_t *pPakConfig = config.mods[config.valve_mod_index].pPak;
   int index;

   while (pPakConfig)
   {
      for (index=0; index < pPakConfig->num_entries; index++)
      {
         if (stricmp(pPakConfig->pakinfo[index].filename, filename) == 0)
         {
            *pakconfig = pPakConfig;
            *pakinfo = &pPakConfig->pakinfo[index];

            return TRUE;
         }
      }

      pPakConfig = pPakConfig->next_config;
   }

   // if a MOD directory is known and PAK file not found so far, return error
   if (config.bsp_mod_index >= 0)
      return FALSE;

   for (mod_index=0; mod_index < config.num_mods; mod_index++)
   {
      pakconfig_t *pPakConfig = config.mods[mod_index].pPak;
      int index;

      while (pPakConfig)
      {
         for (index=0; index < pPakConfig->num_entries; index++)
         {
            if (stricmp(pPakConfig->pakinfo[index].filename, filename) == 0)
            {
               *pakconfig = pPakConfig;
               *pakinfo = &pPakConfig->pakinfo[index];

               return TRUE;
            }
         }

         pPakConfig = pPakConfig->next_config;
      }
   }

   return FALSE;
}


extern dheader_t *header;

/*
====================
Load BSP file from PAK config info
====================
*/

bool LoadPakBSPFile(char *filename)
{
   char bspfilename[256];
   pakconfig_t *pakconfig = NULL;
   pakinfo_t *pakinfo = NULL;
   int i;

   strcpy(bspfilename, filename);

   if (!SearchPakFilename(bspfilename, &pakconfig, &pakinfo))
   {
      strcpy(bspfilename, "maps/");
      strcat(bspfilename, filename);

      SearchPakFilename(bspfilename, &pakconfig , &pakinfo);
   }

   if (pakinfo)
   {
      for (i = 0; i < config.num_mods; i++)
      {
         if (pakconfig == config.mods[i].pPak)
         {
            config.bsp_mod_index = i;
            break;
         }
      }

      header = (dheader_t *)malloc(pakinfo->file_length);

      P_ReadPakItem(pakconfig->pakhandle, pakinfo, header);

      // swap the header
      for (i=0 ; i< sizeof(dheader_t)/4 ; i++)
         ((int *)header)[i] = LittleLong ( ((int *)header)[i]);

      if (header->version != BSPVERSION)
         Error ("%s is version %i, not %i", filename, header->version, BSPVERSION);

      nummodels = CopyLump (LUMP_MODELS, (void **)&dmodels, sizeof(dmodel_t));
      numvertexes = CopyLump (LUMP_VERTEXES, (void **)&dvertexes, sizeof(dvertex_t));
      numplanes = CopyLump (LUMP_PLANES, (void **)&dplanes, sizeof(dplane_t));
      numleafs = CopyLump (LUMP_LEAFS, (void **)&dleafs, sizeof(dleaf_t));
      numnodes = CopyLump (LUMP_NODES, (void **)&dnodes, sizeof(dnode_t));
      numtexinfo = CopyLump (LUMP_TEXINFO, (void **)&texinfo, sizeof(texinfo_t));
      numclipnodes = CopyLump (LUMP_CLIPNODES, (void **)&dclipnodes, sizeof(dclipnode_t));
      numfaces = CopyLump (LUMP_FACES, (void **)&dfaces, sizeof(dface_t));
      nummarksurfaces = CopyLump (LUMP_MARKSURFACES, (void **)&dmarksurfaces, sizeof(dmarksurfaces[0]));
      numsurfedges = CopyLump (LUMP_SURFEDGES, (void **)&dsurfedges, sizeof(dsurfedges[0]));
      numedges = CopyLump (LUMP_EDGES, (void **)&dedges, sizeof(dedge_t));

      texdatasize = CopyLump (LUMP_TEXTURES, (void **)&dtexdata, 1);
      visdatasize = CopyLump (LUMP_VISIBILITY, (void **)&dvisdata, 1);
      lightdatasize = CopyLump (LUMP_LIGHTING, (void **)&dlightdata, 1);
      entdatasize = CopyLump (LUMP_ENTITIES, (void **)&dentdata, 1);

      free (header); // everything has been copied out

      //
      // swap everything
      //
      SwapBSPFile (false);

      return TRUE;
   }

   return FALSE;
}

