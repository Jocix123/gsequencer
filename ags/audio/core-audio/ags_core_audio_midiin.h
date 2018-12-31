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

#ifndef __AGS_CORE_AUDIO_MIDIIN_H__
#define __AGS_CORE_AUDIO_MIDIIN_H__

#include <glib.h>
#include <glib-object.h>

#include <pthread.h>

#include <ags/libags.h>

#define AGS_TYPE_CORE_AUDIO_MIDIIN                (ags_core_audio_midiin_get_type())
#define AGS_CORE_AUDIO_MIDIIN(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_CORE_AUDIO_MIDIIN, AgsCoreAudioMidiin))
#define AGS_CORE_AUDIO_MIDIIN_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST(class, AGS_TYPE_CORE_AUDIO_MIDIIN, AgsCoreAudioMidiin))
#define AGS_IS_CORE_AUDIO_MIDIIN(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AGS_TYPE_CORE_AUDIO_MIDIIN))
#define AGS_IS_CORE_AUDIO_MIDIIN_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE ((class), AGS_TYPE_CORE_AUDIO_MIDIIN))
#define AGS_CORE_AUDIO_MIDIIN_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS(obj, AGS_TYPE_CORE_AUDIO_MIDIIN, AgsCoreAudioMidiinClass))

#define AGS_CORE_AUDIO_MIDIIN_DEFAULT_BUFFER_SIZE (256)

typedef struct _AgsCoreAudioMidiin AgsCoreAudioMidiin;
typedef struct _AgsCoreAudioMidiinClass AgsCoreAudioMidiinClass;

/**
 * AgsCoreAudioMidiinFlags:
 * @AGS_CORE_AUDIO_MIDIIN_ADDED_TO_REGISTRY: the CoreAudio midiin was added to registry, see #AgsConnectable::add_to_registry()
 * @AGS_CORE_AUDIO_MIDIIN_CONNECTED: indicates the CoreAudio midiin was connected by calling #AgsConnectable::connect()
 * @AGS_CORE_AUDIO_MIDIIN_BUFFER0: ring-buffer 0
 * @AGS_CORE_AUDIO_MIDIIN_BUFFER1: ring-buffer 1
 * @AGS_CORE_AUDIO_MIDIIN_BUFFER2: ring-buffer 2
 * @AGS_CORE_AUDIO_MIDIIN_BUFFER3: ring-buffer 3
 * @AGS_CORE_AUDIO_MIDIIN_ATTACK_FIRST: use first attack, instead of second one
 * @AGS_CORE_AUDIO_MIDIIN_RECORD: is recording
 * @AGS_CORE_AUDIO_MIDIIN_SHUTDOWN: stop recording
 * @AGS_CORE_AUDIO_MIDIIN_START_RECORD: just started recording
 * @AGS_CORE_AUDIO_MIDIIN_NONBLOCKING: do non-blocking calls
 * @AGS_CORE_AUDIO_MIDIIN_INITIALIZED: recording is initialized
 *
 * Enum values to control the behavior or indicate internal state of #AgsCoreAudioMidiin by
 * enable/disable as flags.
 */
typedef enum
{
  AGS_CORE_AUDIO_MIDIIN_ADDED_TO_REGISTRY  = 1,
  AGS_CORE_AUDIO_MIDIIN_CONNECTED          = 1 <<  1,

  AGS_CORE_AUDIO_MIDIIN_BUFFER0            = 1 <<  2,
  AGS_CORE_AUDIO_MIDIIN_BUFFER1            = 1 <<  3,
  AGS_CORE_AUDIO_MIDIIN_BUFFER2            = 1 <<  4,
  AGS_CORE_AUDIO_MIDIIN_BUFFER3            = 1 <<  5,

  AGS_CORE_AUDIO_MIDIIN_ATTACK_FIRST       = 1 <<  6,

  AGS_CORE_AUDIO_MIDIIN_RECORD             = 1 <<  7,

  AGS_CORE_AUDIO_MIDIIN_SHUTDOWN           = 1 <<  8,
  AGS_CORE_AUDIO_MIDIIN_START_RECORD       = 1 <<  9,

  AGS_CORE_AUDIO_MIDIIN_NONBLOCKING        = 1 << 10,
  AGS_CORE_AUDIO_MIDIIN_INITIALIZED        = 1 << 11,
}AgsCoreAudioMidiinFlags;

