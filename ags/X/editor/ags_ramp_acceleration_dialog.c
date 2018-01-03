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

#include <ags/X/editor/ags_ramp_acceleration_dialog.h>
#include <ags/X/editor/ags_ramp_acceleration_dialog_callbacks.h>

#include <ags/libags.h>
#include <ags/libags-audio.h>

#include <ags/X/ags_window.h>
#include <ags/X/ags_automation_window.h>
#include <ags/X/ags_automation_editor.h>
#include <ags/X/ags_machine.h>

#include <ags/i18n.h>

void ags_ramp_acceleration_dialog_class_init(AgsRampAccelerationDialogClass *ramp_acceleration_dialog);
void ags_ramp_acceleration_dialog_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_ramp_acceleration_dialog_applicable_interface_init(AgsApplicableInterface *applicable);
void ags_ramp_acceleration_dialog_init(AgsRampAccelerationDialog *ramp_acceleration_dialog);
void ags_ramp_acceleration_dialog_set_property(GObject *gobject,
					       guint prop_id,
					       const GValue *value,
					       GParamSpec *param_spec);
void ags_ramp_acceleration_dialog_get_property(GObject *gobject,
					       guint prop_id,
					       GValue *value,
					       GParamSpec *param_spec);
void ags_ramp_acceleration_dialog_finalize(GObject *gobject);
void ags_ramp_acceleration_dialog_connect(AgsConnectable *connectable);
void ags_ramp_acceleration_dialog_disconnect(AgsConnectable *connectable);
void ags_ramp_acceleration_dialog_set_update(AgsApplicable *applicable, gboolean update);
void ags_ramp_acceleration_dialog_apply(AgsApplicable *applicable);
void ags_ramp_acceleration_dialog_reset(AgsApplicable *applicable);
gboolean ags_ramp_acceleration_dialog_delete_event(GtkWidget *widget, GdkEventAny *event);

/**
 * SECTION:ags_ramp_acceleration_dialog
 * @short_description: ramp tool
 * @title: AgsRampAccelerationDialog
 * @section_id:
 * @include: ags/X/editor/ags_ramp_acceleration_dialog.h
 *
 * The #AgsRampAccelerationDialog lets you ramp accelerations.
 */

enum{
  PROP_0,
  PROP_APPLICATION_CONTEXT,
  PROP_MAIN_WINDOW,
};

static gpointer ags_ramp_acceleration_dialog_parent_class = NULL;

