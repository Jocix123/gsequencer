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

#ifndef __AGS_LEVEL_H__
#define __AGS_LEVEL_H__

#include <glib.h>
#include <glib-object.h>

#include <gtk/gtk.h>

#define AGS_TYPE_LEVEL                (ags_level_get_type())
#define AGS_LEVEL(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_LEVEL, AgsLevel))
#define AGS_LEVEL_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST((class), AGS_TYPE_LEVEL, AgsLevelClass))
#define AGS_IS_LEVEL(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AGS_TYPE_LEVEL))
#define AGS_IS_LEVEL_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE ((class), AGS_TYPE_LEVEL))
#define AGS_LEVEL_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS (obj, AGS_TYPE_LEVEL, AgsLevelClass))

#define AGS_LEVEL_DEFAULT_LEVEL_WIDTH (60)
#define AGS_LEVEL_DEFAULT_LEVEL_HEIGHT (256)

#define AGS_LEVEL_DEFAULT_LOWER (0.0)
#define AGS_LEVEL_DEFAULT_UPPER (1.0)
#define AGS_LEVEL_DEFAULT_NORMALIZED_VOLUME (0.0)

#define AGS_LEVEL_DEFAULT_STEP_COUNT (0.1)
#define AGS_LEVEL_DEFAULT_PAGE_SIZE (0.25)

#define AGS_LEVEL_DEFAULT_SAMPLERATE (44100)

typedef struct _AgsLevel AgsLevel;
typedef struct _AgsLevelClass AgsLevelClass;

typedef enum{
  AGS_LEVEL_PCM_S8              = 1,
  AGS_LEVEL_PCM_S16             = 1 <<  1,
  AGS_LEVEL_PCM_S24             = 1 <<  2,
  AGS_LEVEL_PCM_S32             = 1 <<  3,
  AGS_LEVEL_PCM_S64             = 1 <<  4,
  AGS_LEVEL_PCM_FLOAT           = 1 <<  5,
  AGS_LEVEL_PCM_DOUBLE          = 1 <<  6,
}AgsLevelFlags;

typedef enum{
  AGS_LEVEL_BUTTON_1_PRESSED     = 1,
}AgsLevelButtonState;

typedef enum{
  AGS_LEVEL_KEY_L_CONTROL       = 1,
  AGS_LEVEL_KEY_R_CONTROL       = 1 <<  1,
  AGS_LEVEL_KEY_L_SHIFT         = 1 <<  2,
  AGS_LEVEL_KEY_R_SHIFT         = 1 <<  3,
}AgsLevelKeyMask;

typedef enum{
  AGS_LEVEL_LAYOUT_VERTICAL,
  AGS_LEVEL_LAYOUT_HORIZONTAL,
}AgsLevelLayout;

typedef enum{
  AGS_LEVEL_STEP_UP,
  AGS_LEVEL_STEP_DOWN,
  AGS_LEVEL_PAGE_UP,
  AGS_LEVEL_PAGE_DOWN,
}AgsLevelAction;

struct _AgsLevel
{
  GtkWidget widget;

  guint flags;

  guint key_mask;
  guint button_state;
  guint layout;

  guint font_size;

  guint level_width;
  guint level_height;

  gdouble lower;
  gdouble upper;

  gdouble normalized_volume;

  guint step_count;
  gdouble page_size;

  guint samplerate;
  
  guint *audio_channel;
  guint audio_channel_count;
};

struct _AgsLevelClass
{
  GtkWidgetClass widget;

  void (*value_changed)(AgsLevel *level,
			gdouble normalized_volume);
};

GType ags_level_get_type(void);

/* properties get/set */
void ags_level_set_level_width(AgsLevel *level,
			       guint level_width);
guint ags_level_get_level_width(AgsLevel *level);

void ags_level_set_level_height(AgsLevel *level,
				guint level_height);
guint ags_level_get_level_height(AgsLevel *level);

void ags_level_set_upper(AgsLevel *level,
			 gdouble upper);
gdouble ags_level_get_upper(AgsLevel *level);

void ags_level_set_lower(AgsLevel *level,
			 gdouble lower);
gdouble ags_level_get_lower(AgsLevel *level);

void ags_level_set_normalized_volume(AgsLevel *level,
				     gdouble normalized_volume);
gdouble ags_level_get_normalized_volume(AgsLevel *level);

/* signal */
void ags_level_value_changed(AgsLevel *level,
			     gdouble normalized_volume);

/* instantiate */
AgsLevel* ags_level_new();

#endif /*__AGS_LEVEL_H__*/
