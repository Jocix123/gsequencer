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

#ifndef __AGS_CHANNEL_H__
#define __AGS_CHANNEL_H__

#include <glib.h>
#include <glib-object.h>

#include <pthread.h>

#include <ags/libags.h>

#include <ags/audio/ags_recall_id.h>
#include <ags/audio/ags_recall.h>
#include <ags/audio/ags_recycling.h>
#include <ags/audio/ags_notation.h>

#include <stdarg.h>

#define AGS_TYPE_CHANNEL                (ags_channel_get_type())
#define AGS_CHANNEL(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_CHANNEL, AgsChannel))
#define AGS_CHANNEL_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST((class), AGS_TYPE_CHANNEL, AgsChannelClass))
#define AGS_IS_CHANNEL(obj)             (G_TYPE_CHECK_INSTANCE_TYPE((obj), AGS_TYPE_CHANNEL))
#define AGS_IS_CHANNEL_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE ((class), AGS_TYPE_CHANNEL))
#define AGS_CHANNEL_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS((obj), AGS_TYPE_CHANNEL, AgsChannelClass))

typedef struct _AgsChannel AgsChannel;
typedef struct _AgsChannelClass AgsChannelClass;

/**
 * AgsChannelFlags:
 * @AGS_CHANNEL_ADDED_TO_REGISTRY: added to registry
 * @AGS_CHANNEL_CONNECTED: indicates the channel was connected by calling #AgsConnectable::connect()
 * @AGS_CHANNEL_RUNNING: channel running
 *
 * Enum values to control the behavior or indicate internal state of #AgsChannel by
 * enable/disable as flags.
 */
typedef enum{
  AGS_CHANNEL_ADDED_TO_REGISTRY  = 1,
  AGS_CHANNEL_CONNECTED          = 1 <<  1,
  AGS_CHANNEL_RUNNING            = 1 <<  2,
}AgsChannelFlags;

typedef enum{
  AGS_CHANNEL_RECALL_ID_RUN_STAGE,
  AGS_CHANNEL_RECALL_ID_CANCEL,
}AgsChannelRecallIDMode;

#define AGS_CHANNEL_ERROR (ags_channel_error_quark())

typedef enum{
  AGS_CHANNEL_ERROR_LOOP_IN_LINK,
}AgsChannelError;

struct _AgsChannel
{
  GObject object;

  guint flags;

  pthread_mutex_t *obj_mutex;
  pthread_mutexattr_t *obj_mutexattr;

  GObject *audio;
  GObject *soundcard;

  guint samplerate;
  guint buffer_size;
  guint format;
  
  AgsChannel *prev;
  AgsChannel *prev_pad;
  AgsChannel *next;
  AgsChannel *next_pad;

  guint pad;
  guint audio_channel;
  guint line;

  char *note;

  GList *remote_channel;
  
  GObject *playback;

  // GObject *recycling_context; // contains child recycling
  GList *recall_id; // there may be several recall's running

  pthread_mutexattr_t *recall_mutexattr;
  pthread_mutex_t *recall_mutex;

  pthread_mutexattr_t *play_mutexattr;
  pthread_mutex_t *play_mutex;

  GList *container;
  GList *recall;
  GList *play;

  AgsChannel *link;

  AgsRecycling *first_recycling;
  AgsRecycling *last_recycling;

  GList *pattern;

  GObject *line_widget;
  gpointer file_data;
};

struct _AgsChannelClass
{
  GObjectClass object;

  GList* (*add_effect)(AgsChannel *channel,
		       gchar *filename,
		       gchar *effect);
  void (*remove_effect)(AgsChannel *channel,
			guint nth);

  void (*recycling_changed)(AgsChannel *channel,
			    AgsRecycling *old_start_region, AgsRecycling *old_end_region,
			    AgsRecycling *new_start_region, AgsRecycling *new_end_region,
			    AgsRecycling *old_start_changed_region, AgsRecycling *old_end_changed_region,
			    AgsRecycling *new_start_changed_region, AgsRecycling *new_end_changed_region);

  void (*done)(AgsChannel *channel,
	       AgsRecallID *recall_id);
};

GType ags_channel_get_type();

GQuark ags_channel_error_quark();

AgsRecall* ags_channel_find_recall(AgsChannel *channel, char *effect, char *name);

AgsChannel* ags_channel_first(AgsChannel *channel);
AgsChannel* ags_channel_last(AgsChannel *channel);
AgsChannel* ags_channel_nth(AgsChannel *channel, guint nth);

AgsChannel* ags_channel_pad_first(AgsChannel *channel);
AgsChannel* ags_channel_pad_last(AgsChannel *channel);
AgsChannel* ags_channel_pad_nth(AgsChannel *channel, guint nth);

AgsChannel* ags_channel_first_with_recycling(AgsChannel *channel);
AgsChannel* ags_channel_last_with_recycling(AgsChannel *channel);
AgsChannel* ags_channel_prev_with_recycling(AgsChannel *channel);
AgsChannel* ags_channel_next_with_recycling(AgsChannel *channel);

void ags_channel_set_soundcard(AgsChannel *channel, GObject *soundcard);

