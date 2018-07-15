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

#ifndef __AGS_RECALL_H__
#define __AGS_RECALL_H__

#include <glib.h>
#include <glib-object.h>

#include <pthread.h>

#include <ags/libags.h>

#include <ags/audio/ags_sound_enums.h>
#include <ags/audio/ags_port.h>
#include <ags/audio/ags_recall_id.h>
#include <ags/audio/ags_recall_dependency.h>

#define AGS_TYPE_RECALL                (ags_recall_get_type())
#define AGS_RECALL(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_RECALL, AgsRecall))
#define AGS_RECALL_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST((class), AGS_TYPE_RECALL, AgsRecallClass))
#define AGS_IS_RECALL(obj)             (G_TYPE_CHECK_INSTANCE_TYPE((obj), AGS_TYPE_RECALL))
#define AGS_IS_RECALL_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE((class), AGS_TYPE_RECALL))
#define AGS_RECALL_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS((obj), AGS_TYPE_RECALL, AgsRecallClass))

#define AGS_RECALL_HANDLER(handler)    ((AgsRecallHandler *)(handler))

#define AGS_RECALL_DEFAULT_VERSION "2.0.0"
#define AGS_RECALL_DEFAULT_BUILD_ID "Tue Feb  6 14:27:35 UTC 2018"

typedef struct _AgsRecall AgsRecall;
typedef struct _AgsRecallClass AgsRecallClass;
typedef struct _AgsRecallHandler AgsRecallHandler;

/**
 * AgsRecallFlags:
 * @AGS_RECALL_ADDED_TO_REGISTRY: the recall was added to registry, see #AgsConnectable::add_to_registry()
 * @AGS_RECALL_CONNECTED: indicates the port was connected by calling #AgsConnectable::connect()
 * @AGS_RECALL_TEMPLATE: is template
 * @AGS_RECALL_DEFAULT_TEMPLATE: 
 * @AGS_RECALL_HAS_OUTPUT_PORT: has output port
 * @AGS_RECALL_BYPASS: don't apply effect processing
 * @AGS_RECALL_INITIAL_RUN: initial run, first attack to audio data
 * 
 * Enum values to control the behavior or indicate internal state of #AgsRecall by
 * enable/disable as flags.
 */
typedef enum{
  AGS_RECALL_ADDED_TO_REGISTRY     = 1,
  AGS_RECALL_CONNECTED             = 1 <<  1,
  AGS_RECALL_TEMPLATE              = 1 <<  2,
  AGS_RECALL_DEFAULT_TEMPLATE      = 1 <<  3,
  AGS_RECALL_HAS_OUTPUT_PORT       = 1 <<  4,
  AGS_RECALL_BYPASS                = 1 <<  5,
  AGS_RECALL_INITIAL_RUN           = 1 <<  6,
}AgsRecallFlags;

/**
 * AgsRecallNotifyDependencyMode:
 * @AGS_RECALL_NOTIFY_RUN: notify dependency as calling run
 * @AGS_RECALL_NOTIFY_AUDIO: notify dependency audio
 * @AGS_RECALL_NOTIFY_AUDIO_RUN: notifiy dependency audio run
 * @AGS_RECALL_NOTIFY_CHANNEL: notifiy dependency channel
 * @AGS_RECALL_NOTIFY_CHANNEL_RUN: notifiy dependency channel run
 * @AGS_RECALL_NOTIFY_RECALL: notifiy dependency recall
 * 
 * Modes to notify of dependencies.
 */
typedef enum{
  AGS_RECALL_NOTIFY_RUN,
  AGS_RECALL_NOTIFY_AUDIO,
  AGS_RECALL_NOTIFY_AUDIO_RUN,
  AGS_RECALL_NOTIFY_CHANNEL,
  AGS_RECALL_NOTIFY_CHANNEL_RUN,
  AGS_RECALL_NOTIFY_RECALL,
}AgsRecallNotifyDependencyMode;

struct _AgsRecall
{
  GObject object;

  guint flags;
  guint ability_flags;
  guint behaviour_flags;
  gint sound_scope;
  guint staging_flags;
  guint state_flags;
  
  //  gboolean rt_safe; note replace by globals

  pthread_mutex_t *obj_mutex;
  pthread_mutexattr_t *obj_mutexattr;
  
  AgsUUID *uuid;

