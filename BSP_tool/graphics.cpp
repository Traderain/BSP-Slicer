//
// BSP_tool - botman's Half-Life BSP utilities
//
// (http://planethalflife.com/botman/)
//
// graphics.cpp
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
#include <stdlib.h>
#include <string.h>
#include <memory.h>

#include "paklib.h"

#pragma pack(push)
#pragma pack(1)

typedef struct
{
   unsigned char id_length;
   unsigned char colormap_type;
   unsigned char image_type;
   unsigned short colormap_index;
   unsigned short colormap_length;
   unsigned char colormap_size;
   unsigned short x_origin, y_origin;
   unsigned short width, height;
   unsigned char pixel_size;
   unsigned char attributes;
} TGAHeader;

#pragma pack(pop)


int TGADecodeScanLine(unsigned char *buffer, unsigned int LineLength,
                      unsigned int PixelSize, FILE *fp, unsigned char *pdata)
{
   unsigned int  i;           /* loop counter                                 */
   short int     byteCount;   /* number of bytes written to the buffer        */
   unsigned char runCount;    /* the pixel run count                          */
   unsigned int  bufIndex;    /* the index of DecodedBuffer                   */
   unsigned int  bufMark;     /* index marker of DecodedBuffer                */
   unsigned int  pixelCount;  /* the number of pixels read from the scan line */

   bufIndex   = 0;  /* initialize buffer index  */
   byteCount  = 0;  /* initialize byte count    */
   pixelCount = 0;  /* initialize pixel counter */

   /* Main decoding loop */
   while (pixelCount < LineLength)
   {
      /* Get the pixel count */
      if (fp)
         fread(&runCount, 1, 1, fp);
      else
      {
         runCount = *pdata;
         pdata++;
      }
     
      /* Make sure writing this next run will not overflow the buffer */
      if (pixelCount + (runCount & 0x7f) + 1 > LineLength)
         return(-1);     /* Pixel run will overflow buffer */

      /* If the run is encoded... */
      if (runCount & 0x80)
      {
         runCount &= ~0x80;              /* Mask off the upper bit       */

         bufMark = bufIndex;             /* Save the start-of-run index  */

         /* Update total pixel count */
         pixelCount += (runCount + 1);

         /* Update the buffer byte count */ 
         byteCount += ((runCount + 1) * PixelSize);

         /* Write the first pixel of the run to the buffer */
         for (i = 0; i < PixelSize; i++)
         {
            if (fp)
               fread(&buffer[bufIndex++], 1, 1, fp);
            else
            {
               buffer[bufIndex++] = *pdata;
               pdata++;
            }
         }

         /* Write remainder of pixel run to buffer 'runCount' times */
         while (runCount--)
         {
            for (i = 0; i < PixelSize; i++)
               buffer[bufIndex++] = buffer[bufMark + i];
         }
     }
     else    /* ...the run is unencoded (raw) */
     {
         /* Update total pixel count */
         pixelCount += (runCount + 1);
            
         /* Update the buffer byte count */
         byteCount  += ((runCount + 1) * PixelSize);

         /* Write runCount pixels */
         do
         {
            for (i = 0; i < PixelSize; i++)
            {
               if (fp)
                  fread(&buffer[bufIndex++], 1, 1, fp);
               else
               {
                  buffer[bufIndex++] = *pdata;
                  pdata++;
               }
            }
         }
         while (runCount--);
      }
   }

   return (byteCount);
}


