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

#ifndef __AGS_START_SOUNDCARD_H__
#define __AGS_START_SOUNDCARD_H__

#include <glib.h>
#include <glib-object.h>

#include <ags/libags.h>

#define AGS_TYPE_START_SOUNDCARD                (ags_start_soundcard_get_type())
#define AGS_START_SOUNDCARD(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_START_SOUNDCARD, AgsStartSoundcard))
#define AGS_START_SOUNDCARD_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST((class), AGS_TYPE_START_SOUNDCARD, AgsStartSoundcardClass))
#define AGS_IS_START_SOUNDCARD(obj)             (G_TYPE_CHECK_INSTANCE_TYPE((obj), AGS_TYPE_START_SOUNDCARD))
#define AGS_IS_START_SOUNDCARD_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE((class), AGS_TYPE_START_SOUNDCARD))
#define AGS_START_SOUNDCARD_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS((obj), AGS_TYPE_START_SOUNDCARD, AgsStartSoundcardClass))

typedef struct _AgsStartSoundcard AgsStartSoundcard;
typedef struct _AgsStartSoundcardClass AgsStartSoundcardClass;

struct _AgsStartSoundcard
{
  AgsTask task;

  AgsApplicationContext *application_context;
};

struct _AgsStartSoundcardClass
{
  AgsTaskClass task;
};

GType ags_start_soundcard_get_type();

AgsStartSoundcard* ags_start_soundcard_new(AgsApplicationContext *application_context);

#endif /*__AGS_START_SOUNDCARD_H__*/

