//
// BSP_tool - botman's Half-Life BSP utilities
//
// (http://planethalflife.com/botman/)
//
// camera.cpp
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

#ifdef WIN32
#include <windows.h>
#endif

#include <memory.h>
#include <GL/gl.h>

#include "cmdlib.h"
#include "camera.h"

void Camera::move_delta(float xd, float yd, float zd)
{
   if ((m_eye[0] + xd > -4096.0f) && (m_eye[0] + xd < 4096.0f) &&
       (m_eye[1] + yd > -4096.0f) && (m_eye[1] + yd < 4096.0f) &&
       (m_eye[2] + zd > -4096.0f) && (m_eye[2] + zd < 4096.0f))
      m_eye[0]+=xd, m_eye[1]+=yd, m_eye[2]+=zd;
   else
      Error("camera position out of range!\n");
}

void Camera::move_delta(const vec3_t& v)
{
   if ((m_eye[0] + v[0] > -4096.0f) && (m_eye[0] + v[0] < 4096.0f) &&
       (m_eye[1] + v[1] > -4096.0f) && (m_eye[1] + v[1] < 4096.0f) &&
       (m_eye[2] + v[2] > -4096.0f) && (m_eye[2] + v[2] < 4096.0f))
      m_eye[0]+=v[0], m_eye[1]+=v[1], m_eye[2]+=v[2];
   else
      Error("camera position out of range!\n");
}

void Camera::move(float x, float y, float z)
{
   if ((x > -4096.0f) && (x < 4096.0f) &&
       (y > -4096.0f) && (y < 4096.0f) &&
       (z > -4096.0f) && (z < 4096.0f))
      m_eye[0]=x, m_eye[1]=y, m_eye[2]=z;
   else
      Error("camera position out of range!\n");
}

void Camera::move(const vec3_t& v)
{
   if ((v[0] > -4096.0f) && (v[0] < 4096.0f) &&
       (v[1] > -4096.0f) && (v[1] < 4096.0f) &&
       (v[2] > -4096.0f) && (v[2] < 4096.0f))
      m_eye[0]=v[0], m_eye[1]=v[1], m_eye[2]=v[2];
   else
      Error("camera position out of range!\n");
}

void Camera::rotate_delta(float pitch_delta, float roll_delta, float yaw_delta)
{
   m_pitch += pitch_delta;
   m_roll += roll_delta;
   m_yaw += yaw_delta;
   pitch_bounds();
}

void Camera::rotate(float pitch, float roll, float yaw)
{
   m_pitch = pitch;
   m_roll = roll;
   m_yaw = yaw;
   pitch_bounds();
}

void Camera::rotate_delta(const vec3_t& v)
{
   m_pitch += v[0];
   m_roll += v[1];
   m_yaw += v[2];
   pitch_bounds();
}

void Camera::rotate(const vec3_t& v)
{
   m_pitch = v[0];
   m_roll = v[1];
   m_yaw = v[2];
   pitch_bounds();
}

void Camera::pitch_bounds()
{
   if (m_pitch > 0.0f)
      m_pitch=0.0f;
   else if (m_pitch < -180.f)
      m_pitch=-180.0f;
}

void Camera::load()
{
   glLoadMatrixf(cam);
}

void Camera::transform()
{
   cam.load_identity();
   matrix_t r;
   r.xrot(m_pitch);
   cam*=r;

   r.load_identity();
   r.yrot(m_roll);
   cam*=r;

   r.load_identity();
   r.zrot(m_yaw);
   cam*=r;

   cam.translate(-m_eye[0], -m_eye[1], -m_eye[2]);
}

void Camera::right_vector(vec3_t out)
{
   out[0] = cam[0];  out[1] = cam[4];  out[2] = cam[8];
}

void Camera::up_vector(vec3_t out)
{
   out[0] = cam[1];  out[1] = cam[5];  out[2] = cam[9];
}

void Camera::fwd_vec(vec3_t out)
{
   out[0] = cam[2];  out[1] = cam[6];  out[2] = cam[10];
}
