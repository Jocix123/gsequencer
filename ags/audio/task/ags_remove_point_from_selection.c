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

#include <ags/audio/task/ags_remove_point_from_selection.h>

#include <ags/libags.h>

#include <ags/i18n.h>

void ags_remove_point_from_selection_class_init(AgsRemovePointFromSelectionClass *remove_point_from_selection);
void ags_remove_point_from_selection_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_remove_point_from_selection_init(AgsRemovePointFromSelection *remove_point_from_selection);
void ags_remove_point_from_selection_set_property(GObject *gobject,
						  guint prop_id,
						  const GValue *value,
						  GParamSpec *param_spec);
void ags_remove_point_from_selection_get_property(GObject *gobject,
						  guint prop_id,
						  GValue *value,
						  GParamSpec *param_spec);
void ags_remove_point_from_selection_connect(AgsConnectable *connectable);
void ags_remove_point_from_selection_disconnect(AgsConnectable *connectable);
void ags_remove_point_from_selection_dispose(GObject *gobject);
void ags_remove_point_from_selection_finalize(GObject *gobject);

void ags_remove_point_from_selection_launch(AgsTask *task);

/**
 * SECTION:ags_remove_point_from_selection
 * @short_description: remove point from notation selection
 * @title: AgsRemovePointFromSelection
 * @section_id:
 * @include: ags/audio/task/ags_remove_point_from_selection.h
 *
 * The #AgsRemovePointFromSelection task removes the specified #AgsNote from selection of #AgsNotation.
 */

static gpointer ags_remove_point_from_selection_parent_class = NULL;
static AgsConnectableInterface *ags_remove_point_from_selection_parent_connectable_interface;

enum{
  PROP_0,
  PROP_AUDIO,
  PROP_NOTATION,
  PROP_X,
  PROP_Y,
};

GType
ags_remove_point_from_selection_get_type()
{
  static volatile gsize g_define_type_id__volatile = 0;

  if(g_once_init_enter (&g_define_type_id__volatile)){
    GType ags_type_remove_point_from_selection;

    static const GTypeInfo ags_remove_point_from_selection_info = {
      sizeof (AgsRemovePointFromSelectionClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_remove_point_from_selection_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsRemovePointFromSelection),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_remove_point_from_selection_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_remove_point_from_selection_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_remove_point_from_selection = g_type_register_static(AGS_TYPE_TASK,
								  "AgsRemovePointFromSelection",
								  &ags_remove_point_from_selection_info,
								  0);
    
    g_type_add_interface_static(ags_type_remove_point_from_selection,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);

    g_once_init_leave (&g_define_type_id__volatile, ags_type_remove_point_from_selection);
  }

  return g_define_type_id__volatile;
}

void
ags_remove_point_from_selection_class_init(AgsRemovePointFromSelectionClass *remove_point_from_selection)
{
  GObjectClass *gobject;
  AgsTaskClass *task;

  GParamSpec *param_spec;

  ags_remove_point_from_selection_parent_class = g_type_class_peek_parent(remove_point_from_selection);

  /* gobject */
  gobject = (GObjectClass *) remove_point_from_selection;

  gobject->set_property = ags_remove_point_from_selection_set_property;
  gobject->get_property = ags_remove_point_from_selection_get_property;

  gobject->dispose = ags_remove_point_from_selection_dispose;
  gobject->finalize = ags_remove_point_from_selection_finalize;

  /* properties */
  /**
   * AgsRemovePointFromSelection:audio:
   *
   * The assigned #AgsAudio
   * 
   * Since: 1.2.2
   */
  param_spec = g_param_spec_object("audio",
				   i18n_pspec("audio of remove point from selection"),
				   i18n_pspec("The audio of remove point from selection task"),
				   AGS_TYPE_AUDIO,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_AUDIO,
				  param_spec);

  /**
   * AgsRemovePointFromSelection:notation:
   *
   * The assigned #AgsNotation
   * 
   * Since: 1.0.0
   */
  param_spec = g_param_spec_object("notation",
				   i18n_pspec("notation of remove point from selection"),
				   i18n_pspec("The notation of remove point from selection task"),
				   AGS_TYPE_NOTATION,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_NOTATION,
				  param_spec);
  
  /**
   * AgsRemovePointFromSelection:x:
   *
   * Note offset x.
   * 
   * Since: 1.0.0
   */
  param_spec = g_param_spec_uint("x",
				 i18n_pspec("offset x"),
				 i18n_pspec("The x offset"),
				 0,
				 G_MAXUINT,
				 0,
				 G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_X,
				  param_spec);

  /**
   * AgsRemovePointFromSelection:y:
   *
   * Note offset y.
   * 
   * Since: 1.0.0
   */
  param_spec = g_param_spec_uint("y",
				 i18n_pspec("offset y"),
				 i18n_pspec("The y offset"),
				 0,
				 G_MAXUINT,
				 0,
				 G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_Y,
				  param_spec);

  /* task */
  task = (AgsTaskClass *) remove_point_from_selection;

  task->launch = ags_remove_point_from_selection_launch;
}

void
ags_remove_point_from_selection_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_remove_point_from_selection_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_remove_point_from_selection_connect;
  connectable->disconnect = ags_remove_point_from_selection_disconnect;
}

void
ags_remove_point_from_selection_init(AgsRemovePointFromSelection *remove_point_from_selection)
{
  remove_point_from_selection->audio = NULL;
  remove_point_from_selection->notation = NULL;

  remove_point_from_selection->x = 0;
  remove_point_from_selection->y = 0;
}

