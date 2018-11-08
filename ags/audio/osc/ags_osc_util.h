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

#ifndef __AGS_OSC_UTIL_H__
#define __AGS_OSC_UTIL_H__

#include <glib.h>
#include <glib-object.h>

#define AGS_OSC_UTIL_TYPE_TAG_STRING_INT32 'i'
#define AGS_OSC_UTIL_TYPE_TAG_STRING_FLOAT 'f'
#define AGS_OSC_UTIL_TYPE_TAG_STRING_STRING 's'
#define AGS_OSC_UTIL_TYPE_TAG_STRING_BLOB 'b'

#define AGS_OSC_UTIL_TYPE_TAG_STRING_INT64 'h'
#define AGS_OSC_UTIL_TYPE_TAG_STRING_TIMETAG 't'
#define AGS_OSC_UTIL_TYPE_TAG_STRING_DOUBLE 'd'
#define AGS_OSC_UTIL_TYPE_TAG_STRING_SYMBOL 'S'
#define AGS_OSC_UTIL_TYPE_TAG_STRING_CHAR 'c'
#define AGS_OSC_UTIL_TYPE_TAG_STRING_RGBA 'r'
#define AGS_OSC_UTIL_TYPE_TAG_STRING_MIDI 'm'
#define AGS_OSC_UTIL_TYPE_TAG_STRING_TRUE 'T'
#define AGS_OSC_UTIL_TYPE_TAG_STRING_FALSE 'F'
#define AGS_OSC_UTIL_TYPE_TAG_STRING_NIL 'N'
#define AGS_OSC_UTIL_TYPE_TAG_STRING_INFINITE 'I'
#define AGS_OSC_UTIL_TYPE_TAG_STRING_ARRAY_START '['
#define AGS_OSC_UTIL_TYPE_TAG_STRING_ARRAY_END ']'

#define AGS_OSC_UTIL_ADDRESS_PATTERN_VALID_CHARS_PATTERN "([a-zA-Z \\#\\*\\,\\/\\?\\[\\]\\{\\}]+)"

guint ags_osc_util_type_tag_string_count_type(gchar *type_tag_string);

#endif /*__AGS_OSC_UTIL_H__*/