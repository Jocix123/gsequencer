/* GSequencer - Advanced GTK Sequencer
 * Copyright (C) 2005-2015 Joël Krähemann
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

#ifndef __AGS_SET_FORMAT_H__
#define __AGS_SET_FORMAT_H__

#include <glib.h>
#include <glib-object.h>

#include <ags/thread/ags_task.h>

#define AGS_TYPE_SET_FORMAT                (ags_set_format_get_type())
#define AGS_SET_FORMAT(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_SET_FORMAT, AgsSetFormat))
#define AGS_SET_FORMAT_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST((class), AGS_TYPE_SET_FORMAT, AgsSetFormatClass))
#define AGS_IS_SET_FORMAT(obj)             (G_TYPE_CHECK_INSTANCE_TYPE((obj), AGS_TYPE_SET_FORMAT))
#define AGS_IS_SET_FORMAT_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE((class), AGS_TYPE_SET_FORMAT))
#define AGS_SET_FORMAT_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS((obj), AGS_TYPE_SET_FORMAT, AgsSetFormatClass))

typedef struct _AgsSetFormat AgsSetFormat;
typedef struct _AgsSetFormatClass AgsSetFormatClass;

struct _AgsSetFormat
{
  AgsTask task;

  GObject *gobject;
  guint format;
};

struct _AgsSetFormatClass
{
  AgsTaskClass task;
};

GType ags_set_format_get_type();

AgsSetFormat* ags_set_format_new(GObject *gobject,
				     guint format);

#endif /*__AGS_SET_FORMAT_H__*/
