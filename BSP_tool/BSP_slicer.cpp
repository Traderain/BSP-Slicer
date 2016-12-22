//
// BSP_tool - botman's Half-Life BSP utilities
//
// (http://planethalflife.com/botman/)
//
// bsp_slicer.cpp
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
#include <string>

#include "cmdlib.h"
#include "config.h"
#include "world.h"
#include "bspfile.h"
#include "mathlib.h"

#pragma pack(push)
#pragma pack(1)

typedef struct rgb_s
{
   unsigned char rgbBlue;
   unsigned char rgbGreen;
   unsigned char rgbRed;
   unsigned char rgbReserved;
} rgb_t;

typedef struct bitmapfileheader_s
{
   short int     bfType;
   unsigned int  bfSize;
   short int     bfReserved1;
   short int     bfReserved2;
   unsigned int  bfOffBits;
} bitmapfileheader_t;

typedef struct bitmapinfoheader_s
{
   unsigned int  biSize;
   long int      biWidth;
   long int      biHeight;
   short int     biPlanes;
   short int     biBitCount;
   unsigned int  biCompression;
   unsigned int  biSizeImage;
   long int      biXPelsPerMeter;
   long int      biYPelsPerMeter;
   unsigned int  biClrUsed;
   unsigned int  biClrImportant;
} bitmapinfoheader_t;

#pragma pack(pop)


#define MAX(a,b) (a > b ? a : b)


Config config("BSP_slicer.cfg");
World world;

int x_size, y_size;
bool rotated = FALSE;
bool isWindowsApp = FALSE;


bool CreateBitmapFile(char *filename, unsigned char *lpBits)
{
   FILE *fp;
   bitmapfileheader_t hdr;
   bitmapinfoheader_t bih;
   int index;
   rgb_t colors[256];

   colors[0].rgbBlue = 0xff;  // R=255, B=255, G=255 is WHITE
   colors[0].rgbGreen = 0xff;
   colors[0].rgbRed = 0xff;
   colors[0].rgbReserved = 0x00;

   for (index = 1; index < 256; index++)
   {
      colors[index].rgbBlue = 0x00;  // R=0, B=0, G=0 is BLACK
      colors[index].rgbGreen = 0x00;
      colors[index].rgbRed = 0x00;
      colors[index].rgbReserved = 0x00;
   }

   memset((void *)&bih, 0, sizeof(bih));

   if (rotated)
   {
      bih.biWidth = y_size;
      bih.biHeight = x_size;
   }
   else
   {
      bih.biWidth = x_size;
      bih.biHeight = y_size;
   }
   bih.biSize = sizeof(bih);
   bih.biSizeImage = x_size * y_size;
   bih.biPlanes = 1;
   bih.biBitCount = 8;
   bih.biClrUsed = 256;
   bih.biClrImportant = 256;

   // create the .bmp file...
   fp = fopen(filename, "wb");

   if (fp == NULL)
      return FALSE;  // error creating file

   hdr.bfType = 0x4d42;  // 0x42='B' 0x4d='M'

   // calculate the size of the entire .bmp file...
   hdr.bfSize = (sizeof(bitmapfileheader_t) + bih.biSize +
                bih.biClrUsed * sizeof(rgb_t) + bih.biSizeImage);

   hdr.bfReserved1 = 0;  // not used
   hdr.bfReserved2 = 0;

   // calculate the offset to the array of color indexes...
   hdr.bfOffBits = sizeof(bitmapfileheader_t) + bih.biSize +
                   bih.biClrUsed * sizeof(rgb_t);

   // write the bitmapfileheader info to the .bmp file...
   if (fwrite(&hdr, sizeof(bitmapfileheader_t), 1, fp) != 1)
   {
      fclose(fp);
      return FALSE;  // error writing to file
   }

   // write the bitmapinfoheader info to the .bmp file...
   if (fwrite(&bih, sizeof(bitmapinfoheader_t), 1, fp) != 1)
   {
      fclose(fp);
      return FALSE;  // error writing to file
   }

   // write the RGB palette colors to the .bmp file...
   if (fwrite(&colors, bih.biClrUsed * sizeof(rgb_t), 1, fp) != 1)
   {
      fclose(fp);
      return FALSE;  // error writing to file
   }

   // write the array of color indexes to the .bmp file...
   if (fwrite(lpBits, bih.biSizeImage, 1, fp) != 1)
   {
      fclose(fp);
      return FALSE;  // error writing to file
   }

   fclose(fp);

   return TRUE;
}


