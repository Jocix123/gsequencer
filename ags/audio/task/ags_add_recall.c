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

#include <ags/audio/task/ags_add_recall.h>

#include <ags/libags.h>

#include <ags/audio/ags_audio.h>
#include <ags/audio/ags_channel.h>
#include <ags/audio/ags_recall_audio.h>
#include <ags/audio/ags_recall_channel.h>

#include <ags/i18n.h>

void ags_add_recall_class_init(AgsAddRecallClass *add_recall);
void ags_add_recall_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_add_recall_init(AgsAddRecall *add_recall);
void ags_add_recall_set_property(GObject *gobject,
				 guint prop_id,
				 const GValue *value,
				 GParamSpec *param_spec);
void ags_add_recall_get_property(GObject *gobject,
				 guint prop_id,
				 GValue *value,
				 GParamSpec *param_spec);
void ags_add_recall_connect(AgsConnectable *connectable);
void ags_add_recall_disconnect(AgsConnectable *connectable);
void ags_add_recall_dispose(GObject *gobject);
void ags_add_recall_finalize(GObject *gobject);

void ags_add_recall_launch(AgsTask *task);

enum{
  PROP_0,
  PROP_CONTEXT,
  PROP_RECALL,
  PROP_IS_PLAY,
};

/**
 * SECTION:ags_add_recall
 * @short_description: add recall object to context
 * @title: AgsAddRecall
 * @section_id:
 * @include: ags/audio/task/ags_add_recall.h
 *
 * The #AgsAddRecall task adds #AgsRecall to context.
 */

static gpointer ags_add_recall_parent_class = NULL;
static AgsConnectableInterface *ags_add_recall_parent_connectable_interface;