  gchar *version;
  gchar *build_id;

  gchar *name;

  gchar *filename;
  gchar *effect;
  guint effect_index;
  
  gchar *xml_type;
  
  GObject *recall_container;

  GObject *output_soundcard;
  gint output_soundcard_channel;
  
  GObject *input_soundcard;
  gint input_soundcard_channel;
  
  guint samplerate;
  guint buffer_size;
  guint format;

  guint pad;
  guint audio_channel;

  guint line;

  GList *port;
  GList *automation_port;
  
  AgsRecallID *recall_id;

  GList *recall_dependency;
  GList *recall_handler;  

  AgsRecall *parent;
  
  GType child_type;
  
  guint n_child_params;
  gchar **child_parameter_name;
  GValue *child_value;

  GList *children;  
};

struct _AgsRecallClass
{
  GObjectClass object;

  void (*resolve_dependency)(AgsRecall *recall);
  void (*check_rt_data)(AgsRecall *recall);
  
  void (*run_init_pre)(AgsRecall *recall);
  void (*run_init_inter)(AgsRecall *recall);
  void (*run_init_post)(AgsRecall *recall);
  
  void (*feed_input_queue)(AgsRecall *recall);
  void (*automate)(AgsRecall *recall);
  
  void (*run_pre)(AgsRecall *recall);
  void (*run_inter)(AgsRecall *recall);
  void (*run_post)(AgsRecall *recall);

  void (*do_feedback)(AgsRecall *recall);
  void (*feed_output_queue)(AgsRecall *recall);
  
  void (*stop_persistent)(AgsRecall *recall);
  void (*cancel)(AgsRecall *recall);
  void (*done)(AgsRecall *recall);

  AgsRecall* (*duplicate)(AgsRecall *recall,
			  AgsRecallID *recall_id,
			  guint *n_params, gchar **parameter_name, GValue *value);

  void (*notify_dependency)(AgsRecall *recall, guint dependency, gboolean increase);

  void (*child_added)(AgsRecall *recall, AgsRecall *child);
};

/**
 * AgsRecallHandler:
 * @signal_name: the signal to listen
 * @callback: the callback to use
 * @data: user data to pass
 * @handler: the handler id
 *
 * A #AgsRecallHandler-struct acts as a callback definition
 */
struct _AgsRecallHandler
{
  const gchar *signal_name;
  GCallback callback;
  GObject *data;
};

GType ags_recall_get_type();

pthread_mutex_t* ags_recall_get_class_mutex();

gboolean ags_recall_global_get_omit_event();
gboolean ags_recall_global_get_performance_mode();
gboolean ags_recall_global_get_rt_safe();

gboolean ags_recall_test_flags(AgsRecall *recall, guint flags);
void ags_recall_set_flags(AgsRecall *recall, guint flags);
void ags_recall_unset_flags(AgsRecall *recall, guint flags);

/* ability flags */
gboolean ags_recall_test_ability_flags(AgsRecall *recall, guint ability_flags);
void ags_recall_set_ability_flags(AgsRecall *recall, guint ability_flags);
void ags_recall_unset_ability_flags(AgsRecall *recall, guint ability_flags);

gboolean ags_recall_check_ability_flags(AgsRecall *recall, guint ability_flags);
gboolean ags_recall_match_ability_flags_to_scope(AgsRecall *recall, gint sound_scope);

/* behaviour flags */
gboolean ags_recall_test_behaviour_flags(AgsRecall *recall, guint behaviour_flags);
void ags_recall_set_behaviour_flags(AgsRecall *recall, guint behaviour_flags);
void ags_recall_unset_behaviour_flags(AgsRecall *recall, guint behaviour_flags);

gboolean ags_recall_check_behaviour_flags(AgsRecall *recall, guint behaviour_flags);

/* scope */
void ags_recall_set_sound_scope(AgsRecall *recall, gint sound_scope);

gboolean ags_recall_check_sound_scope(AgsRecall *recall, gint sound_scope);

/* staging flags */
gboolean ags_recall_test_staging_flags(AgsRecall *recall, guint behaviour_flags);
void ags_recall_set_staging_flags(AgsRecall *recall, guint staging_flags);
void ags_recall_unset_staging_flags(AgsRecall *recall, guint staging_flags);

gboolean ags_recall_check_staging_flags(AgsRecall *recall, guint staging_flags);

