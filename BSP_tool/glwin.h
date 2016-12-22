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

#ifndef __glwin_h__
#define __glwin_h__

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>

// OpenGL stuff
#include <gl/gl.h>

#ifndef WORLD_H
#include "world.h"
#endif

#define MAX_KEYS    (256)

extern bool m_center_window;
extern bool m_fullscreen;

// stc++
#pragma warning(disable:4786)
#include <vector>
#include <string>
using namespace std;

class GLWindow {
public:
    class GLInfo {
    public:
        GLInfo() {}
        void init();

        const char* vendor() const { return m_vendor.c_str(); }
        const char* version() const { return m_version.c_str(); }
        const char* renderer() const { return m_renderer.c_str(); }

        const vector<string>& extensions() const { return m_extensions; }

        bool has_extension(const char* name) const {
            for(int i = 0; i < m_extensions.size(); i++) {
                if(m_extensions[i] == name) {
                    return true;
                }
            }
            return false;
        }

    private:
        string m_vendor;
        string m_version;
        string m_renderer;
        vector<string> m_extensions;
    };

public:
    GLWindow() 
        : m_bpp(0), 
          m_hinst(0),
          m_hwnd(0),
          m_hdc(0),
          m_hrc(0),
          m_active(FALSE)
    {}
    ~GLWindow() { m_instance = 0; destroy(); }

    void create(int width, int height, int bpp, int hz = 0, bool fullscreen = false);
    void destroy();

    void flip() const { SwapBuffers(m_hdc); }

protected:
    // window proc
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
    {
        if ((msg == WM_CREATE) && (m_fullscreen == false) && (m_center_window))
        {
            CenterWindow( hwnd );
        }

        if (m_instance) {
            return m_instance->window_proc(msg, wparam, lparam);
        }
        return DefWindowProc(hwnd, msg, wparam, lparam);
    }

    LRESULT window_proc(UINT msg, WPARAM wparam, LPARAM lparam);

public:
    static const char* className;
    static void enum_display();
    static vector<DEVMODE> m_modes;
    static GLInfo gl_info;

    // extensions
    static bool enable_multitexture();

protected:
    virtual void init() {}
    virtual void resize() {}

protected:
    string m_title;
    int    m_bpp;
    bool   m_active;

protected:
    HINSTANCE m_hinst;
    HWND      m_hwnd;
    HDC       m_hdc;
    HGLRC     m_hrc;

    static GLWindow* m_instance;
};

#endif // __glwin_h__

