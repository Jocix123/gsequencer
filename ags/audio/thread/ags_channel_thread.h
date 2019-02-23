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

#ifndef __AGS_CHANNEL_THREAD_H__
#define __AGS_CHANNEL_THREAD_H__

#include <glib.h>
#include <glib-object.h>

#include <ags/libags.h>

#define AGS_TYPE_CHANNEL_THREAD                (ags_channel_thread_get_type())
#define AGS_CHANNEL_THREAD(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_CHANNEL_THREAD, AgsChannelThread))
#define AGS_CHANNEL_THREAD_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST(class, AGS_TYPE_CHANNEL_THREAD, AgsChannelThreadClass))
#define AGS_IS_CHANNEL_THREAD(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AGS_TYPE_CHANNEL_THREAD))
#define AGS_IS_CHANNEL_THREAD_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE ((class), AGS_TYPE_CHANNEL_THREAD))
#define AGS_CHANNEL_THREAD_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS(obj, AGS_TYPE_CHANNEL_THREAD, AgsChannelThreadClass))

#define AGS_CHANNEL_THREAD_DEFAULT_JIFFIE (ceil(AGS_SOUNDCARD_DEFAULT_SAMPLERATE / AGS_SOUNDCARD_DEFAULT_BUFFER_SIZE) + AGS_SOUNDCARD_DEFAULT_OVERCLOCK)

typedef struct _AgsChannelThread AgsChannelThread;
typedef struct _AgsChannelThreadClass AgsChannelThreadClass;

/**
 * AgsChannelThreadFlags:
 * @AGS_CHANNEL_THREAD_DONE: sync done parent thread, initial wait during #AgsThread::run()
 * @AGS_CHANNEL_THREAD_WAIT: sync wait parent thread, initial wait during #AgsThread::run()
 * @AGS_CHANNEL_THREAD_DONE_SYNC: sync done parent thread, signal completed during #AgsThread::run()
 * @AGS_CHANNEL_THREAD_WAIT_SYNC: sync wait parent thread, signal completed during #AgsThread::run()
 *
 * Enum values to control the behavior or indicate internal state of #AgsChannelThread by
 * enable/disable as flags.
 */
typedef enum{
  AGS_CHANNEL_THREAD_DONE            = 1,
  AGS_CHANNEL_THREAD_WAIT            = 1 <<  1,
  AGS_CHANNEL_THREAD_DONE_SYNC       = 1 <<  2,
  AGS_CHANNEL_THREAD_WAIT_SYNC       = 1 <<  3,
}AgsChannelThreadFlags;

struct _AgsChannelThread
{
  AgsThread thread;

  volatile guint flags;

  GObject *default_output_soundcard;
    
  pthread_mutexattr_t wakeup_attr;
  pthread_mutex_t *wakeup_mutex;
  pthread_cond_t *wakeup_cond;

  pthread_mutexattr_t done_attr;
  pthread_mutex_t *done_mutex;
  pthread_cond_t *done_cond;

  GObject *channel;
  gint sound_scope;
};

struct _AgsChannelThreadClass
{
  AgsThreadClass thread;
};

GType ags_channel_thread_get_type();

void ags_channel_thread_set_sound_scope(AgsChannelThread *channel_thread,
					gint sound_scope);

AgsChannelThread* ags_channel_thread_new(GObject *default_output_soundcard,
					 GObject *channel);

#endif /*__AGS_CHANNEL_THREAD_H__*/
