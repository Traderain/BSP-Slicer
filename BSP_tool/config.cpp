//
// BSP_tool - botman's Half-Life BSP utilities
//
// (http://planethalflife.com/botman/)
//
// config.cpp
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

#ifdef WIN32
#include "windows.h"
#include "direct.h"
#else
#include "stdio.h"
#endif

#include "scriplib.h"
#include "config.h"
#include "paklib.h"
#include "wadlib.h"

extern Config config;


Config::Config(char *config_file)
{
   int index;
   char config_filename[MAX_PATH];

   // set up configuration defaults...

   width = 640;
   height = 480;
   bits_per_pixel = 16;
   refresh_rate = 60;
   enable_fullscreen = FALSE;

   x_pos = -1;  // default is to center the display
   y_pos = -1;

   enable_lighting = TRUE;
   brightness = 0.75;
   gamma_adjust = 1.0;

   movement_speed = 1;
   mouse_sensitivity = 0.15f;

   enable_inverted_mouse = FALSE;
   enable_noclip = FALSE;

   render_special_textures = FALSE;
   special_texture_transparency = 80;  // about 30% translucent

   show_edges = 0;

   halflife_dir[0] = 0;
   bspname[0] = 0;
   spawnpoint[0] = 0;

   num_models = 0;

   for (index = 0; index < MAX_MODELS; index++)
   {
      model_classname[index][0] = 0;
      model_filename[index][0] = 0;
      studio_model[index] = NULL;
   }

   for (index = 0; index < MAX_MODS; index++)
   {
      mods[index].dir[0] = 0;
      mods[index].pPak = NULL;
      mods[index].pWad = NULL;
   }

   num_mods = 0;
   bsp_mod_index = -1;
   valve_mod_index = -1;

   _getcwd(szPath, MAX_PATH);
#ifdef WIN32
   strcat(szPath, "\\");
#else
   strcat(szPath, "/");
#endif

   strcpy(config_filename, szPath);
   strcat(config_filename, config_file);

   //LoadScriptFile(config_filename);

   //ParseScriptFile();

   //if (valve_mod_index == -1)
      //Error("Can't find Half-Life\\valve directory!\n");
}


Config::~Config(void)
{
   int index;
   pakconfig_t *pPakConfig;
   wadconfig_t *pWadConfig;

   for (index = 0; index < num_models; index++)
   {
      if (studio_model[index])
         delete studio_model[index];
   }

   for (index = 0; index < MAX_MODS; index++)
   {
      while (mods[index].pPak)  // free the PAK config linked list
      {
         pPakConfig = mods[index].pPak;
         free(mods[index].pPak->pakinfo);       // free the pakinfo data
         fclose(mods[index].pPak->pakhandle);   // close the file
         mods[index].pPak = mods[index].pPak->next_config;
         free(pPakConfig);          // free the pak config structure
      }

      while (mods[index].pWad)  // free the WAD config linked list
      {
         pWadConfig = mods[index].pWad;
         free(mods[index].pWad->lumpinfo);      // free the lumpinfo data
         fclose(mods[index].pWad->wadhandle);   // close the file
         mods[index].pWad = mods[index].pWad->next_config;
         free(pWadConfig);          // free the wad config structure
      }
   }
}


