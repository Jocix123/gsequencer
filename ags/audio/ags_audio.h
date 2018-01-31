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

#ifndef __AGS_AUDIO_H__
#define __AGS_AUDIO_H__

#include <glib.h>
#include <glib-object.h>

#include <pthread.h>

#include <ags/audio/ags_sound_enums.h>
#include <ags/audio/ags_channel.h>
#include <ags/audio/ags_recall_id.h>

#define AGS_TYPE_AUDIO                (ags_audio_get_type ())
#define AGS_AUDIO(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_AUDIO, AgsAudio))
#define AGS_AUDIO_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST((class), AGS_TYPE_AUDIO, AgsAudioClass))
#define AGS_IS_AUDIO(obj)             (G_TYPE_CHECK_INSTANCE_TYPE((obj), AGS_TYPE_AUDIO))
#define AGS_IS_AUDIO_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE((class), AGS_TYPE_AUDIO))
#define AGS_AUDIO_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS((obj), AGS_TYPE_AUDIO, AgsAudioClass))

typedef struct _AgsAudio AgsAudio;
typedef struct _AgsAudioClass AgsAudioClass;

/**
 * AgsAudioFlags:
 * @AGS_AUDIO_CONNECTED: the audio was connected by #AgsConnectable::connect()
 * @AGS_AUDIO_NO_OUTPUT: no output provided
 * @AGS_AUDIO_NO_INPUT: no input provided
 * @AGS_AUDIO_SYNC: input/output is mapped synchronously
 * @AGS_AUDIO_ASYNC: input/output is mapped asynchronously
 * @AGS_AUDIO_OUTPUT_HAS_RECYCLING: output has recycling
 * @AGS_AUDIO_INPUT_HAS_RECYCLING: input has recycling
 * @AGS_AUDIO_INPUT_HAS_FILE_LINK: input takes file
 * @AGS_AUDIO_INPUT_HAS_SYNTH_GENERATOR: input takes synth
 * @AGS_AUDIO_HAS_NOTATION: has notation
 * @AGS_AUDIO_PATTERN_MODE: notation only contains fixed size notes
 * @AGS_AUDIO_NOTATION_DEFAULT: notation default mapped on input channels
 * @AGS_AUDIO_REVERSE_MAPPING: interpret notation reverse
 * @AGS_AUDIO_HAS_PATTERN: has pattern
 * @AGS_AUDIO_HAS_WAVE: has wave
 * @AGS_AUDIO_WAVE_DEFAULT: wave default mapped on input channels
 * @AGS_AUDIO_RUNNING: it is running
 * @AGS_AUDIO_PLAYING: it is playing
 * @AGS_AUDIO_CAN_NEXT_ACTIVE: can determine next active input
 * @AGS_AUDIO_SKIP_OUTPUT: skip output as processing audio data
 * @AGS_AUDIO_SKIP_INPUT: skip input as processing audio data
 *
 * Enum values to control the behavior or indicate internal state of #AgsAudio by
 * enable/disable as flags.
 */
typedef enum{
  AGS_AUDIO_CONNECTED                   = 1,
  AGS_AUDIO_NO_OUTPUT                   = 1 <<  1,
  AGS_AUDIO_NO_INPUT                    = 1 <<  2,
  AGS_AUDIO_SYNC                        = 1 <<  3, // can be combined with below
  AGS_AUDIO_ASYNC                       = 1 <<  4,
  AGS_AUDIO_OUTPUT_HAS_RECYCLING        = 1 <<  5,
  AGS_AUDIO_INPUT_HAS_RECYCLING         = 1 <<  6,
  AGS_AUDIO_INPUT_HAS_FILE              = 1 <<  7,
  AGS_AUDIO_INPUT_HAS_SYNTH             = 1 <<  8,
  AGS_AUDIO_HAS_NOTATION                = 1 <<  9,
  AGS_AUDIO_PATTERN_MODE                = 1 << 10,
  AGS_AUDIO_NOTATION_DEFAULT            = 1 << 11,
  AGS_AUDIO_REVERSE_MAPPING             = 1 << 12,
  AGS_AUDIO_HAS_PATTERN                 = 1 << 13,
  AGS_AUDIO_HAS_WAVE                    = 1 << 14,
  AGS_AUDIO_WAVE_DEFAULT                = 1 << 15,
  AGS_AUDIO_RUNNING                     = 1 << 16,
  AGS_AUDIO_PLAYING                     = 1 << 17,
  AGS_AUDIO_CAN_NEXT_ACTIVE             = 1 << 18,
  AGS_AUDIO_SKIP_OUTPUT                 = 1 << 19,
  AGS_AUDIO_SKIP_INPUT                  = 1 << 20,
}AgsAudioFlags;

struct _AgsAudio
{
  GObject object;

  guint flags;

  pthread_mutex_t *obj_mutex;
  pthread_mutexattr_t *obj_mutexattr;

  GObject *output_soundcard;
  GObject *input_soundcard;
  guint level;

  GObject *output_sequencer;
  GObject *input_sequencer;
  
  guint samplerate;
  guint buffer_size;
  guint format;
  guint sequence_length;

  guint bank_dim[3];

  guint max_audio_channels;
  guint max_output_pads;
  guint max_input_pads;

  guint audio_channels;

  guint output_pads;
  guint output_lines;

  guint input_pads;
  guint input_lines;

  guint audio_start_mapping;
  guint audio_end_mapping;

