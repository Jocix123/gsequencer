/* GSequencer - Advanced GTK Sequencer
 * Copyright (C) 2005-2017 Joël Krähemann
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

#ifndef __AGS_HLEVEL_BOX_H__
#define __AGS_HLEVEL_BOX_H__

#include <glib.h>
#include <glib-object.h>

#include <gtk/gtk.h>

#include <ags/widget/ags_level_box.h>

#define AGS_TYPE_HLEVEL_BOX                (ags_hlevel_box_get_type())
#define AGS_HLEVEL_BOX(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_HLEVEL_BOX, AgsHLevelBox))
#define AGS_HLEVEL_BOX_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST((class), AGS_TYPE_HLEVEL_BOX, AgsHLevelBoxClass))
#define AGS_IS_HLEVEL_BOX(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AGS_TYPE_HLEVEL_BOX))
#define AGS_IS_HLEVEL_BOX_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE ((class), AGS_TYPE_HLEVEL_BOX))
#define AGS_HLEVEL_BOX_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS (obj, AGS_TYPE_HLEVEL_BOX, AgsHLevelBoxClass))

typedef struct _AgsHLevelBox AgsHLevelBox;
typedef struct _AgsHLevelBoxClass AgsHLevelBoxClass;

struct _AgsHLevelBox
{
  AgsLevelBox level_box;
};

struct _AgsHLevelBoxClass
{
  AgsLevelBoxClass level_box;
};

GType ags_hlevel_box_get_type(void);

AgsHLevelBox* ags_hlevel_box_new();

#endif /*__AGS_HLEVEL_BOX_H__*/
