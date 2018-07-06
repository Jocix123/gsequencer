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

#include <ags/audio/task/ags_remove_recall.h>

#include <ags/audio/ags_audio.h>
#include <ags/audio/ags_channel.h>
#include <ags/audio/ags_recall_container.h>
#include <ags/audio/ags_recall_audio_run.h>
#include <ags/audio/ags_recall_channel_run.h>

#include <math.h>

#include <ags/i18n.h>

void ags_remove_recall_class_init(AgsRemoveRecallClass *remove_recall);
void ags_remove_recall_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_remove_recall_init(AgsRemoveRecall *remove_recall);
void ags_remove_recall_set_property(GObject *gobject,
				    guint prop_id,
				    const GValue *value,
				    GParamSpec *param_spec);
void ags_remove_recall_get_property(GObject *gobject,
				    guint prop_id,
				    GValue *value,
				    GParamSpec *param_spec);
void ags_remove_recall_connect(AgsConnectable *connectable);
void ags_remove_recall_disconnect(AgsConnectable *connectable);
void ags_remove_recall_dispose(GObject *gobject);
void ags_remove_recall_finalize(GObject *gobject);

void ags_remove_recall_launch(AgsTask *task);

/**
 * SECTION:ags_remove_recall
 * @short_description: remove recall object from context
 * @title: AgsRemoveRecall
 * @section_id:
 * @include: ags/audio/task/ags_remove_recall.h
 *
 * The #AgsRemoveRecall task removes #AgsRecall from context.
 */

static gpointer ags_remove_recall_parent_class = NULL;
static AgsConnectableInterface *ags_remove_recall_parent_connectable_interface;

enum{
  PROP_0,
  PROP_CONTEXT,
  PROP_RECALL,
  PROP_IS_PLAY,
  PROP_REMOVE_ALL,
};

GType
ags_remove_recall_get_type()
{
  static volatile gsize g_define_type_id__volatile = 0;

  if(g_once_init_enter (&g_define_type_id__volatile)){
    GType ags_type_remove_recall;

    static const GTypeInfo ags_remove_recall_info = {
      sizeof (AgsRemoveRecallClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_remove_recall_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsRemoveRecall),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_remove_recall_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_remove_recall_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_remove_recall = g_type_register_static(AGS_TYPE_TASK,
						    "AgsRemoveRecall",
						    &ags_remove_recall_info,
						    0);

    g_type_add_interface_static(ags_type_remove_recall,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);

    g_once_init_leave (&g_define_type_id__volatile, ags_type_remove_recall);
  }

  return g_define_type_id__volatile;
}

void
ags_remove_recall_class_init(AgsRemoveRecallClass *remove_recall)
{
  GObjectClass *gobject;
  AgsTaskClass *task;
  GParamSpec *param_spec;

  ags_remove_recall_parent_class = g_type_class_peek_parent(remove_recall);

  /* GObjectClass */
  gobject = (GObjectClass *) remove_recall;

  gobject->set_property = ags_remove_recall_set_property;
  gobject->get_property = ags_remove_recall_get_property;

  gobject->finalize = ags_remove_recall_finalize;

  /**
   * AgsRemoveRecall:context:
   *
   * The assigned #AgsContext
   * 
   * Since: 1.0.0
   */
  param_spec = g_param_spec_object("context",
				   i18n_pspec("context of remove recall"),
				   i18n_pspec("The context of remove recall task"),
				   G_TYPE_OBJECT,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_CONTEXT,
				  param_spec);

  /**
   * AgsRemoveRecall:recall:
   *
   * The assigned #AgsRecall
   * 
   * Since: 1.0.0
   */
  param_spec = g_param_spec_object("recall",
				   i18n_pspec("recall of remove recall"),
				   i18n_pspec("The recall of remove recall task"),
				   AGS_TYPE_RECALL,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_RECALL,
				  param_spec);
  
  /**
   * AgsRemoveRecall:is-play:
   *
   * The recall's context is-play.
   * 
   * Since: 1.0.0
   */
  param_spec =  g_param_spec_boolean("is-play",
				     i18n_pspec("is play context"),
				     i18n_pspec("Remove recall to play context"),
				     FALSE,
				     G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_IS_PLAY,
				  param_spec);

  /**
   * AgsRemoveRecall:remove-all:
   *
   * Do remove-all related recalls.
   * 
   * Since: 1.0.0
   */
  param_spec =  g_param_spec_boolean("remove-all",
				     i18n_pspec("remove all"),
				     i18n_pspec("Remove all related recall"),
				     FALSE,
				     G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_REMOVE_ALL,
				  param_spec);

  /* AgsTaskClass */
  task = (AgsTaskClass *) remove_recall;

  task->launch = ags_remove_recall_launch;
}

void
ags_remove_recall_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_remove_recall_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_remove_recall_connect;
  connectable->disconnect = ags_remove_recall_disconnect;
}