void Bresenham(int x0, int y0, int x1, int y1, unsigned char *buffer)
{
   int dy = y1 - y0;
   int dx = x1 - x0;
   int stepx, stepy;
   int offset, max_offset;

   if (dy < 0) { dy = -dy;  stepy = -1; } else { stepy = 1; }
   if (dx < 0) { dx = -dx;  stepx = -1; } else { stepx = 1; }
   dy <<= 1;  // dy is now 2*dy
   dx <<= 1;  // dx is now 2*dx

   max_offset = x_size * y_size;

   if (rotated)
      offset = x0 * y_size + y0;
   else
      offset = y0 * x_size + x0;

   if ((offset >= 0) && (offset < max_offset))
      buffer[offset] = 0xff;
   else
      return;

   if (dx > dy)
   {
      int fraction = dy - (dx >> 1);  // same as 2*dy - dx
      while (x0 != x1)
      {
         if (fraction >= 0)
         {
            y0 += stepy;
            fraction -= dx;           // same as fraction -= 2*dx
         }
         x0 += stepx;
         fraction += dy;              // same as fraction -= 2*dy

         if (rotated)
            offset = x0 * y_size + y0;
         else
            offset = y0 * x_size + x0;

         if ((offset >= 0) && (offset < max_offset))
            buffer[offset] = 0xff;
         else
            return;
      }
   }
   else
   {
      int fraction = dx - (dy >> 1);
      while (y0 != y1)
      {
         if (fraction >= 0)
         {
            x0 += stepx;
            fraction -= dy;
         }
         y0 += stepy;
         fraction += dx;

         if (rotated)
            offset = x0 * y_size + y0;
         else
            offset = y0 * x_size + x0;

         if ((offset >= 0) && (offset < max_offset))
            buffer[offset] = 0xff;
         else
            return;
      }
   }
}


