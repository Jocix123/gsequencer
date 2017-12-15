/* GSequencer - Advanced GTK Sequencer
 * Copyright (C) 2005-2017 Joël Krähemann
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

#include <ags/audio/task/ags_init_audio.h>

#include <ags/audio/ags_playback_domain.h>
#include <ags/audio/ags_playback.h>
#include <ags/audio/ags_audio.h>
#include <ags/audio/ags_channel.h>
#include <ags/audio/ags_recall_id.h>
#include <ags/audio/ags_playback_domain.h>
#include <ags/audio/ags_playback.h>

#include <ags/i18n.h>

void ags_init_audio_class_init(AgsInitAudioClass *init_audio);
void ags_init_audio_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_init_audio_init(AgsInitAudio *init_audio);
void ags_init_audio_set_property(GObject *gobject,
				 guint prop_id,
				 const GValue *value,
				 GParamSpec *param_spec);
void ags_init_audio_get_property(GObject *gobject,
				 guint prop_id,
				 GValue *value,
				 GParamSpec *param_spec);
void ags_init_audio_connect(AgsConnectable *connectable);
void ags_init_audio_disconnect(AgsConnectable *connectable);
void ags_init_audio_dispose(GObject *gobject);
void ags_init_audio_finalize(GObject *gobject);

void ags_init_audio_launch(AgsTask *task);

/**
 * SECTION:ags_init_audio
 * @short_description: init audio task
 * @title: AgsInitAudio
 * @section_id:
 * @include: ags/audio/task/ags_init_audio.h
 *
 * The #AgsInitAudio task inits #AgsAudio.
 */

static gpointer ags_init_audio_parent_class = NULL;
static AgsConnectableInterface *ags_init_audio_parent_connectable_interface;

enum{
  PROP_0,
  PROP_AUDIO,
  PROP_DO_PLAYBACK,
  PROP_DO_SEQUENCER,
  PROP_DO_NOTATION,
};

