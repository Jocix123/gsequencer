/* GSequencer - Advanced GTK Sequencer
 * Copyright (C) 2005-2020 Joël Krähemann
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

#ifndef __AGS_MATH_UTIL__
#define __AGS_MATH_UTIL__

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

void ags_math_util_find_parantheses_all(gchar *str,
					gint **open_position, gint **close_position,
					guint *open_position_count, guint *close_position_count);
void ags_math_util_find_exponent_parantheses(gchar *str,
					     gint **exponent_open_position, gint **exponent_close_position,
					     guint *exponent_open_position_count, guint *exponent_close_position_count);
void ags_math_util_find_function_parantheses(gchar *str,
					     gint **function_open_position, gint **function_close_position,
					     guint *function_open_position_count, guint *function_close_position_count);
void ags_math_util_find_term_parantheses(gchar *str,
					 gint **term_open_position, gint **term_close_position,
					 guint *term_open_position_count, guint *term_close_position_count);

gchar** ags_math_util_find_literals(gchar *str,
				    guint *symbol_count);

gboolean ags_math_util_is_term(gchar *term);

void ags_math_util_lookup_exponent(gchar *equation_str,
				   gchar *exponent,
				   gint **exponent_position, guint *exponent_position_count);
void ags_math_util_lookup_function(gchar *equation_str,
				   gchar *function,
				   gint **function_position, guint *function_position_count);
void ags_math_util_lookup_term(gchar *equation_str,
			       gchar *term,
			       gint **term_position, guint *term_position_count);

G_END_DECLS

#endif /*__AGS_MATH_UTIL__*/