void SliceTheWorld(dmodel_t *model, float min_x, float max_x, float min_y, float max_y,
                   float min_z, float max_z, float stepsize, int scale,
                   bool single_contour, bool multiple_contour)
{
   // slice the world in Z planes going from min_z to max_z by stepsize...
   unsigned char *z_buffer;
   dface_t *face;
   dplane_t *plane;
   int slice_index, number_of_slices;
   int face_index, edge_index, edge;
   vec3_t z_distance, z_normal;
   float z_height;
   vec3_t v[2];
   vec3_t v_temp, intersect_point;
   float line_x1, line_y1, line_x2, line_y2;
   bool first, both;
   char filename[256], c_slice[4];

   x_size = int(max_x - min_x) + 1;
   y_size = int(max_y - min_y) + 1;

   x_size = x_size / scale;
   y_size = y_size / scale;

   // make sure size is a even multiple of 4 (for .bmp file requirements)
   x_size = ((x_size + 3)/4) * 4;
   y_size = ((y_size + 3)/4) * 4;


   z_buffer = (unsigned char *)malloc(x_size * y_size);

   if (z_buffer == NULL)
      Error("Error allocating memory for Z buffer bitmap!\n");

   number_of_slices = (int)((max_z - min_z) / stepsize);

   // clear the "display" buffer...
   memset(z_buffer, 0, x_size * y_size);

   for (slice_index = 0; slice_index < number_of_slices; slice_index++)
   {
      // if not doing a contour map then clear the buffer each pass...
      if ((!single_contour) && (!multiple_contour))
         memset(z_buffer, 0, x_size * y_size);

      z_height = min_z + (slice_index * stepsize);

      z_distance[0] = 0;  z_distance[1] = 0;  z_distance[2] = z_height;
      z_normal[0] = 0;  z_normal[1] = 0;  z_normal[2] = 1.0f;

      // loop through all the faces of the BSP file...
      for (face_index = 0; face_index < model->numfaces; face_index++)
      {
         face = &dfaces[model->firstface + face_index];

         plane = &dplanes[face->planenum];

         first = TRUE;
         both = FALSE;

         // for each face, loop though all of the edges, getting the vertexes...
         for (edge_index = 0; edge_index < face->numedges; edge_index++)
         {
            // get the coordinates of the vertex of this edge...
            edge = dsurfedges[face->firstedge + edge_index];

            if (edge < 0)
            {
               edge = -edge;
               dedge_t* e = &dedges[edge];
               v[0][0] = dvertexes[e->v[1]].point[0];
               v[0][1] = dvertexes[e->v[1]].point[1];
               v[0][2] = dvertexes[e->v[1]].point[2];
            }
            else
            {
               dedge_t* e = &dedges[edge];
               v[0][0] = dvertexes[e->v[0]].point[0];
               v[0][1] = dvertexes[e->v[0]].point[1];
               v[0][2] = dvertexes[e->v[0]].point[2];
            }

            // get the coordinates of the vertex of the next edge...
            edge = dsurfedges[face->firstedge + ((edge_index + 1) % face->numedges)];

            if (edge < 0)
            {
               edge = -edge;
               dedge_t* e = &dedges[edge];
               v[1][0] = dvertexes[e->v[1]].point[0];
               v[1][1] = dvertexes[e->v[1]].point[1];
               v[1][2] = dvertexes[e->v[1]].point[2];
            }
            else
            {
               dedge_t* e = &dedges[edge];
               v[1][0] = dvertexes[e->v[0]].point[0];
               v[1][1] = dvertexes[e->v[0]].point[1];
               v[1][2] = dvertexes[e->v[0]].point[2];
            }

            // vector from v[0] to v[1] intersect the Z plane?
            if (((v[0][2] < z_height) && (v[1][2] > z_height)) ||
                ((v[1][2] < z_height) && (v[0][2] > z_height)))
            {
               // create a normalized vector between the 2 vertexes...
               VectorSubtract(v[1], v[0], v_temp);
               VectorNormalize(v_temp);

               // find the point where the vertor intersects the Z plane...
               VectorIntersectPlane(v[0], v_temp, z_distance, z_normal, intersect_point);

               // is this the first or second Z plane intersection point?
               if (first)
               {
                  first = FALSE;
                  line_x1 = intersect_point[0];
                  line_y1 = intersect_point[1];
               }
               else
               {
                  both = TRUE;
                  line_x2 = intersect_point[0];
                  line_y2 = intersect_point[1];
               }
            }

            VectorCopy(v[1], v[0]);  // move v[1] to v[0]
         }

         if (both)  // did we find 2 points that intersected the Z plane?
         {
            line_x1 -= min_x;  // offset by min_x to start at 0
            line_x2 -= min_x;
            line_y1 -= min_y;  // offset by min_y to start at 0
            line_y2 -= min_y;

            line_x1 = line_x1 / scale;
            line_y1 = line_y1 / scale;
            line_x2 = line_x2 / scale;
            line_y2 = line_y2 / scale;

            // are these points within the bounding box of the world?
            if ((line_x1 >= 0) && (line_x1 < x_size) &&
                (line_y1 >= 0) && (line_y1 < y_size) &&
                (line_x2 >= 0) && (line_x2 < x_size) &&
                (line_y2 >= 0) && (line_y2 < y_size))
            {
               int x1, y1, x2, y2;

               x1 = (int)line_x1;
               y1 = (int)line_y1;
               x2 = (int)line_x2;
               y2 = (int)line_y2;

               if (rotated)
                  Bresenham(x1, y_size - y1, x2, y_size - y2, z_buffer);
               else
                  Bresenham(x1, y1, x2, y2, z_buffer);
            }
         }
      }

      if (!single_contour)
      {
         strcpy(filename, world.bspname);
         StripExtension(filename);
         sprintf(c_slice, "%d", slice_index + 1);
         strcat(filename, c_slice);
         strcat(filename, ".bmp");

         printf("creating %s...\n", filename);

         CreateBitmapFile(filename, z_buffer);
      }
   }

   if (single_contour)
   {
      strcpy(filename, world.bspname);
      StripExtension(filename);
      strcat(filename, ".bmp");

      printf("creating %s...\n", filename);

      CreateBitmapFile("result.bmp", z_buffer);
   }

   free(z_buffer);
}