GType
ags_ramp_acceleration_dialog_get_type(void)
{
  static GType ags_type_ramp_acceleration_dialog = 0;

  if (!ags_type_ramp_acceleration_dialog){
    static const GTypeInfo ags_ramp_acceleration_dialog_info = {
      sizeof (AgsRampAccelerationDialogClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_ramp_acceleration_dialog_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsRampAccelerationDialog),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_ramp_acceleration_dialog_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_ramp_acceleration_dialog_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    static const GInterfaceInfo ags_applicable_interface_info = {
      (GInterfaceInitFunc) ags_ramp_acceleration_dialog_applicable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_ramp_acceleration_dialog = g_type_register_static(GTK_TYPE_DIALOG,
							       "AgsRampAccelerationDialog", &ags_ramp_acceleration_dialog_info,
							       0);
    
    g_type_add_interface_static(ags_type_ramp_acceleration_dialog,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);

    g_type_add_interface_static(ags_type_ramp_acceleration_dialog,
				AGS_TYPE_APPLICABLE,
				&ags_applicable_interface_info);
  }

  return (ags_type_ramp_acceleration_dialog);
}

void
ags_ramp_acceleration_dialog_class_init(AgsRampAccelerationDialogClass *ramp_acceleration_dialog)
{
  GObjectClass *gobject;
  GtkWidgetClass *widget;

  GParamSpec *param_spec;

  ags_ramp_acceleration_dialog_parent_class = g_type_class_peek_parent(ramp_acceleration_dialog);

  /* GObjectClass */
  gobject = (GObjectClass *) ramp_acceleration_dialog;

  gobject->set_property = ags_ramp_acceleration_dialog_set_property;
  gobject->get_property = ags_ramp_acceleration_dialog_get_property;

  gobject->finalize = ags_ramp_acceleration_dialog_finalize;

  /* properties */
  /**
   * AgsRampAccelerationDialog:application-context:
   *
   * The assigned #AgsApplicationContext to give control of application.
   * 
   * Since: 1.0.0
   */
  param_spec = g_param_spec_object("application-context",
				   i18n_pspec("assigned application context"),
				   i18n_pspec("The AgsApplicationContext it is assigned with"),
				   G_TYPE_OBJECT,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_APPLICATION_CONTEXT,
				  param_spec);

  /**
   * AgsRampAccelerationDialog:main-window:
   *
   * The assigned #AgsWindow.
   * 
   * Since: 1.0.0
   */
  param_spec = g_param_spec_object("main-window",
				   i18n_pspec("assigned main window"),
				   i18n_pspec("The assigned main window"),
				   AGS_TYPE_WINDOW,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_MAIN_WINDOW,
				  param_spec);

  /* GtkWidgetClass */
  widget = (GtkWidgetClass *) ramp_acceleration_dialog;

  widget->delete_event = ags_ramp_acceleration_dialog_delete_event;
}

void
ags_ramp_acceleration_dialog_connectable_interface_init(AgsConnectableInterface *connectable)
{
  connectable->is_ready = NULL;
  connectable->is_connected = NULL;
  connectable->connect = ags_ramp_acceleration_dialog_connect;
  connectable->disconnect = ags_ramp_acceleration_dialog_disconnect;
}

void
ags_ramp_acceleration_dialog_applicable_interface_init(AgsApplicableInterface *applicable)
{
  applicable->set_update = ags_ramp_acceleration_dialog_set_update;
  applicable->apply = ags_ramp_acceleration_dialog_apply;
  applicable->reset = ags_ramp_acceleration_dialog_reset;
}

void
ags_ramp_acceleration_dialog_init(AgsRampAccelerationDialog *ramp_acceleration_dialog)
{
  GtkVBox *vbox;
  GtkHBox *hbox;
  GtkLabel *label;

  ramp_acceleration_dialog->flags = 0;

  g_object_set(ramp_acceleration_dialog,
	       "title", i18n("ramp accelerations"),
	       NULL);

  vbox = (GtkVBox *) gtk_vbox_new(FALSE,
				  0);
  gtk_box_pack_start((GtkBox *) ramp_acceleration_dialog->dialog.vbox,
		     GTK_WIDGET(vbox),
		     FALSE, FALSE,
		     0);  

  /* automation */
  ramp_acceleration_dialog->port = (GtkVBox *) gtk_combo_box_text_new();
  gtk_box_pack_start((GtkBox *) vbox,
		     GTK_WIDGET(ramp_acceleration_dialog->port),
		     FALSE, FALSE,
		     0);  
  
  /* ramp x0 - hbox */
  hbox = (GtkVBox *) gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start((GtkBox *) vbox,
		     GTK_WIDGET(hbox),
		     FALSE, FALSE,
		     0);

  /* ramp x0 - label */
  label = (GtkLabel *) gtk_label_new(i18n("ramp x0"));
  gtk_box_pack_start((GtkBox *) hbox,
		     GTK_WIDGET(label),
		     FALSE, FALSE,
		     0);

  /* ramp x0 - spin button */
  ramp_acceleration_dialog->ramp_x0 = (GtkSpinButton *) gtk_spin_button_new_with_range(0.0,
										       AGS_RAMP_ACCELERATION_MAX_BEATS,
										       0.25);
  gtk_spin_button_set_digits(ramp_acceleration_dialog->ramp_x0,
			     2);
  gtk_spin_button_set_value(ramp_acceleration_dialog->ramp_x0,
			    0.0);
  gtk_box_pack_start((GtkBox *) hbox,
		     GTK_WIDGET(ramp_acceleration_dialog->ramp_x0),
		     FALSE, FALSE,
		     0);

  /* ramp y0 - hbox */
  hbox = (GtkVBox *) gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start((GtkBox *) vbox,
		     GTK_WIDGET(hbox),
		     FALSE, FALSE,
		     0);

  /* ramp y0 - label */
  label = (GtkLabel *) gtk_label_new(i18n("ramp y0"));
  gtk_box_pack_start((GtkBox *) hbox,
		     GTK_WIDGET(label),
		     FALSE, FALSE,
		     0);

  /* ramp y0 - spin button */
  ramp_acceleration_dialog->ramp_y0 = (GtkSpinButton *) gtk_spin_button_new_with_range(0.0,
										       1.0,
										       0.001);
  gtk_spin_button_set_value(ramp_acceleration_dialog->ramp_y0,
			    0.0);
  gtk_box_pack_start((GtkBox *) hbox,
		     GTK_WIDGET(ramp_acceleration_dialog->ramp_y0),
		     FALSE, FALSE,
		     0);
  
  /* ramp x1 - hbox */
  hbox = (GtkVBox *) gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start((GtkBox *) vbox,
		     GTK_WIDGET(hbox),
		     FALSE, FALSE,
		     0);

  /* ramp x1 - label */
  label = (GtkLabel *) gtk_label_new(i18n("ramp x1"));
  gtk_box_pack_start((GtkBox *) hbox,
		     GTK_WIDGET(label),
		     FALSE, FALSE,
		     0);

  /* ramp x1 - spin button */
  ramp_acceleration_dialog->ramp_x1 = (GtkSpinButton *) gtk_spin_button_new_with_range(0.0,
										       AGS_RAMP_ACCELERATION_MAX_BEATS,
										       0.25);
  gtk_spin_button_set_digits(ramp_acceleration_dialog->ramp_x1,
			     2);
  gtk_spin_button_set_value(ramp_acceleration_dialog->ramp_x1,
			    0.0);
  gtk_box_pack_start((GtkBox *) hbox,
		     GTK_WIDGET(ramp_acceleration_dialog->ramp_x1),
		     FALSE, FALSE,
		     0);

  /* ramp y1 - hbox */
  hbox = (GtkVBox *) gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start((GtkBox *) vbox,
		     GTK_WIDGET(hbox),
		     FALSE, FALSE,
		     0);

  /* ramp y1 - label */
  label = (GtkLabel *) gtk_label_new(i18n("ramp y1"));
  gtk_box_pack_start((GtkBox *) hbox,
		     GTK_WIDGET(label),
		     FALSE, FALSE,
		     0);

  /* ramp y1 - spin button */
  ramp_acceleration_dialog->ramp_y1 = (GtkSpinButton *) gtk_spin_button_new_with_range(0.0,
										       1.0,
										       0.001);
  gtk_spin_button_set_value(ramp_acceleration_dialog->ramp_y1,
			    0.0);
  gtk_box_pack_start((GtkBox *) hbox,
		     GTK_WIDGET(ramp_acceleration_dialog->ramp_y1),
		     FALSE, FALSE,
		     0);

  /* ramp step count - hbox */
  hbox = (GtkVBox *) gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start((GtkBox *) vbox,
		     GTK_WIDGET(hbox),
		     FALSE, FALSE,
		     0);

  /* ramp step count - label */
  label = (GtkLabel *) gtk_label_new(i18n("ramp step count"));
  gtk_box_pack_start((GtkBox *) hbox,
		     GTK_WIDGET(label),
		     FALSE, FALSE,
		     0);

  /* ramp step count - spin button */
  ramp_acceleration_dialog->ramp_step_count = (GtkSpinButton *) gtk_spin_button_new_with_range(0.0,
											       AGS_RAMP_ACCELERATION_MAX_STEPS,
											       1.0);
  gtk_spin_button_set_value(ramp_acceleration_dialog->ramp_step_count,
			    0.0);
  gtk_box_pack_start((GtkBox *) hbox,
		     GTK_WIDGET(ramp_acceleration_dialog->ramp_step_count),
		     FALSE, FALSE,
		     0);

  /* dialog buttons */
  gtk_dialog_add_buttons((GtkDialog *) ramp_acceleration_dialog,
			 GTK_STOCK_APPLY, GTK_RESPONSE_APPLY,
			 GTK_STOCK_OK, GTK_RESPONSE_OK,
			 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			 NULL);
}

void
ags_ramp_acceleration_dialog_set_property(GObject *gobject,
					  guint prop_id,
					  const GValue *value,
					  GParamSpec *param_spec)
{
  AgsRampAccelerationDialog *ramp_acceleration_dialog;

  ramp_acceleration_dialog = AGS_RAMP_ACCELERATION_DIALOG(gobject);

  switch(prop_id){
  case PROP_APPLICATION_CONTEXT:
    {
      AgsApplicationContext *application_context;

      application_context = (AgsApplicationContext *) g_value_get_object(value);

      if((AgsApplicationContext *) ramp_acceleration_dialog->application_context == application_context){
	return;
      }
      
      if(ramp_acceleration_dialog->application_context != NULL){
	g_object_unref(ramp_acceleration_dialog->application_context);
      }

      if(application_context != NULL){
	g_object_ref(application_context);
      }

      ramp_acceleration_dialog->application_context = (GObject *) application_context;
    }
    break;
  case PROP_MAIN_WINDOW:
    {
      AgsWindow *main_window;

      main_window = (AgsWindow *) g_value_get_object(value);

      if((AgsWindow *) ramp_acceleration_dialog->main_window == main_window){
	return;
      }

      if(ramp_acceleration_dialog->main_window != NULL){
	g_object_unref(ramp_acceleration_dialog->main_window);
      }

      if(main_window != NULL){
	g_object_ref(main_window);
      }

      ramp_acceleration_dialog->main_window = (GObject *) main_window;
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_ramp_acceleration_dialog_get_property(GObject *gobject,
					  guint prop_id,
					  GValue *value,
					  GParamSpec *param_spec)
{
  AgsRampAccelerationDialog *ramp_acceleration_dialog;

  ramp_acceleration_dialog = AGS_RAMP_ACCELERATION_DIALOG(gobject);

  switch(prop_id){
  case PROP_APPLICATION_CONTEXT:
    {
      g_value_set_object(value, ramp_acceleration_dialog->application_context);
    }
    break;
  case PROP_MAIN_WINDOW:
    {
      g_value_set_object(value, ramp_acceleration_dialog->main_window);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_ramp_acceleration_dialog_connect(AgsConnectable *connectable)
{
  AgsAutomationEditor *automation_editor;
  AgsRampAccelerationDialog *ramp_acceleration_dialog;

  ramp_acceleration_dialog = AGS_RAMP_ACCELERATION_DIALOG(connectable);

  if((AGS_RAMP_ACCELERATION_DIALOG_CONNECTED & (ramp_acceleration_dialog->flags)) != 0){
    return;
  }

  ramp_acceleration_dialog->flags |= AGS_RAMP_ACCELERATION_DIALOG_CONNECTED;

  automation_editor = AGS_WINDOW(ramp_acceleration_dialog->main_window)->automation_window->automation_editor;
  
  g_signal_connect(ramp_acceleration_dialog, "response",
		   G_CALLBACK(ags_ramp_acceleration_dialog_response_callback), ramp_acceleration_dialog);

  g_signal_connect(ramp_acceleration_dialog->port, "changed",
		   G_CALLBACK(ags_ramp_acceleration_dialog_port_callback), ramp_acceleration_dialog);

  /* machine changed */
  g_signal_connect_after(automation_editor, "machine-changed",
			 G_CALLBACK(ags_ramp_acceleration_dialog_machine_changed_callback), ramp_acceleration_dialog);
}

void
ags_ramp_acceleration_dialog_disconnect(AgsConnectable *connectable)
{
  AgsAutomationEditor *automation_editor;
  AgsRampAccelerationDialog *ramp_acceleration_dialog;

  ramp_acceleration_dialog = AGS_RAMP_ACCELERATION_DIALOG(connectable);

  if((AGS_RAMP_ACCELERATION_DIALOG_CONNECTED & (ramp_acceleration_dialog->flags)) == 0){
    return;
  }

  ramp_acceleration_dialog->flags &= (~AGS_RAMP_ACCELERATION_DIALOG_CONNECTED);

  automation_editor = AGS_WINDOW(ramp_acceleration_dialog->main_window)->automation_window->automation_editor;

  g_object_disconnect(G_OBJECT(ramp_acceleration_dialog),
		      "any_signal::response",
		      G_CALLBACK(ags_ramp_acceleration_dialog_response_callback),
		      ramp_acceleration_dialog,
		      NULL);

  g_object_disconnect(G_OBJECT(ramp_acceleration_dialog->port),
		      "any_signal::changed",
		      G_CALLBACK(ags_ramp_acceleration_dialog_port_callback),
		      ramp_acceleration_dialog,
		      NULL);

  g_object_disconnect(G_OBJECT(automation_editor),
		      "any_signal::machine-changed",
		      G_CALLBACK(ags_ramp_acceleration_dialog_machine_changed_callback),
		      ramp_acceleration_dialog,
		      NULL);
}

void
ags_ramp_acceleration_dialog_finalize(GObject *gobject)
{
  AgsRampAccelerationDialog *ramp_acceleration_dialog;

  ramp_acceleration_dialog = (AgsRampAccelerationDialog *) gobject;

  if(ramp_acceleration_dialog->application_context != NULL){
    g_object_unref(ramp_acceleration_dialog->application_context);
  }
  
  G_OBJECT_CLASS(ags_ramp_acceleration_dialog_parent_class)->finalize(gobject);
}

void
ags_ramp_acceleration_dialog_set_update(AgsApplicable *applicable, gboolean update)
{
  /* empty */
}

void
ags_ramp_acceleration_dialog_apply(AgsApplicable *applicable)
{
  AgsRampAccelerationDialog *ramp_acceleration_dialog;
  AgsWindow *window;
  AgsAutomationEditor *automation_editor;
  AgsAutomationToolbar *automation_toolbar;
  AgsMachine *machine;
  AgsNotebook *notebook;

  AgsAudio *audio;
  AgsAutomation *current;
  AgsAcceleration *acceleration;

  AgsMutexManager *mutex_manager;
  AgsTimestamp *timestamp;

  AgsApplicationContext *application_context;

  GList *list_automation;
  GList *list_acceleration, *list_acceleration_start;

  gchar *specifier;
  
  GType channel_type;

  gdouble gui_y;
  gdouble tact;
  
  gdouble c_y0, c_y1;
  gdouble val;
  gdouble upper, lower, range, step;
  gdouble c_upper, c_lower, c_range;

  guint tmp;
  guint x0, x1;
  gdouble y0, y1;
  guint step_count;
  gint line;
  guint i, i_start, i_stop;
  guint nth_match, match_count;

  gboolean is_audio, is_output, is_input;
  gboolean success;
  
  pthread_mutex_t *application_mutex;
  pthread_mutex_t *audio_mutex;
  
  ramp_acceleration_dialog = AGS_RAMP_ACCELERATION_DIALOG(applicable);

  window = ramp_acceleration_dialog->main_window;
  automation_editor = window->automation_window->automation_editor;

  machine = automation_editor->selected_machine;

  if(machine == NULL ||
     automation_editor->focused_automation_edit == NULL){
    return;
  }

  automation_toolbar = automation_editor->automation_toolbar;
  
  specifier = gtk_combo_box_text_get_active_text(ramp_acceleration_dialog->port);

  if(automation_editor->focused_automation_edit->channel_type == G_TYPE_NONE){
    notebook = NULL;
    channel_type = G_TYPE_NONE;

    is_audio = TRUE;
  }else if(automation_editor->focused_automation_edit->channel_type == AGS_TYPE_OUTPUT){
    notebook = automation_editor->output_notebook;
    channel_type = AGS_TYPE_OUTPUT;
      
    is_output = TRUE;
  }else if(automation_editor->focused_automation_edit->channel_type == AGS_TYPE_INPUT){
    notebook = automation_editor->input_notebook;
    channel_type = AGS_TYPE_INPUT;
      
    is_input = TRUE;
  }
  
  audio = machine->audio;

  /* get some values */
  x0 = gtk_spin_button_get_value_as_int(ramp_acceleration_dialog->ramp_x0);
  y0 = gtk_spin_button_get_value(ramp_acceleration_dialog->ramp_y0);

  x1 = gtk_spin_button_get_value_as_int(ramp_acceleration_dialog->ramp_x1);
  y1 = gtk_spin_button_get_value(ramp_acceleration_dialog->ramp_y1);
  
  step_count = gtk_spin_button_get_value_as_int(ramp_acceleration_dialog->ramp_step_count);

  /* application context and mutex manager */
  application_context = window->application_context;

  mutex_manager = ags_mutex_manager_get_instance();
  application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);

  /* get audio mutex */
  pthread_mutex_lock(application_mutex);

  audio_mutex = ags_mutex_manager_lookup(mutex_manager,
					 (GObject *) audio);
  
  pthread_mutex_unlock(application_mutex);  

  /* remove acceleration of region */
  line = 0;
  
  while(notebook == NULL ||
	(line = ags_notebook_next_active_tab(notebook,
					     line)) != -1){
    list_automation = audio->automation;

    while((list_automation = ags_automation_find_specifier_with_type_and_line(list_automation,
									      specifier,
									      channel_type,
									      line)) != NULL){
      current = AGS_AUTOMATION(list_automation->data);

      if(current->timestamp->timer.ags_offset.offset < x0){
	list_automation = list_automation->next;
	
	continue;
      }

      if(current->timestamp->timer.ags_offset.offset > x1){
	break;
      }
      
      upper = current->upper;
      lower = current->lower;
	
      range = upper - lower;

      if(AGS_PORT(current->port)->conversion != NULL){
	c_upper = ags_conversion_convert(AGS_PORT(current->port)->conversion,
					 upper,
					 FALSE);
	c_lower = ags_conversion_convert(AGS_PORT(current->port)->conversion,
					 lower,
					 FALSE);
	c_range = c_upper - c_lower;
      }else{
	c_upper = upper;
	c_lower = lower;
	  
	c_range = range;
      }
	
      if(range == 0.0){
	list_automation = list_automation->next;
	g_warning("ags_ramp_acceleration_dialog.c - range = 0.0");
	  
	continue;
      }
	
      /* check steps */
      gui_y = current->steps;

      val = c_lower + (gui_y * (c_range / current->steps));
      c_y0 = val;

      /* conversion */
      if(AGS_PORT(current->port)->conversion != NULL){
	c_y0 = ags_conversion_convert(AGS_PORT(current->port)->conversion,
				      c_y0,
				      TRUE);
      }

      /* check steps */
      gui_y = 0;

      val = c_lower + (gui_y * (c_range / current->steps));
      c_y1 = val;

      /* conversion */
      if(AGS_PORT(current->port)->conversion != NULL){
	c_y1 = ags_conversion_convert(AGS_PORT(current->port)->conversion,
				      c_y1,
				      TRUE);
      }

      /* find and remove region */
      list_acceleration =
	list_acceleration_start = ags_automation_find_region(list_automation->data,
							     x0, c_y0,
							     x1, c_y1,
							     FALSE);
      
      while(list_acceleration != NULL){
	current->acceleration = g_list_remove(current->acceleration,
					      list_acceleration->data);
	
	list_acceleration = list_acceleration->next;
      }

      g_list_free(list_acceleration_start);
      
      list_automation = list_automation->next;
    }

    if(notebook == NULL){
      break;
    }

    line++;
  }
  
  /* ramp acceleration */
  line = 0;

  timestamp = ags_timestamp_new();

  timestamp->flags &= (~AGS_TIMESTAMP_UNIX);
  timestamp->flags |= AGS_TIMESTAMP_OFFSET;
  
  while(notebook == NULL ||
	(line = ags_notebook_next_active_tab(notebook,
					     line)) != -1){
    AgsChannel *channel;
    AgsPort *play_port, *recall_port;

    guint j;
    guint tmp;
    
    play_port = NULL;
    recall_port = NULL;
    
    if(channel_type == AGS_TYPE_OUTPUT){
      channel = ags_channel_nth(machine->audio->output,
				line);

      play_port = ags_channel_find_port_by_specifier_and_scope(channel,
							       specifier, TRUE);

      recall_port = ags_channel_find_port_by_specifier_and_scope(channel,
								 specifier, FALSE);
    }else if(channel_type == AGS_TYPE_INPUT){
      channel = ags_channel_nth(machine->audio->input,
				line);

      play_port = ags_channel_find_port_by_specifier_and_scope(channel,
							       specifier, TRUE);

      recall_port = ags_channel_find_port_by_specifier_and_scope(channel,
								 specifier, FALSE);
    }else{
      play_port = ags_audio_find_port_by_specifier_and_scope(machine->audio,
							     specifier, TRUE);

      recall_port = ags_audio_find_port_by_specifier_and_scope(machine->audio,
							       specifier, FALSE);
    }
        
    if(play_port != NULL){
      match_count = 0;
      tmp = x1 - x0;

      if(x0 % (guint) AGS_AUTOMATION_DEFAULT_OFFSET != 0 &&
	 x0 / (guint) AGS_AUTOMATION_DEFAULT_OFFSET != (x1 - AGS_AUTOMATION_DEFAULT_OFFSET) / (guint) AGS_AUTOMATION_DEFAULT_OFFSET){
	match_count = 1;

	tmp -= (x0 % (guint) AGS_AUTOMATION_DEFAULT_OFFSET);
      }

      match_count += (guint) ceil(tmp / (guint) AGS_AUTOMATION_DEFAULT_OFFSET);
    
      success = FALSE;

      timestamp->timer.ags_offset.offset = AGS_AUTOMATION_DEFAULT_OFFSET * floor(x0 / AGS_AUTOMATION_DEFAULT_OFFSET);

      for(nth_match = 0; nth_match < match_count; nth_match++){
	list_automation = play_port->automation;
	list_automation = ags_automation_find_near_timestamp(list_automation, line,
							     timestamp);

	if(list_automation == NULL){
	  current = ags_automation_new(machine->audio,
				       line,
				       channel_type, specifier);
	  g_object_set(current,
		       "port", play_port,
		       NULL);

	  current->timestamp->timer.ags_offset.offset = timestamp->timer.ags_offset.offset;

	  ags_audio_add_automation(machine->audio,
				   current);
	  play_port->automation = ags_automation_add(play_port->automation,
						     current);	  
	}else{
	  current = list_automation->data;
	}

	upper = current->upper;
	lower = current->lower;
	
	range = upper - lower;
	
	if(range == 0.0){
	  g_warning("ags_ramp_acceleration_dialog.c - range = 0.0");
	  
	  break;
	}
	    
	/* ramp value and move offset */
	if(!success){
	  i = 0;

	  if(AGS_AUTOMATION_DEFAULT_OFFSET - tmp > x1){
	    i_stop = step_count;

	    tmp += (x1 - x0);
	  }else{
	    i_stop = tmp / (x1 - x0) * step_count;

	    tmp += AGS_AUTOMATION_DEFAULT_OFFSET;
	  }
	
	  success = TRUE;
	}else{
	  i_stop = (x1 - tmp) / (x1 - x0) * step_count;
	
	  tmp += AGS_AUTOMATION_DEFAULT_OFFSET;
	}
	
	for(; i < step_count && i < i_stop; i++){
	  acceleration = ags_acceleration_new();
	  acceleration->x = ((gdouble) x0 + (gdouble) ((gdouble) (x1 - x0) * (gdouble) ((gdouble) (i) / ((gdouble) step_count))));
	  acceleration->y = ((gdouble) y0 + ((gdouble) (y1 - y0) * (gdouble) ((gdouble) (i) / ((gdouble) step_count))));

#ifdef AGS_DEBUG
	  g_message("line %d - %d %f", line, acceleration->x, acceleration->y);
#endif
	
	  ags_automation_add_acceleration(current,
					  acceleration,
					  FALSE);
	
	}

	timestamp->timer.ags_offset.offset += AGS_AUTOMATION_DEFAULT_OFFSET;
      }
    }

    if(recall_port != NULL){
      match_count = 0;
      tmp = x1 - x0;

      if(x0 % (guint) AGS_AUTOMATION_DEFAULT_OFFSET != 0 &&
	 x0 / (guint) AGS_AUTOMATION_DEFAULT_OFFSET != (x1 - AGS_AUTOMATION_DEFAULT_OFFSET) / (guint) AGS_AUTOMATION_DEFAULT_OFFSET){
	match_count = 1;

	tmp -= (x0 % (guint) AGS_AUTOMATION_DEFAULT_OFFSET);
      }

      match_count += (guint) ceil(tmp / (guint) AGS_AUTOMATION_DEFAULT_OFFSET);
      
      success = FALSE;

      timestamp->timer.ags_offset.offset = AGS_AUTOMATION_DEFAULT_OFFSET * floor(x0 / AGS_AUTOMATION_DEFAULT_OFFSET);

      for(nth_match = 0; nth_match < match_count; nth_match++){
	list_automation = recall_port->automation;
	list_automation = ags_automation_find_near_timestamp(list_automation, line,
							     timestamp);

	if(list_automation == NULL){
	  current = ags_automation_new(machine->audio,
				       line,
				       channel_type, specifier);
	  g_object_set(current,
		       "port", recall_port,
		       NULL);

	  current->timestamp->timer.ags_offset.offset = timestamp->timer.ags_offset.offset;

	  ags_audio_add_automation(machine->audio,
				   current);
	  recall_port->automation = ags_automation_add(recall_port->automation,
						       current);	  
	}else{
	  current = list_automation->data;
	}

	upper = current->upper;
	lower = current->lower;
	
	range = upper - lower;
	
	if(range == 0.0){
	  g_warning("ags_ramp_acceleration_dialog.c - range = 0.0");
	  
	  break;
	}
	    
	/* ramp value and move offset */
	if(!success){
	  i = 0;

	  if(AGS_AUTOMATION_DEFAULT_OFFSET - tmp > x1){
	    i_stop = step_count;

	    tmp += (x1 - x0);
	  }else{
	    i_stop = tmp / (x1 - x0) * step_count;

	    tmp += AGS_AUTOMATION_DEFAULT_OFFSET;
	  }
	
	  success = TRUE;
	}else{
	  i_stop = (x1 - tmp) / (x1 - x0) * step_count;
	
	  tmp += AGS_AUTOMATION_DEFAULT_OFFSET;
	}
	
	for(; i < step_count && i < i_stop; i++){
	  acceleration = ags_acceleration_new();
	  acceleration->x = ((gdouble) x0 + (gdouble) ((gdouble) (x1 - x0) * (gdouble) ((gdouble) (i) / ((gdouble) step_count))));
	  acceleration->y = ((gdouble) y0 + ((gdouble) (y1 - y0) * (gdouble) ((gdouble) (i) / ((gdouble) step_count))));

#ifdef AGS_DEBUG
	  g_message("line %d - %d %f", line, acceleration->x, acceleration->y);
#endif
	
	  ags_automation_add_acceleration(current,
					  acceleration,
					  FALSE);
	
	}

	timestamp->timer.ags_offset.offset += AGS_AUTOMATION_DEFAULT_OFFSET;
      }
    }

    if(notebook == NULL){
      break;
    }
    
    line++;
  }
}

void
ags_ramp_acceleration_dialog_reset(AgsApplicable *applicable)
{
  AgsWindow *window;
  AgsAutomationEditor *automation_editor;
  AgsMachine *machine;
  AgsRampAccelerationDialog *ramp_acceleration_dialog;

  AgsAudio *audio;
  
  AgsMutexManager *mutex_manager;

  gchar **specifier;

  pthread_mutex_t *application_mutex;
  pthread_mutex_t *audio_mutex;

  ramp_acceleration_dialog = AGS_RAMP_ACCELERATION_DIALOG(applicable);

  window = AGS_WINDOW(ramp_acceleration_dialog->main_window);
  automation_editor = window->automation_window->automation_editor;

  machine = automation_editor->selected_machine;

  gtk_list_store_clear(gtk_combo_box_get_model(ramp_acceleration_dialog->port));
  
  if(machine == NULL){
    return;
  }
  
  audio = machine->audio;
  
  /* get mutex manager and application mutex */
  mutex_manager = ags_mutex_manager_get_instance();
  application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);
  
  /* get audio mutex */
  pthread_mutex_lock(application_mutex);

  audio_mutex = ags_mutex_manager_lookup(mutex_manager,
					 (GObject *) audio);
  
  pthread_mutex_unlock(application_mutex);

  /*  */  
  pthread_mutex_lock(audio_mutex);

  specifier = ags_automation_get_specifier_unique(audio->automation);

  for(; *specifier != NULL; specifier++){
    gtk_combo_box_text_append_text(ramp_acceleration_dialog->port,
				   g_strdup(*specifier));
  }

  pthread_mutex_unlock(audio_mutex);
}

gboolean
ags_ramp_acceleration_dialog_delete_event(GtkWidget *widget, GdkEventAny *event)
{
  gtk_widget_hide(widget);

  //  GTK_WIDGET_CLASS(ags_ramp_acceleration_dialog_parent_class)->delete_event(widget, event);

  return(TRUE);
}

/**
 * ags_ramp_acceleration_dialog_new:
 * @main_window: the #AgsWindow
 *
 * Create a new #AgsRampAccelerationDialog.
 *
 * Returns: a new #AgsRampAccelerationDialog
 *
 * Since: 1.0.0
 */
AgsRampAccelerationDialog*
ags_ramp_acceleration_dialog_new(GtkWidget *main_window)
{
  AgsRampAccelerationDialog *ramp_acceleration_dialog;

  ramp_acceleration_dialog = (AgsRampAccelerationDialog *) g_object_new(AGS_TYPE_RAMP_ACCELERATION_DIALOG,
									"main-window", main_window,
									NULL);

  return(ramp_acceleration_dialog);
}
