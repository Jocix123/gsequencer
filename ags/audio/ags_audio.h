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

#include <ags/libags.h>

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
 * @AGS_AUDIO_ADDED_TO_REGISTRY: the audio was added to registry, see #AgsConnectable::add_to_registry()
 * @AGS_AUDIO_CONNECTED: the audio was connected by #AgsConnectable::connect()
 * @AGS_AUDIO_NO_OUTPUT: no output provided
 * @AGS_AUDIO_NO_INPUT: no input provided
 * @AGS_AUDIO_SYNC: input/output is mapped synchronously
 * @AGS_AUDIO_ASYNC: input/output is mapped asynchronously
 * @AGS_AUDIO_OUTPUT_HAS_RECYCLING: output has recycling
 * @AGS_AUDIO_INPUT_HAS_RECYCLING: input has recycling
 * @AGS_AUDIO_INPUT_HAS_FILE_LINK: input takes file
 * @AGS_AUDIO_INPUT_HAS_SYNTH_GENERATOR: input takes synth
 * @AGS_AUDIO_SKIP_OUTPUT: skip output as processing audio data
 * @AGS_AUDIO_SKIP_INPUT: skip input as processing audio data
 * @AGS_AUDIO_BYPASS: don't apply any data
 *
 * Enum values to control the behavior or indicate internal state of #AgsAudio by
 * enable/disable as flags.
 */
typedef enum{
  AGS_AUDIO_ADDED_TO_REGISTRY           = 1,
  AGS_AUDIO_CONNECTED                   = 1 <<  1,
  AGS_AUDIO_NO_OUTPUT                   = 1 <<  2,
  AGS_AUDIO_NO_INPUT                    = 1 <<  3,
  AGS_AUDIO_SYNC                        = 1 <<  4, // can be combined with below
  AGS_AUDIO_ASYNC                       = 1 <<  5,
  AGS_AUDIO_OUTPUT_HAS_RECYCLING        = 1 <<  6,
  AGS_AUDIO_OUTPUT_HAS_SYNTH            = 1 <<  7,
  AGS_AUDIO_INPUT_HAS_RECYCLING         = 1 <<  8,
  AGS_AUDIO_INPUT_HAS_SYNTH             = 1 <<  9,
  AGS_AUDIO_INPUT_HAS_FILE              = 1 << 10,
  AGS_AUDIO_CAN_NEXT_ACTIVE             = 1 << 11,
  AGS_AUDIO_SKIP_OUTPUT                 = 1 << 12,
  AGS_AUDIO_SKIP_INPUT                  = 1 << 13,
  AGS_AUDIO_BYPASS                      = 1 << 14,
}AgsAudioFlags;

struct _AgsAudio
{
  GObject object;

  guint flags;
  guint ability_flags;
  guint behaviour_flags;
  guint staging_flags[AGS_SOUND_SCOPE_LAST];
  
  pthread_mutex_t *obj_mutex;
  pthread_mutexattr_t *obj_mutexattr;

  AgsUUID *uuid;

  gchar *audio_name;
  
  GObject *output_soundcard;
  gint *output_soundcard_channel_map;
  
  GObject *input_soundcard;
  gint *input_soundcard_channel_map;

  GObject *output_sequencer;
  GObject *input_sequencer;
  
  guint samplerate;
  guint buffer_size;
  guint format;

  gdouble bpm;

  guint bank_dim[3];

  guint min_audio_channels;
  guint max_audio_channels;
  
  guint min_output_pads;
  guint max_output_pads;
  
  guint min_input_pads;
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

  guint numerator;
  guint denominator;
  gchar *time_signature;

  gboolean is_minor;
  guint sharp_flats;
  
  gint octave;
  guint key;
  gint absolute_key;

  guint64 loop_start;
  guint64 loop_end;
  guint64 offset;
  
  AgsChannel *output;
  AgsChannel *input;

  GList *preset;

  GObject *playback_domain;

  GList *synth_generator;
  
  GList *notation;
  GList *automation;
  
  GList *wave;
  GObject *output_audio_file;
  GObject *input_audio_file;  

  gchar *instrument_name;
  gchar *track_name;
  
  GList *midi;
  GObject *output_midi_file;
  GObject *input_midi_file;
  
