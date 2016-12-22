//
// BSP_tool - botman's Half-Life BSP utilities
//
// (http://planethalflife.com/botman/)
//
// bsp_view.cpp
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

#include <time.h>

//#include <gltk.h>

#include "render.h"
#include "config.h"
#include "world.h"


bool isWindowsApp = TRUE;

Config config("BSP_view.cfg");
World world;
Render render;

#ifndef __linux__
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
    MSG msg;

    // initialize the random seed (for random spawn point)
    srand((unsigned)time(NULL));

    world.LoadBSP(lpCmdLine);

    render.create(config.width, config.height, config.bits_per_pixel,
                  config.refresh_rate, config.enable_fullscreen);

    render.InitRender();
    
    ZeroMemory(&msg, sizeof(msg));

    while(msg.message != WM_QUIT)
    {
        if(PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } 
        else
            render.frame();
    }

    render.shutdown();

    return 0;
}
#else

extern "C" {
   void tkExposeFunc(void (*)(int, int));
   void tkReshapeFunc(void (*)(int, int));
   void tkSwapBuffers(void);
   void tkInitPosition(int, int, int, int);
   GLenum tkInitDisplayMode(unsigned int);
   GLenum tkInitWindow(char *);
   void tkQuit(void);
   void tkKeyDownFunc(GLenum (*)(int, GLenum));
   void tkMouseMoveFunc(GLenum (*)(int, int, GLenum));
   void tkDisplayFunc(void (*)(void));
   void tkIdleFunc(void (*)(void));
   void tkExec(void);
};

#define TK_RGB 0
#define TK_DOUBLE 2
#define TK_DEPTH 32

#define TK_ESCAPE 0x1B
#define TK_LEFT   0x25
#define TK_UP     0x26
#define TK_RIGHT  0x27
#define TK_DOWN   0x28

#define VK_ESCAPE TK_ESCAPE
#define VK_UP     TK_UP
#define VK_DOWN   TK_DOWN
#define VK_LEFT   TK_LEFT
#define VK_RIGHT  TK_RIGHT

extern bool keys[256];
extern int m_width;
extern int m_height;
extern int xpos;
extern int ypos;


void reshape(int width, int height)
{
   printf("in reshape...\n");

   m_width = width;
   m_height = height;

   render.resize();

   // Set the new viewport size
   glViewport(0, 0, (GLint)width, (GLint)height);
}

void draw(void)
{
   if (keys[VK_ESCAPE])
   {
      keys[VK_ESCAPE] = 0;

      render.shutdown();

      tkQuit();
   }
   else
   {
      render.frame();

      glFlush();

      tkSwapBuffers();
   }
}

GLenum key_down(int key, GLenum state)
{
   if (key == TK_ESCAPE)
   {
      printf("ESCAPE pressed!\n");
      keys[VK_ESCAPE] = 1;
   }
   if (key == TK_UP)
      keys[VK_UP] = 1;
   if (key == TK_DOWN)
      keys[VK_DOWN] = 1;
   if (key == TK_LEFT)
      keys[VK_LEFT] = 1;
   if (key == TK_DOWN)
      keys[VK_DOWN] = 1;
}

GLenum mouse_move(int x, int y, GLenum state)
{
   xpos = x;
   ypos = y;
}

int main(int argc, char *argv[])
{
    GLenum type;

    // initialize the random seed (for random spawn point)
    srand((unsigned)time(NULL));

    // Set top left corner of window to be at location (0, 0)
    tkInitPosition(0, 0, config.width, config.height);

    // Initialize the RGB and Depth buffers
    tkInitDisplayMode(TK_RGB | TK_DEPTH | TK_DOUBLE);

    // Open a window, name it "BSP_view"
    if (tkInitWindow("BSP_view") == GL_FALSE)
        tkQuit();

    if (argc > 1)
       world.LoadBSP(argv[1]);
    else
       world.LoadBSP(NULL);

    printf("BSP file loaded, rendering...\n");

    render.init();

    render.InitRender();

    // Assign reshape() to be the function called whenever an expose event occurs
    tkExposeFunc(reshape);

    // Assign reshape() to be the function called whenever a reshape event occurs
    tkReshapeFunc(reshape);

    // Assign key_down() to be the function called whenever a key is pressed
    tkKeyDownFunc(key_down);

    tkMouseMoveFunc(mouse_move);

    // Assign draw() to be the function called whenever a display
    // event occurs, generally after a resize or expose event
    tkDisplayFunc(draw);

    // Assign idle() to be the function called whenever there are no events
    tkIdleFunc(draw);

    // Pass program control to tk's event handling code
    // In other words, loop forever
    tkExec();
}
#endif

