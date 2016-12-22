//
// BSP_tool - botman's Half-Life BSP utilities
//
// (http://planethalflife.com/botman/)
//
// gl_win.cpp
//

/* 
 * HL rendering engine
 * Copyright (c) 2000,2001 Bart Sekura
 *
 * Permission to use, copy, modify and distribute this software
 * is hereby granted, provided that both the copyright notice and 
 * this permission notice appear in all copies of the software, 
 * derivative works or modified versions.
 *
 * THE AUTHOR ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION AND DISCLAIMS ANY LIABILITY OF ANY KIND FOR ANY DAMAGES 
 * WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 *
 * OpenGL window
 */

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
#include "config.h"
#include "glwin.h"

extern bool keys[MAX_KEYS];
extern bool left_button;
extern bool right_button;
extern int  xpos;
extern int  ypos;
extern int  m_width;
extern int  m_height;
extern bool m_center_window;
extern bool m_fullscreen;

extern Config config;

/////////////////////////////////////////////////////////////////

void
GLWindow::GLInfo::init()
{
    const char* vendor = (const char*) glGetString(GL_VENDOR);
    const char* version = (const char*) glGetString(GL_VERSION);
    const char* renderer = (const char*) glGetString(GL_RENDERER);

    const char* error = "error";
    m_vendor = vendor ? vendor : error;
    m_version = version ? version : error;
    m_renderer = renderer ? renderer : error;

    const char* ext = (const char*) glGetString(GL_EXTENSIONS);
    if(ext) {
        // parse extensions
        char* s = new char[strlen(ext)+1];
        if (!s)
            Error("Error allocating GLInfo memory!\n");
        strcpy(s, ext);
        char seps[] = " ";
        char* token = strtok(s, seps);
        while(token) {
            m_extensions.push_back(token);
            token = strtok(0, seps);
        }
        delete s;
    }
}

///////////////////////////////////////////////////////////////////

const char* GLWindow::className = "GL Engine";
GLWindow* GLWindow::m_instance = 0;
GLWindow::GLInfo GLWindow::gl_info;
vector<DEVMODE> GLWindow::m_modes;

void
GLWindow::create(int width, int height, int bpp, int hz, bool fullscreen)
{
    int x_pos, y_pos;

    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW | CS_OWNDC, 
                      WindowProc, 0L, 0L,
                      GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
                      className, NULL };

    RegisterClassEx(&wc);
    m_hinst = wc.hInstance;

    // if window exists, kill it
    if(m_instance) {
        m_instance->destroy();
        m_instance = 0;
    }

    // go fullscreen in requested
    if(fullscreen) {
        DEVMODE dm;
        memset(&dm, 0, sizeof(dm));
        dm.dmSize=sizeof(dm);
        dm.dmPelsWidth    = width;
        dm.dmPelsHeight    = height;
        dm.dmBitsPerPel    = bpp;
        dm.dmFields        = DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

        if(hz) {
            dm.dmDisplayFrequency = hz;
            dm.dmFields |= DM_DISPLAYFREQUENCY;
        }

        if(ChangeDisplaySettings(&dm, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL) {
            Error("Error starting in Fullscreen mode!\n");
        }
    }

    // Create a window
    DWORD style;
    DWORD exStyle;

    if(fullscreen) {
        exStyle = WS_EX_TOPMOST|WS_EX_APPWINDOW;
        style = WS_POPUP;
    }
    else {
        exStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
        style = WS_OVERLAPPEDWINDOW;
    }

    x_pos = 0;
    y_pos = 0;

    if ((config.x_pos < 0) && (config.y_pos < 0))
       m_center_window = TRUE;
    else
    {
       m_center_window = FALSE;

       x_pos = config.x_pos;
       y_pos = config.y_pos;
    }

    m_hwnd = CreateWindowEx(exStyle,
                            className,
                            "BSP view",
                            style,
                            x_pos,
                            y_pos,
                            width,
                            height,
                            0,
                            0,
                            wc.hInstance,
                            0);
    if (!m_hwnd) {
        Error("Error creating Window!\n");
    }

    m_hdc = GetDC(m_hwnd);
    if(!m_hdc) {
        Error("Error Getting Device Context!\n");
    }

    static PIXELFORMATDESCRIPTOR pfd =
    {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,                                    // Version Number
        PFD_DRAW_TO_WINDOW |                // Format Must Support Window
        PFD_SUPPORT_OPENGL |                // Format Must Support OpenGL
        PFD_DOUBLEBUFFER,                    // Must Support Double Buffering
        PFD_TYPE_RGBA,                        // Request An RGBA Format
        bpp,                                // Select Our Color Depth
        0, 0, 0, 0, 0, 0,                    // Color Bits Ignored
        0,                                    // No Alpha Buffer
        0,                                    // Shift Bit Ignored
        0,                                    // No Accumulation Buffer
        0, 0, 0, 0,                            // Accumulation Bits Ignored
        24,                                    // Z-Buffer (Depth Buffer) bits
        0,                                    // No Stencil Buffer
        0,                                    // No Auxiliary Buffer
        PFD_MAIN_PLANE,                        // Main Drawing Layer
        0,                                    // Reserved
        0, 0, 0                                // Layer Masks Ignored
    };

    GLuint pixFmt = 0;
    if(!(pixFmt = ChoosePixelFormat(m_hdc, &pfd))) {
        Error("Error in ChoosePixelFormat!\n");
    }

    if(!SetPixelFormat(m_hdc, pixFmt, &pfd)) {
        Error("Error in SetPixelFormat!\n");
    }

    if(!(m_hrc = wglCreateContext(m_hdc))) {
        Error("Error in wglCreateContext!\n");
    }

    if(!wglMakeCurrent(m_hdc, m_hrc)) {
        Error("Error in wglMakeCurrent!\n");
    }

    ShowWindow(m_hwnd, SW_SHOW);
    UpdateWindow(m_hwnd);
    SetForegroundWindow(m_hwnd);
    SetFocus(m_hwnd);

    m_instance = this;
    m_width = width;
    m_height = height;
    m_bpp = bpp;
    m_fullscreen = fullscreen;

    enum_display();
    gl_info.init();

    resize();

    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    SwapBuffers(m_hdc);

    init();

    m_active = TRUE;
}

