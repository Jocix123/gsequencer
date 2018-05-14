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

#ifndef __AGS_JACK_DEVOUT_H__
#define __AGS_JACK_DEVOUT_H__

#include <glib.h>
#include <glib-object.h>

#include <pthread.h>

#include <ags/libags.h>

#define AGS_TYPE_JACK_DEVOUT                (ags_jack_devout_get_type())
#define AGS_JACK_DEVOUT(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_JACK_DEVOUT, AgsJackDevout))
#define AGS_JACK_DEVOUT_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST(class, AGS_TYPE_JACK_DEVOUT, AgsJackDevout))
#define AGS_IS_JACK_DEVOUT(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AGS_TYPE_JACK_DEVOUT))
#define AGS_IS_JACK_DEVOUT_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE ((class), AGS_TYPE_JACK_DEVOUT))
#define AGS_JACK_DEVOUT_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS(obj, AGS_TYPE_JACK_DEVOUT, AgsJackDevoutClass))

typedef struct _AgsJackDevout AgsJackDevout;
typedef struct _AgsJackDevoutClass AgsJackDevoutClass;

/**
 * AgsJackDevoutFlags:
 * @AGS_JACK_DEVOUT_ADDED_TO_REGISTRY: the JACK devout was added to registry, see #AgsConnectable::add_to_registry()
 * @AGS_JACK_DEVOUT_CONNECTED: indicates the JACK devout was connected by calling #AgsConnectable::connect()
 * @AGS_JACK_DEVOUT_BUFFER0: ring-buffer 0
 * @AGS_JACK_DEVOUT_BUFFER1: ring-buffer 1
 * @AGS_JACK_DEVOUT_BUFFER2: ring-buffer 2
 * @AGS_JACK_DEVOUT_BUFFER3: ring-buffer 3
 * @AGS_JACK_DEVOUT_ATTACK_FIRST: use first attack, instead of second one
 * @AGS_JACK_DEVOUT_PLAY: do playback
 * @AGS_JACK_DEVOUT_SHUTDOWN: stop playback
 * @AGS_JACK_DEVOUT_START_PLAY: playback starting
 * @AGS_JACK_DEVOUT_NONBLOCKING: do non-blocking calls
 * @AGS_JACK_DEVOUT_INITIALIZED: the soundcard was initialized
 *
 * Enum values to control the behavior or indicate internal state of #AgsJackDevout by
 * enable/disable as flags.
 */

typedef enum{
  AGS_JACK_DEVOUT_ADDED_TO_REGISTRY              = 1,
  AGS_JACK_DEVOUT_CONNECTED                      = 1 <<  1,

  AGS_JACK_DEVOUT_BUFFER0                        = 1 <<  2,
  AGS_JACK_DEVOUT_BUFFER1                        = 1 <<  3,
  AGS_JACK_DEVOUT_BUFFER2                        = 1 <<  4,
  AGS_JACK_DEVOUT_BUFFER3                        = 1 <<  5,

  AGS_JACK_DEVOUT_ATTACK_FIRST                   = 1 <<  6,

  AGS_JACK_DEVOUT_PLAY                           = 1 <<  7,
  AGS_JACK_DEVOUT_SHUTDOWN                       = 1 <<  8,
  AGS_JACK_DEVOUT_START_PLAY                     = 1 <<  9,

  AGS_JACK_DEVOUT_NONBLOCKING                    = 1 << 10,
  AGS_JACK_DEVOUT_INITIALIZED                    = 1 << 11,
}AgsJackDevoutFlags;

/**
 * AgsJackDevoutSyncFlags:
 * @AGS_JACK_DEVOUT_PASS_THROUGH: do not sync
 * @AGS_JACK_DEVOUT_INITIAL_CALLBACK: initial callback
 * @AGS_JACK_DEVOUT_CALLBACK_WAIT: sync wait, soundcard conditional lock
 * @AGS_JACK_DEVOUT_CALLBACK_DONE: sync done, soundcard conditional lock
 * @AGS_JACK_DEVOUT_CALLBACK_FINISH_WAIT: sync wait, client conditional lock
 * @AGS_JACK_DEVOUT_CALLBACK_FINISH_DONE: sync done, client conditional lock
 * 
 * Enum values to control the synchronization between soundcard and client.
 */
typedef enum{
  AGS_JACK_DEVOUT_PASS_THROUGH                   = 1,
  AGS_JACK_DEVOUT_INITIAL_CALLBACK               = 1 <<  1,
  AGS_JACK_DEVOUT_CALLBACK_WAIT                  = 1 <<  2,
  AGS_JACK_DEVOUT_CALLBACK_DONE                  = 1 <<  3,
  AGS_JACK_DEVOUT_CALLBACK_FINISH_WAIT           = 1 <<  4,
  AGS_JACK_DEVOUT_CALLBACK_FINISH_DONE           = 1 <<  5,
}AgsJackDevoutSyncFlags;

#define AGS_JACK_DEVOUT_ERROR (ags_jack_devout_error_quark())

typedef enum{
  AGS_JACK_DEVOUT_ERROR_LOCKED_SOUNDCARD,
}AgsJackDevoutError;

struct _AgsJackDevout
{
  GObject object;

  guint flags;
  volatile guint sync_flags;
  
  pthread_mutex_t *obj_mutex;
  pthread_mutexattr_t *obj_mutexattr;

  AgsApplicationContext *application_context;

  AgsUUID *uuid;

  guint dsp_channels;
  guint pcm_channels;
  guint format;
  guint buffer_size;
  guint samplerate;

  void** buffer;

  double bpm; // beats per minute
  gdouble delay_factor;
  
  gdouble *delay; // count of tics within buffer size
  guint *attack; // where currently tic resides in the stream's offset, measured in 1/64 of bpm

  gdouble tact_counter;
  gdouble delay_counter; // next time attack changeing when delay_counter == delay
  guint tic_counter; // in the range of default period

  guint note_offset;
  guint note_offset_absolute;
  
  guint loop_left;
  guint loop_right;
  gboolean do_loop;
  
  guint loop_offset;

  gchar *card_uri;
  GObject *jack_client;

  gchar **port_name;
  GList *jack_port;

  pthread_mutex_t *callback_mutex;
  pthread_cond_t *callback_cond;

  pthread_mutex_t *callback_finish_mutex;
  pthread_cond_t *callback_finish_cond;

  GObject *notify_soundcard;
};

struct _AgsJackDevoutClass
{
  GObjectClass object;
};

GType ags_jack_devout_get_type();

GQuark ags_jack_devout_error_quark();

pthread_mutex_t* ags_jack_devout_get_class_mutex();

gboolean ags_jack_devout_test_flags(AgsJackDevout *jack_devout, guint flags);
void ags_jack_devout_set_flags(AgsJackDevout *jack_devout, guint flags);
void ags_jack_devout_unset_flags(AgsJackDevout *jack_devout, guint flags);

void ags_jack_devout_switch_buffer_flag(AgsJackDevout *jack_devout);

void ags_jack_devout_adjust_delay_and_attack(AgsJackDevout *jack_devout);
void ags_jack_devout_realloc_buffer(AgsJackDevout *jack_devout);

AgsJackDevout* ags_jack_devout_new(GObject *application_context);

#endif /*__AGS_JACK_DEVOUT_H__*/
