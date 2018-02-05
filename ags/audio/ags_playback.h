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

#ifndef __AGS_PLAYBACK_H__
#define __AGS_PLAYBACK_H__

#include <glib.h>
#include <glib-object.h>

#include <ags/libags.h>

#include <ags/audio/ags_recall_id.h>

#define AGS_TYPE_PLAYBACK                (ags_playback_get_type())
#define AGS_PLAYBACK(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_PLAYBACK, AgsPlayback))
#define AGS_PLAYBACK_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST(class, AGS_TYPE_PLAYBACK, AgsPlayback))
#define AGS_IS_PLAYBACK(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AGS_TYPE_PLAYBACK))
#define AGS_IS_PLAYBACK_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE ((class), AGS_TYPE_PLAYBACK))
#define AGS_PLAYBACK_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS(obj, AGS_TYPE_PLAYBACK, AgsPlaybackClass))

typedef struct _AgsPlayback AgsPlayback;
typedef struct _AgsPlaybackClass AgsPlaybackClass;

/**
 * AgsPlaybackFlags:
 * @AGS_PLAYBACK_CONNECTED: indicates the playback was connected by calling #AgsConnectable::connect()
 * @AGS_PLAYBACK_SINGLE_THREADED: single threaded
 * @AGS_PLAYBACK_SUPER_THREADED_CHANNEL: super threaded channel
 *
 * Enum values to control the behavior or indicate internal state of #AgsPlayback by
 * enable/disable as flags.
 */
typedef enum{
  AGS_PLAYBACK_CONNECTED                    = 1,
  AGS_PLAYBACK_SINGLE_THREADED              = 1 <<  1,
  AGS_PLAYBACK_SUPER_THREADED_CHANNEL       = 1 <<  2,
}AgsPlaybackFlags;

struct _AgsPlayback
{
  GObject gobject;
  
  volatile guint flags;

  GObject *playback_domain;
  
  GObject *source;
  guint audio_channel;

  GObject *play_note;
  
  AgsThread *channel_thread[AGS_SOUND_SCOPE_LAST];
  
  AgsRecallID *recall_id[AGS_SOUND_SCOPE_LAST];
};

struct _AgsPlaybackClass
{
  GObjectClass gobject;
};

GType ags_playback_get_type();

/* get and set */
void ags_playback_set_channel_thread(AgsPlayback *playback,
				     AgsThread *thread,
				     guint scope);
AgsThread* ags_playback_get_channel_thread(AgsPlayback *playback,
					   guint scope);

void ags_playback_set_recall_id(AgsPlayback *playback,
				AgsRecallID *recall_id,
				guint scope);
AgsRecallID* ags_playback_get_recall_id(AgsPlayback *playback,
					guint scope);

/* find */
AgsPlayback* ags_playback_find_source(GList *playback,
				      GObject *source);

/* instance */
AgsPlayback* ags_playback_new();

#endif /*__AGS_PLAYBACK_H__*/
