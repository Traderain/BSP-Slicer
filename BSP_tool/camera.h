//
// BSP_tool - botman's Half-Life BSP utilities
//
// (http://planethalflife.com/botman/)
//
// camera.h
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


#ifndef __MATHLIB__
#include "mathlib.h"
#endif

#ifndef __matrix_h__
#include "matrix.h"
#endif

#ifndef CAMERA_H
#define CAMERA_H

class Camera {
public:
    Camera(float x = 0.0f, float y = 0.0f, float z = 0.0f,
           float pitch = 0.0f, float roll = 0.0f, float yaw = 0.0f)
           : m_pitch(pitch), m_roll(roll), m_yaw(yaw)
    {
        m_eye[0] = x, m_eye[1] = y, m_eye[2] = z;             
    }

    void move_delta(float xd, float yd, float zd);
    void move_delta(const vec3_t& v);
    void move(float x, float y, float z);
    void move(const vec3_t& v);
    void rotate_delta(float pitch_delta, float roll_delta, float yaw_delta);
    void rotate(float pitch, float roll, float yaw);
    void rotate_delta(const vec3_t& v);
    void rotate(const vec3_t& v);
    void pitch_bounds();
    void load();
    void transform();
    void right_vector(vec3_t out);
    void up_vector(vec3_t out);
    void fwd_vec(vec3_t out);

    // accessors
    float pitch() const { return m_pitch; }
    float roll() const { return m_roll; }
    float yaw() const { return m_yaw; }

    const float* eye() const { return m_eye; }
    const matrix_t& matrix() const { return cam; }

private:
    float   m_eye[3];
    float   m_pitch;
    float   m_roll;
    float   m_yaw;

    matrix_t cam;
};

#endif