GType
ags_add_recall_get_type()
{
  static GType ags_type_add_recall = 0;

  if(!ags_type_add_recall){
    static const GTypeInfo ags_add_recall_info = {
      sizeof (AgsAddRecallClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_add_recall_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsAddRecall),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_add_recall_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_add_recall_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_add_recall = g_type_register_static(AGS_TYPE_TASK,
						 "AgsAddRecall",
						 &ags_add_recall_info,
						 0);

    g_type_add_interface_static(ags_type_add_recall,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
  }
  
  return (ags_type_add_recall);
}

void
ags_add_recall_class_init(AgsAddRecallClass *add_recall)
{
  GObjectClass *gobject;
  AgsTaskClass *task;
  GParamSpec *param_spec;

  ags_add_recall_parent_class = g_type_class_peek_parent(add_recall);

  /* gobject */
  gobject = (GObjectClass *) add_recall;

  gobject->set_property = ags_add_recall_set_property;
  gobject->get_property = ags_add_recall_get_property;

  gobject->dispose = ags_add_recall_dispose;
  gobject->finalize = ags_add_recall_finalize;

  /**
   * AgsAddRecall:context:
   *
   * The assigned context
   * 
   * Since: 1.0.0
   */
  param_spec = g_param_spec_object("context",
				   i18n_pspec("context of add recall"),
				   i18n_pspec("The context of add recall task"),
				   G_TYPE_OBJECT,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_CONTEXT,
				  param_spec);

  /**
   * AgsAddRecall:recall:
   *
   * The assigned #AgsRecall
   * 
   * Since: 1.0.0
   */
  param_spec = g_param_spec_object("recall",
				   i18n_pspec("recall of add recall"),
				   i18n_pspec("The recall of add recall task"),
				   AGS_TYPE_RECALL,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_RECALL,
				  param_spec);
  
  /**
   * AgsAddRecall:is-play:
   *
   * The recall's context is-play.
   * 
   * Since: 1.0.0
   */
  param_spec =  g_param_spec_boolean("is-play",
				     i18n_pspec("is play context"),
				     i18n_pspec("Add recall to play context"),
				     FALSE,
				     G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_IS_PLAY,
				  param_spec);

  /* task */
  task = (AgsTaskClass *) add_recall;

  task->launch = ags_add_recall_launch;
}

void
ags_add_recall_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_add_recall_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_add_recall_connect;
  connectable->disconnect = ags_add_recall_disconnect;
}

void
ags_add_recall_init(AgsAddRecall *add_recall)
{
  add_recall->context = NULL;
  add_recall->recall = NULL;
  add_recall->is_play = FALSE;
}

void
ags_add_recall_set_property(GObject *gobject,
			  guint prop_id,
			  const GValue *value,
			  GParamSpec *param_spec)
{
  AgsAddRecall *add_recall;

  add_recall = AGS_ADD_RECALL(gobject);

  switch(prop_id){
  case PROP_CONTEXT:
    {
      GObject *context;

      context = (GObject *) g_value_get_object(value);

      if(add_recall->context == (GObject *) context){
	return;
      }

      if(add_recall->context != NULL){
	g_object_unref(add_recall->context);
      }

      if(context != NULL){
	g_object_ref(context);
      }

      add_recall->context = (GObject *) context;
    }
    break;
  case PROP_RECALL:
    {
      AgsRecall *recall;

      recall = (AgsRecall *) g_value_get_object(value);

      if(add_recall->recall == (GObject *) recall){
	return;
      }

      if(add_recall->recall != NULL){
	g_object_unref(add_recall->recall);
      }

      if(recall != NULL){
	g_object_ref(recall);
      }

      add_recall->recall = (GObject *) recall;
    }
    break;
  case PROP_IS_PLAY:
    {
      add_recall->is_play = g_value_get_boolean(value);
    }
  break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_add_recall_get_property(GObject *gobject,
			  guint prop_id,
			  GValue *value,
			  GParamSpec *param_spec)
{
  AgsAddRecall *add_recall;

  add_recall = AGS_ADD_RECALL(gobject);

  switch(prop_id){
  case PROP_CONTEXT:
    {
      g_value_set_object(value, add_recall->context);
    }
    break;
  case PROP_RECALL:
    {
      g_value_set_object(value, add_recall->recall);
    }
    break;
  case PROP_IS_PLAY:
    {
      g_value_set_boolean(value, add_recall->is_play);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_add_recall_connect(AgsConnectable *connectable)
{
  ags_add_recall_parent_connectable_interface->connect(connectable);

  /* empty */
}

void
ags_add_recall_disconnect(AgsConnectable *connectable)
{
  ags_add_recall_parent_connectable_interface->disconnect(connectable);

  /* empty */
}

void
ags_add_recall_dispose(GObject *gobject)
{
  AgsAddRecall *add_recall;

  add_recall = AGS_ADD_RECALL(gobject);

  if(add_recall->context != NULL){
    g_object_unref(add_recall->context);

    add_recall->context = NULL;
  }

  if(add_recall->recall != NULL){
    g_object_unref(add_recall->recall);

    add_recall->recall = NULL;
  }
  
  /* call parent */
  G_OBJECT_CLASS(ags_add_recall_parent_class)->dispose(gobject);
}

void
ags_add_recall_finalize(GObject *gobject)
{
  AgsAddRecall *add_recall;

  add_recall = AGS_ADD_RECALL(gobject);

  if(add_recall->context != NULL){
    g_object_unref(add_recall->context);
  }

  if(add_recall->recall != NULL){
    g_object_unref(add_recall->recall);
  }

  /* call parent */
  G_OBJECT_CLASS(ags_add_recall_parent_class)->finalize(gobject);
}

void
ags_add_recall_launch(AgsTask *task)
{
  AgsAddRecall *add_recall;  

  AgsMutexManager *mutex_manager;

  pthread_mutex_t *application_mutex;
  pthread_mutex_t *audio_mutex;
  pthread_mutex_t *channel_mutex;
  
  /* get mutex manager and application mutex */
  mutex_manager = ags_mutex_manager_get_instance();
  application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);

  add_recall = AGS_ADD_RECALL(task);

  if(AGS_IS_AUDIO(add_recall->context)){
    AgsRecall *current;

    GList *recall_id;

    /* get audio mutex */
    pthread_mutex_lock(application_mutex);

    audio_mutex = ags_mutex_manager_lookup(mutex_manager,
					   (GObject *) add_recall->context);

    pthread_mutex_unlock(application_mutex);

    /* check container */
    if(g_list_find(AGS_AUDIO(add_recall->context)->recall_container,
		   add_recall->recall->container) == NULL){
      ags_audio_add_recall_container(AGS_AUDIO(add_recall->context),
				     add_recall->recall->container);
    }

    /* add */
    ags_audio_add_recall(AGS_AUDIO(add_recall->context),
			 (GObject *) add_recall->recall,
			 add_recall->is_play);

    /* set up playback, sequencer or notation */
    pthread_mutex_lock(audio_mutex);
    
    recall_id = AGS_AUDIO(add_recall->context)->recall_id;

    if(!AGS_IS_RECALL_AUDIO(add_recall->recall)){
      while(recall_id != NULL){
	if((!add_recall->is_play && AGS_RECALL_ID(recall_id->data)->recycling_context->parent == NULL) ||
	   (add_recall->is_play && AGS_RECALL_ID(recall_id->data)->recycling_context->parent != NULL)){
	  recall_id = recall_id->next;
	
	  continue;
	}

	current = ags_recall_duplicate(add_recall->recall, recall_id->data);

	/* set appropriate flag */
	if((AGS_RECALL_ID_PLAYBACK & (AGS_RECALL_ID(recall_id->data)->flags)) != 0){
	  ags_recall_set_flags(current, AGS_RECALL_PLAYBACK);
	}else if((AGS_RECALL_ID_SEQUENCER & (AGS_RECALL_ID(recall_id->data)->flags)) != 0){
	  ags_recall_set_flags(current, AGS_RECALL_SEQUENCER);
	}else if((AGS_RECALL_ID_NOTATION & (AGS_RECALL_ID(recall_id->data)->flags)) != 0){
	  ags_recall_set_flags(current, AGS_RECALL_NOTATION);
	}

	/* append to AgsAudio */
	if(add_recall->is_play)
	  AGS_AUDIO(add_recall->context)->play = g_list_append(AGS_AUDIO(add_recall->context)->play, current);
	else
	  AGS_AUDIO(add_recall->context)->recall = g_list_append(AGS_AUDIO(add_recall->context)->recall, current);

	/* connect */
	ags_connectable_connect(AGS_CONNECTABLE(current));

	/* notify run */
	ags_recall_notify_dependency(current, AGS_RECALL_NOTIFY_RUN, 1);

	/* resolve */
	ags_recall_resolve_dependencies(current);

	/* init */
	ags_dynamic_connectable_connect_dynamic(AGS_DYNAMIC_CONNECTABLE(current));
      
	current->flags &= (~AGS_RECALL_HIDE);
	ags_recall_run_init_pre(current);
	current->flags &= (~AGS_RECALL_REMOVE);
      
	ags_recall_run_init_inter(current);
	ags_recall_run_init_post(current);

	/* iterate */
	recall_id = recall_id->next;
      }
    }
    
    pthread_mutex_unlock(audio_mutex);
  }else if(AGS_IS_CHANNEL(add_recall->context)){
    AgsRecall *current;

    GList *recall_id;

    /* get channel mutex */
    pthread_mutex_lock(application_mutex);

    channel_mutex = ags_mutex_manager_lookup(mutex_manager,
					   (GObject *) add_recall->context);

    pthread_mutex_unlock(application_mutex);

    /* check container */
    if(g_list_find(AGS_CHANNEL(add_recall->context)->container,
		   add_recall->recall->container) == NULL){
      ags_channel_add_recall_container(AGS_CHANNEL(add_recall->context),
				       add_recall->recall->container);
    }

    /* add */
    ags_channel_add_recall(AGS_CHANNEL(add_recall->context),
			   (GObject *) add_recall->recall,
			   add_recall->is_play);

    /* set up playback, sequencer or notation */
    pthread_mutex_lock(channel_mutex);
    
    recall_id = AGS_CHANNEL(add_recall->context)->recall_id;

    if(!AGS_IS_RECALL_CHANNEL(add_recall->recall)){
      while(recall_id != NULL){
	if((!add_recall->is_play && AGS_RECALL_ID(recall_id->data)->recycling_context->parent == NULL) ||
	   (add_recall->is_play && AGS_RECALL_ID(recall_id->data)->recycling_context->parent != NULL)){
	  recall_id = recall_id->next;
	
	  continue;
	}
      
	current = ags_recall_duplicate(add_recall->recall, recall_id->data);

	/* set appropriate flag */
	if((AGS_RECALL_ID_PLAYBACK & (AGS_RECALL_ID(recall_id->data)->flags)) != 0){
	  ags_recall_set_flags(current, AGS_RECALL_PLAYBACK);
	}else if((AGS_RECALL_ID_SEQUENCER & (AGS_RECALL_ID(recall_id->data)->flags)) != 0){
	  ags_recall_set_flags(current, AGS_RECALL_SEQUENCER);
	}else if((AGS_RECALL_ID_NOTATION & (AGS_RECALL_ID(recall_id->data)->flags)) != 0){
	  ags_recall_set_flags(current, AGS_RECALL_NOTATION);
	}

	/* append to AgsChannel */
	if(add_recall->is_play)
	  AGS_CHANNEL(add_recall->context)->play = g_list_append(AGS_CHANNEL(add_recall->context)->play, current);
	else
	  AGS_CHANNEL(add_recall->context)->recall = g_list_append(AGS_CHANNEL(add_recall->context)->recall, current);

	/* connect */
	ags_connectable_connect(AGS_CONNECTABLE(current));

	/* notify run */
	ags_recall_notify_dependency(current, AGS_RECALL_NOTIFY_RUN, 1);

	/* resolve */
	ags_recall_resolve_dependencies(current);

	/* init */
	ags_dynamic_connectable_connect_dynamic(AGS_DYNAMIC_CONNECTABLE(current));
      
	current->flags &= (~AGS_RECALL_HIDE);
	ags_recall_run_init_pre(current);
	current->flags &= (~AGS_RECALL_REMOVE);
      
	ags_recall_run_init_inter(current);
	ags_recall_run_init_post(current);

	/* iterate */
	recall_id = recall_id->next;
      }

      pthread_mutex_unlock(channel_mutex);
    }
  }else if(AGS_IS_RECALL(add_recall->context)){
    ags_recall_add_child(AGS_RECALL(add_recall->context),
			 add_recall->recall);
  }

  ags_connectable_connect(AGS_CONNECTABLE(add_recall->recall));
}

/**
 * ags_add_recall_new:
 * @context: may be #AgsAudio, #AgsChannel or #AgsRecall
 * @recall: the #AgsRecall to add
 * @is_play: if %TRUE non-complex recall
 *
 * Creates an #AgsAddRecall.
 *
 * Returns: an new #AgsAddRecall.
 *
 * Since: 1.0.0
 */
AgsAddRecall*
ags_add_recall_new(GObject *context,
		   AgsRecall *recall,
		   gboolean is_play)
{
  AgsAddRecall *add_recall;

  add_recall = (AgsAddRecall *) g_object_new(AGS_TYPE_ADD_RECALL,
					     "context", context,
					     "recall", recall,
					     "is-play", is_play,
					     NULL);

  return(add_recall);
}
