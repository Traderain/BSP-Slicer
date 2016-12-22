//
// BSP_tool - botman's Half-Life BSP utilities
//
// (http://planethalflife.com/botman/)
//
// world.cpp
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
#include "paklib.h"
#include "config.h"
#include "entity.h"
#include "world.h"

extern Config config;


World::World(void)
{
}


World::~World(void)
{
   FreeWorld();
}


void World::FreeWorld(void)
{
   FreeEntities();

   if (dmodels)
   {
      free(dmodels);
      dmodels = NULL;
      nummodels = 0;
   }

   if (dvisdata)
   {
      free(dvisdata);
      dvisdata = NULL;
      visdatasize = 0;
   }

   if (dlightdata)
   {
      free(dlightdata);
      dlightdata = NULL;
      lightdatasize = 0;
   }

   if (dtexdata)
   {
      free(dtexdata);
      dtexdata = NULL;
      texdatasize = 0;
   }

   if (dentdata)
   {
      free(dentdata);
      dentdata = NULL;
      entdatasize = 0;
   }

   if (dleafs)
   {
      free(dleafs);
      dleafs = NULL;
      numleafs = 0;
   }

   if (dplanes)
   {
      free(dplanes);
      dplanes = NULL;
      numplanes = 0;
   }

   if (dvertexes)
   {
      free(dvertexes);
      dvertexes = NULL;
      numvertexes = 0;
   }

   if (dnodes)
   {
      free(dnodes);
      dnodes = NULL;
      numnodes = 0;
   }

   if (texinfo)
   {
      free(texinfo);
      texinfo = NULL;
      numtexinfo = 0;
   }

   if (dfaces)
   {
      free(dfaces);
      dfaces = NULL;
      numfaces = 0;
   }

   if (dclipnodes)
   {
      free(dclipnodes);
      dclipnodes = NULL;
      numclipnodes = 0;
   }

   if (dedges)
   {
      free(dedges);
      dedges = NULL;
      numedges = 0;
   }

   if (dmarksurfaces)
   {
      free(dmarksurfaces);
      dmarksurfaces = NULL;
      nummarksurfaces = 0;
   }

   if (dsurfedges)
   {
      free(dsurfedges);
      dsurfedges = NULL;
      numsurfedges = 0;
   }
}


