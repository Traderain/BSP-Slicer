//
// BSP_tool - botman's Half-Life BSP utilities
//
// (http://planethalflife.com/botman/)
//
// bsp_tool.cpp
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

#include <stdio.h>
#include <string.h>

#include "cmdlib.h"
#include "config.h"
#include "world.h"
#include "bspfile.h"
#include "print.h"
#include "waypoint.h"


bool isWindowsApp = FALSE;

Config config("BSP_tool.cfg");
World world;


void main (int argc, char **argv)
{
   char filename[256];
   int n;
   int grid_size;
   bool do_sizes = FALSE;
   bool do_models = FALSE;
   bool do_vertexes = FALSE;
   bool do_planes = FALSE;
   bool do_leafs = FALSE;
   bool do_nodes = FALSE;
   bool do_texinfo = FALSE;
   bool do_miptex = FALSE;
   bool do_faces = FALSE;
   bool do_edges = FALSE;
   bool do_entities = FALSE;
   bool do_autowaypoint = FALSE;

   if (argc < 2)
   {
      printf("\n");
      printf("usage: BSP_tool -s -m -v -p -l -n -t -mip -f -e -ent -wN bspfile\n");
      printf("\n");
      printf("where: -s = print BSP item sizes\n");
      printf("       -m = print models\n");
      printf("       -v = print vertexes\n");
      printf("       -p = print planes\n");
      printf("       -l = print leaves\n");
      printf("       -n = print nodes\n");
      printf("       -t = print texinfo\n");
      printf("       -mip = print miptex\n");
      printf("       -f = print faces\n");
      printf("       -e = print edges\n");
      printf("       -ent = print entities\n");
      printf("       -wN = run autowaypoint on BSP file\n");
      printf("             (where N is the grid size: 64, 72, 80, 100, 120, 150, 200)\n");
      return;
   }

   for (n = 1; n < argc; n++)
   {
      if (strcmp(argv[n], "-s") == 0)
         do_sizes = TRUE;
      if (strcmp(argv[n], "-m") == 0)
         do_models = TRUE;
      if (strcmp(argv[n], "-v") == 0)
         do_vertexes = TRUE;
      if (strcmp(argv[n], "-p") == 0)
         do_planes = TRUE;
      if (strcmp(argv[n], "-l") == 0)
         do_leafs = TRUE;
      if (strcmp(argv[n], "-n") == 0)
         do_nodes = TRUE;
      if (strcmp(argv[n], "-t") == 0)
         do_texinfo = TRUE;
      if (strcmp(argv[n], "-mip") == 0)
         do_miptex = TRUE;
      if (strcmp(argv[n], "-f") == 0)
         do_faces = TRUE;
      if (strcmp(argv[n], "-e") == 0)
         do_edges = TRUE;
      if (strcmp(argv[n], "-ent") == 0)
         do_entities = TRUE;
      if (strncmp(argv[n], "-w", 2) == 0)
      {
         do_autowaypoint = TRUE;
         if (sscanf(&argv[n][2], "%d", &grid_size) < 1)
            grid_size = 100;  // default to 100 units for grid size
      }
      else
         strcpy(filename, argv[n]);
   }

   world.LoadBSP(filename);

   if (do_sizes)
   {
      PrintBSPFileSizes();
      printf("\n");
   }

   if (do_models)
      print_models();
   if (do_vertexes)
      print_vertexes();
   if (do_planes)
      print_planes();
   if (do_leafs)
      print_leafs();
   if (do_nodes)
      print_nodes();
   if (do_texinfo)
      print_texinfo();
   if (do_miptex)
      print_miptex();
   if (do_faces)
      print_faces();
   if (do_edges)
      print_edges();
   if (do_entities)
      print_entities();

   if (do_autowaypoint)
      WaypointLevel(grid_size);  // auto waypoint the BSP world
}