void
ags_remove_point_from_selection_set_property(GObject *gobject,
					     guint prop_id,
					     const GValue *value,
					     GParamSpec *param_spec)
{
  AgsRemovePointFromSelection *remove_point_from_selection;

  remove_point_from_selection = AGS_REMOVE_POINT_FROM_SELECTION(gobject);

  switch(prop_id){
  case PROP_AUDIO:
    {
      AgsAudio *audio;

      audio = (AgsAudio *) g_value_get_object(value);

      if(remove_point_from_selection->audio == (GObject *) audio){
	return;
      }

      if(remove_point_from_selection->audio != NULL){
	g_object_unref(remove_point_from_selection->audio);
      }

      if(audio != NULL){
	g_object_ref(audio);
      }

      remove_point_from_selection->audio = (GObject *) audio;
    }
    break;
  case PROP_NOTATION:
    {
      AgsNotation *notation;

      notation = (AgsNotation *) g_value_get_object(value);

      if(remove_point_from_selection->notation == (GObject *) notation){
	return;
      }

      if(remove_point_from_selection->notation != NULL){
	g_object_unref(remove_point_from_selection->notation);
      }

      if(notation != NULL){
	g_object_ref(notation);
      }

      remove_point_from_selection->notation = (GObject *) notation;
    }
    break;
  case PROP_X:
    {
      remove_point_from_selection->x = g_value_get_uint(value);
    }
    break;
  case PROP_Y:
    {
      remove_point_from_selection->y = g_value_get_uint(value);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_remove_point_from_selection_get_property(GObject *gobject,
					     guint prop_id,
					     GValue *value,
					     GParamSpec *param_spec)
{
  AgsRemovePointFromSelection *remove_point_from_selection;

  remove_point_from_selection = AGS_REMOVE_POINT_FROM_SELECTION(gobject);

  switch(prop_id){
  case PROP_AUDIO:
    {
      g_value_set_object(value, remove_point_from_selection->audio);
    }
    break;
  case PROP_NOTATION:
    {
      g_value_set_object(value, remove_point_from_selection->notation);
    }
    break;
  case PROP_X:
    {
      g_value_set_uint(value, remove_point_from_selection->x);
    }
    break;
  case PROP_Y:
    {
      g_value_set_uint(value, remove_point_from_selection->y);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_remove_point_from_selection_connect(AgsConnectable *connectable)
{
  ags_remove_point_from_selection_parent_connectable_interface->connect(connectable);

  /* empty */
}

void
ags_remove_point_from_selection_disconnect(AgsConnectable *connectable)
{
  ags_remove_point_from_selection_parent_connectable_interface->disconnect(connectable);

  /* empty */
}

void
ags_remove_point_from_selection_dispose(GObject *gobject)
{
  AgsRemovePointFromSelection *remove_point_from_selection;

  remove_point_from_selection = AGS_REMOVE_POINT_FROM_SELECTION(gobject);

  if(remove_point_from_selection->audio != NULL){
    g_object_unref(remove_point_from_selection->audio);

    remove_point_from_selection->audio = NULL;
  }
    
  if(remove_point_from_selection->notation != NULL){
    g_object_unref(remove_point_from_selection->notation);

    remove_point_from_selection->notation = NULL;
  }
  
  /* Call parent */
  G_OBJECT_CLASS(ags_remove_point_from_selection_parent_class)->dispose(gobject);
}

void
ags_remove_point_from_selection_finalize(GObject *gobject)
{
  AgsRemovePointFromSelection *remove_point_from_selection;

  remove_point_from_selection = AGS_REMOVE_POINT_FROM_SELECTION(gobject);

  if(remove_point_from_selection->audio != NULL){
    g_object_unref(remove_point_from_selection->audio);
  }

  if(remove_point_from_selection->notation != NULL){
    g_object_unref(remove_point_from_selection->notation);
  }
  
  /* call parent */
  G_OBJECT_CLASS(ags_remove_point_from_selection_parent_class)->finalize(gobject);
}

void
ags_remove_point_from_selection_launch(AgsTask *task)
{
  AgsRemovePointFromSelection *remove_point_from_selection;

  AgsMutexManager *mutex_manager;

  pthread_mutex_t *application_mutex;
  pthread_mutex_t *audio_mutex;

  /* get mutex manager and application mutex */
  mutex_manager = ags_mutex_manager_get_instance();
  application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);

  remove_point_from_selection = AGS_REMOVE_POINT_FROM_SELECTION(task);

  /* get audio mutex */
  pthread_mutex_lock(application_mutex);

  audio_mutex = ags_mutex_manager_lookup(mutex_manager,
					 (GObject *) remove_point_from_selection->audio);

  pthread_mutex_unlock(application_mutex);

  /* remove note */
  pthread_mutex_lock(audio_mutex);
  
  ags_notation_remove_point_from_selection(remove_point_from_selection->notation,
					   remove_point_from_selection->x, remove_point_from_selection->y);

  pthread_mutex_unlock(audio_mutex);
}

/**
 * ags_remove_point_from_selection_new:
 * @notation: the #AgsNotation providing the selection
 * @x: x coordinate
 * @y: y coordinate
 *
 * WARNING you should provide the #AgsRemovePointFromSelection:audio property.
 * Creates an #AgsRemovePointFromSelection.
 *
 * Returns: an new #AgsRemovePointFromSelection.
 *
 * Since: 1.0.0
 */
AgsRemovePointFromSelection*
ags_remove_point_from_selection_new(AgsNotation *notation,
				    guint x, guint y)
{
  AgsRemovePointFromSelection *remove_point_from_selection;

  remove_point_from_selection = (AgsRemovePointFromSelection *) g_object_new(AGS_TYPE_REMOVE_POINT_FROM_SELECTION,
									     "notation", notation,
									     "x", x,
									     "y", y,
									     NULL);

  return(remove_point_from_selection);
}
