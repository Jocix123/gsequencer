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

#include <ags/audio/ags_automation.h>

#include <ags/libags.h>

#include <ags/plugin/ags_base_plugin.h>

#include <ags/audio/ags_audio.h>
#include <ags/audio/ags_port.h>

#include <ags/config.h>

#include <pthread.h>

#include <stdlib.h>

#include <math.h>
#include <errno.h>

#include <ags/i18n.h>

void ags_automation_class_init(AgsAutomationClass *automation);
void ags_automation_init(AgsAutomation *automation);
void ags_automation_set_property(GObject *gobject,
				 guint prop_id,
				 const GValue *value,
				 GParamSpec *param_spec);
void ags_automation_get_property(GObject *gobject,
				 guint prop_id,
				 GValue *value,
				 GParamSpec *param_spec);
void ags_automation_dispose(GObject *gobject);
void ags_automation_finalize(GObject *gobject);

void ags_automation_insert_native_scale_from_clipboard(AgsAutomation *automation,
						       xmlNode *root_node, char *version,
						       char *x_boundary, char *y_boundary,
						       gboolean from_x_offset, guint x_offset,
						       gboolean from_y_offset, guint y_offset,
						       gboolean match_line, gboolean no_duplicates);

/**
 * SECTION:ags_automation
 * @short_description: Automation class supporting selection and clipboard.
 * @title: AgsAutomation
 * @section_id:
 * @include: ags/audio/ags_automation.h
 *
 * #AgsAutomation acts as a container of #AgsAcceleration.
 */

enum{
  PROP_0,
  PROP_AUDIO,
  PROP_CHANNEL_TYPE,
  PROP_LINE,
  PROP_TIMESTAMP,
  PROP_CONTROL_NAME,
  PROP_PORT,
  PROP_STEPS,
  PROP_UPPER,
  PROP_LOWER,
  PROP_DEFAULT_VALUE,
  PROP_ACCELERATION,
};

static gpointer ags_automation_parent_class = NULL;

static pthread_mutex_t ags_automation_class_mutex = PTHREAD_MUTEX_INITIALIZER;

GType
ags_automation_get_type()
{
  static GType ags_type_automation = 0;

  if(!ags_type_automation){
    static const GTypeInfo ags_automation_info = {
      sizeof(AgsAutomationClass),
      NULL,
      NULL,
      (GClassInitFunc) ags_automation_class_init,
      NULL,
      NULL,
      sizeof(AgsAutomation),
      0,
      (GInstanceInitFunc) ags_automation_init,
    };

    ags_type_automation = g_type_register_static(G_TYPE_OBJECT,
						 "AgsAutomation",
						 &ags_automation_info,
						 0);
  }

  return(ags_type_automation);
}

void 
ags_automation_class_init(AgsAutomationClass *automation)
{
  GObjectClass *gobject;
  GParamSpec *param_spec;

  ags_automation_parent_class = g_type_class_peek_parent(automation);

  gobject = (GObjectClass *) automation;

  gobject->set_property = ags_automation_set_property;
  gobject->get_property = ags_automation_get_property;

  gobject->dispose = ags_automation_dispose;
  gobject->finalize = ags_automation_finalize;

  /* properties */
  /**
   * AgsAutomation:audio:
   *
   * The assigned #AgsAudio
   * 
   * Since: 1.0.0
   */
  param_spec = g_param_spec_object("audio",
				   i18n_pspec("audio of automation"),
				   i18n_pspec("The audio of automation"),
				   AGS_TYPE_AUDIO,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_AUDIO,
				  param_spec);


  /**
   * AgsAutomation:line:
   *
   * The effect's line.
   * 
   * Since: 1.0.0
   */
  param_spec =  g_param_spec_uint("line",
				  i18n_pspec("line of effect"),
				  i18n_pspec("The numerical line of effect"),
				  0,
				  65535,
				  0,
				  G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_LINE,
				  param_spec);

  /**
   * AgsAutomation:channel-type:
   *
   * The effect's assigned channel type.
   * 
   * Since: 1.0.0
   */
  param_spec =  g_param_spec_gtype("channel-type",
				   i18n_pspec("channel type to apply"),
				   i18n_pspec("The channel type to apply"),
				   G_TYPE_NONE,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_CHANNEL_TYPE,
				  param_spec);

  
  /**
   * AgsAutomation:control-name:
   *
   * The effect's assigned control name.
   * 
   * Since: 1.0.0
   */
  param_spec =  g_param_spec_string("control-name",
				    i18n_pspec("control name"),
				    i18n_pspec("The control name"),
				    NULL,
				    G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_CONTROL_NAME,
				  param_spec);

  /**
   * AgsAutomation:port:
   *
   * The assigned #AgsPort
   * 
   * Since: 1.0.0
   */
  param_spec = g_param_spec_object("port",
				   i18n_pspec("port of automation"),
				   i18n_pspec("The port of automation"),
				   AGS_TYPE_PORT,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_PORT,
				  param_spec);

  /**
   * AgsAutomation:steps:
   *
   * The effect's steps.
   * 
   * Since: 1.0.0
   */
  param_spec =  g_param_spec_uint("steps",
				  i18n_pspec("steps of effect"),
				  i18n_pspec("The steps of effect"),
				  0,
				  65535,
				  0,
				  G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_STEPS,
				  param_spec);

  /**
   * AgsAutomation:upper:
   *
   * The effect's upper.
   * 
   * Since: 1.0.0
   */
  param_spec =  g_param_spec_double("upper",
				    i18n_pspec("upper of effect"),
				    i18n_pspec("The upper of effect"),
				    -65535.0,
				    65535.0,
				    0,
				    G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_UPPER,
				  param_spec);

  /**
   * AgsAutomation:lower:
   *
   * The effect's lower.
   * 
   * Since: 1.0.0
   */
  param_spec =  g_param_spec_double("lower",
				    i18n_pspec("lower of effect"),
				    i18n_pspec("The lower of effect"),
				    -65535.0,
				    65535.0,
				    0,
				    G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_LOWER,
				  param_spec);


  /**
   * AgsAutomation:default-value:
   *
   * The effect's default-value.
   * 
   * Since: 1.0.0
   */
  param_spec =  g_param_spec_double("default-value",
				    i18n_pspec("default value of effect"),
				    i18n_pspec("The default value of effect"),
				    -65535.0,
				    65535.0,
				    0,
				    G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_DEFAULT_VALUE,
				  param_spec);


  /**
   * AgsAutomation:acceleration:
   *
   * The acceleration list.
   * 
   * Since: 1.0.0
   */
  param_spec = g_param_spec_pointer("acceleration",
				    i18n_pspec("acceleration"),
				    i18n_pspec("The acceleration"),
				    G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_ACCELERATION,
				  param_spec);

  /**
   * AgsAutomation:timestamp:
   *
   * The automation's timestamp.
   * 
   * Since: 1.0.0
   */
  param_spec = g_param_spec_object("timestamp",
				   i18n_pspec("timestamp of automation"),
				   i18n_pspec("The timestamp of automation"),
				   AGS_TYPE_TIMESTAMP,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_TIMESTAMP,
				  param_spec);
}

