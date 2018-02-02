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

#include <ags/audio/task/ags_init_channel.h>

#include <ags/audio/ags_playback_domain.h>
#include <ags/audio/ags_playback.h>
#include <ags/audio/ags_audio.h>
#include <ags/audio/ags_channel.h>
#include <ags/audio/ags_recall_id.h>
#include <ags/audio/ags_playback_domain.h>
#include <ags/audio/ags_playback.h>

void ags_init_channel_class_init(AgsInitChannelClass *init_channel);
void ags_init_channel_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_init_channel_init(AgsInitChannel *init_channel);
void ags_init_channel_set_property(GObject *gobject,
				   guint prop_id,
				   const GValue *value,
				   GParamSpec *param_spec);
void ags_init_channel_get_property(GObject *gobject,
				   guint prop_id,
				   GValue *value,
				   GParamSpec *param_spec);
void ags_init_channel_connect(AgsConnectable *connectable);
void ags_init_channel_disconnect(AgsConnectable *connectable);
void ags_init_channel_dispose(GObject *gobject);
void ags_init_channel_finalize(GObject *gobject);

void ags_init_channel_launch(AgsTask *task);

/**
 * SECTION:ags_init_channel
 * @short_description: init channel task
 * @title: AgsInitChannel
 * @section_id:
 * @include: ags/audio/task/ags_init_channel.h
 *
 * The #AgsInitChannel task inits #AgsChannel.
 */

static gpointer ags_init_channel_parent_class = NULL;
static AgsConnectableInterface *ags_init_channel_parent_connectable_interface;

enum{
  PROP_0,
  PROP_CHANNEL,
  PROP_PLAY_PAD,
  PROP_DO_PLAYBACK,
  PROP_DO_SEQUENCER,
  PROP_DO_NOTATION,
};