#ifndef __linux__
BOOL CenterWindow(HWND hWnd)
{
    RECT    rRect, rParentRect;
    HWND    hParentWnd;
    int     wParent, hParent, xNew, yNew;
    int     w, h;

    GetWindowRect (hWnd, &rRect);
    w = rRect.right - rRect.left;
    h = rRect.bottom - rRect.top;

    hParentWnd = GetDesktopWindow();

    GetWindowRect( hParentWnd, &rParentRect );

    wParent = rParentRect.right - rParentRect.left;
    hParent = rParentRect.bottom - rParentRect.top;

    xNew = wParent/2 - w/2 + rParentRect.left;
    yNew = hParent/2 - h/2 + rParentRect.top;

    return SetWindowPos (hWnd, NULL, xNew, yNew, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}


UINT CALLBACK CenterOpenDlgBox(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   LPOFNOTIFY pNotify;

   switch (uMsg)
   {
      case WM_NOTIFY:
         pNotify = (LPOFNOTIFY)lParam;

         if (pNotify->hdr.code == CDN_INITDONE)
         {
            CenterWindow( GetParent(hWnd) );
            return( TRUE );
         }
   }

   return( FALSE );
}
#endif


void World::LoadBSP(char *bspfile)
{
   char bsp_filename[256];
   char pathname[256];
   bool bsp_found;
   int index, mod_index;
   char modname[256];
   int len;

   bsp_found = FALSE;

   // did we specify a filename on the command line?
   if ((bspfile != NULL) && (*bspfile != 0))
   {
      strcpy(bspname, bspfile);

      if (FileTime(bspname) != -1)  // does the specified file exist?
      {
         LoadBSPFile(bspname);

         // is this BSP file in a MOD directory?
         ExtractFilePath (bspname, pathname);

         len = strlen(pathname);  // remove trailing '/'

         if (len > 0)
            pathname[len-1] = 0;

         if (pathname[0] != 0)
         {
            ExtractFileBase (pathname, modname);

            if (stricmp(modname, "maps") == 0)
            {
               ExtractFilePath (pathname, modname);

               if (modname[0] != 0)
               {
                  for (int i = 0; i < config.num_mods; i++)
                  {
                     if (stricmp(modname, config.mods[i].dir) == 0)
                        config.bsp_mod_index = i;
                  }
               }
            }
         }

         bsp_found = TRUE;
      }
   }
   else if (config.bspname[0] == 0)  // no filename specified in config file?
   {
#ifndef __linux__
      OPENFILENAME ofn;
      char szFile[256], szFileTitle[256];
      char szFilter[] = {"BSP Files (*.bsp)|*.bsp|"};

      memset( &ofn, 0, sizeof( ofn ) );

      for (index=0; szFilter[index] != '\0'; index++)
         if (szFilter[index] == '|')
            szFilter[index] = '\0';

      szFile[0] = '\0';

      ofn.lStructSize = sizeof( ofn );
      ofn.hwndOwner = NULL;
      ofn.hInstance = NULL;
      ofn.lpstrFilter = szFilter;
      ofn.nFilterIndex = 1;
      ofn.lpstrFile = szFile;
      ofn.nMaxFile = sizeof(szFile);
      ofn.lpstrFileTitle = szFileTitle;
      ofn.nMaxFileTitle = sizeof(szFileTitle);
      if (config.mods[0].dir[0])
         ofn.lpstrInitialDir = config.halflife_dir;
      else
         ofn.lpstrInitialDir = NULL;
      ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_EXPLORER |
                  OFN_ENABLESIZING | OFN_ENABLEHOOK;
      ofn.lpfnHook = CenterOpenDlgBox;

      if(!GetOpenFileName( &ofn ))
         Error("You must specify a BSP file!\n");

      strcpy(bsp_filename, ofn.lpstrFile);

      LoadBSPFile(bsp_filename);

      strcpy(bspname, bsp_filename);

      // is this BSP file in a MOD directory?
      ExtractFilePath (bspname, pathname);

      len = strlen(pathname);  // remove trailing '/'

      if (len > 0)
         pathname[len-1] = 0;

      if (pathname[0] != 0)
      {
         ExtractFileBase (pathname, modname);

         if (stricmp(modname, "maps") == 0)
         {
            ExtractFilePath (pathname, modname);

            if (modname[0] != 0)
            {
               for (int i = 0; i < config.num_mods; i++)
               {
                  if (stricmp(modname, config.mods[i].dir) == 0)
                        config.bsp_mod_index = i;
               }
            }
         }
      }

      bsp_found = TRUE;
#else
      Error("You must specify a BSP filename to load!\n");
#endif
   }
   else
      strcpy(bspname, config.bspname);  // use filename from config file

   mod_index = 0;

   while ((bsp_found == FALSE) && (mod_index < config.num_mods))
   {
      strcpy(bsp_filename, bspname);

      StripExtension(bsp_filename);
      DefaultExtension(bsp_filename, ".bsp");

      strcpy(pathname, config.mods[mod_index].dir);
#ifdef WIN32
      strcat(pathname, "maps\\");
#else
      strcat(pathname, "maps/");
#endif
      strcat(pathname, bsp_filename);

      // check if the .bsp file exists...
      if (FileTime(pathname) != -1)
      {
         strcpy(bsp_filename, pathname);

         LoadBSPFile(bsp_filename);

         config.bsp_mod_index = mod_index;

         bsp_found = TRUE;
      }

      mod_index++;
   }

   if (!bsp_found)  // otherwise, see if it exists in a PAK file...
   {
      bsp_found = LoadPakBSPFile(bsp_filename);
   }

   if (!bsp_found)
      Error("Can't load map: %s\n", bspname);

   ParseEntities();

   LoadEntVars();
}