void main (int argc, char **argv)
{
   int n;
   char filename[256];
   dmodel_t *model;
   float min_x, max_x, min_y, max_y, min_z, max_z;
   float thickness;
   int scale;
   int number_of_slices;
   int width, height;
   bool override_begin = FALSE;
   bool override_end = FALSE;
   bool override_thickness = FALSE;
   bool override_scale = FALSE;
   bool single_contour = FALSE;
   bool multiple_contour = FALSE;

   if (argc < 2)
   {
      printf("\n");
      printf("usage: BSP_slicer -bN -eN -tN -sN -c -m bspfile\n");
      printf("\n");
      printf("where: -bN = begin slicing at Z coordinate N.\n");
      printf("where: -eN = end slicing at Z coordinate N.\n");
      printf("where: -tN = use slice thickness of N units.\n");
      printf("where: -sN = use scale factor of N.\n");
      printf("where: -c = create a single contour map.\n");
      printf("where: -m = create multiple contour maps.\n");
      printf("\n");
      printf("For example: BSP_slicer -b-756 -e1076 -t64 -s8 blastzone.bsp\n");
      printf("would slice the map \"blastzone.bsp\" from -756 up to 1076 on the Z axis\n");
      printf("moving by 64 units between slices and would create a 1/8th scale bitmap file.\n");
      printf("\n");
      printf("The defaults for -b and -e is the lower limit and upper limit of the map.\n");
      printf("The default for -t is 64 units.\n");
      printf("The default for -s will depend on the size of the map.  The resultant bitmap\n");
      printf("file will be no larger than 1024 by 768\n");
      return;
   }

   for (n = 1; n < argc; n++)
   {
      if (strncmp(argv[n], "-b", 2) == 0)
      {
         if (sscanf(&argv[n][2], "%f", &min_z) == 1)
            override_begin = TRUE;
      }
      else if (strncmp(argv[n], "-e", 2) == 0)
      {
         if (sscanf(&argv[n][2], "%f", &max_z) == 1)
            override_end = TRUE;
      }
      else if (strncmp(argv[n], "-t", 2) == 0)
      {
         if (sscanf(&argv[n][2], "%f", &thickness) == 1)
            override_thickness = TRUE;
      }
      else if (strncmp(argv[n], "-s", 2) == 0)
      {
         if (sscanf(&argv[n][2], "%d", &scale) == 1)
            override_scale = TRUE;
      }
      else if (strcmp(argv[n], "-c") == 0)
      {
         single_contour = TRUE;
      }
      else if (strcmp(argv[n], "-m") == 0)
      {
         multiple_contour = TRUE;
      }
	  else
		  strcpy(filename, argv[n]);
   }
   LoadBSPFile(filename);

   model = &dmodels[0];  // model 0 is the BSP world

   min_x = model->mins[0];
   max_x = model->maxs[0];
   min_y = model->mins[1];
   max_y = model->maxs[1];

   if (!override_begin)
      min_z = model->mins[2];

   if (!override_end)
      max_z = model->maxs[2];

   if (!override_thickness)
      thickness = 64.0f;

   width = (int)(max_x - min_x);
   height = (int)(max_y - min_y);

   // is the map higher than it is wide?
   if (height > width)
   {
      rotated = TRUE;  // rotate the map 90 degrees
      width = (int)(max_y - min_y);  // swap the width and height
      height = (int)(max_x - min_x);
   }

   if (!override_scale)
   {
      int x_scale, y_scale;

      // determine the scale based on min and max X and Y values...
      x_scale = (width + 1023) / 1024;
      y_scale = (height + 767) / 768;

      scale = MAX(x_scale, y_scale);
   }

   number_of_slices = (int)((max_z - min_z) / thickness);

   printf("slicing the world from %5.0f to %5.0f using slices %3.0f units thick.\n",
          min_z, max_z, thickness);

   if ((!single_contour) && (number_of_slices > 1))
      printf("%d bitmap files will be created at 1/%d scale.\n", number_of_slices, scale);
   else
      printf("1 bitmap file will be created at 1/%d scale.\n", scale);

   // don't start or end on exact multiples of the Z slice thickness
   // (if you do, it causes "weirdness" in the plane intersect function)

   min_z = (float)floor(min_z) + 0.01f;  // offset a tiny bit
   max_z = (float)floor(max_z) + 0.01f;  // offset a tiny bit

   SliceTheWorld(model, min_x, max_x, min_y, max_y, min_z, max_z,
                 thickness, scale, single_contour, multiple_contour);
}

