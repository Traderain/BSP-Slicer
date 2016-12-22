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
 * hopefully fast math routines
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

#ifndef __fastmath_h__
#define __fastmath_h__

#ifdef WIN32

inline static
__declspec(naked) float __fastcall 
fsin(float a) 
{
    __asm {
        fld        DWORD PTR [esp+4] 
        fsin
        ret 4
    }
}

inline static
__declspec(naked) float __fastcall 
fcos(float a) 
{
    __asm {
        fld        DWORD PTR [esp+4] 
        fcos
        ret 4
    }
}

#else

#define fsin sin
#define fcos cos

#endif  // WIN32

#define __PI        (3.14159265358979323846264338327950288f)
#define __DEG2RAD    (__PI/180)

#define deg2rad(x)    ((x)*__DEG2RAD)

inline static float 
fast_cos(float angle) 
{
    return fcos(deg2rad(angle));
}

inline static float 
fast_sin(float angle) 
{
    return fsin(deg2rad(angle));
}

#endif // __fastmath_h__