void
GLWindow::destroy()
{
    if(m_fullscreen) {
        ChangeDisplaySettings(0, 0);
        ShowCursor(TRUE);
    }

    if(m_hrc) {
        if(!wglMakeCurrent(0, 0)) {
        }

        if(!wglDeleteContext(m_hrc)) {
        }

        m_hrc = 0;
    }

    if(m_hdc) {
        if(!ReleaseDC(m_hwnd, m_hdc)) {
        }
    }

    if(m_hwnd) {
        if(!DestroyWindow(m_hwnd)) {
        }
    }

    UnregisterClass(className, m_hinst);
    m_hinst = 0;
    m_width = 0;
    m_height = 0;
    m_bpp = 0;
    m_fullscreen = FALSE;
    m_active = FALSE;
}

LRESULT
GLWindow::window_proc(UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch(msg) {
        case WM_ACTIVATE:
            if(wparam == WA_INACTIVE) {
                m_active = FALSE;
            }
            else if ((wparam == WA_ACTIVE) ||
                     (wparam == WA_CLICKACTIVE)) {
                m_active = TRUE;
            }

            return 0;

        case WM_CLOSE:
            PostQuitMessage(0);
            return 0;

        case WM_SIZE:
            if(wparam == SIZE_MINIMIZED) {
                m_active = FALSE;
                return 0;
            }
            else 
            if ((wparam == SIZE_RESTORED) ||
                (wparam == SIZE_MAXIMIZED)) {
                m_active = TRUE;
            }
            m_width = LOWORD(lparam);
            m_height = HIWORD(lparam);
            resize();
            return 0;

        case WM_KEYDOWN:
            keys[wparam] = TRUE;
            if(wparam == VK_ESCAPE) {
                PostQuitMessage(0);
            }
            return 0;

        case WM_KEYUP:
            keys[wparam] = FALSE;
            return 0;

        case WM_MOUSEMOVE:
            {
                POINT p;

                GetCursorPos(&p);

                if(!m_fullscreen)
                    ScreenToClient(m_hwnd,&p);

                xpos = (float) p.x;
                ypos = (float) p.y;
            }
            return 0;

        case WM_LBUTTONDOWN:
            left_button = TRUE;
            return 0;

        case WM_LBUTTONUP:
            left_button = FALSE;
            return 0;
    }

    return DefWindowProc(m_hwnd, msg, wparam, lparam);
}

void
GLWindow::enum_display()
{
    if(m_modes.size()) {
        return;
    }

    int c  = 0;
    DEVMODE mode;
    while(EnumDisplaySettings(0, c++, &mode)) {
        if(mode.dmBitsPerPel >= 16 &&   // only 16bpp or more
            mode.dmPelsWidth >= 640 &&  // at least 640x...
            mode.dmDisplayFlags == 0)   // color and noninterlaced
        {
            m_modes.push_back(mode);
        }
    }
}

