//
// BSP_tool - botman's Half-Life BSP utilities
//
// (http://planethalflife.com/botman/)
//
// entity.cpp
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

#include <math.h>

#include "cmdlib.h"
#include "bspfile.h"
#include "entity.h"
#include "config.h"

vec3_t spawn_point;
float spawn_point_yaw;

epair_t *pEpair = NULL;

int num_entvars = 0;
entvars_t entvars[MAX_MAP_ENTITIES];

extern Config config;


void LoadEntVars(void)
{
   int ent_index = 0;
   char *value;

   while (ent_index < num_entities)
   {
      value = ValueForKey(&entities[ent_index], "classname");

      if (value[0])
      {
         strcpy(entvars[num_entvars].classname, value);

         // initialize the default entvars fields...
         entvars[num_entvars].origin[0] = 0.0f;
         entvars[num_entvars].origin[1] = 0.0f;
         entvars[num_entvars].origin[2] = 0.0f;

         entvars[num_entvars].angles[0] = 0.0f;
         entvars[num_entvars].angles[1] = 0.0f;
         entvars[num_entvars].angles[2] = 0.0f;

         entvars[num_entvars].rendermode = 0;
         entvars[num_entvars].renderamt = 1.0f;
         entvars[num_entvars].rendercolor[0] = 1.0f;
         entvars[num_entvars].rendercolor[1] = 1.0f;
         entvars[num_entvars].rendercolor[2] = 1.0f;
         entvars[num_entvars].renderfx = 0;

         entvars[num_entvars].brush_model_index = 0;

         entvars[num_entvars].studio_model = NULL;

         value = ValueForKey(&entities[ent_index], "origin");
         if (value[0])
         {
            sscanf(value, "%f %f %f", &entvars[num_entvars].origin[0],
                   &entvars[num_entvars].origin[1],
                   &entvars[num_entvars].origin[2]);
         }

         value = ValueForKey(&entities[ent_index], "angle");
         if (value[0])
         {
            // set the yaw angle...
            sscanf(value, "%f", &entvars[num_entvars].angles[1]);
         }

         value = ValueForKey(&entities[ent_index], "renderamt");
         if (value[0])
         {
            int  n_renderamt;

            sscanf(value, "%d", &n_renderamt);
            entvars[num_entvars].renderamt = n_renderamt / 255.0f;
         }

         value = ValueForKey(&entities[ent_index], "rendercolor");
         if (value[0])
         {
            int  n_color_r, n_color_b, n_color_g;

            sscanf(value, "%d %d %d", &n_color_r, &n_color_g, &n_color_b);
            entvars[num_entvars].rendercolor[0] = n_color_r / 255.0f;
            entvars[num_entvars].rendercolor[1] = n_color_g / 255.0f;
            entvars[num_entvars].rendercolor[2] = n_color_b / 255.0f;
         }

         value = ValueForKey(&entities[ent_index], "model");
         if (value[0])
         {
            if (sscanf(value, "*%d", &entvars[num_entvars].brush_model_index) == 1)
            {
               dmodel_t *model;

               // calculate the origin for this brush model...
               model = &dmodels[entvars[num_entvars].brush_model_index];

               entvars[num_entvars].origin[0] = (model->mins[0] + model->maxs[0]) / 2.0f;
               entvars[num_entvars].origin[1] = (model->mins[1] + model->maxs[1]) / 2.0f;
               entvars[num_entvars].origin[2] = (model->mins[2] + model->maxs[2]) / 2.0f;
            }
         }

         if ((strcmp(entvars[num_entvars].classname, "func_button") == 0) ||
             (strcmp(entvars[num_entvars].classname, "func_door") == 0))
         {
            // always render func_button and func_door entities...
            entvars[num_entvars].renderamt = 255;
         }

         num_entvars++;
      }

      ent_index++;
   }
}


void InitSpawnPoint(void)
{
   int ent_index;
   int count = 0;
   char *value;
   int pick, loop;

   spawn_point[0] = 0.0;
   spawn_point[1] = 0.0;
   spawn_point[2] = 0.0;
   spawn_point_yaw = 0.0;

   if (config.spawnpoint[0] == 0)
      return;  // no spawn points configured, just return

   ent_index = -1;
   // find the number of spawn points...
   while ((ent_index = FindEntityByClassname(ent_index, config.spawnpoint)) != -1)
      count++;

   if (count == 0)
      return;

   if (count > 1)  // is there more than one spawn point?
      pick = rand() % count;
   else
      pick = 0;  // there can be only one!

   loop = 0;
   ent_index = -1;
   while ((ent_index = FindEntityByClassname(ent_index, config.spawnpoint)) != -1)
   {
      if (loop == pick)  // is this the one we want?
      {
         value = ValueForKey(&entities[ent_index], "origin");
         if (value[0])
         {
            sscanf(value, "%f %f %f", &spawn_point[0],
                   &spawn_point[1], &spawn_point[2]);
         }

         value = ValueForKey(&entities[ent_index], "angle");
         if (value[0])
         {
            sscanf(value, "%f", &spawn_point_yaw);
         }

         break;  // break out of loop
      }

      loop++;
   }

   return;
}
