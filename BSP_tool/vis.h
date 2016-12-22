//
// BSP_tool - botman's Half-Life BSP utilities
//
// (http://planethalflife.com/botman/)
//
// vis.h
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

#ifndef VIS_H
#define VIS_H

typedef struct
{
   int num_leafs;  // number of leafs in this visibility set
   int *leafs;     // pointer to list of leafs in this visibility set
} vis_leaf_t;

class Visibility
{
   public:

   vis_leaf_t *leaf_visibility;  // pointer to array of vis_leaf_t structures

   Visibility();
   ~Visibility();

   void Decompress_VIS(void);
   vis_leaf_t &VisLeaf(int index);

};

#endif