void ags_channel_set_samplerate(AgsChannel *channel, guint samplerate);
void ags_channel_set_buffer_size(AgsChannel *channel, guint buffer_size);
void ags_channel_set_format(AgsChannel *channel, guint format);

void ags_channel_add_remote_channel(AgsChannel *channel, GObject *remote_channel);
void ags_channel_remove_remote_channel(AgsChannel *channel, GObject *remote_channel);

void ags_channel_add_recall_id(AgsChannel *channel, AgsRecallID *recall_id);
void ags_channel_remove_recall_id(AgsChannel *channel, AgsRecallID *recall_id);

void ags_channel_add_recall_container(AgsChannel *channel, GObject *recall_container);
void ags_channel_remove_recall_container(AgsChannel *channel, GObject *recall_container);

void ags_channel_add_recall(AgsChannel *channel, GObject *recall, gboolean play);
void ags_channel_remove_recall(AgsChannel *channel, GObject *recall, gboolean play);

void ags_channel_add_pattern(AgsChannel *channel, GObject *pattern);
void ags_channel_remove_pattern(AgsChannel *channel, GObject *pattern);

GList* ags_channel_add_effect(AgsChannel *channel,
			      char *filename,
			      gchar *effect);
void ags_channel_remove_effect(AgsChannel *channel,
			       guint nth);

void ags_channel_safe_resize_audio_signal(AgsChannel *channel,
					  guint length);

void ags_channel_duplicate_recall(AgsChannel *channel,
				  AgsRecallID *recall_id);
void ags_channel_resolve_recall(AgsChannel *channel,
				AgsRecallID *recall_id);
void ags_channel_init_recall(AgsChannel *channel, gint stage,
			     AgsRecallID *recall_id);
void ags_channel_play(AgsChannel *channel,
		      AgsRecallID *recall_id, gint stage);
void ags_channel_done(AgsChannel *channel,
		      AgsRecallID *recall_id);

void ags_channel_cancel(AgsChannel *channel, AgsRecallID *recall_id);
void ags_channel_remove(AgsChannel *channel, AgsRecallID *recall_id);

void ags_channel_recall_id_set(AgsChannel *output, AgsRecallID *recall_id, gboolean ommit_own_channel,
			       guint mode, ...);

GList* ags_channel_find_port(AgsChannel *channel);
GObject* ags_channel_find_port_by_specifier_and_scope(AgsChannel *channel,
						      gchar *specifier,
						      gboolean play);

void ags_channel_set_link(AgsChannel *channel, AgsChannel *link,
			  GError **error);
void ags_channel_set_recycling(AgsChannel *channel,
			       AgsRecycling *first_recycling, AgsRecycling *last_recycling,
			       gboolean update, gboolean destroy_old);
void ags_channel_recursive_reset_recycling_context(AgsChannel *channel,
						   AgsRecyclingContext *old_recycling_context,
						   AgsRecyclingContext *recycling_context);
void ags_channel_recycling_changed(AgsChannel *channel,
				   AgsRecycling *old_start_region, AgsRecycling *old_end_region,
				   AgsRecycling *new_start_region, AgsRecycling *new_end_region,
				   AgsRecycling *old_start_changed_region, AgsRecycling *old_end_changed_region,
				   AgsRecycling *new_start_changed_region, AgsRecycling *new_end_changed_region);

void ags_channel_recursive_set_property(AgsChannel *channel,
					GParameter *parameter, gint n_params);

AgsRecallID* ags_channel_recursive_play_init(AgsChannel *channel, gint stage,
					     gboolean arrange_recall_id, gboolean duplicate_templates,
					     gboolean do_playback, gboolean do_sequencer, gboolean do_notation,
					     gboolean resolve_dependencies,
					     AgsRecallID *recall_id);
void ags_channel_recursive_play_threaded(AgsChannel *channel,
					 AgsRecallID *recall_id, gint stage);
void ags_channel_recursive_play(AgsChannel *channel,
				AgsRecallID *recall_id, gint stage);

void ags_channel_tillrecycling_cancel(AgsChannel *channel, AgsRecallID *recall_id);

void ags_channel_recursive_reset_recall_ids(AgsChannel *channel, AgsChannel *link,
					    AgsChannel *old_channel_link, AgsChannel *old_link_link);

AgsChannel* ags_channel_get_level(AgsChannel *channel);
void ags_channel_recursive_reset_recall_id(AgsChannel *channel,
					   AgsRecallID *valid_recall_id, gboolean valid_set_up, gboolean valid_set_down,
					   AgsRecallID *invalid_recall_id, gboolean invalid_set_up, gboolean invalid_set_down);
void ags_channel_recursive_init(AgsChannel *channel,
				AgsRecallID *recall_id,
				gint stage, gint init_stage,
				gboolean init_up, gboolean init_down);
void ags_channel_recursive_run(AgsChannel *channel,
			       AgsRecallID *recall_id,
			       gint run_stage,
			       gboolean run_up, gboolean run_down,
			       gboolean current_level_only);
void ags_channel_recursive_cancel(AgsChannel *channel,
				  AgsRecallID *recall_id,
				  gboolean cancel_up, gboolean cancel_down);

AgsChannel* ags_channel_new(GObject *audio);

#endif /*__AGS_CHANNEL_H__*/
