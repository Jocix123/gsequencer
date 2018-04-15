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

#ifndef __AGS_AUDIO_SIGNAL_H__
#define __AGS_AUDIO_SIGNAL_H__

#include <glib.h>
#include <glib-object.h>

#include <pthread.h>

#include <ags/libags.h>

#define AGS_TYPE_AUDIO_SIGNAL                (ags_audio_signal_get_type())
#define AGS_AUDIO_SIGNAL(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_AUDIO_SIGNAL, AgsAudioSignal))
#define AGS_AUDIO_SIGNAL_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST((class), AGS_TYPE_AUDIO_SIGNAL, AgsAudioSignalClass))
#define AGS_IS_AUDIO_SIGNAL(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AGS_TYPE_AUDIO_SIGNAL))
#define AGS_IS_AUDIO_SIGNAL_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE ((class), AGS_TYPE_AUDIO_SIGNAL))
#define AGS_AUDIO_SIGNAL_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), AGS_TYPE_AUDIO_SIGNAL, AgsAudioSignalClass))

typedef struct _AgsAudioSignal AgsAudioSignal;
typedef struct _AgsAudioSignalClass AgsAudioSignalClass;

/**
 * AgsAudioSignalFlags:
 * @AGS_AUDIO_SIGNAL_ADDED_TO_REGISTRY: the audio signal was added to registry, see #AgsConnectable::add_to_registry()
 * @AGS_AUDIO_SIGNAL_CONNECTED: indicates the audio signal was connected by calling #AgsConnectable::connect()
 * @AGS_AUDIO_SIGNAL_TEMPLATE: the audio signal acts as a template
 * @AGS_AUDIO_SIGNAL_RT_TEMPLATE: the audio signal acts as a realtime template
 * 
 * Enum values to control the behavior or indicate internal state of #AgsAudioSignal by
 * enable/disable as flags.
 */
typedef enum{
  AGS_AUDIO_SIGNAL_ADDED_TO_REGISTRY    = 1,
  AGS_AUDIO_SIGNAL_CONNECTED            = 1 <<  1,
  AGS_AUDIO_SIGNAL_TEMPLATE             = 1 <<  2,
  AGS_AUDIO_SIGNAL_RT_TEMPLATE          = 1 <<  3,
  //  AGS_AUDIO_SIGNAL_PLAY_DONE            = 1 <<  3,
  //  AGS_AUDIO_SIGNAL_STANDALONE           = 1 <<  4,
}AgsAudioSignalFlags;

struct _AgsAudioSignal
{
  GObject object;

  guint flags;

  pthread_mutex_t *obj_mutex;
  pthread_mutexattr_t *obj_mutexattr;

  AgsUUID *uuid;
  
  GObject *recycling;

  GObject *output_soundcard;
  gint output_soundcard_channel;

  GObject *input_soundcard;
  gint input_soundcard_channel;

  guint samplerate;
  guint buffer_size;
  guint format;
  guint word_size;
  
  guint length;
  guint first_frame;
  guint last_frame; // the last frame at stream_end

  guint frame_count; // initial size
  guint loop_start;
  guint loop_end;

  gdouble delay;
  guint attack;

  AgsComplex damping;
  AgsComplex vibration;
  guint timbre_start;
  guint timbre_end;

  GObject *template;
  
  GObject *rt_template;
  GList *note;

  GObject *recall_id; // AGS_TYPE_RECALL_ID to identify the AgsAudioSignal

  pthread_mutexattr_t *stream_mutexattr;
  pthread_mutex_t *stream_mutex;
  
  GList *stream;
  GList *stream_current;
  GList *stream_end;
};

struct _AgsAudioSignalClass
{
  GObjectClass object;

  void (*add_note)(AgsAudioSignal *audio_signal,
		   GObject *note);
  void (*remove_note)(AgsAudioSignal *audio_signal,
		      GObject *note);

  void (*refresh_data)(AgsAudioSignal *audio_signal);
};

GType ags_audio_signal_get_type();

pthread_mutex_t* ags_audio_signal_get_class_mutex();

void ags_audio_signal_set_flags(AgsAudioSignal *audio_signal, guint flags);
void ags_audio_signal_unset_flags(AgsAudioSignal *audio_signal, guint flags);

void* ags_stream_alloc(guint buffer_size,
		       guint format);
void ags_stream_free(void *buffer);

/* soundcard */
void ags_audio_signal_set_output_soundcard(AgsAudioSignal *audio_signal, GObject *output_soundcard);
void ags_audio_signal_set_input_soundcard(AgsAudioSignal *audio_signal, GObject *input_soundcard);

/* presets */
void ags_audio_signal_set_samplerate(AgsAudioSignal *audio_signal, guint samplerate);
void ags_audio_signal_set_buffer_size(AgsAudioSignal *audio_signal, guint buffer_size);
void ags_audio_signal_set_format(AgsAudioSignal *audio_signal, guint format);

/* children */
void ags_audio_signal_add_note(AgsAudioSignal *audio_signal,
			       GObject *note);
void ags_audio_signal_remove_note(AgsAudioSignal *audio_signal,
				  GObject *note);

/* presets related */
void ags_audio_signal_refresh_data(AgsAudioSignal *audio_signal);

/* control */
void ags_audio_signal_add_stream(AgsAudioSignal *audio_signal);
void ags_audio_signal_stream_resize(AgsAudioSignal *audio_signal, guint length);
void ags_audio_signal_stream_safe_resize(AgsAudioSignal *audio_signal, guint length);

void ags_audio_signal_duplicate_stream(AgsAudioSignal *audio_signal,
				       AgsAudioSignal *template);

void ags_audio_signal_feed(AgsAudioSignal *audio_signal,
			   AgsAudioSignal *template,
			   guint frame_count);

/* query */
guint ags_audio_signal_get_length_till_current(AgsAudioSignal *audio_signal);

AgsAudioSignal* ags_audio_signal_get_template(GList *audio_signal);
GList* ags_audio_signal_get_rt_template(GList *audio_signal);

GList* ags_audio_signal_find_stream_current(GList *audio_signal,
					   GObject *recall_id);
GList* ags_audio_signal_find_by_recall_id(GList *audio_signal,
					  GObject *recall_id);

gboolean ags_audio_signal_is_active(GList *audio_signal,
				    GObject *recall_id);

/* instantiate */
AgsAudioSignal* ags_audio_signal_new(GObject *output_soundcard,
				     GObject *recycling,
				     GObject *recall_id);

AgsAudioSignal* ags_audio_signal_new_with_length(GObject *output_soundcard,
						 GObject *recycling,
						 GObject *recall_id,
						 guint length);

#endif /*__AGS_AUDIO_SIGNAL_H__*/