void
ags_automation_init(AgsAutomation *automation)
{
  pthread_mutex_t *mutex;
  pthread_mutexattr_t *attr;

  automation->flags = AGS_AUTOMATION_BYPASS;

  /* add automation mutex */
  automation->obj_mutexattr = 
    attr = (pthread_mutexattr_t *) malloc(sizeof(pthread_mutexattr_t));
  pthread_mutexattr_init(attr);
  pthread_mutexattr_settype(attr,
			    PTHREAD_MUTEX_RECURSIVE);

#ifdef __linux__
  pthread_mutexattr_setprotocol(attr,
				PTHREAD_PRIO_INHERIT);
#endif

  automation->obj_mutex = 
    mutex = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(mutex,
		     attr);  

  /*  */
  automation->audio = NULL;
  automation->channel_type = G_TYPE_NONE;
  automation->line = 0;

  automation->timestamp = ags_timestamp_new();

  automation->timestamp->flags &= (~AGS_TIMESTAMP_UNIX);
  automation->timestamp->flags |= AGS_TIMESTAMP_OFFSET;

  automation->timestamp->timer.ags_offset.offset = 0;

  g_object_ref(automation->timestamp);
  
  automation->control_name = NULL;

  automation->steps = 8;
  automation->upper = 1.0;
  automation->lower = 0.0;
  automation->default_value = 0.0;

  automation->source_function = NULL;

  automation->port = NULL;
  
  automation->acceleration = NULL;

  automation->loop_start = 0.0;
  automation->loop_end = 0.0;
  automation->offset = 0.0;
  
  automation->selection = NULL;
}

