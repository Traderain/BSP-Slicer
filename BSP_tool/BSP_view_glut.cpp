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

#include "render.h"
#include "config.h"
#include "world.h"

//#include <gltk.h>


Config config("BSP_tool.cfg");
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

    render.create(config.width, config.height, config.bits_per_pixel,
                  config.refresh_rate, config.enable_fullscreen);

    world.LoadBSP();

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

    return 0;
}
#else

GLenum doubleBuffer;

static void Args(int argc, char **argv)
{
    GLint i;

    doubleBuffer = GL_FALSE;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-sb") == 0) {
            doubleBuffer = GL_FALSE;
        } else if (strcmp(argv[i], "-db") == 0) {
            doubleBuffer = GL_TRUE;
        }
    }
}

void display(void)
{
   render.frame();
}

int main(int argc, char *argv[])
{
    GLenum type;

    // initialize the random seed (for random spawn point)
    srand((unsigned)time(NULL));

    glutInit(&argc, argv);
    Args(argc, argv);

    type = GLUT_RGB | GLUT_DEPTH;
    type |= (doubleBuffer) ? GLUT_DOUBLE : GLUT_SINGLE;
    glutInitDisplayMode(type);
    glutInitWindowSize(config.width, config.height);
    glutCreateWindow("BSP_view");

    world.LoadBSP();

    printf("BSP file loaded, rendering...\n");

    render.init();

    render.InitRender();

    glutDisplayFunc(display);
    glutIdleFunc(display);
    glutMainLoop();
}

#endif

