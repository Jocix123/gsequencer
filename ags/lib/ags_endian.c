/* GSequencer - Advanced GTK Sequencer
 * Copyright (C) 2005-2018 Joël Krähemann
 *
 * This file is part of GSequencer.
 *
 * GSequencer is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GSequencer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GSequencer.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <ags/lib/ags_endian.h>

/**
 * SECTION:ags_endian
 * @short_description: dealing with endiannes
 * @title: AgsEndian
 * @section_id:
 * @include: ags/lib/ags_endian.h
 *
 * Some common routines dealing with endiannes.
 */

/**
 * ags_endian_host_is_le:
 * 
 * Check host is Little Endian.
 * 
 * Returns: %TRUE if LE, otherwise %FALSE
 * 
 * Since: 2.0.0
 */
gboolean
ags_endian_host_is_le()
{
  int i = 1;
  
  return(*((char *)&i));
}

/**
 * ags_endian_host_is_be:
 * 
 * Check host is Big Endian.
 * 
 * Returns: %TRUE if BE, otherwise %FALSE
 * 
 * Since: 2.0.0
 */
gboolean
ags_endian_host_is_be()
{
  int i = 1;
  
  return(!(*((char *)&i)));
}

/**
 * ags_endian_swap_float:
 * @x: the float to swap
 * 
 * Swaps the floating point numbers endianness.
 *
 * Returns: the byte-swapped float
 * 
 * Since: 2.0.0
 */
gfloat
ags_endian_swap_float(gfloat x)
{
  char c;
  union{
    gfloat float_data;
    char char_data[4];
  }data;

  data.float_data = x;

  c = data.char_data[0];  
  data.char_data[0] = data.char_data[3];
  data.char_data[3] = c;

  c = data.char_data[1];
  data.char_data[1] = data.char_data[2];
  data.char_data[2] = c;
  
  return(data.float_data);
}