HANDLE FindDirectory(HANDLE hFile, CHAR *dirname, CHAR *filename)
{
   WIN32_FIND_DATA pFindFileData;

   dirname[0] = 0;

   if (hFile == NULL)
   {
      hFile = FindFirstFile(filename, &pFindFileData);

      if (hFile == INVALID_HANDLE_VALUE)
         hFile = NULL;

      while (!(pFindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
      {
         if (FindNextFile(hFile, &pFindFileData) == 0)
         {
            FindClose(hFile);
            hFile = NULL;
            return hFile;
         }
      }

      strcpy(dirname, pFindFileData.cFileName);

      return hFile;
   }
   else
   {
      if (FindNextFile(hFile, &pFindFileData) == 0)
      {
         FindClose(hFile);
         hFile = NULL;
         return hFile;
      }

      while (!(pFindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
      {
         if (FindNextFile(hFile, &pFindFileData) == 0)
         {
            FindClose(hFile);
            hFile = NULL;
            return hFile;
         }
      }

      strcpy(dirname, pFindFileData.cFileName);

      return hFile;
   }
}


HANDLE FindFile(HANDLE hFile, CHAR *file, CHAR *filename)
{
   WIN32_FIND_DATA pFindFileData;

   if (hFile == NULL)
   {
      hFile = FindFirstFile(filename, &pFindFileData);

      if (hFile == INVALID_HANDLE_VALUE)
         hFile = NULL;
   }
   else
   {
      if (FindNextFile(hFile, &pFindFileData) == 0)
      {
         FindClose(hFile);
         hFile = NULL;
      }
   }

   if (hFile != NULL)
      strcpy(file, pFindFileData.cFileName);
   
   return hFile;
}


void Config::AddPakFile(int mod_index, char *pakfilename)
{
   FILE        *pakhandle;
   pakheader_t  pakheader;
   pakconfig_t *pPakConfig;
   int num_entries;
   int size;

   pakhandle = P_OpenPak(pakfilename);

   if (!P_ReadPakHeader(pakhandle, &pakheader))
   {
      fclose(pakhandle);
      return;
   }

   num_entries = pakheader.dir_length / sizeof(pakinfo_t);

   size = num_entries * sizeof(pakinfo_t);

   if (pakheader.dir_length != size)
      return;  // dir_length NOT even multiple of pakinfo size

   pPakConfig = (pakconfig_t *)malloc(sizeof(pakconfig_t));

   pPakConfig->next_config = mods[mod_index].pPak;
   mods[mod_index].pPak = pPakConfig;

   pPakConfig->pakhandle = pakhandle;
   pPakConfig->pakheader = pakheader;

   pPakConfig->pakinfo = (pakinfo_t *)malloc(pakheader.dir_length);

   pPakConfig->num_entries = num_entries;

   P_ReadPakInfo(pakhandle, pakheader.dir_offset, num_entries, pPakConfig->pakinfo);
}


void Config::AddWadFile(int mod_index, char *wadfilename)
{
   FILE        *wadhandle;
   wadinfo_t    wadinfo;
   wadconfig_t *pWadConfig;
   int size;

   wadhandle = W_OpenWad(wadfilename);

   if (!W_ReadWadHeader(wadhandle, &wadinfo))
   {
      fclose(wadhandle);
      return;
   }

   pWadConfig = (wadconfig_t *)malloc(sizeof(wadconfig_t));

   pWadConfig->next_config = mods[mod_index].pWad;
   mods[mod_index].pWad = pWadConfig;

   pWadConfig->wadhandle = wadhandle;
   pWadConfig->wadinfo = wadinfo;

   size = wadinfo.numlumps * sizeof(lumpinfo_t);
   pWadConfig->lumpinfo = (lumpinfo_t *)malloc(size);

   pWadConfig->num_lumps = wadinfo.numlumps;

   W_ReadWadLumps(wadhandle, wadinfo.infotableofs, wadinfo.numlumps, pWadConfig->lumpinfo);
}


void Config::AddHalfLifeDir(char *halflife_dir)
{
#ifdef WIN32
   HANDLE hDir;
   HANDLE hFile;
#endif
   char dirspec[256];
   char dirname[256];
   char filespec[256];
   char filename[256];
   char pakfilename[256];

   strcpy(dirspec, halflife_dir);
#ifndef __linux__
   strcat(dirspec, "\\*");
#else
   strcat(dirspec, "/*");  //??? Linux dir wildcard
#endif

   hDir = NULL;

   while ((hDir = FindDirectory(hDir, dirname, dirspec)) != NULL)
   {
      if ((strcmp(dirname, ".") == 0) || (strcmp(dirname, "..") == 0))
         continue;

      strcpy(filename, halflife_dir);
#ifndef __linux__
      strcat(filename, "\\");
      strcat(filename, dirname);
      strcat(filename, "\\liblist.gam");
#else
      strcat(filename, "/");
      strcat(filename, dirname);
      strcat(filename, "/liblist.gam");
#endif

      // is this a MOD directory? (i.e. does "liblist.gam" exist?)...
      if (FileTime(filename) != -1)
      {
         if (stricmp(dirname, "valve") == 0)
            valve_mod_index = num_mods;

         strcpy(mods[num_mods].dir, halflife_dir);
#ifndef __linux__
         strcat(mods[num_mods].dir, "\\");
         strcat(mods[num_mods].dir, dirname);
         strcat(mods[num_mods].dir, "\\");
#else
         strcat(mods[num_mods].dir, "/");
         strcat(mods[num_mods].dir, dirname);
         strcat(mods[num_mods].dir, "/");
#endif

         // search for PAK files...
         hFile = NULL;
         strcpy(filespec, mods[num_mods].dir);
         strcat(filespec, "*.pak");

         while ((hFile = FindFile(hFile, filename, filespec)) != NULL)
         {
            strcpy(pakfilename, mods[num_mods].dir);
            strcat(pakfilename, filename);
            AddPakFile(num_mods, pakfilename);
         }

         num_mods++;

         if (num_mods == MAX_MODS)
            Error("You have WAY too many MODs!");
      }
   }
}


bool Config::ParseScriptFile(void)
{
   while (1)
   {
      do
      {
         GetToken (true);  // look for a line starting with a '$'
         if (endofscript)
            return 0;  // indicate success
         if (token[0] == '$')
            break;
         while (TokenAvailable())
            GetToken (false);
      } while (1);

      if (strcmp(token, "$width") == 0)
      {
         GetToken(false);
         sscanf(token, "%d", &width);
      }
      else if (strcmp(token, "$height") == 0)
      {
         GetToken(false);
         sscanf(token, "%d", &height);
      }
      else if (strcmp(token, "$bpp") == 0)
      {
         GetToken(false);
         sscanf(token, "%d", &bits_per_pixel);
      }
      else if (strcmp(token, "$hertz") == 0)
      {
         GetToken(false);
         sscanf(token, "%d", &refresh_rate);
      }
      else if (strcmp(token, "$enable_fullscreen") == 0)
      {
         GetToken(false);
         sscanf(token, "%d", &enable_fullscreen);
      }
      else if (strcmp(token, "$x_pos") == 0)
      {
         GetToken(false);
         sscanf(token, "%d", &x_pos);
      }
      else if (strcmp(token, "$y_pos") == 0)
      {
         GetToken(false);
         sscanf(token, "%d", &y_pos);
      }
      else if (strcmp(token, "$enable_lighting") == 0)
      {
         GetToken(false);
         sscanf(token, "%d", &enable_lighting);
      }
      else if (strcmp(token, "$brightness") == 0)
      {
         GetToken(false);
         sscanf(token, "%f", &brightness);
         if ((brightness < 0.0) || (brightness > 1.0))
            brightness = 0.75;
      }
      else if (strcmp(token, "$gamma_adjust") == 0)
      {
         GetToken(false);
         sscanf(token, "%f", &gamma_adjust);
         if ((gamma_adjust < 0.5) || (gamma_adjust > 2.0))
            gamma_adjust = 1.0;
      }
      else if (strcmp(token, "$movement_speed") == 0)
      {
         GetToken(false);
         sscanf(token, "%d", &movement_speed);
         if (movement_speed < 1)
            movement_speed = 1;
         if (movement_speed > 5)
            movement_speed = 5;
      }
      else if (strcmp(token, "$mouse_sensitivity") == 0)
      {
         GetToken(false);
         sscanf(token, "%f", &mouse_sensitivity);
         if (mouse_sensitivity < 0.0)
            mouse_sensitivity = 0.15f;

         if (mouse_sensitivity > 1.0)
            mouse_sensitivity = 1.0;
      }
      else if (strcmp(token, "$enable_inverted_mouse") == 0)
      {
         GetToken(false);
         sscanf(token, "%d", &enable_inverted_mouse);
      }
      else if (strcmp(token, "$enable_noclip") == 0)
      {
         GetToken(false);
         sscanf(token, "%d", &enable_noclip);
      }
      else if (strcmp(token, "$render_special_textures") == 0)
      {
         GetToken(false);
         sscanf(token, "%d", &render_special_textures);
      }
      else if (strcmp(token, "$special_texture_transparency") == 0)
      {
         GetToken(false);
         sscanf(token, "%d", &special_texture_transparency);
         if ((special_texture_transparency < 0) || (special_texture_transparency > 255))
            special_texture_transparency = 80;  // about 30% translucent
      }
      else if (strcmp(token, "$show_edges") == 0)
      {
         GetToken(false);
         sscanf(token, "%d", &show_edges);
         if ((show_edges < 0) || (show_edges > 2))
            show_edges = 0;
      }
      else if (strcmp(token, "$bspfile") == 0)
      {
         if (bspname[0])
            Error("Duplicate $bspfile entry found in .cfg file!\n");

         GetToken(false);
         strcpy(bspname, token);
      }
      else if (strcmp(token, "$spawnpoint") == 0)
      {
         if (spawnpoint[0])
            Error("multiple $spawnpoint found in .cfg file!\n");

         GetToken(false);
         sscanf(token, "%s", spawnpoint);
      }
      else if (strcmp(token, "$halflife_dir") == 0)
      {
         GetToken(false);

         if (FileTime(token) == -1)
            Error("Half-Life directory %s doen't exist\nPlease edit the .cfg file to match your installation\n", token);

         strcpy(halflife_dir, token);
         AddHalfLifeDir(token);
      }
      else if (strcmp(token, "$models") == 0)
      {
         GetToken(true);

         if (strcmp(token, "{") != 0)
            Error("{ not found after $models tag in .cfg file!\n");

         while (1)
         {
            GetToken(true);  // get the entity name

            if (endofscript)
               Error("matching } not found after $models { in .cfg file!\n");

            if (strcmp(token, "}") == 0)
               break;

            strcpy(model_classname[num_models], token);

            GetToken(false);  // get the model filename

            if (strcmp(token, "}") == 0)
               Error("closing } found where model filename expected in .cfg file!\n");

            strcpy(model_filename[num_models], token);

            if (num_models < MAX_MODELS-1)
               num_models++;
         }
      }
      else
         Error("Unknown Config file option: %s\n", token);
   }
}


// open a file by searching MOD directories first, then PAK files...
FILE *OpenMODFile(char *filename)
{
   FILE *fp;
   char filepath[256];
   int mod_index;

   // is the MOD directory known from the BSP file?
   if (config.bsp_mod_index >= 0)
   {
      strcpy(filepath, config.mods[config.bsp_mod_index].dir);
      strcat(filepath, filename);

      if (FileTime(filepath) != -1)  // does this wad file exist?
      {
         if ((fp = fopen(filepath, "rb")) == NULL)
            Error("Error opening file: %s\n", filepath);

         return fp;
      }
   }

   // use the valve directory by default...

   strcpy(filepath, config.mods[config.valve_mod_index].dir);
   strcat(filepath, filename);

   if (FileTime(filepath) != -1)  // does this wad file exist?
   {
      if ((fp = fopen(filepath, "rb")) == NULL)
         Error("Error opening file: %s\n", filepath);

      return fp;
   }

   // if the MOD directory is known and not found so far, return error
   if (config.bsp_mod_index >= 0)
      return NULL;

   // search ALL of the MOD directories for this wad file...
   for (mod_index = 0; mod_index < config.num_mods; mod_index++)
   {
      strcpy(filepath, config.mods[mod_index].dir);
      strcat(filepath, filename);

      if (FileTime(filepath) != -1)  // does this wad file exist?
      {
         if ((fp = fopen(filepath, "rb")) == NULL)
            Error("Error opening file: %s\n", filepath);

         return fp;
      }
   }

   return NULL;  // file not found!
}