  GList *recall_id;
  GList *recycling_context;

  GList *recall_container;

  pthread_mutexattr_t *play_mutexattr;
  pthread_mutex_t *play_mutex;

  GList *play;

  pthread_mutexattr_t *recall_mutexattr;
  pthread_mutex_t *recall_mutex;

  GList *recall;
  
  gpointer machine_widget;
  gpointer file_data;
};

struct _AgsAudioClass
{
  GObjectClass object;
  
  void (*set_audio_channels)(AgsAudio *audio,
			     guint audio_channels, guint audio_channels_old);
  void (*set_pads)(AgsAudio *audio,
		   GType channel_type,
		   guint pads, guint pads_old);  

  void (*duplicate_recall)(AgsAudio *audio,
			   AgsRecallID *recall_id,
			   guint pad, guint audio_channel,
			   guint line);
  void (*resolve_recall)(AgsAudio *audio,
			 AgsRecallID *recall_id);

  void (*init_recall)(AgsAudio *audio,
		      AgsRecallID *recall_id, guint staging_flags);
  void (*play_recall)(AgsAudio *audio,
		      AgsRecallID *recall_id, guint staging_flags);

  void (*done_recall)(AgsAudio *audio,
		      AgsRecallID *recall_id);
  void (*cancel_recall)(AgsAudio *audio,
			AgsRecallID *recall_id);

  void (*cleanup_recall)(AgsAudio *audio,
			 AgsRecallID *recall_id);

  GList* (*start)(AgsAudio *audio,
		  gint sound_scope);
  void (*stop)(AgsAudio *audio,
	       GList *recall_id, gint sound_scope);

  GList* (*check_scope)(AgsAudio *audio, gint sound_scope);
  
  void (*recursive_run_stage)(AgsAudio *audio,
			      gint sound_scope, guint staging_flags);
};

GType ags_audio_get_type();

pthread_mutex_t* ags_audio_get_class_mutex();

gboolean ags_audio_test_flags(AgsAudio *audio, guint flags);
void ags_audio_set_flags(AgsAudio *audio, guint flags);
void ags_audio_unset_flags(AgsAudio *audio, guint flags);

gboolean ags_audio_test_ability_flags(AgsAudio *audio, guint ability_flags);
void ags_audio_set_ability_flags(AgsAudio *audio, guint ability_flags);
void ags_audio_unset_ability_flags(AgsAudio *audio, guint ability_flags);

gboolean ags_audio_test_behaviour_flags(AgsAudio *audio, guint behaviour_flags);
void ags_audio_set_behaviour_flags(AgsAudio *audio, guint behaviour_flags);
void ags_audio_unset_behaviour_flags(AgsAudio *audio, guint behaviour_flags);

gboolean ags_audio_test_staging_flags(AgsAudio *audio, gint sound_scope,
				      guint staging_flags);
void ags_audio_set_staging_flags(AgsAudio *audio, gint sound_scope,
				 guint staging_flags);
void ags_audio_unset_staging_flags(AgsAudio *audio, gint sound_scope,
				   guint staging_flags);

/* matching */
GList* ags_audio_find_name(GList *audio,
			   gchar *audio_name);

/* channel alignment */
void ags_audio_set_max_audio_channels(AgsAudio *audio,
				      guint max_audio_channels);

void ags_audio_set_max_pads(AgsAudio *audio,
			    GType channel_type,
			    guint max_pads);

void ags_audio_set_audio_channels(AgsAudio *audio,
				  guint audio_channels, guint audio_channels_old);
void ags_audio_set_pads(AgsAudio *audio,
			GType channel_type,
			guint pads, guint pads_old);

/* soundcard */
void ags_audio_set_output_soundcard(AgsAudio *audio,
				    GObject *soundcard);
void ags_audio_set_input_soundcard(AgsAudio *audio,
				   GObject *soundcard);

/* sequencer */
void ags_audio_set_output_sequencer(AgsAudio *audio,
				    GObject *sequencer);
void ags_audio_set_input_sequencer(AgsAudio *audio,
				   GObject *sequencer);

/* presets */
void ags_audio_set_samplerate(AgsAudio *audio, guint samplerate);
void ags_audio_set_buffer_size(AgsAudio *audio, guint buffer_size);
void ags_audio_set_format(AgsAudio *audio, guint format);