  guint midi_start_mapping;
  guint midi_end_mapping;

  guint midi_channel;
  
  GList *audio_connection;

  GList *preset;
  
  AgsChannel *output;
  AgsChannel *input;

  GObject *playback_domain;
  
  GList *notation;
  GList *automation;

  GObject *output_audio_file;
  GObject *input_audio_file;  
  GList *wave;

  GObject *output_midi_file;
  GObject *input_midi_file;
  GList *midi;
  
  GList *recall_id;
  GList *recycling_context;

  pthread_mutexattr_t *recall_mutexattr;
  pthread_mutex_t *recall_mutex;

  pthread_mutexattr_t *play_mutexattr;
  pthread_mutex_t *play_mutex;

  GList *recall_container;
  GList *recall;
  GList *play;
  
  GObject *machine;
};

struct _AgsAudioClass
{
  GObjectClass object;

  void (*check_connection)(AgsAudio *audio);
  
  void (*set_audio_channels)(AgsAudio *audio,
			     guint audio_channels, guint audio_channels_old);
  void (*set_pads)(AgsAudio *audio,
		   GType channel_type,
		   guint pads, guint pads_old);
  
  void (*tact)(AgsAudio *audio, AgsRecallID *recall_id);
  void (*done)(AgsAudio *audio, AgsRecallID *recall_id);
};

GType ags_audio_get_type();

void ags_audio_set_flags(AgsAudio *audio, guint flags);
void ags_audio_unset_flags(AgsAudio *audio, guint flags);

void ags_audio_check_connection(AgsAudio *audio);

/* channel alignment */
void ags_audio_set_audio_channels(AgsAudio *audio, guint audio_channels);
void ags_audio_set_pads(AgsAudio *audio, GType channel_type, guint pads);

/* presets */
void ags_audio_set_soundcard(AgsAudio *audio, GObject *soundcard);

void ags_audio_set_samplerate(AgsAudio *audio, guint samplerate);
void ags_audio_set_buffer_size(AgsAudio *audio, guint buffer_size);
void ags_audio_set_format(AgsAudio *audio, guint format);
void ags_audio_set_sequence_length(AgsAudio *audio, guint sequence_length);

/* children */
void ags_audio_add_audio_connection(AgsAudio *audio,
				    GObject *audio_connection);
void ags_audio_remove_audio_connection(AgsAudio *audio,
				       GObject *audio_connection);

void ags_audio_add_preset(AgsAudio *audio,
			  GObject *preset);
void ags_audio_remove_preset(AgsAudio *audio,
			     GObject *preset);

void ags_audio_add_notation(AgsAudio *audio,
			    GObject *notation);
void ags_audio_remove_notation(AgsAudio *audio,
			       GObject *notation);

void ags_audio_add_automation(AgsAudio *audio,
			      GObject *automation);
void ags_audio_remove_automation(AgsAudio *audio,
				 GObject *automation);

void ags_audio_add_wave(AgsAudio *audio,
			GObject *wave);
void ags_audio_remove_wave(AgsAudio *audio,
			   GObject *wave);

/* recall related */
void ags_audio_add_recall_id(AgsAudio *audio, GObject *recall_id);
void ags_audio_remove_recall_id(AgsAudio *audio, GObject *recall_id);

void ags_audio_add_recycling_context(AgsAudio *audio, GObject *recycling_context);
void ags_audio_remove_recycling_context(AgsAudio *audio, GObject *recycling_context);

void ags_audio_add_recall_container(AgsAudio *audio, GObject *recall_container);
void ags_audio_remove_recall_container(AgsAudio *audio, GObject *recall_container);

void ags_audio_add_recall(AgsAudio *audio, GObject *recall, gboolean play);
void ags_audio_remove_recall(AgsAudio *audio, GObject *recall, gboolean play);

/* stages */
void ags_audio_duplicate_recall(AgsAudio *audio,
				AgsRecallID *recall_id);
void ags_audio_init_recall(AgsAudio *audio, gint stage,
			   AgsRecallID *recall_id);
void ags_audio_resolve_recall(AgsAudio *audio,
			      AgsRecallID *recall_id);

void ags_audio_play(AgsAudio *audio,
		    AgsRecallID *recall_id,
		    gint stage);
void ags_audio_tact(AgsAudio *audio, AgsRecallID *recall_id);
void ags_audio_done(AgsAudio *audio, AgsRecallID *recall_id);

void ags_audio_cancel(AgsAudio *audio,
		      AgsRecallID *recall_id);
void ags_audio_remove(AgsAudio *audio,
		      AgsRecallID *recall_id);

/* query */
gboolean ags_audio_is_playing(AgsAudio *audio);

GList* ags_audio_collect_all_audio_ports(AgsAudio *audio);

GObject* ags_audio_collect_all_audio_ports_by_specifier_and_context(AgsAudio *audio,
								    gchar *specifier,
								    gboolean play_context);

void ags_audio_open_files(AgsAudio *audio,
			  GSList *filenames,
			  gboolean overwrite_channels,
			  gboolean create_channels);

void ags_audio_recursive_set_property(AgsAudio *audio,
				      GParameter *parameter, gint n_params);

GList* ags_audio_recursive_play_init(AgsAudio *audio,
				     guint sound_scope);

AgsAudio* ags_audio_new(GObject *soundcard);

#endif /*__AGS_AUDIO_H__*/