void
ags_remove_recall_init(AgsRemoveRecall *remove_recall)
{
  remove_recall->context = NULL;
  remove_recall->recall = NULL;
  remove_recall->is_play = FALSE;
  remove_recall->remove_all = FALSE;
}

void
ags_remove_recall_set_property(GObject *gobject,
			       guint prop_id,
			       const GValue *value,
			       GParamSpec *param_spec)
{
  AgsRemoveRecall *remove_recall;

  remove_recall = AGS_REMOVE_RECALL(gobject);

  switch(prop_id){
  case PROP_CONTEXT:
    {
      GObject *context;

      context = (GObject *) g_value_get_object(value);

      if(remove_recall->context == (GObject *) context){
	return;
      }

      if(remove_recall->context != NULL){
	g_object_unref(remove_recall->context);
      }

      if(context != NULL){
	g_object_ref(context);
      }

      remove_recall->context = (GObject *) context;
    }
    break;
  case PROP_RECALL:
    {
      AgsRecall *recall;

      recall = (AgsRecall *) g_value_get_object(value);

      if(remove_recall->recall == (GObject *) recall){
	return;
      }

      if(remove_recall->recall != NULL){
	g_object_unref(remove_recall->recall);
      }

      if(recall != NULL){
	g_object_ref(recall);
      }

      remove_recall->recall = (GObject *) recall;
    }
    break;
  case PROP_IS_PLAY:
    {
      remove_recall->is_play = g_value_get_boolean(value);
    }
    break;
  case PROP_REMOVE_ALL:
    {
      remove_recall->remove_all = g_value_get_boolean(value);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_remove_recall_get_property(GObject *gobject,
			       guint prop_id,
			       GValue *value,
			       GParamSpec *param_spec)
{
  AgsRemoveRecall *remove_recall;

  remove_recall = AGS_REMOVE_RECALL(gobject);

  switch(prop_id){
  case PROP_CONTEXT:
    {
      g_value_set_object(value, remove_recall->context);
    }
    break;
  case PROP_RECALL:
    {
      g_value_set_object(value, remove_recall->recall);
    }
    break;
  case PROP_IS_PLAY:
    {
      g_value_set_boolean(value, remove_recall->is_play);
    }
    break;
  case PROP_REMOVE_ALL:
    {
      g_value_set_boolean(value, remove_recall->remove_all);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_remove_recall_connect(AgsConnectable *connectable)
{
  ags_remove_recall_parent_connectable_interface->connect(connectable);

  /* empty */
}

void
ags_remove_recall_disconnect(AgsConnectable *connectable)
{
  ags_remove_recall_parent_connectable_interface->disconnect(connectable);

  /* empty */
}

void
ags_remove_recall_dispose(GObject *gobject)
{
  AgsRemoveRecall *remove_recall;

  remove_recall = AGS_REMOVE_RECALL(gobject);

  if(remove_recall->context != NULL){
    g_object_unref(remove_recall->context);

    remove_recall->context = NULL;
  }

  if(remove_recall->recall != NULL){
    g_object_unref(remove_recall->recall);

    remove_recall->recall = NULL;
  }
  
  /* call parent */
  G_OBJECT_CLASS(ags_remove_recall_parent_class)->dispose(gobject);
}

void
ags_remove_recall_finalize(GObject *gobject)
{
  AgsRemoveRecall *remove_recall;

  remove_recall = AGS_REMOVE_RECALL(gobject);

  if(remove_recall->context != NULL){
    g_object_unref(remove_recall->context);
  }

  if(remove_recall->recall != NULL){
    g_object_unref(remove_recall->recall);
  }
  
  /* call parent */
  G_OBJECT_CLASS(ags_remove_recall_parent_class)->finalize(gobject);
}

void
ags_remove_recall_launch(AgsTask *task)
{
  AgsRemoveRecall *remove_recall;

  AgsRecallContainer *recall_container;
  
  AgsMutexManager *mutex_manager;

  pthread_mutex_t *application_mutex;
  pthread_mutex_t *mutex;

  /* get mutex manager and application mutex */
  mutex_manager = ags_mutex_manager_get_instance();
  application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);

  remove_recall = AGS_REMOVE_RECALL(task);

  if(AGS_IS_AUDIO(remove_recall->context)){
    if(remove_recall->remove_all){
      GList *list_start, *list;
      
      if(AGS_IS_RECALL_AUDIO_RUN(remove_recall->recall)){
	/* get audio mutex */
	pthread_mutex_lock(application_mutex);

	mutex = ags_mutex_manager_lookup(mutex_manager,
					 (GObject *) remove_recall->context);

	pthread_mutex_unlock(application_mutex);

	/* remove recall */
	pthread_mutex_lock(mutex);

	recall_container = remove_recall->recall->container;
	
	list_start =
	  list = g_list_copy(recall_container->recall_audio_run);

	pthread_mutex_unlock(mutex);
	
	while(list != NULL){
	  if(list->data != remove_recall->recall){
	    ags_audio_remove_recall(AGS_AUDIO(remove_recall->context),
				    (GObject *) list->data,
				    remove_recall->is_play);
	  }
	  
	  list = list->next;	  
	}

	g_list_free(list_start);
      }
    }

    ags_audio_remove_recall(AGS_AUDIO(remove_recall->context),
			    (GObject *) remove_recall->recall,
			    remove_recall->is_play);

  }else if(AGS_IS_CHANNEL(remove_recall->context)){
    if(remove_recall->remove_all){
      GList *list_start, *list;

      if(AGS_IS_RECALL_CHANNEL_RUN(remove_recall->recall)){
	/* get channel mutex */
	pthread_mutex_lock(application_mutex);

	mutex = ags_mutex_manager_lookup(mutex_manager,
					 (GObject *) remove_recall->context);

	pthread_mutex_unlock(application_mutex);

	/* remove recall */
	pthread_mutex_lock(mutex);

	recall_container = remove_recall->recall->container;
	
	list_start =
	  list = g_list_copy(recall_container->recall_channel_run);

	pthread_mutex_unlock(mutex);

	while(list != NULL){
	  if(list->data != remove_recall->recall){
	    ags_channel_remove_recall(AGS_CHANNEL(remove_recall->context),
				      (GObject *) list->data,
				      remove_recall->is_play);
	  }
	  
	  list = list->next;
	}

	g_list_free(list_start);
      }
    }

    ags_channel_remove_recall(AGS_CHANNEL(remove_recall->context),
			      (GObject *) remove_recall->recall,
			      remove_recall->is_play);
  }else if(AGS_IS_RECALL(remove_recall->context)){
    ags_recall_remove_child(AGS_RECALL(remove_recall->context),
			    remove_recall->recall);
  }else{
    ags_recall_remove(remove_recall->recall);
  }
}

/**
 * ags_remove_recall_new:
 * @context: may be #AgsAudio, #AgsChannel or #AgsRecall
 * @recall: the #AgsRecall to remove
 * @is_play: if %TRUE non-complex recall
 * @remove_all: if %TRUE all related will be removed
 *
 * Creates an #AgsRemoveRecall.
 *
 * Returns: an new #AgsRemoveRecall.
 *
 * Since: 1.0.0
 */
AgsRemoveRecall*
ags_remove_recall_new(GObject *context,
		      AgsRecall *recall,
		      gboolean is_play,
		      gboolean remove_all)
{
  AgsRemoveRecall *remove_recall;

  remove_recall = (AgsRemoveRecall *) g_object_new(AGS_TYPE_REMOVE_RECALL,
						   "context", context,
						   "recall", recall,
						   "is-play", is_play,
						   "remove-all", remove_all,
						   NULL);

  return(remove_recall);
}