/* children */
void ags_audio_add_preset(AgsAudio *audio, GObject *preset);
void ags_audio_remove_preset(AgsAudio *audio, GObject *preset);

void ags_audio_add_synth_generator(AgsAudio *audio, GObject *synth_generator);
void ags_audio_remove_synth_generator(AgsAudio *audio, GObject *synth_generator);

void ags_audio_add_notation(AgsAudio *audio, GObject *notation);
void ags_audio_remove_notation(AgsAudio *audio, GObject *notation);

void ags_audio_add_automation(AgsAudio *audio, GObject *automation);
void ags_audio_remove_automation(AgsAudio *audio, GObject *automation);

void ags_audio_add_wave(AgsAudio *audio, GObject *wave);
void ags_audio_remove_wave(AgsAudio *audio, GObject *wave);

void ags_audio_add_midi(AgsAudio *audio, GObject *midi);
void ags_audio_remove_midi(AgsAudio *audio, GObject *midi);

/* recall related */
void ags_audio_add_recall_id(AgsAudio *audio, GObject *recall_id);
void ags_audio_remove_recall_id(AgsAudio *audio, GObject *recall_id);

void ags_audio_add_recycling_context(AgsAudio *audio, GObject *recycling_context);
void ags_audio_remove_recycling_context(AgsAudio *audio, GObject *recycling_context);

void ags_audio_add_recall_container(AgsAudio *audio, GObject *recall_container);
void ags_audio_remove_recall_container(AgsAudio *audio, GObject *recall_container);

void ags_audio_add_recall(AgsAudio *audio, GObject *recall,
			  gboolean play_context);
void ags_audio_remove_recall(AgsAudio *audio, GObject *recall,
			     gboolean play_context);

/* stages */
void ags_audio_duplicate_recall(AgsAudio *audio,
				AgsRecallID *recall_id,
				guint pad, guint audio_channel,
				guint line);
void ags_audio_resolve_recall(AgsAudio *audio,
			      AgsRecallID *recall_id);

void ags_audio_init_recall(AgsAudio *audio,
			   AgsRecallID *recall_id, guint staging_flags);
void ags_audio_play_recall(AgsAudio *audio,
			   AgsRecallID *recall_id, guint staging_flags);

void ags_audio_done_recall(AgsAudio *audio,
			   AgsRecallID *recall_id);
void ags_audio_cancel_recall(AgsAudio *audio,
			     AgsRecallID *recall_id);

void ags_audio_cleanup_recall(AgsAudio *audio,
			      AgsRecallID *recall_id);

/* control */
GList* ags_audio_start(AgsAudio *audio,
		       gint sound_scope);

void ags_audio_stop(AgsAudio *audio,
		    GList *recall_id, gint sound_scope);

/* query */
GList* ags_audio_check_scope(AgsAudio *audio, gint sound_scope);

GList* ags_audio_collect_all_audio_ports(AgsAudio *audio);

GList* ags_audio_collect_all_audio_ports_by_specifier_and_context(AgsAudio *audio,
								  gchar *specifier,
								  gboolean play_context);

/* file IO */
void ags_audio_open_audio_file_as_channel(AgsAudio *audio,
					  GSList *filename,
					  gboolean overwrite_channels,
					  gboolean create_channels);
void ags_audio_open_audio_file_as_wave(AgsAudio *audio,
				       const gchar *filename,
				       gboolean overwrite_channels,
				       gboolean create_channels);

void ags_audio_open_midi_file_as_midi(AgsAudio *audio,
				      const gchar *filename,
				      const gchar *instrument,
				      const gchar *track_name,
				      guint midi_channel);
void ags_audio_open_midi_file_as_notation(AgsAudio *audio,
					  const gchar *filename,
					  const gchar *instrument,
					  const gchar *track_name,
					  guint midi_channel);

/* recursive functions */
void ags_audio_recursive_set_property(AgsAudio *audio,
				      gint n_params,
				      const gchar *parameter_name[], const GValue value[]);

void ags_audio_recursive_run_stage(AgsAudio *audio,
				   gint sound_scope, guint staging_flags);

/* instantiate */
AgsAudio* ags_audio_new(GObject *output_soundcard);

#endif /*__AGS_AUDIO_H__*/
