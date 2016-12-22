//
// BSP_tool - botman's Half-Life BSP utilities
//
// (http://planethalflife.com/botman/)
//
// sys_time.cpp
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

#include "sys_time.h"


// NOTE: frametime may not be very useful since on very fast machines this
// value can fluctuate from 10 or less all the way down to 0.  The millisecond
// timer sampling of Windows GetTickCount() does NOT lead to very good precision
// in the frametime value.  If you are using frametime you should probably
// average the values over a 1/10th of a second period (using gametime).

int frametime = 0;  // time to render the last frame (in milliseconds)

float gametime = 0.0f;  // time since the game has been running (in seconds)
float frames_per_second = 0.0;  // number of frames rendered per second (DUH!)


// SysTime_Update should be called once at the start of each video frame

void SysTime_Update(void)
{
   static int frame_count = 0;
   static float previous_update_time = -1.0f;  // for updating FPS (in seconds)
   long delta_time;  // in milliseconds
   float diff;  // in seconds

#ifdef WIN32
   static DWORD previous_time = 0;  // in milliseconds
   DWORD current_time;  // in milliseconds
#else
   static long previous_time = 0;  // in seconds
   static long previous_millisec = 0;
   long delta_seconds;
   long delta_millisec;
   struct timeval current;
#endif

#ifdef WIN32
   current_time = GetTickCount();

   if (current_time < previous_time)  // wrap around?
   {
      // wraps around every 49.7 days.  does anybody's Windows machine
      // actually stay up and running that long???  :)
      previous_time = 0;
   }
#else
   gettimeofday(&current, NULL);
#endif

   frame_count++;

   if (previous_time != 0)
   {

#ifdef WIN32
      delta_time = current_time - previous_time;  // in milliseconds
#else
      delta_seconds = current.tv_sec - previous_time;
      delta_millisec = (current.tv_usec / 1000) - previous_millisec;

      delta_time = (delta_seconds * 1000) + delta_millisec;  // in milliseconds
#endif

      gametime += (float)delta_time / 1000.0f;  // convert to seconds

      if (gametime >= (previous_update_time + 1.0f))  // time to update frames per second?
      {
         diff = gametime - previous_update_time;

         // calculate the frames per second...
         frames_per_second = frame_count * diff;

         if (frames_per_second > 999.9)
            frames_per_second = 999.9f;

         frame_count = 0;
         previous_update_time = gametime;
      }

      frametime = delta_time;  // in milliseconds
   }

#ifdef WIN32
   previous_time = current_time;
#else
   previous_time = current.tv_sec;
   previous_millisec = current.tv_usec / 1000;
#endif

   if (previous_update_time < 0.0)
      previous_update_time = gametime;
}

