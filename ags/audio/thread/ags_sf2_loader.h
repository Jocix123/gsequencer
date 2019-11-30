/* GSequencer - Advanced GTK Sequencer
 * Copyright (C) 2005-2019 Joël Krähemann
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

#ifndef __AGS_SF2_LOADER_H__
#define __AGS_SF2_LOADER_H__

#include <glib.h>
#include <glib-object.h>

#include <ags/libags.h>

#include <ags/audio/ags_audio.h>

#include <ags/audio/file/ags_audio_container.h>

#define AGS_TYPE_SF2_LOADER                (ags_sf2_loader_get_type())
#define AGS_SF2_LOADER(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_SF2_LOADER, AgsSF2Loader))
#define AGS_SF2_LOADER_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST((class), AGS_TYPE_SF2_LOADER, AgsSF2LoaderClass))
#define AGS_IS_SF2_LOADER(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AGS_TYPE_SF2_LOADER))
#define AGS_IS_SF2_LOADER_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE ((class), AGS_TYPE_SF2_LOADER))
#define AGS_SF2_LOADER_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), AGS_TYPE_SF2_LOADER, AgsSF2LoaderClass))

#define AGS_SF2_LOADER_GET_OBJ_MUTEX(obj) (((AgsSF2Loader *) obj)->obj_mutex)

typedef struct _AgsSF2Loader AgsSF2Loader;
typedef struct _AgsSF2LoaderClass AgsSF2LoaderClass;

typedef enum{
  AGS_SF2_LOADER_HAS_COMPLETED   = 1,
}AgsSF2LoaderFlags;

struct _AgsSF2Loader
{
  GObject gobject;

  guint flags;

  pthread_mutex_t *obj_mutex;
  pthread_mutexattr_t *obj_mutexattr;

  pthread_t *thread;

  AgsAudio *audio;

  gchar *filename;

  gchar *preset;
  gchar *instrument;
  
  AgsAudioContainer *audio_container;
};

struct _AgsSF2LoaderClass
{
  GObjectClass gobject;
};

GType ags_sf2_loader_get_type();

pthread_mutex_t* ags_sf2_loader_get_class_mutex();

gboolean ags_sf2_loader_test_flags(AgsSF2Loader *sf2_loader, guint flags);
void ags_sf2_loader_set_flags(AgsSF2Loader *sf2_loader, guint flags);
void ags_sf2_loader_unset_flags(AgsSF2Loader *sf2_loader, guint flags);

void ags_sf2_loader_start(AgsSF2Loader *sf2_loader);

AgsSF2Loader* ags_sf2_loader_new(AgsAudio *audio,
				 gchar *filename,
				 gchar *preset,
				 gchar *instrument);

#endif /*__AGS_SF2_LOADER_H__*/