GType
ags_init_channel_get_type()
{
  static GType ags_type_init_channel = 0;

  if(!ags_type_init_channel){
    static const GTypeInfo ags_init_channel_info = {
      sizeof (AgsInitChannelClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_init_channel_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsInitChannel),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_init_channel_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_init_channel_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_init_channel = g_type_register_static(AGS_TYPE_TASK,
						   "AgsInitChannel",
						   &ags_init_channel_info,
						   0);

    g_type_add_interface_static(ags_type_init_channel,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
  }
  
  return (ags_type_init_channel);
}

void
ags_init_channel_class_init(AgsInitChannelClass *init_channel)
{
  GObjectClass *gobject;
  AgsTaskClass *task;
  GParamSpec *param_spec;

  ags_init_channel_parent_class = g_type_class_peek_parent(init_channel);

  /* GObjectClass */
  gobject = (GObjectClass *) init_channel;

  gobject->set_property = ags_init_channel_set_property;
  gobject->get_property = ags_init_channel_get_property;

  gobject->dispose = ags_init_channel_dispose;
  gobject->finalize = ags_init_channel_finalize;

  /* properties */
  /**
   * AgsInitChannel:channel:
   *
   * The assigned #AgsChannel
   * 
   * Since: 1.0.0
   */
  param_spec = g_param_spec_object("channel",
				   "channel of init channel",
				   "The channel of init channel task",
				   AGS_TYPE_CHANNEL,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_CHANNEL,
				  param_spec);

  /**
   * AgsInitChannel:play-pad:
   *
   * Do play pad.
   * 
   * Since: 1.0.0
   */
  param_spec =  g_param_spec_boolean("play-pad",
				     "play pad",
				     "play pad of channel",
				     FALSE,
				     G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_PLAY_PAD,
				  param_spec);

  /**
   * AgsInitChannel:do-playback:
   *
   * The effects do-playback.
   * 
   * Since: 1.0.0
   */
  param_spec =  g_param_spec_boolean("do-playback",
				     "do playback",
				     "Do playback of channel",
				     FALSE,
				     G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_DO_PLAYBACK,
				  param_spec);

  /**
   * AgsInitChannel:do-sequencer:
   *
   * The effects do-sequencer.
   * 
   * Since: 1.0.0
   */
  param_spec =  g_param_spec_boolean("do-sequencer",
				     "do sequencer",
				     "Do sequencer of channel",
				     FALSE,
				     G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_DO_SEQUENCER,
				  param_spec);

  /**
   * AgsInitChannel:do-notation:
   *
   * The effects do-notation.
   * 
   * Since: 1.0.0
   */
  param_spec =  g_param_spec_boolean("do-notation",
				     "do notation",
				     "Do notation of channel",
				     FALSE,
				     G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_DO_NOTATION,
				  param_spec);

  /* AgsTaskClass */
  task = (AgsTaskClass *) init_channel;

  task->launch = ags_init_channel_launch;
}

void
ags_init_channel_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_init_channel_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_init_channel_connect;
  connectable->disconnect = ags_init_channel_disconnect;
}

void
ags_init_channel_init(AgsInitChannel *init_channel)
{
  init_channel->channel = NULL;
  init_channel->play_pad = FALSE;

  init_channel->do_playback = FALSE;
  init_channel->do_sequencer = FALSE;
  init_channel->do_notation = FALSE;
}

void
ags_init_channel_set_property(GObject *gobject,
			      guint prop_id,
			      const GValue *value,
			      GParamSpec *param_spec)
{
  AgsInitChannel *init_channel;

  init_channel = AGS_INIT_CHANNEL(gobject);

  switch(prop_id){
  case PROP_CHANNEL:
    {
      AgsChannel *channel;

      channel = (AgsChannel *) g_value_get_object(value);

      if(init_channel->channel == (GObject *) channel){
	return;
      }

      if(init_channel->channel != NULL){
	g_object_unref(init_channel->channel);
      }

      if(channel != NULL){
	g_object_ref(channel);
      }

      init_channel->channel = (GObject *) channel;
    }
    break;
  case PROP_PLAY_PAD:
    {
      gboolean play_pad;

      play_pad = g_value_get_boolean(value);

      init_channel->play_pad = play_pad;
    }
    break;
  case PROP_DO_PLAYBACK:
    {
      gboolean do_playback;

      do_playback = g_value_get_boolean(value);

      init_channel->do_playback = do_playback;
    }
    break;
  case PROP_DO_SEQUENCER:
    {
      gboolean do_sequencer;

      do_sequencer = g_value_get_boolean(value);

      init_channel->do_sequencer = do_sequencer;
    }
    break;
  case PROP_DO_NOTATION:
    {
      gboolean do_notation;

      do_notation = g_value_get_boolean(value);

      init_channel->do_notation = do_notation;
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_init_channel_get_property(GObject *gobject,
			      guint prop_id,
			      GValue *value,
			      GParamSpec *param_spec)
{
  AgsInitChannel *init_channel;

  init_channel = AGS_INIT_CHANNEL(gobject);

  switch(prop_id){
  case PROP_CHANNEL:
    {
      g_value_set_object(value, init_channel->channel);
    }
    break;
  case PROP_PLAY_PAD:
    {
      g_value_set_boolean(value, init_channel->play_pad);
    }
    break;
  case PROP_DO_PLAYBACK:
    {
      g_value_set_boolean(value, init_channel->do_playback);
    }
    break;
  case PROP_DO_SEQUENCER:
    {
      g_value_set_boolean(value, init_channel->do_sequencer);
    }
    break;
  case PROP_DO_NOTATION:
    {
      g_value_set_boolean(value, init_channel->do_notation);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_init_channel_connect(AgsConnectable *connectable)
{
  ags_init_channel_parent_connectable_interface->connect(connectable);

  /* empty */
}

void
ags_init_channel_disconnect(AgsConnectable *connectable)
{
  ags_init_channel_parent_connectable_interface->disconnect(connectable);

  /* empty */
}

void
ags_init_channel_dispose(GObject *gobject)
{
  AgsInitChannel *init_channel;

  init_channel = AGS_INIT_CHANNEL(gobject);

  if(init_channel->channel != NULL){
    g_object_unref(init_channel->channel);

    init_channel->channel = NULL;
  }
  
  /* call parent */
  G_OBJECT_CLASS(ags_init_channel_parent_class)->dispose(gobject);
}

void
ags_init_channel_finalize(GObject *gobject)
{
  AgsInitChannel *init_channel;

  init_channel = AGS_INIT_CHANNEL(gobject);

  if(init_channel->channel != NULL){
    g_object_unref(init_channel->channel);
  }

  /* call parent */
  G_OBJECT_CLASS(ags_init_channel_parent_class)->finalize(gobject);
}

void
ags_init_channel_launch(AgsTask *task)
{
  AgsAudio *audio;
  AgsChannel *channel, *current;
  AgsPlaybackDomain *playback_domain;
  AgsPlayback *playback;
  AgsRecallID *recall_id;
  
  AgsInitChannel *init_channel;

  AgsMutexManager *mutex_manager;

  GList *list, *list_start;
  GList *start_queue;

  gint stage;
  guint nth_domain;
  gboolean arrange_recall_id, duplicate_templates, resolve_dependencies;
  gboolean init_playback, init_sequencer, init_notation;
  
  pthread_mutex_t *application_mutex;
  pthread_mutex_t *audio_mutex;
  pthread_mutex_t *channel_mutex;
  pthread_mutex_t *current_mutex;

  mutex_manager = ags_mutex_manager_get_instance();
  application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);

  init_channel = AGS_INIT_CHANNEL(task);

  channel = init_channel->channel;

  /* get channel mutex */
  pthread_mutex_lock(application_mutex);

  channel_mutex = ags_mutex_manager_lookup(mutex_manager,
					 (GObject *) channel);
  
  pthread_mutex_unlock(application_mutex);

  /* get some fields */
  pthread_mutex_lock(channel_mutex);

  audio = channel->audio;

  pthread_mutex_unlock(channel_mutex);

  /* get audio mutex */
  pthread_mutex_lock(application_mutex);

  audio_mutex = ags_mutex_manager_lookup(mutex_manager,
					 (GObject *) audio);
  
  pthread_mutex_unlock(application_mutex);
  
  /* get some fields */
  pthread_mutex_lock(audio_mutex);

  playback_domain = (AgsPlaybackDomain *) audio->playback_domain;

  pthread_mutex_unlock(audio_mutex);
  
  list = NULL;
  list_start = NULL;

  start_queue = NULL;

  /* init channel */
  init_playback = FALSE;
  init_sequencer = FALSE;
  init_notation = FALSE;
  
  if(init_channel->play_pad){
    AgsChannel *next_pad;

    /* get some fields */
    pthread_mutex_lock(channel_mutex);
    
    next_pad = channel->next_pad;

    pthread_mutex_unlock(channel_mutex);

    for(stage = 0; stage < 4; stage++){
      current = channel;
      list = list_start;
      
      if(stage == 0){
	arrange_recall_id = TRUE;
	duplicate_templates = TRUE;
	resolve_dependencies = TRUE;
      }else{
	arrange_recall_id = FALSE;
	duplicate_templates = FALSE;
	resolve_dependencies = FALSE;

	if(list == NULL){
	  break;
	}
      }
      
      while(current != next_pad){	
	/* get current mutex */
	pthread_mutex_lock(application_mutex);

	current_mutex = ags_mutex_manager_lookup(mutex_manager,
						 (GObject *) current);
  
	pthread_mutex_unlock(application_mutex);

	/* get some fields */
	pthread_mutex_lock(current_mutex);

	playback = current->playback;

	pthread_mutex_unlock(current_mutex);

	if(stage == 0){
	  if(init_channel->do_playback &&
	     (AGS_PLAYBACK_PLAYBACK & (g_atomic_int_get(&(playback->flags)))) == 0){
	    /* recursive play init */
	    nth_domain = 0;
	    init_playback = TRUE;

	    g_atomic_int_or(&(playback->flags),
			    AGS_PLAYBACK_PLAYBACK);

	    recall_id = ags_channel_recursive_play_init(current, stage,
							arrange_recall_id, duplicate_templates,
							TRUE, FALSE, FALSE,
							resolve_dependencies,
							NULL);

	    list_start = g_list_append(list_start,
				       recall_id);

	    /* start queue */
	    pthread_mutex_lock(current_mutex);
	    
	    playback->recall_id[AGS_PLAYBACK_SCOPE_PLAYBACK] = recall_id;

	    start_queue = g_list_prepend(start_queue,
					 playback->channel_thread[AGS_PLAYBACK_SCOPE_PLAYBACK]);

	    pthread_mutex_unlock(current_mutex);
	  }
	  
	  if(init_channel->do_sequencer &&
	     (AGS_PLAYBACK_SEQUENCER & (g_atomic_int_get(&(playback->flags)))) == 0){
	    /* recursive play init */
	    nth_domain = 1;
	    init_sequencer = TRUE;

	    g_atomic_int_or(&(playback->flags),
			      AGS_PLAYBACK_SEQUENCER);

	    recall_id = ags_channel_recursive_play_init(current, stage,
							arrange_recall_id, duplicate_templates,
							FALSE, TRUE, FALSE,
							resolve_dependencies,
							NULL);
	    
	    list_start = g_list_append(list_start,
				       recall_id);
	    
	    /* start queue */
	    pthread_mutex_lock(current_mutex);

	    playback->recall_id[AGS_PLAYBACK_SCOPE_SEQUENCER] = recall_id;

	    start_queue = g_list_prepend(start_queue,
					 playback->channel_thread[AGS_PLAYBACK_SCOPE_SEQUENCER]);

	    pthread_mutex_unlock(current_mutex);
	  }
	  
	  if(init_channel->do_notation &&
	     (AGS_PLAYBACK_NOTATION & (g_atomic_int_get(&(playback->flags)))) == 0){
	    nth_domain = 2;
	    init_notation = TRUE;

	    g_atomic_int_or(&(playback->flags),
			    AGS_PLAYBACK_NOTATION);

	    recall_id = ags_channel_recursive_play_init(current, stage,
							arrange_recall_id, duplicate_templates,
							FALSE, FALSE, TRUE,
							resolve_dependencies,
							NULL);
	    
	    list_start = g_list_append(list_start,
				       recall_id);
	    
	    /* start queue */
	    pthread_mutex_lock(current_mutex);

	    AGS_PLAYBACK(current->playback)->recall_id[AGS_PLAYBACK_SCOPE_NOTATION] = recall_id;

	    start_queue = g_list_prepend(start_queue,
					 playback->channel_thread[AGS_PLAYBACK_SCOPE_NOTATION]);

	    pthread_mutex_unlock(current_mutex);
	  }
	}else{
	  if(init_channel->do_playback &&
	     init_playback){
	    nth_domain = 0;

	    ags_channel_recursive_play_init(current, stage,
					    arrange_recall_id, duplicate_templates,
					    TRUE, FALSE, FALSE,
					    resolve_dependencies,
					    AGS_RECALL_ID(list->data));
	  }

	  if(init_channel->do_sequencer &&
	     init_sequencer){
	    nth_domain = 1;
	    
	    ags_channel_recursive_play_init(current, stage,
					    arrange_recall_id, duplicate_templates,
					    FALSE, TRUE, FALSE,
					    resolve_dependencies,
					    AGS_RECALL_ID(list->data));
	  }

	  if(init_channel->do_notation &&
	     init_notation){
	    nth_domain = 2;
	    
	    ags_channel_recursive_play_init(current, stage,
					    arrange_recall_id, duplicate_templates,
					    FALSE, FALSE, TRUE,
					    resolve_dependencies,
					    AGS_RECALL_ID(list->data));	
	  }

	  list = list->next;
	}

	current = current->next;
      }
    }
    
    g_list_free(list_start);
  }else{
    AgsRecallID *recall_id;

    for(stage = 0; stage < 4; stage++){
      current = channel;

      /* get current mutex */
      pthread_mutex_lock(application_mutex);

      current_mutex = ags_mutex_manager_lookup(mutex_manager,
					       (GObject *) current);
  
      pthread_mutex_unlock(application_mutex);

      /* get some fields */
      pthread_mutex_lock(current_mutex);

      playback = current->playback;

      pthread_mutex_unlock(current_mutex);

      /* set flags */
      list = list_start;
      
      if(stage == 0){
	arrange_recall_id = TRUE;
	duplicate_templates = TRUE;
	resolve_dependencies = TRUE;
      }else{
	arrange_recall_id = FALSE;
	duplicate_templates = FALSE;
	resolve_dependencies = FALSE;

	if(list == NULL){
	  break;
	}
      }
      
      if(stage == 0){
	recall_id = NULL;
	
	if(init_channel->do_playback &&
	   (AGS_PLAYBACK_PLAYBACK & (g_atomic_int_get(&(playback->flags)))) == 0 &&
	   playback->recall_id[0] == NULL){
	  /* recursive play init */
	  nth_domain = 0;
	  init_playback = TRUE;
	  
	  g_atomic_int_or(&(playback->flags),
			  AGS_PLAYBACK_PLAYBACK);
	  
	  recall_id = ags_channel_recursive_play_init(current, stage,
						      arrange_recall_id, duplicate_templates,
						      TRUE, FALSE, FALSE,
						      resolve_dependencies,
						      NULL);
	  list_start = g_list_append(list_start,
				     recall_id);

	  /* start queue */
	  pthread_mutex_lock(current_mutex);
	  
	  playback->recall_id[AGS_PLAYBACK_SCOPE_PLAYBACK] = recall_id;

	  start_queue = g_list_prepend(start_queue,
				       playback->channel_thread[AGS_PLAYBACK_SCOPE_PLAYBACK]);

	  pthread_mutex_unlock(current_mutex);
	}
	  
	if(init_channel->do_sequencer &&
	   (AGS_PLAYBACK_SEQUENCER & (g_atomic_int_get(&(playback->flags)))) == 0 &&
	   playback->recall_id[1] == NULL){
	  nth_domain = 1;
	  init_sequencer = TRUE;
	    
	  g_atomic_int_or(&(playback->flags),
			  AGS_PLAYBACK_SEQUENCER);

	  recall_id = ags_channel_recursive_play_init(current, stage,
						      arrange_recall_id, duplicate_templates,
						      FALSE, TRUE, FALSE,
						      resolve_dependencies,
						      NULL);
	  list_start = g_list_append(list_start,
				     recall_id);

	  playback->recall_id[AGS_PLAYBACK_SCOPE_SEQUENCER] = recall_id;

	  start_queue = g_list_prepend(start_queue,
				       playback->channel_thread[AGS_PLAYBACK_SCOPE_SEQUENCER]);
	}
	  
	if(init_channel->do_notation &&
	   (AGS_PLAYBACK_NOTATION & (g_atomic_int_get(&(playback->flags)))) == 0 &&
	   AGS_PLAYBACK(current->playback)->recall_id[0] == NULL){
	  nth_domain = 2;
	  init_notation = TRUE;
	  
	  g_atomic_int_or(&(playback->flags),
			  AGS_PLAYBACK_NOTATION);

	  recall_id = ags_channel_recursive_play_init(current, stage,
						      arrange_recall_id, duplicate_templates,
						      FALSE, FALSE, TRUE,
						      resolve_dependencies,
						      NULL);
	  list_start = g_list_append(list_start,
				     recall_id);

	  /* start queue */
	  pthread_mutex_lock(current_mutex);
	  
	  playback->recall_id[AGS_PLAYBACK_SCOPE_NOTATION] = recall_id;

	  start_queue = g_list_prepend(start_queue,
				       playback->channel_thread[AGS_PLAYBACK_SCOPE_NOTATION]);

	  pthread_mutex_unlock(current_mutex);
	}
      }else{
	if(init_channel->do_playback &&
	   init_playback){
	  ags_channel_recursive_play_init(current, stage,
					  arrange_recall_id, duplicate_templates,
					  TRUE, FALSE, FALSE,
					  resolve_dependencies,
					  AGS_RECALL_ID(list->data));

	  list = list->next;
	}

	if(init_channel->do_sequencer &&
	   init_sequencer){
	  ags_channel_recursive_play_init(current, stage,
					  arrange_recall_id, duplicate_templates,
					  FALSE, TRUE, FALSE,
					  resolve_dependencies,
					  AGS_RECALL_ID(list->data));
	  
	  list = list->next;
	}

	if(init_channel->do_notation &&
	   init_notation){
	  ags_channel_recursive_play_init(current, stage,
					  arrange_recall_id, duplicate_templates,
					  FALSE, FALSE, TRUE,
					  resolve_dependencies,
					  AGS_RECALL_ID(list->data));
	  
	  list = list->next;
	}
      }
    }

    g_list_free(list_start);
  }

  /*  */
  if(start_queue != NULL){
    if(g_atomic_pointer_get(&(AGS_THREAD(playback_domain->audio_thread[nth_domain])->start_queue)) != NULL){
      g_atomic_pointer_set(&(AGS_THREAD(playback_domain->audio_thread[nth_domain])->start_queue),
			   g_list_concat(start_queue,
					 g_atomic_pointer_get(&(AGS_THREAD(playback_domain->audio_thread[nth_domain])->start_queue))));
    }else{
      g_atomic_pointer_set(&(AGS_THREAD(playback_domain->audio_thread[nth_domain])->start_queue),
			   start_queue);
    }
  }
}

/**
 * ags_init_channel_new:
 * @channel: the #AgsChannel
 * @play_pad: %TRUE all channels of accordig pad are initialized
 * @do_playback: init playback
 * @do_sequencer: init sequencer
 * @do_notation: init notation
 *
 * Creates an #AgsInitChannel.
 *
 * Returns: an new #AgsInitChannel.
 *
 * Since: 1.0.0
 */
AgsInitChannel*
ags_init_channel_new(AgsChannel *channel, gboolean play_pad,
		     gboolean do_playback, gboolean do_sequencer, gboolean do_notation)
{
  AgsInitChannel *init_channel;

  init_channel = (AgsInitChannel *) g_object_new(AGS_TYPE_INIT_CHANNEL,
						 "channel", channel,
						 "play-pad", play_pad,
						 "do-playback", do_playback,
						 "do-sequencer", do_sequencer,
						 "do-notation", do_notation,
						 NULL);

  return(init_channel);
}