/**
 * AgsCoreAudioMidiinSyncFlags:
 * @AGS_CORE_AUDIO_MIDIIN_PASS_THROUGH: do not sync
 * @AGS_CORE_AUDIO_MIDIIN_INITIAL_CALLBACK: initial callback
 * @AGS_CORE_AUDIO_MIDIIN_CALLBACK_WAIT: sync wait, soundcard conditional lock
 * @AGS_CORE_AUDIO_MIDIIN_CALLBACK_DONE: sync done, soundcard conditional lock
 * @AGS_CORE_AUDIO_MIDIIN_CALLBACK_FINISH_WAIT: sync wait, client conditional lock
 * @AGS_CORE_AUDIO_MIDIIN_CALLBACK_FINISH_DONE: sync done, client conditional lock
 * 
 * Enum values to control the synchronization between soundcard and client.
 */
typedef enum{
  AGS_CORE_AUDIO_MIDIIN_PASS_THROUGH                   = 1,
  AGS_CORE_AUDIO_MIDIIN_INITIAL_CALLBACK               = 1 <<  1,
  AGS_CORE_AUDIO_MIDIIN_CALLBACK_WAIT                  = 1 <<  2,
  AGS_CORE_AUDIO_MIDIIN_CALLBACK_DONE                  = 1 <<  3,
  AGS_CORE_AUDIO_MIDIIN_CALLBACK_FINISH_WAIT           = 1 <<  4,
  AGS_CORE_AUDIO_MIDIIN_CALLBACK_FINISH_DONE           = 1 <<  5,
  AGS_CORE_AUDIO_MIDIIN_DO_SYNC                        = 1 <<  6,
}AgsCoreAudioMidiinSyncFlags;

#define AGS_CORE_AUDIO_MIDIIN_ERROR (ags_core_audio_midiin_error_quark())

typedef enum{
  AGS_CORE_AUDIO_MIDIIN_ERROR_LOCKED_SOUNDCARD,
}AgsCoreAudioMidiinError;

struct _AgsCoreAudioMidiin
{
  GObject object;

  guint flags;
  volatile guint sync_flags;
  
  pthread_mutex_t *obj_mutex;
  pthread_mutexattr_t *obj_mutexattr;

  AgsApplicationContext *application_context;
  
  AgsUUID *uuid;
  
  char **buffer;
  guint buffer_size[4];

  double bpm; // beats per minute

  gdouble delay;
  gdouble delay_factor;
  guint latency;
  
  gdouble tact_counter;
  gdouble delay_counter; // next time attack changeing when delay_counter == delay
  guint tic_counter; // in the range of default period

  guint start_note_offset;
  guint note_offset;
  guint note_offset_absolute;

  gchar *card_uri;
  GObject *core_audio_client;

  gchar **port_name;
  GList *core_audio_port;

  pthread_mutex_t *callback_mutex;
  pthread_cond_t *callback_cond;

  pthread_mutex_t *callback_finish_mutex;
  pthread_cond_t *callback_finish_cond;
};

struct _AgsCoreAudioMidiinClass
{
  GObjectClass object;
};

GType ags_core_audio_midiin_get_type();

GQuark ags_core_audio_midiin_error_quark();

pthread_mutex_t* ags_core_audio_midiin_get_class_mutex();

gboolean ags_core_audio_midiin_test_flags(AgsCoreAudioMidiin *core_audio_midiin, guint flags);
void ags_core_audio_midiin_set_flags(AgsCoreAudioMidiin *core_audio_midiin, guint flags);
void ags_core_audio_midiin_unset_flags(AgsCoreAudioMidiin *core_audio_midiin, guint flags);

void ags_core_audio_midiin_switch_buffer_flag(AgsCoreAudioMidiin *core_audio_midiin);

AgsCoreAudioMidiin* ags_core_audio_midiin_new(AgsApplicationContext *application_context);

#endif /*__AGS_CORE_AUDIO_MIDIIN_H__*/
