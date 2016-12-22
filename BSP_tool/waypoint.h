//
// BSP_tool - botman's Half-Life BSP utilities
//
// (http://planethalflife.com/botman/)
//
// waypoint.h
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

#ifndef WAYPOINT_H
#define WAYPOINT_H


// the HPB bot can handle a maximum of 1024 waypoints
#define MAX_WAYPOINTS 1024

// defines for waypoint flags field (32 bits are available) - from the HPB bot

#define W_FL_TEAM        ((1<<0) + (1<<1))  /* allow for 4 teams (0-3) */
#define W_FL_TEAM_SPECIFIC (1<<2)  /* waypoint only for specified team */
#define W_FL_CROUCH      (1<<3)  /* must crouch to reach this waypoint */
#define W_FL_LADDER      (1<<4)  /* waypoint on a ladder */
#define W_FL_LIFT        (1<<5)  /* wait for lift to be down before approaching this waypoint */
#define W_FL_DOOR        (1<<6)  /* wait for door to open */
#define W_FL_HEALTH      (1<<7)  /* health kit (or wall mounted) location */
#define W_FL_ARMOR       (1<<8)  /* armor (or HEV) location */
#define W_FL_AMMO        (1<<9)  /* ammo location */
#define W_FL_SNIPER      (1<<10) /* sniper waypoint (a good sniper spot) */

#define W_FL_FLAG        (1<<11) /* flag position (or hostage or president) */
#define W_FL_FLF_CAP     (1<<11) /* Front Line Force capture point */

#define W_FL_FLAG_GOAL   (1<<12) /* flag return position (or rescue zone) */
#define W_FL_FLF_DEFEND  (1<<12) /* Front Line Force defend point */

#define W_FL_PRONE       (1<<13) /* go prone (laying down) */
#define W_FL_AIMING      (1<<14) /* aiming waypoint */

#define W_FL_SENTRYGUN   (1<<15) /* sentry gun waypoint for TFC */
#define W_FL_DISPENSER   (1<<16) /* dispenser waypoint for TFC */

#define W_FL_WEAPON      (1<<17) /* weapon_ entity location */
#define W_FL_JUMP        (1<<18) /* jump waypoint */

#define W_FL_DELETED     (1<<31) /* used by waypoint allocation code */


#define WAYPOINT_VERSION 4

// define the waypoint file header structure...
typedef struct {
   char filetype[8];  // should be "HPB_bot\0"
   int  waypoint_file_version;
   int  waypoint_file_flags;  // not currently used
   int  number_of_waypoints;
   char mapname[32];  // name of map for these waypoints
} WAYPOINT_HDR;


// define the structure for waypoints...
typedef struct {
   int    flags;    // button, lift, flag, health, ammo, etc.
   vec3_t origin;   // location
} WAYPOINT;


bool CheckWaypoint(const vec3_t &coord, vec3_t new_coord);
void RecursiveFloodFill(const vec3_t &coord);
void WaypointAddEntities(char *item_name, int waypoint_flags);
void WaypointAddWallMountedEntities(char *item_name, int waypoint_flags);
void WaypointAddLadders(int waypoint_flags);
void WaypointLevel(int grid_size);
void WriteHPBWaypointFile(void);

#endif