void
ags_automation_set_property(GObject *gobject,
			    guint prop_id,
			    const GValue *value,
			    GParamSpec *param_spec)
{
  AgsAutomation *automation;

  automation = AGS_AUTOMATION(gobject);

  switch(prop_id){
  case PROP_AUDIO:
    {
      AgsAudio *audio;

      audio = (AgsAudio *) g_value_get_object(value);

      if(automation->audio == (GObject *) audio){
	return;
      }

      if(automation->audio != NULL){
	g_object_unref(automation->audio);
      }

      if(audio != NULL){
	g_object_ref(audio);
      }

      automation->audio = (GObject *) audio;
    }
    break;
  case PROP_LINE:
    {
      guint line;

      line = g_value_get_uint(value);

      automation->line = line;
    }
    break;
  case PROP_CHANNEL_TYPE:
    {
      GType channel_type;

      channel_type = (GType) g_value_get_gtype(value);

      automation->channel_type = channel_type;
    }
    break;
  case PROP_CONTROL_NAME:
    {
      gchar *control_name;

      control_name = g_value_get_string(value);

      if(automation->control_name != NULL){
	g_free(automation->control_name);
      }

      automation->control_name = g_strdup(control_name);
    }
    break;
  case PROP_PORT:
    {
      AgsPort *port;

      port = g_value_get_object(value);

      if(automation->port == (GObject *) port){
	return;
      }

      if(automation->port != NULL){
	g_object_unref(automation->port);
      }

      if(port != NULL){
	g_object_ref(port);

	if((AGS_PORT_INFINITE_RANGE & (port->flags)) != 0){
	  automation->steps = AGS_AUTOMATION_MAXIMUM_STEPS;
	}
      }

      automation->port = (GObject *) port;

      if(port->port_descriptor != NULL){
	automation->lower = g_value_get_float(AGS_PORT_DESCRIPTOR(port->port_descriptor)->lower_value);
	automation->upper = g_value_get_float(AGS_PORT_DESCRIPTOR(port->port_descriptor)->upper_value);

	if((AGS_PORT_DESCRIPTOR_TOGGLED & (AGS_PORT_DESCRIPTOR(port->port_descriptor)->flags)) != 0){
	  automation->lower = 0.0;
	  automation->upper = 1.0;
	  automation->steps = 1;
	}else if((AGS_PORT_DESCRIPTOR_INTEGER & (AGS_PORT_DESCRIPTOR(port->port_descriptor)->flags)) != 0){
	  automation->steps = AGS_PORT_DESCRIPTOR(port->port_descriptor)->scale_steps;
	}else{
	  automation->steps = AGS_AUTOMATION_DEFAULT_PRECISION;
	}
      }
    }
    break;
  case PROP_STEPS:
    {
      guint steps;

      steps = g_value_get_uint(value);

      automation->steps = steps;
    }
    break;
  case PROP_UPPER:
    {
      gdouble upper;

      upper = g_value_get_double(value);

      automation->upper = upper;      
    }
    break;
  case PROP_LOWER:
    {
      gdouble lower;

      lower = g_value_get_double(value);

      automation->lower = lower;      
    }
    break;
  case PROP_DEFAULT_VALUE:
    {
      gdouble default_value;

      default_value = g_value_get_double(value);

      automation->default_value = default_value;      
    }
    break;
  case PROP_ACCELERATION:
    {
      AgsAcceleration *acceleration;

      acceleration = (AgsAcceleration *) g_value_get_object(value);

      if(acceleration == NULL ||
	 g_list_find(automation->acceleration, acceleration) != NULL){
	return;
      }

      ags_automation_add_acceleration(automation,
				      acceleration,
				      FALSE);
    }
    break;
  case PROP_TIMESTAMP:
    {
      AgsTimestamp *timestamp;

      timestamp = (AgsTimestamp *) g_value_get_object(value);

      if(automation->timestamp == (GObject *) timestamp){
	return;
      }

      if(automation->timestamp != NULL){
	g_object_unref(G_OBJECT(automation->timestamp));
      }

      if(timestamp != NULL){
	g_object_ref(G_OBJECT(timestamp));
      }

      automation->timestamp = (GObject *) timestamp;
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_automation_get_property(GObject *gobject,
			    guint prop_id,
			    GValue *value,
			    GParamSpec *param_spec)
{
  AgsAutomation *automation;

  automation = AGS_AUTOMATION(gobject);

  switch(prop_id){
  case PROP_AUDIO:
    {
      g_value_set_object(value, automation->audio);
    }
    break;
  case PROP_LINE:
    {
      g_value_set_uint(value, automation->line);
    }
    break;
  case PROP_CHANNEL_TYPE:
    {
      g_value_set_gtype(value, automation->channel_type);
    }
    break;
  case PROP_CONTROL_NAME:
    {
      g_value_set_string(value, automation->control_name);
    }
    break;
  case PROP_PORT:
    {
      g_value_set_object(value, automation->port);
    }
    break;
  case PROP_STEPS:
    {
      g_value_set_uint(value, automation->steps);
    }
    break;
  case PROP_UPPER:
    {
      g_value_set_double(value, automation->upper);
    }
    break;
  case PROP_LOWER:
    {
      g_value_set_double(value, automation->lower);
    }
    break;
  case PROP_DEFAULT_VALUE:
    {
      g_value_set_double(value, automation->default_value);
    }
    break;
  case PROP_TIMESTAMP:
    {
      g_value_set_object(value, automation->timestamp);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_automation_dispose(GObject *gobject)
{
  AgsAutomation *automation;

  GList *list;
  
  automation = AGS_AUTOMATION(gobject);

  /* timestamp */
  if(automation->timestamp != NULL){
    g_object_unref(automation->timestamp);

    automation->timestamp = NULL;
  }

  /* audio */
  if(automation->audio != NULL){
    g_object_unref(automation->audio);

    automation->audio = NULL;
  }

  /* source function */
  if(automation->source_function != NULL){
    g_object_run_dispose(automation->source_function);
    
    g_object_unref(automation->source_function);

    automation->source_function = NULL;
  }

  /* acceleration */
  list = automation->acceleration;

  while(list != NULL){
    g_object_run_dispose(G_OBJECT(list->data));

    list = list->next;
  }
  
  g_list_free_full(automation->acceleration,
		   g_object_unref);
  g_list_free(automation->selection);

  automation->acceleration = NULL;
  automation->selection = NULL;

  /* port */
  if(automation->port != NULL){
    g_object_unref(automation->port);

    automation->port = NULL;
  }

  /* call parent */
  G_OBJECT_CLASS(ags_automation_parent_class)->dispose(gobject);
}

void
ags_automation_finalize(GObject *gobject)
{
  AgsAutomation *automation;

  automation = AGS_AUTOMATION(gobject);

  pthread_mutex_destroy(automation->obj_mutex);
  free(automation->obj_mutex);

  pthread_mutexattr_destroy(automation->obj_mutexattr);
  free(automation->obj_mutexattr);

  /* timestamp */
  if(automation->timestamp != NULL){
    g_object_unref(automation->timestamp);
  }
  
  /* audio */
  if(automation->audio != NULL){
    g_object_unref(automation->audio);
  }

  /* control name */
  if(automation->control_name != NULL){
    free(automation->control_name);
  }

  /* source function */
  if(automation->source_function != NULL){
    g_object_unref(automation->source_function);
  }
  
  /* acceleration */
  g_list_free_full(automation->acceleration,
		   g_object_unref);

  g_list_free(automation->selection);

  /* port */
  if(automation->port != NULL){
    g_object_unref(automation->port);
  }
  
  /* call parent */
  G_OBJECT_CLASS(ags_automation_parent_class)->finalize(gobject);
}

/**
 * ags_automation_get_class_mutex:
 * 
 * Use this function's returned mutex to access mutex fields.
 *
 * Returns: the class mutex
 * 
 * Since: 2.0.0
 */
pthread_mutex_t*
ags_automation_get_class_mutex()
{
  return(&ags_automation_class_mutex);
}

/**
 * ags_automation_find_port:
 * @automation: the #GList-struct containing #AgsAutomation
 * @port: the #AgsPort to match
 * 
 * Find automation by port.
 * 
 * Returns: next matching automation as #GList-struct or %NULL if not found
 * 
 * Since: 1.3.0
 */
GList*
ags_automation_find_port(GList *automation,
			 GObject *port)
{
  if(automation == NULL ||
     !AGS_IS_PORT(port)){
    return(NULL);
  }

  while(automation != NULL){
    if(AGS_AUTOMATION(automation->data)->port == port){
      break;
    }
    
    automation = automation->next;
  }

  return(automation);
}

/**
 * ags_automation_find_near_timestamp:
 * @automation: a #GList containing #AgsAutomation
 * @line: the matching audio channel
 * @timestamp: the matching timestamp
 *
 * Retrieve appropriate automation for timestamp.
 *
 * Returns: Next match.
 *
 * Since: 1.0.0
 */
GList*
ags_automation_find_near_timestamp(GList *automation, guint line,
				   AgsTimestamp *timestamp)
{
  AgsTimestamp *current_timestamp;

  while(automation != NULL){
    if(AGS_AUTOMATION(automation->data)->line != line){
      automation = automation->next;
      
      continue;
    }

    if(timestamp == NULL){
      return(automation);
    }
    
    current_timestamp = (AgsTimestamp *) AGS_AUTOMATION(automation->data)->timestamp;

    if(current_timestamp != NULL){
      if((AGS_TIMESTAMP_UNIX & (timestamp->flags)) != 0 &&
	 (AGS_TIMESTAMP_UNIX & (current_timestamp->flags)) != 0){
	if(current_timestamp->timer.unix_time.time_val >= timestamp->timer.unix_time.time_val &&
	   current_timestamp->timer.unix_time.time_val < timestamp->timer.unix_time.time_val + AGS_AUTOMATION_DEFAULT_DURATION){
	  return(automation);
	}
      }else if((AGS_TIMESTAMP_OFFSET & (timestamp->flags)) != 0 &&
	       (AGS_TIMESTAMP_OFFSET & (current_timestamp->flags)) != 0){
	if(current_timestamp->timer.ags_offset.offset >= timestamp->timer.ags_offset.offset &&
	   current_timestamp->timer.ags_offset.offset < timestamp->timer.ags_offset.offset + AGS_AUTOMATION_DEFAULT_OFFSET){
	  return(automation);
	}
      }
    }
    
    automation = automation->next;
  }

  return(NULL);
}

/**
 * ags_automation_find_near_timestamp_extended:
 * @automation: a #GList containing #AgsAutomation
 * @line: the matching audio channel
 * @channel_type: the matching channel type
 * @control_name: the matching control name
 * @timestamp: the matching timestamp
 *
 * Retrieve appropriate automation for timestamp.
 *
 * Returns: Next match.
 *
 * Since: 1.3.0
 */
GList*
ags_automation_find_near_timestamp_extended(GList *automation, guint line,
					    GType channel_type, gchar *control_name,
					    AgsTimestamp *timestamp)
{
  AgsTimestamp *current_timestamp;

  while(automation != NULL){
    if(AGS_AUTOMATION(automation->data)->line != line ||
       AGS_AUTOMATION(automation->data)->channel_type != channel_type ||
       g_strcmp0(AGS_AUTOMATION(automation->data)->control_name,
		 control_name)){
      automation = automation->next;
      
      continue;
    }

    if(timestamp == NULL){
      return(automation);
    }
    
    current_timestamp = (AgsTimestamp *) AGS_AUTOMATION(automation->data)->timestamp;

    if(current_timestamp != NULL){
      if((AGS_TIMESTAMP_UNIX & (timestamp->flags)) != 0 &&
	 (AGS_TIMESTAMP_UNIX & (current_timestamp->flags)) != 0){
	if(current_timestamp->timer.unix_time.time_val >= timestamp->timer.unix_time.time_val &&
	   current_timestamp->timer.unix_time.time_val < timestamp->timer.unix_time.time_val + AGS_AUTOMATION_DEFAULT_DURATION){
	  return(automation);
	}
      }else if((AGS_TIMESTAMP_OFFSET & (timestamp->flags)) != 0 &&
	       (AGS_TIMESTAMP_OFFSET & (current_timestamp->flags)) != 0){
	if(current_timestamp->timer.ags_offset.offset >= timestamp->timer.ags_offset.offset &&
	   current_timestamp->timer.ags_offset.offset < timestamp->timer.ags_offset.offset + AGS_AUTOMATION_DEFAULT_OFFSET){
	  return(automation);
	}
      }
    }
    
    automation = automation->next;
  }

  return(NULL);
}

/**
 * ags_automation_add:
 * @automation: the #GList-struct containing #AgsAutomation
 * @new_automation: the automation to add
 * 
 * Add @new_automation sorted to @automation
 * 
 * Returns: the new beginning of @automation
 * 
 * Since: 1.3.0
 */
GList*
ags_automation_add(GList *automation,
		   AgsAutomation *new_automation)
{
  auto gint ags_automation_add_compare(gconstpointer a,
				       gconstpointer b);
  
  gint ags_automation_add_compare(gconstpointer a,
				  gconstpointer b)
  {
    if(AGS_AUTOMATION(a)->timestamp->timer.ags_offset.offset == AGS_AUTOMATION(b)->timestamp->timer.ags_offset.offset){
      return(0);
    }else if(AGS_AUTOMATION(a)->timestamp->timer.ags_offset.offset < AGS_AUTOMATION(b)->timestamp->timer.ags_offset.offset){
      return(-1);
    }else if(AGS_AUTOMATION(a)->timestamp->timer.ags_offset.offset > AGS_AUTOMATION(b)->timestamp->timer.ags_offset.offset){
      return(1);
    }

    return(0);
  }
  
  if(!AGS_IS_AUTOMATION(new_automation) ||
     !AGS_IS_TIMESTAMP(new_automation->timestamp)){
    return(automation);
  }
  
  automation = g_list_insert_sorted(automation,
				    new_automation,
				    ags_automation_add_compare);
  
  return(automation);
}

/**
 * ags_automation_add_acceleration:
 * @automation: an #AgsAutomation
 * @acceleration: the #AgsAcceleration to add
 * @use_selection_list: if %TRUE add to selection, else to default automation
 *
 * Adds a acceleration to automation.
 *
 * Since: 1.0.0
 */
void
ags_automation_add_acceleration(AgsAutomation *automation,
				AgsAcceleration *acceleration,
				gboolean use_selection_list)
{
  GList *list, *list_new;

  auto gint ags_automation_add_acceleration_compare_function(gpointer a, gpointer b);

  gint ags_automation_add_acceleration_compare_function(gpointer a, gpointer b){
    if(AGS_ACCELERATION(a)->x == AGS_ACCELERATION(b)->x){
      return(0);
    }

    if(AGS_ACCELERATION(a)->x < AGS_ACCELERATION(b)->x){
      return(-1);
    }else{
      return(1);
    }
  }

  if(acceleration == NULL){
    return;
  }

  g_object_ref(acceleration);
  
  if(use_selection_list){
    automation->selection = g_list_insert_sorted(automation->selection,
						 acceleration,
						 (GCompareFunc) ags_automation_add_acceleration_compare_function);
  }else{
    automation->acceleration = g_list_insert_sorted(automation->acceleration,
						    acceleration,
						    (GCompareFunc) ags_automation_add_acceleration_compare_function);
  }
}

/**
 * ags_automation_remove_acceleration_at_position:
 * @automation: an #AgsAutomation
 * @x: offset
 * @y: acceleration
 *
 * Removes one #AgsAcceleration of automation.
 *
 * Returns: %TRUE if successfully removed acceleration.
 *
 * Since: 1.0.0
 */
gboolean
ags_automation_remove_acceleration_at_position(AgsAutomation *automation,
					       guint x, gdouble y)
{
  GList *list, *current;
  gboolean retval;
  
  list = automation->acceleration;
  current = NULL;
  retval = FALSE;
  
  while(list != NULL){
    if(AGS_ACCELERATION(list->data)->x == x &&
       (AGS_ACCELERATION(list->data)->y - ((automation->upper - automation->lower) / AGS_AUTOMATION_MAXIMUM_STEPS) <= y &&
	AGS_ACCELERATION(list->data)->y + ((automation->upper - automation->lower) / AGS_AUTOMATION_MAXIMUM_STEPS) >= y)){
      current = list;
      retval = TRUE;
      break;
    }

    if(AGS_ACCELERATION(list->data)->x > x){
      break;
    }
    
    list = list->next;
  }

  automation->acceleration = g_list_delete_link(automation->acceleration,
						current);

  return(retval);
}

/**
 * ags_automation_get_selection:
 * @automation: the #AgsAutomation
 *
 * Retrieve selection.
 *
 * Returns: the selection.
 *
 * Since: 1.0.0
 */
GList*
ags_automation_get_selection(AgsAutomation *automation)
{
  return(automation->selection);
}

/**
 * ags_automation_is_acceleration_selected:
 * @automation: the #AgsAutomation
 * @acceleration: the #AgsAcceleration to check for
 *
 * Check selection for acceleration.
 *
 * Returns: %TRUE if selected
 *
 * Since: 1.0.0
 */
gboolean
ags_automation_is_acceleration_selected(AgsAutomation *automation, AgsAcceleration *acceleration)
{
  GList *selection;

  selection = automation->selection;

  while(selection != NULL && AGS_ACCELERATION(selection->data)->x <= acceleration->x){
    if(selection->data == acceleration){
      return(TRUE);
    }

    selection = selection->next;
  }

  return(FALSE);
}

/**
 * ags_automation_find_point:
 * @automation: an #AgsAutomation
 * @x: offset
 * @y: acceleration, will be ignored
 * @use_selection_list: if %TRUE selection is searched
 *
 * Find acceleration by offset and acceleration.
 *
 * Returns: the matching acceleration.
 *
 * Since: 1.0.0
 */ 
AgsAcceleration*
ags_automation_find_point(AgsAutomation *automation,
			  guint x, gdouble y,
			  gboolean use_selection_list)
{
  GList *acceleration;

  if(use_selection_list){
    acceleration = automation->selection;
  }else{
    acceleration = automation->acceleration;
  }

  while(acceleration != NULL && AGS_ACCELERATION(acceleration->data)->x < x){
    acceleration = acceleration->next;
  }

  if(acceleration == NULL ||
     AGS_ACCELERATION(acceleration->data)->x != x){
    return(NULL);
  }else{
    return(acceleration->data);
  }
}

/**
 * ags_automation_find_region:
 * @automation: an #AgsAutomation
 * @x0: start offset
 * @y0: start tone
 * @x1: end offset
 * @y1: end tone
 * @use_selection_list: if %TRUE selection is searched
 *
 * Find acceleration by offset and tone region.
 *
 * Returns: the matching acceleration as #GList.
 *
 * Since: 1.0.0
 */
GList*
ags_automation_find_region(AgsAutomation *automation,
			   guint x0, gdouble y0,
			   guint x1, gdouble y1,
			   gboolean use_selection_list)
{
  AgsAcceleration *current;

  GList *acceleration;
  GList *region;

  if(y0 > y1){
    gdouble tmp_y;

    tmp_y = y0;
    y0 = y1;
    y1 = tmp_y;
  }
  
  if(use_selection_list){
    acceleration = automation->selection;
  }else{
    acceleration = automation->acceleration;
  }

  while(acceleration != NULL && AGS_ACCELERATION(acceleration->data)->x < x0){
    acceleration = acceleration->next;
  }

  region = NULL;

  while(acceleration != NULL && (current = AGS_ACCELERATION(acceleration->data))->x < x1){
    if(current->y >= y0 && current->y < y1){
      region = g_list_prepend(region, current);
    }

    acceleration = acceleration->next;
  }

  region = g_list_reverse(region);

  return(region);
}

/**
 * ags_automation_free_selection:
 * @automation: an #AgsAutomation
 *
 * Clear selection.
 *
 * Since: 1.0.0
 */
void
ags_automation_free_selection(AgsAutomation *automation)
{
  AgsAcceleration *acceleration;
  GList *list;

  list = automation->selection;
  
  while(list != NULL){
    acceleration = AGS_ACCELERATION(list->data);
    acceleration->flags &= (~AGS_ACCELERATION_IS_SELECTED);
    g_object_unref(G_OBJECT(acceleration));
    
    list = list->next;
  }

  list = automation->selection;
  automation->selection = NULL;
  g_list_free(list);
}

/**
 * ags_automation_add_point_to_selection:
 * @automation: an #AgsAutomation
 * @x: offset
 * @y: tone
 * @replace_current_selection: if %TRUE selection is replaced
 *
 * Select acceleration at position.
 *
 * Since: 1.0.0
 */ 
void
ags_automation_add_point_to_selection(AgsAutomation *automation,
				      guint x, gdouble y,
				      gboolean replace_current_selection)
{
  AgsAcceleration *acceleration;

  acceleration = ags_automation_find_point(automation,
					   x, y,
					   FALSE);

  if(acceleration == NULL){
    /* there is nothing to be selected */
    if(replace_current_selection){
      ags_automation_free_selection(automation);
    }
  }else{
    /* add to or replace selection */
    acceleration->flags |= AGS_ACCELERATION_IS_SELECTED;
    g_object_ref(acceleration);

    if(replace_current_selection){
      GList *list;

      list = g_list_alloc();
      list->data = acceleration;
      
      ags_automation_free_selection(automation);
      automation->selection = list;
    }else{
      if(!ags_automation_is_acceleration_selected(automation, acceleration)){
	ags_automation_add_acceleration(automation, acceleration, TRUE);
      }
    }
  }
}

/**
 * ags_automation_remove_point_from_selection:
 * @automation: an #AgsAutomation
 * @x: offset
 * @y: tone
 *
 * Remove acceleration at position of selection.
 *
 * Since: 1.0.0
 */ 
void
ags_automation_remove_point_from_selection(AgsAutomation *automation,
					   guint x, gdouble y)
{
  AgsAcceleration *acceleration;

  acceleration = ags_automation_find_point(automation,
					   x, y,
					   FALSE);

  if(acceleration != NULL){
    acceleration->flags &= (~AGS_ACCELERATION_IS_SELECTED);

    /* remove acceleration from selection */
    automation->selection = g_list_remove(automation->selection, acceleration);

    g_object_unref(acceleration);
  }
}

void
ags_automation_add_region_to_selection(AgsAutomation *automation,
				       guint x0, gdouble y0,
				       guint x1, gdouble y1,
				       gboolean replace_current_selection)
{
  AgsAcceleration *acceleration;
  GList *region, *list;

  region = ags_automation_find_region(automation,
				      x0, y0,
				      x1, y1,
				      FALSE);

  if(replace_current_selection){
    ags_automation_free_selection(automation);

    list = region;

    while(list != NULL){
      AGS_ACCELERATION(list->data)->flags |= AGS_ACCELERATION_IS_SELECTED;
      g_object_ref(G_OBJECT(list->data));

      list = list->next;
    }

    automation->selection = region;
  }else{
    while(region != NULL){
      acceleration = AGS_ACCELERATION(region->data);

      if(!ags_automation_is_acceleration_selected(automation, acceleration)){
	acceleration->flags |= AGS_ACCELERATION_IS_SELECTED;
	g_object_ref(G_OBJECT(acceleration));
	ags_automation_add_acceleration(automation,
					acceleration,
					TRUE);
      }
      
      region = region->next;
    }
    
    g_list_free(region);
  }
}

/**
 * ags_automation_remove_region_from_selection:
 * @automation: an #AgsAutomation
 * @x0: start offset
 * @y0: start tone
 * @x1: end offset
 * @y1: end tone
 *
 * Remove acceleration within region of selection.
 *
 * Since: 1.0.0
 */ 
void
ags_automation_remove_region_from_selection(AgsAutomation *automation,
					    guint x0, gdouble y0,
					    guint x1, gdouble y1)
{
  AgsAcceleration *acceleration;
  GList *region;

  region = ags_automation_find_region(automation,
				      x0, y0,
				      x1, y1,
				      TRUE);

  while(region != NULL){
    acceleration = AGS_ACCELERATION(region->data);
    acceleration->flags &= (~AGS_ACCELERATION_IS_SELECTED);

    automation->selection = g_list_remove(automation->selection, acceleration);
    g_object_unref(G_OBJECT(acceleration));

    region = region->next;
  }

  g_list_free(region);
}

void
ags_automation_add_all_to_selection(AgsAutomation *automation)
{
  GList *acceleration;

  if(automation == NULL ||
     automation->acceleration == NULL){
    return;
  }
  
  acceleration = automation->acceleration;
  acceleration = acceleration->next;
  
  while(acceleration->next != NULL){
    AGS_ACCELERATION(acceleration->data)->flags |= AGS_ACCELERATION_IS_SELECTED;
    
    acceleration = acceleration->next;
  }
}

/**
 * ags_automation_copy_selection:
 * @automation: an #AgsAutomation
 *
 * Copy selection to clipboard.
 *
 * Returns: the selection as XML.
 *
 * Since: 1.0.0
 */
xmlNode*
ags_automation_copy_selection(AgsAutomation *automation)
{
  AgsAcceleration *acceleration;

  xmlNode *automation_node, *current_acceleration;

  GList *selection;

  guint x_boundary;
  gdouble y_boundary;
  
  selection = automation->selection;

  /* create root node */
  automation_node = xmlNewNode(NULL, BAD_CAST "automation");

  xmlNewProp(automation_node, BAD_CAST "program", BAD_CAST "ags");
  xmlNewProp(automation_node, BAD_CAST "type", BAD_CAST AGS_AUTOMATION_CLIPBOARD_TYPE);
  xmlNewProp(automation_node, BAD_CAST "version", BAD_CAST AGS_AUTOMATION_CLIPBOARD_VERSION);
  xmlNewProp(automation_node, BAD_CAST "format", BAD_CAST AGS_AUTOMATION_CLIPBOARD_FORMAT);
  xmlNewProp(automation_node, "control-name", automation->control_name);
  xmlNewProp(automation_node, "line", g_strdup_printf("%u", automation->line));

  selection = automation->selection;

  if(selection != NULL){
    x_boundary = AGS_ACCELERATION(selection->data)->x;
    y_boundary = G_MAXDOUBLE;
  }else{
    x_boundary = 0;
    y_boundary = 0.0;
  }

  while(selection != NULL){
    acceleration = AGS_ACCELERATION(selection->data);
    current_acceleration = xmlNewChild(automation_node, NULL, BAD_CAST "acceleration", NULL);

    xmlNewProp(current_acceleration, BAD_CAST "x", BAD_CAST g_strdup_printf("%u", acceleration->x));
    xmlNewProp(current_acceleration, BAD_CAST "y", BAD_CAST g_strdup_printf("%f", acceleration->y));

    if(y_boundary > acceleration->y){
      y_boundary = acceleration->y;
    }

    selection = selection->next;
  }

  xmlNewProp(automation_node, BAD_CAST "x-boundary", BAD_CAST g_strdup_printf("%u", x_boundary));
  xmlNewProp(automation_node, BAD_CAST "y-boundary", BAD_CAST g_strdup_printf("%f", y_boundary));
  
  return(automation_node);
}

/**
 * ags_automation_cut_selection:
 * @automation: an #AgsAutomation
 *
 * Cut selection to clipboard.
 *
 * Returns: the selection as XML.
 *
 * Since: 1.0.0
 */
xmlNode*
ags_automation_cut_selection(AgsAutomation *automation)
{
  xmlNode *automation_node;
  GList *selection, *acceleration;
  
  automation_node = ags_automation_copy_selection(automation);

  selection = automation->selection;
  acceleration = automation->acceleration;

  while(selection != NULL){
    acceleration = g_list_find(acceleration, selection->data);

    if(acceleration->prev == NULL){
      automation->acceleration = g_list_remove_link(acceleration, acceleration);
      acceleration = automation->acceleration;
    }else{
      GList *next_acceleration;

      next_acceleration = acceleration->next;
      acceleration->prev->next = next_acceleration;

      if(next_acceleration != NULL){
	next_acceleration->prev = acceleration->prev;
      }
      
      g_list_free1(acceleration);

      acceleration = next_acceleration;
    }

    AGS_ACCELERATION(selection->data)->flags &= (~AGS_ACCELERATION_IS_SELECTED);
    g_object_unref(selection->data);

    selection = selection->next;
  }

  ags_automation_free_selection(automation);

  return(automation_node);
}

void
ags_automation_insert_native_scale_from_clipboard(AgsAutomation *automation,
						  xmlNode *root_node, char *version,
						  char *x_boundary, char *y_boundary,
						  gboolean from_x_offset, guint x_offset,
						  gboolean from_y_offset, guint y_offset,
						  gboolean match_line, gboolean no_duplicates)
{
  gboolean match_timestamp;
  
  auto void ags_automation_insert_from_clipboard_version_0_4_3();
  
  void ags_automation_insert_from_clipboard_version_0_4_3()
  {
    AgsAcceleration *acceleration;
    xmlNode *node;
    char *endptr;
    guint x_boundary_val, y_boundary_val;
    char *x, *y;
    guint x_val;
    gdouble y_val;
    guint base_x_difference, base_y_difference;
    gboolean subtract_x, subtract_y;

    node = root_node->children;

    /* retrieve x values for resetting */
    if(from_x_offset){
      if(x_boundary != NULL){
	errno = 0;
	x_boundary_val = strtoul(x_boundary, &endptr, 10);

	if(errno == ERANGE){
	  goto dont_reset_x_offset;
	} 

	if(x_boundary == endptr){
	  goto dont_reset_x_offset;
	}

	if(x_boundary_val < x_offset){
	  base_x_difference = x_offset - x_boundary_val;
	  subtract_x = FALSE;
	}else{
	  base_x_difference = x_boundary_val - x_offset;
	  subtract_x = TRUE;
	}
      }else{
      dont_reset_x_offset:
	from_x_offset = FALSE;
      }
    }

    /* retrieve y values for resetting */
    if(from_y_offset){
      if(y_boundary != NULL){
	errno = 0;
	y_boundary_val = strtoul(y_boundary, &endptr, 10);

	if(errno == ERANGE){
	  goto dont_reset_y_offset;
	} 

	if(y_boundary == endptr){
	  goto dont_reset_y_offset;
	}

	if(y_boundary_val < y_offset){
	  base_y_difference = y_offset - y_boundary_val;
	  subtract_y = FALSE;
	}else{
	  base_y_difference = y_boundary_val - y_offset;
	  subtract_y = TRUE;
	}
      }else{
      dont_reset_y_offset:
	from_y_offset = FALSE;
      }
    }
    
    for(; node != NULL; node = node->next){
      if(node->type == XML_ELEMENT_NODE && !xmlStrncmp("acceleration", node->name, 5)){
	/* retrieve x0 offset */
	x = xmlGetProp(node, "x");

	if(x == NULL){
	  continue;
	}

	errno = 0;
	x_val = strtoul(x, &endptr, 10);

	if(errno == ERANGE){
	  continue;
	} 

	if(x == endptr){
	  continue;
	}
	
	/* retrieve y offset */
	y = xmlGetProp(node, "y");

	if(y == NULL){
	  continue;
	}

	errno = 0;
	y_val = strtod(y,
		       &endptr);

	if(errno == ERANGE){
	  continue;
	} 

	if(y == endptr){
	  continue;
	}

	/* calculate new offset */
	if(from_x_offset){
	  errno = 0;

	  if(subtract_x){
	    x_val -= base_x_difference;

	    if(errno != 0){
	      continue;
	    }
	  }else{
	    x_val += base_x_difference;

	    if(errno != 0){
	      continue;
	    }
	  }
	}

	if(from_y_offset){
	  errno = 0;

	  if(subtract_y){
	    y_val -= base_y_difference;
	  }else{
	    y_val += base_y_difference;
	  }

	  if(errno != 0){
	    continue;
	  }
	}

	/* check duplicate */
	if(no_duplicates &&
	   ags_automation_find_point(automation,
				     x_val, y_val,
				     FALSE) != NULL){
	  continue;
	}
	
	/* add acceleration */
	if(!match_timestamp ||
	   x_val < automation->timestamp->timer.ags_offset.offset + AGS_AUTOMATION_DEFAULT_OFFSET){
	  acceleration = ags_acceleration_new();
	  
	  acceleration->x = x_val;
	  acceleration->y = y_val;

#ifdef AGS_DEBUG
	  g_message("adding acceleration at: [%u|%f]\n", x_val, y_val);
#endif
	  
	  ags_automation_add_acceleration(automation,
					  acceleration,
					  FALSE);
	}
      }
    }
  }
  
  if(!AGS_IS_AUTOMATION(automation)){
    return;
  }  

  match_timestamp = TRUE;
  
  if(!xmlStrncmp("0.4.3", version, 6)){
    ags_automation_insert_from_clipboard_version_0_4_3();
  }else if(!xmlStrncmp("1.3.0", version, 6)){
    match_timestamp = TRUE;
    
    if(match_line &&
       automation->line != g_ascii_strtoull(xmlGetProp(root_node,
						       "line"),
					    NULL,
					    10)){
      return;
    }

    ags_automation_insert_from_clipboard_version_0_4_3();
  }
}

void
ags_automation_merge_clipboard(xmlNode *audio_node,
			       xmlNode *automation_node)
{
  xmlNode *find_automation;
  xmlNode *find_acceleration;
  xmlNode *child;

  auto gboolean ags_automation_merge_clipboard_find_acceleration(xmlNode *automation_node,
								 xmlChar *x,
								 xmlChar *y);
  
  gboolean ags_automation_merge_clipboard_find_acceleration(xmlNode *automation_node,
							    xmlChar *x,
							    xmlChar *y){
    xmlNode *child;
    
    child = automation_node->children;
  
    while(child != NULL){
      if(child->type == XML_ELEMENT_NODE){
	if(!xmlStrncmp(child->name,
		       "acceleration",
		       13)){
	  if(!xmlStrcmp(xmlGetProp(child,
				   "x"),
			x) &&
	     !xmlStrcmp(xmlGetProp(child,
				   "y"),
			y)){
	    return(TRUE);
	  }
	}
      }

      child = child->next;
    }
    
    return(FALSE);
  }
  
  if(audio_node == NULL ||
     automation_node == NULL){
    return;
  }

  find_automation = audio_node->children;

  while(find_automation != NULL){
    if(find_automation->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp(find_automation->name,
		     "automation",
		     11)){
	if(!xmlStrcmp(xmlGetProp(find_automation,
				 "line"),
		      xmlGetProp(automation_node,
				 "line")) &&
	   !xmlStrcmp(xmlGetProp(find_automation,
				 "control-name"),
		      xmlGetProp(automation_node,
				 "control-name"))){
	  break;
	}
      }
    }

    find_automation = find_automation->next;
  }

  if(find_automation == NULL){
    xmlAddChild(audio_node,
		xmlCopyNode(automation_node,
			    1));
    
    return;
  }

  child = automation_node->children;
  
  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp(child->name,
		     "acceleration",
		     13)){
	if(!ags_automation_merge_clipboard_find_acceleration(find_automation,
							     xmlGetProp(child,
									"x"),
							     xmlGetProp(child,
									"y"))){
	  xmlAddChild(find_automation,
		      xmlCopyNode(child,
				  1));
	}
      }
    }

    child = child->next;
  }
}

void
ags_automation_insert_from_clipboard(AgsAutomation *automation,
				     xmlNode *automation_node,
				     gboolean reset_x_offset, guint x_offset,
				     gboolean reset_y_offset, gdouble y_offset)
{
  ags_automation_insert_from_clipboard_extended(automation,
						automation_node,
						reset_x_offset, x_offset,
						reset_y_offset, y_offset,
						FALSE, FALSE);
}

void
ags_automation_insert_from_clipboard_extended(AgsAutomation *automation,
					      xmlNode *automation_node,
					      gboolean reset_x_offset, guint x_offset,
					      gboolean reset_y_offset, gdouble y_offset,
					      gboolean match_line, gboolean no_duplicates)
{
  char *program, *version, *type, *format;
  char *base_frequency;
  char *x_boundary, *y_boundary;

  while(automation_node != NULL){
    if(automation_node->type == XML_ELEMENT_NODE && !xmlStrncmp("automation", automation_node->name, 11)){
      break;
    }

    automation_node = automation_node->next;
  }

  if(automation_node != NULL){
    program = xmlGetProp(automation_node, "program");

    if(!xmlStrncmp("ags", program, 4)){
      version = xmlGetProp(automation_node, "version");
      type = xmlGetProp(automation_node, "type");
      format = xmlGetProp(automation_node, "format");

      if(!xmlStrcmp(AGS_AUTOMATION_CLIPBOARD_FORMAT,
		    format) ||
	 !xmlStrcmp(AGS_AUTOMATION_CLIPBOARD_LEGACY_FORMAT,
		    format)){
	x_boundary = xmlGetProp(automation_node, "x_boundary");
	y_boundary = xmlGetProp(automation_node, "y_boundary");

	ags_automation_insert_native_scale_from_clipboard(automation,
							  automation_node, version,
							  x_boundary, y_boundary,
							  reset_x_offset, x_offset,
							  reset_y_offset, y_offset,
							  match_line, no_duplicates);
      }
    }
  }
}

GList*
ags_automation_get_current(AgsAutomation *automation)
{
  //TODO:JK: implement me

  return(NULL);
}

/**
 * ags_automation_get_specifier_unique:
 * @automation: a #GList containing #AgsAutomation
 *
 * Retrieve automation port specifier.
 *
 * Returns: a %NULL terminated string array
 *
 * Since: 1.0.0
 */
gchar**
ags_automation_get_specifier_unique(GList *automation)
{
  gchar **specifier, **current;

  guint length, i;
  gboolean contains_control_name;
    
  specifier = (gchar **) malloc(sizeof(gchar*));
  specifier[0] = NULL;
  length = 1;
  
  while(automation != NULL){
    current = specifier;
    
#ifdef HAVE_GLIB_2_44
    contains_control_name = g_strv_contains(specifier,
					    AGS_AUTOMATION(automation->data)->control_name);
#else
    contains_control_name = ags_strv_contains(specifier,
					      AGS_AUTOMATION(automation->data)->control_name);
#endif
    
    if(!contains_control_name){
      specifier = (gchar **) realloc(specifier,
				     (length + 1) * sizeof(gchar *));
      specifier[length - 1] = AGS_AUTOMATION(automation->data)->control_name;
      specifier[length] = NULL;

      length++;
    }
      
    automation = automation->next;
  }
    
  return(specifier);
}

/**
 * ags_automation_get_specifier_unique_with_channel_type:
 * @automation: a #GList containing #AgsAutomation
 * @channel_type: the channel's #GType
 *
 * Retrieve automation port specifier.
 *
 * Returns: a %NULL terminated string array
 *
 * Since: 1.3.0
 */
gchar**
ags_automation_get_specifier_unique_with_channel_type(GList *automation,
						      GType channel_type)
{
  gchar **specifier, **current;

  guint length, i;
  gboolean contains_control_name;
    
  specifier = (gchar **) malloc(sizeof(gchar*));
  specifier[0] = NULL;
  length = 1;
  
  while(automation != NULL){
    current = specifier;

    if(AGS_AUTOMATION(automation->data)->channel_type != channel_type){
      automation = automation->next;

      continue;
    }
    
#ifdef HAVE_GLIB_2_44
    contains_control_name = g_strv_contains(specifier,
					    AGS_AUTOMATION(automation->data)->control_name);
#else
    contains_control_name = ags_strv_contains(specifier,
					      AGS_AUTOMATION(automation->data)->control_name);
#endif
    
    if(!contains_control_name){
      specifier = (gchar **) realloc(specifier,
				     (length + 1) * sizeof(gchar *));
      specifier[length - 1] = AGS_AUTOMATION(automation->data)->control_name;
      specifier[length] = NULL;

      length++;
    }
      
    automation = automation->next;
  }
    
  return(specifier);
}

/**
 * ags_automation_find_specifier:
 * @automation: a #GList-struct containing #AgsAutomation
 * @specifier: the string specifier to find
 *
 * Find port specifier.
 *
 * Returns: Next matching #GList
 *
 * Since: 1.0.0
 */
GList*
ags_automation_find_specifier(GList *automation,
			      gchar *specifier)
{
  while(automation != NULL){
    if(!g_ascii_strcasecmp(AGS_AUTOMATION(automation->data)->control_name,
			   specifier)){
      break;
    }

    automation = automation->next;
  }

  return(automation);
}

/**
 * ags_automation_find_channel_type_with_control_name:
 * @automation: the #GList-struct containing #AgsAutomation
 * @channel_type: the #AgsPort to match
 * 
 * Find automation by port.
 * 
 * Returns: next matching automation as #GList-struct or %NULL if not found
 * 
 * Since: 1.3.0
 */
GList*
ags_automation_find_channel_type_with_control_name(GList *automation,
						   GType channel_type, gchar *specifier)
{
  if(automation == NULL){
    return(NULL);
  }

  while(automation != NULL){
    if(AGS_AUTOMATION(automation->data)->channel_type == channel_type &&
       !g_strcmp0(AGS_AUTOMATION(automation->data)->control_name,
		  specifier)){
      break;
    }
    
    automation = automation->next;
  }

  return(automation);
}

/**
 * ags_automation_find_specifier_with_type_and_line:
 * @automation: a #GList-struct containing #AgsAutomation
 * @specifier: the string specifier to find
 * @channel_type: the channel #GType
 * @line: the line
 *
 * Find port specifier with channel type and line.
 *
 * Returns: Next matching #GList-struct
 *
 * Since: 1.0.0
 */
GList*
ags_automation_find_specifier_with_type_and_line(GList *automation,
						 gchar *specifier,
						 GType channel_type,
						 guint line)
{
  if(specifier == NULL){
    return(NULL);
  }
  
  while(automation != NULL){
    if(AGS_AUTOMATION(automation->data)->control_name != NULL &&
       !g_ascii_strcasecmp(AGS_AUTOMATION(automation->data)->control_name,
			   specifier) &&
       AGS_AUTOMATION(automation->data)->channel_type == channel_type &&
       AGS_AUTOMATION(automation->data)->line == AGS_AUTOMATION(automation->data)->line){

      break;
    }

    automation = automation->next;
  }

  return(automation);
}

/**
 * ags_automation_get_value:
 * @automation: the #AgsAutomation
 * @x: the x-offset
 * @x_end: the x-end-offset
 * @use_prev_on_failure: if %TRUE use previous value, else return G_MAXUINT
 * @value: the return location of value
 *
 * Get automation value.
 *
 * Returns: the x_offset
 *
 * Since: 1.0.0
 */
guint
ags_automation_get_value(AgsAutomation *automation,
			 guint x, guint x_end,
			 gboolean use_prev_on_failure,
			 GValue *value)
{
  AgsPort *port;

  GList *acceleration;

  guint ret_x;
  
  port = (AgsPort *) automation->port;
  acceleration = automation->acceleration;

  ret_x = 0;
  
  if(acceleration != NULL){
    while(acceleration != NULL){
      if(AGS_ACCELERATION(acceleration->data)->x >= x &&
	 AGS_ACCELERATION(acceleration->data)->x < x_end){
	break;
      }

      if(AGS_ACCELERATION(acceleration->data)->x > x_end){
	if(use_prev_on_failure){
	  acceleration = acceleration->prev;

	  break;
	}else{
	  return(G_MAXUINT);
	}
      }

      acceleration = acceleration->next;
    }
    
    if(acceleration == NULL){
      return(G_MAXUINT);
    }
    
    ret_x = AGS_ACCELERATION(acceleration->data)->x;
  }
  
  if(!port->port_value_is_pointer){
    if(port->port_value_type == G_TYPE_BOOLEAN){
      gboolean current;

      current = FALSE;

      if(acceleration == NULL){
	if(automation->default_value != 0){
	  current = TRUE;
	}
      }else{
	if(AGS_ACCELERATION(acceleration->data)->y != 0){
	  current = TRUE;
	}
      }
      
      g_value_init(value,
		   G_TYPE_BOOLEAN);
      g_value_set_boolean(value,
			  current);
    }else if(port->port_value_type == G_TYPE_INT64){
      gint64 current;
      
      current = floor(AGS_ACCELERATION(acceleration->data)->y);

      g_value_init(value,
		   G_TYPE_INT64);
      g_value_set_int64(value,
			current);
    }else if(port->port_value_type == G_TYPE_UINT64){
      guint64 current;

      current = floor(AGS_ACCELERATION(acceleration->data)->y);
      
      g_value_init(value,
		   G_TYPE_UINT64);
      g_value_set_uint64(value,
			 current);
    }else if(port->port_value_type == G_TYPE_FLOAT){
      gfloat current;
	
      current = AGS_ACCELERATION(acceleration->data)->y;
      
      g_value_init(value,
		   G_TYPE_FLOAT);
      g_value_set_float(value,
			current);
    }else if(port->port_value_type == G_TYPE_DOUBLE){
      gdouble current;

      current = AGS_ACCELERATION(acceleration->data)->y;
      
      g_value_init(value,
		   G_TYPE_DOUBLE);
      g_value_set_double(value,
			 current);
    }else if(port->port_value_type == G_TYPE_POINTER){
      g_warning("ags_automation.c - unsupported value type pointer");
    }else if(port->port_value_type == G_TYPE_OBJECT){
      g_warning("ags_automation.c - unsupported value type object");
    }else{
      g_warning("ags_automation.c - unknown type");
    }
  }else{
    g_warning("ags_automation.c - unsupported value type pointer");
  }

  return(ret_x);
}

/**
 * ags_automation_new:
 * @audio: an #AgsAudio
 * @line: the line to apply
 * @channel_type: the channel type
 * @control_name: the control name
 *
 * Creates an #AgsAutomation.
 *
 * Returns: a new #AgsAutomation
 *
 * Since: 1.0.0
 */
AgsAutomation*
ags_automation_new(GObject *audio,
		   guint line,
		   GType channel_type,
		   gchar *control_name)
{
  AgsAutomation *automation;

  automation = (AgsAutomation *) g_object_new(AGS_TYPE_AUTOMATION,
					      "audio", audio,
					      "line", line,
					      "channel-type", channel_type,
					      "control-name", control_name,
					      NULL);

  return(automation);
}