/* state flags */
gboolean ags_recall_test_state_flags(AgsRecall *recall, guint behaviour_flags);
void ags_recall_set_state_flags(AgsRecall *recall, guint state_flags);
void ags_recall_unset_state_flags(AgsRecall *recall, guint state_flags);

gboolean ags_recall_check_state_flags(AgsRecall *recall, guint state_flags);

/* children */
void ags_recall_set_recall_id(AgsRecall *recall, AgsRecallID *recall_id);

void ags_recall_add_recall_dependency(AgsRecall *recall, AgsRecallDependency *recall_dependency);
void ags_recall_remove_recall_dependency(AgsRecall *recall, AgsRecallDependency *recall_dependency);

void ags_recall_add_child(AgsRecall *parent, AgsRecall *child);
void ags_recall_remove_child(AgsRecall *recall, AgsRecall *child);

void ags_recall_handler_free(AgsRecallHandler *recall_handler);
AgsRecallHandler* ags_recall_handler_alloc(const gchar *signal_name,
					   GCallback callback,
					   GObject *data);

void ags_recall_add_recall_handler(AgsRecall *recall,
				   AgsRecallHandler *recall_handler);
void ags_recall_remove_recall_handler(AgsRecall *recall,
				      AgsRecallHandler *recall_handler);

/* soundcard */
void ags_recall_set_output_soundcard(AgsRecall *recall, GObject *output_soundcard);
void ags_recall_set_input_soundcard(AgsRecall *recall, GObject *input_soundcard);

/* presets */
void ags_recall_set_samplerate(AgsRecall *recall, guint samplerate);
void ags_recall_set_buffer_size(AgsRecall *recall, guint buffer_size);
void ags_recall_set_format(AgsRecall *recall, guint format);

/* events */
void ags_recall_resolve_dependency(AgsRecall *recall);
void ags_recall_check_rt_data(AgsRecall *recall);

void ags_recall_run_init_pre(AgsRecall *recall);
void ags_recall_run_init_inter(AgsRecall *recall);
void ags_recall_run_init_post(AgsRecall *recall);

void ags_recall_feed_input_queue(AgsRecall *recall);
void ags_recall_automate(AgsRecall *recall);

void ags_recall_run_pre(AgsRecall *recall);
void ags_recall_run_inter(AgsRecall *recall);
void ags_recall_run_post(AgsRecall *recall);

void ags_recall_do_feedback(AgsRecall *recall);
void ags_recall_feed_output_queue(AgsRecall *recall);

void ags_recall_stop_persistent(AgsRecall *recall);
void ags_recall_cancel(AgsRecall *recall);
void ags_recall_done(AgsRecall *recall);

AgsRecall* ags_recall_duplicate(AgsRecall *recall,
				AgsRecallID *recall_id,
				guint *n_params, gchar **parameter_name, GValue *value);

void ags_recall_notify_dependency(AgsRecall *recall, guint dependency, gboolean increase);
void ags_recall_child_added(AgsRecall *parent, AgsRecall *child);

/* query */
gboolean ags_recall_is_done(GList *recalls, GObject *recycling_context);

GList* ags_recall_get_by_effect(GList *list, gchar *filename, gchar *effect);
GList* ags_recall_find_recall_id_with_effect(GList *list, AgsRecallID *recall_id, gchar *filename, gchar *effect);

GList* ags_recall_find_type(GList *recall, GType type);
GList* ags_recall_find_template(GList *recall);
GList* ags_recall_template_find_type(GList *recall, GType type);
GList* ags_recall_template_find_all_type(GList *recall, ...);
GList* ags_recall_find_type_with_recycling_context(GList *recall, GType type, GObject *recycling_context);
GList* ags_recall_find_recycling_context(GList *recall, GObject *recycling_context);
GList* ags_recall_find_provider(GList *recall, GObject *provider);
GList* ags_recall_template_find_provider(GList *recall, GObject *provider);
GList* ags_recall_find_provider_with_recycling_context(GList *recall, GObject *provider, GObject *recycling_context);

/* control */
void ags_recall_lock_port(AgsRecall *recall);
void ags_recall_unlock_port(AgsRecall *recall);

/* instantiate */
AgsRecall* ags_recall_new();

#endif /*__AGS_RECALL_H__*/
