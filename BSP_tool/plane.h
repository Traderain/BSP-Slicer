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

#ifndef __MATHLIB__
#include "mathlib.h"
#endif

#ifndef __plane_h__
#define __plane_h__

///////////////////////////////////////////////////////////////

typedef vec3_t vertex_t;

#define EPSILON (1.0e-05f)

class plane_t {
public:
    plane_t() : d(0.0f) {}
    plane_t(const vertex_t& a, const vertex_t& b, const vertex_t& c) {
        vertex_t p;
        VectorSubtract(b, a, p);
        vertex_t q;
        VectorSubtract(c, a, q);
        CrossProduct(p, q, n);
        VectorNormalize(n);
        d = -DotProduct(n, a);
    }
    plane_t(const plane_t& p) {
        VectorCopy(p.normal(), n);
        d = p.dist();
    }
    plane_t(const vec3_t& _n, const vec3_t& pt) {
        VectorCopy(_n, n);
        d = -DotProduct(n, pt);
    }
    plane_t(const vec3_t& _n, float dist) {
        VectorCopy(_n, n);
        d = dist;
    }

    void perpendicular(vertex_t& a, vertex_t& b, vec3_t& normal) {
        vec3_t temp;
        VectorAdd(b, normal, temp);
        *this = plane_t(a, b, temp);
    }

    plane_t perpendicular(const vertex_t& a, const vertex_t& b) const {
        vec3_t temp;
        VectorAdd(b, n, temp);
        return plane_t(a, b, temp);
    }

    float dist_to_point(const vec3_t& pt) const {
        return DotProduct(n, pt) + d;
    }

    bool intersect(const vec3_t& start, 
                   const vec3_t& dir, 
                   const vec3_t& pt, 
                   vec3_t& ret) const
    {
        float denom = DotProduct(n, dir);
        if(denom <= -EPSILON || denom >= EPSILON) {
            vec3_t temp;
            VectorSubtract(pt, start, temp);
            float numer = DotProduct(n, temp);
            float u = numer/denom;
            if(u > 0 && u <= 1.0f) {
                vec3_t vec_scale;
                VectorScale(dir, u, vec_scale);
                VectorAdd(start, vec_scale, ret);
                return true;
            }
        }
        return false;
    }

    // accessors
    const vec3_t& normal() const { return n; }
    float dist() const { return d; }

    vec3_t   n;
    float    d;
};

#endif // __plane_h__