unsigned char *TGA_Load(const char *filename, int *width, int *height)
{
   FILE *fp;
   TGAHeader hdr;
   int bpp;
   unsigned int bytes, size;
   unsigned char *data, *scanline;
   int offset;

   if ((fp = fopen(filename, "rb")) == NULL)
      return NULL;

   if (fread(&hdr, 1, sizeof(hdr), fp) != sizeof(hdr))
      return NULL;

   bpp = hdr.pixel_size;

   bytes = bpp >> 3;

   size = hdr.width * hdr.height * bytes;

   data = (unsigned char *)malloc(size);

   if (data == NULL)
      return NULL;

   if (hdr.id_length)
      fseek(fp, hdr.id_length, SEEK_CUR);

   if ((hdr.image_type == 9) || (hdr.image_type == 10) ||
       (hdr.image_type == 11))
   {
      scanline = (unsigned char *)malloc(hdr.width * bytes);
      offset = 0;

      for (int i = 0; i < hdr.height; i++)
      {
         if (TGADecodeScanLine(scanline, hdr.width, bytes, fp, NULL) < 0)
            return NULL;

         memcpy(&data[offset], scanline, hdr.width * bytes);
         offset += (hdr.width * bytes);
      }

      free(scanline);
   }
   else  // not compressed, just raw data...
   {
      if (fread(data, 1, size, fp) != size)
      {
         free(data);
         return NULL;
      }
   }

   // swap bgr to rgb for bpp>8
   if (bytes > 1)
   {
      for (unsigned int c = 0; c < size; c += bytes)
      {
         unsigned char tmp = data[c];
         data[c] = data[c+2];
         data[c+2] = tmp;
      }
   }

   if (hdr.attributes & 0x20)  // bottom up, need to swap scanlines
   {
      unsigned char *temp = (unsigned char *)malloc(hdr.width * bytes);

      for (int i = 0; i < hdr.height/2; i++)
      {
         memcpy(temp, data + i*hdr.width*bytes, hdr.width*bytes);
         memcpy(data + i*hdr.width*bytes,
                data + (hdr.height-i-1)*hdr.width*bytes, hdr.width*bytes);
         memcpy(data + (hdr.height-i-1)*hdr.width*bytes, temp, hdr.width*bytes);
      }

      free(temp);
   }

   fclose(fp);

   *width = hdr.width;
   *height = hdr.height;

   return data;
}


unsigned char *TGA_LoadPak(const char *filename, int *width, int *height)
{
   char TGAfilename[256];
   pakconfig_t *pakconfig = NULL;
   pakinfo_t *pakinfo = NULL;
   unsigned char *pdata, *data, *scanline;
   TGAHeader *hdr;
   int bpp;
   unsigned int bytes, size;
   int offset;

   strcpy(TGAfilename, filename);

   if (SearchPakFilename(TGAfilename, &pakconfig, &pakinfo))
   {
      pdata = (unsigned char *)malloc(pakinfo->file_length);

      P_ReadPakItem(pakconfig->pakhandle, pakinfo, pdata);

      hdr = (TGAHeader *)pdata;  // point to header

      // adjust pdata to point to color map data...
      pdata = pdata + sizeof(TGAHeader) + hdr->id_length;

      bpp = hdr->pixel_size;

      bytes = bpp >> 3;

      size = hdr->width * hdr->height * bytes;

      data = (unsigned char *)malloc(size);

      if (data == NULL)
      {
         free(pdata);
         return NULL;
      }

      if ((hdr->image_type == 9) || (hdr->image_type == 10) ||
          (hdr->image_type == 11))
      {
         scanline = (unsigned char *)malloc(hdr->width * bytes);
         offset = 0;

         for (int i = 0; i < hdr->height; i++)
         {
            if (TGADecodeScanLine(scanline, hdr->width, bytes, NULL, pdata) < 0)
               return NULL;

            memcpy(&data[offset], scanline, hdr->width * bytes);
            offset += (hdr->width * bytes);
         }

         free(scanline);
      }
      else  // not compressed, just raw data...
      {
         memcpy(data, pdata, size);
      }

      // swap bgr to rgb for bpp>8
      if (bytes > 1)
      {
         for (unsigned int c = 0; c < size; c += bytes)
         {
            unsigned char tmp = data[c];
            data[c] = data[c+2];
            data[c+2] = tmp;
         }
      }

      if (hdr->attributes & 0x20)  // bottom up, need to swap scanlines
      {
         unsigned char *temp = (unsigned char *)malloc(hdr->width * bytes);

         for (int i = 0; i < hdr->height/2; i++)
         {
            memcpy(temp, data + i*hdr->width*bytes, hdr->width*bytes);
            memcpy(data + i*hdr->width*bytes,
                   data + (hdr->height-i-1)*hdr->width*bytes, hdr->width*bytes);
            memcpy(data + (hdr->height-i-1)*hdr->width*bytes, temp, hdr->width*bytes);
         }

         free(temp);
      }

      *width = hdr->width;
      *height = hdr->height;

      free(hdr);  // actually frees original pdata

      return data;
   }

   return NULL;
}