GType
ags_init_audio_get_type()
{
  static GType ags_type_init_audio = 0;

  if(!ags_type_init_audio){
    static const GTypeInfo ags_init_audio_info = {
      sizeof (AgsInitAudioClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_init_audio_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsInitAudio),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_init_audio_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_init_audio_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_init_audio = g_type_register_static(AGS_TYPE_TASK,
						 "AgsInitAudio",
						 &ags_init_audio_info,
						 0);

    g_type_add_interface_static(ags_type_init_audio,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
  }
  
  return (ags_type_init_audio);
}

void
ags_init_audio_class_init(AgsInitAudioClass *init_audio)
{
  GObjectClass *gobject;
  AgsTaskClass *task;
  GParamSpec *param_spec;
  
  ags_init_audio_parent_class = g_type_class_peek_parent(init_audio);

  /* gobject */
  gobject = (GObjectClass *) init_audio;

  gobject->set_property = ags_init_audio_set_property;
  gobject->get_property = ags_init_audio_get_property;

  gobject->dispose = ags_init_audio_dispose;
  gobject->finalize = ags_init_audio_finalize;

  /* properties */
  /**
   * AgsInitAudio:audio:
   *
   * The assigned #AgsAudio
   * 
   * Since: 1.0.0
   */
  param_spec = g_param_spec_object("audio",
				   i18n_pspec("audio of init audio"),
				   i18n_pspec("The audio of init audio task"),
				   AGS_TYPE_AUDIO,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_AUDIO,
				  param_spec);

  /**
   * AgsInitAudio:do-playback:
   *
   * The effects do-playback.
   * 
   * Since: 1.0.0
   */
  param_spec =  g_param_spec_boolean("do-playback",
				     i18n_pspec("do playback"),
				     i18n_pspec("Do playback of audio"),
				     FALSE,
				     G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_DO_PLAYBACK,
				  param_spec);

  /**
   * AgsInitAudio:do-sequencer:
   *
   * The effects do-sequencer.
   * 
   * Since: 1.0.0
   */
  param_spec =  g_param_spec_boolean("do-sequencer",
				     i18n_pspec("do sequencer"),
				     i18n_pspec("Do sequencer of audio"),
				     FALSE,
				     G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_DO_SEQUENCER,
				  param_spec);

  /**
   * AgsInitAudio:do-notation:
   *
   * The effects do-notation.
   * 
   * Since: 1.0.0
   */
  param_spec =  g_param_spec_boolean("do-notation",
				     i18n_pspec("do notation"),
				     i18n_pspec("Do notation of audio"),
				     FALSE,
				     G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_DO_NOTATION,
				  param_spec);

  /* task */
  task = (AgsTaskClass *) init_audio;

  task->launch = ags_init_audio_launch;
}

void
ags_init_audio_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_init_audio_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_init_audio_connect;
  connectable->disconnect = ags_init_audio_disconnect;
}

void
ags_init_audio_init(AgsInitAudio *init_audio)
{
  init_audio->audio = NULL;

  init_audio->do_playback = FALSE;
  init_audio->do_sequencer = FALSE;
  init_audio->do_notation = FALSE;
}

void
ags_init_audio_set_property(GObject *gobject,
			    guint prop_id,
			    const GValue *value,
			    GParamSpec *param_spec)
{
  AgsInitAudio *init_audio;

  init_audio = AGS_INIT_AUDIO(gobject);

  switch(prop_id){
  case PROP_AUDIO:
    {
      AgsAudio *audio;

      audio = (AgsAudio *) g_value_get_object(value);

      if(init_audio->audio == (GObject *) audio){
	return;
      }

      if(init_audio->audio != NULL){
	g_object_unref(init_audio->audio);
      }

      if(audio != NULL){
	g_object_ref(audio);
      }

      init_audio->audio = (GObject *) audio;
    }
    break;
  case PROP_DO_PLAYBACK:
    {
      gboolean do_playback;

      do_playback = g_value_get_boolean(value);

      init_audio->do_playback = do_playback;
    }
    break;
  case PROP_DO_SEQUENCER:
    {
      gboolean do_sequencer;

      do_sequencer = g_value_get_boolean(value);

      init_audio->do_sequencer = do_sequencer;
    }
    break;
  case PROP_DO_NOTATION:
    {
      gboolean do_notation;

      do_notation = g_value_get_boolean(value);

      init_audio->do_notation = do_notation;
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_init_audio_get_property(GObject *gobject,
			    guint prop_id,
			    GValue *value,
			    GParamSpec *param_spec)
{
  AgsInitAudio *init_audio;

  init_audio = AGS_INIT_AUDIO(gobject);

  switch(prop_id){
  case PROP_AUDIO:
    {
      g_value_set_object(value, init_audio->audio);
    }
    break;
  case PROP_DO_PLAYBACK:
    {
      g_value_set_boolean(value, init_audio->do_playback);
    }
    break;
  case PROP_DO_SEQUENCER:
    {
      g_value_set_boolean(value, init_audio->do_sequencer);
    }
    break;
  case PROP_DO_NOTATION:
    {
      g_value_set_boolean(value, init_audio->do_notation);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_init_audio_connect(AgsConnectable *connectable)
{
  ags_init_audio_parent_connectable_interface->connect(connectable);

  /* empty */
}

void
ags_init_audio_disconnect(AgsConnectable *connectable)
{
  ags_init_audio_parent_connectable_interface->disconnect(connectable);

  /* empty */
}

void
ags_init_audio_dispose(GObject *gobject)
{
  AgsInitAudio *init_audio;
  
  init_audio = AGS_INIT_AUDIO(gobject);

  if(init_audio->audio != NULL){
    g_object_unref(init_audio->audio);
    
    init_audio->audio = NULL;
  }
  
  /* call parent */
  G_OBJECT_CLASS(ags_init_audio_parent_class)->dispose(gobject);
}

void
ags_init_audio_finalize(GObject *gobject)
{
  AgsInitAudio *init_audio;
  
  init_audio = AGS_INIT_AUDIO(gobject);

  if(init_audio->audio != NULL){
    g_object_unref(init_audio->audio);    
  }

  /* call parent */
  G_OBJECT_CLASS(ags_init_audio_parent_class)->finalize(gobject);
}

void
ags_init_audio_launch(AgsTask *task)
{
  AgsAudio *audio;
  AgsPlaybackDomain *playback_domain;
  AgsRecallID *recall_id;

  AgsInitAudio *init_audio;

  AgsMutexManager *mutex_manager;

  GList *playback_start, *playback;
  GList *list, *list_start;

  pthread_mutex_t *application_mutex;
  pthread_mutex_t *audio_mutex;

  mutex_manager = ags_mutex_manager_get_instance();
  application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);
  
  init_audio = AGS_INIT_AUDIO(task);

  audio = init_audio->audio;

  /* get audio mutex */
  pthread_mutex_lock(application_mutex);

  audio_mutex = ags_mutex_manager_lookup(mutex_manager,
					 (GObject *) audio);
  
  pthread_mutex_unlock(application_mutex);

  /* get some fields */
  pthread_mutex_lock(audio_mutex);

  playback_domain = AGS_PLAYBACK_DOMAIN(audio->playback_domain);

  pthread_mutex_unlock(audio_mutex);

  /* init audio */
  if(init_audio->do_playback){    
    g_atomic_int_or(&(playback_domain->flags),
		    AGS_PLAYBACK_DOMAIN_PLAYBACK);

    g_object_get(playback_domain,
		 "playback", &playback_start,
		 NULL);
    
    list_start = 
      list = ags_audio_recursive_play_init(audio,
					   TRUE, FALSE, FALSE);

    playback = playback_start;
    
    while(playback != NULL){
      //      AGS_PLAYBACK(playback->data)->recall_id[0] = list->data;
      g_atomic_int_or(&(AGS_PLAYBACK(playback->data)->flags),
		      AGS_PLAYBACK_PLAYBACK);

      playback = playback->next;
    }

    g_list_free(playback_start);
    g_list_free(list_start);
  }

  if(init_audio->do_sequencer &&
     (AGS_PLAYBACK_DOMAIN_SEQUENCER & (g_atomic_int_get(&(playback_domain->flags)))) == 0){
    g_atomic_int_or(&(playback_domain->flags),
		    AGS_PLAYBACK_DOMAIN_SEQUENCER);

    g_object_get(playback_domain,
		 "playback", &playback_start,
		 NULL);

    list_start = 
      list = ags_audio_recursive_play_init(audio,
					   FALSE, TRUE, FALSE);

    playback = playback_start;

    while(playback != NULL){
      //      AGS_PLAYBACK(playback->data)->recall_id[1] = list->data;
      g_atomic_int_or(&(AGS_PLAYBACK(playback->data)->flags),
		      AGS_PLAYBACK_SEQUENCER);
      
      playback = playback->next;
    }

    g_list_free(playback_start);
    g_list_free(list_start);
  }
  
  if(init_audio->do_notation &&
     (AGS_PLAYBACK_DOMAIN_NOTATION & (g_atomic_int_get(&(playback_domain->flags)))) == 0){
    g_atomic_int_or(&(playback_domain->flags),
		    AGS_PLAYBACK_DOMAIN_NOTATION);

    g_object_get(playback_domain,
		 "playback", &playback_start,
		 NULL);

    list_start = 
      list = ags_audio_recursive_play_init(audio,
					   FALSE, FALSE, TRUE);

    playback = playback_start;

    while(playback != NULL){
      //      AGS_PLAYBACK(playback->data)->recall_id[2] = list->data;
      g_atomic_int_or(&(AGS_PLAYBACK(playback->data)->flags),
		      AGS_PLAYBACK_NOTATION);

      playback = playback->next;
    }

    g_list_free(playback_start);
    g_list_free(list_start);
  }
}

/**
 * ags_init_audio_new:
 * @audio: the #AgsAudio
 * @do_playback: init playback
 * @do_sequencer: init sequencer
 * @do_notation: init notation
 *
 * Creates an #AgsInitAudio.
 *
 * Returns: an new #AgsInitAudio.
 *
 * Since: 1.0.0
 */
AgsInitAudio*
ags_init_audio_new(AgsAudio *audio,
		   gboolean do_playback, gboolean do_sequencer, gboolean do_notation)
{
  AgsInitAudio *init_audio;

  init_audio = (AgsInitAudio *) g_object_new(AGS_TYPE_INIT_AUDIO,
					     "audio", audio,
					     "do-playback", do_playback,
					     "do-sequencer", do_sequencer,
					     "do-notation", do_notation,
					     NULL);

  return(init_audio);
}
