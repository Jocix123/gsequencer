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

#include <ags/X/ags_export_window.h>
#include <ags/X/ags_export_window_callbacks.h>

#include <ags/libags.h>
#include <ags/libags-audio.h>

#include <ags/X/ags_window.h>
#include <ags/X/ags_navigation.h>
#include <ags/X/ags_export_soundcard.h>

#include <stdlib.h>
#include <ags/i18n.h>

void ags_export_window_class_init(AgsExportWindowClass *export_window);
void ags_export_window_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_export_window_init(AgsExportWindow *export_window);
void ags_export_window_set_property(GObject *gobject,
				    guint prop_id,
				    const GValue *value,
				    GParamSpec *param_spec);
void ags_export_window_get_property(GObject *gobject,
				    guint prop_id,
				    GValue *value,
				    GParamSpec *param_spec);
void ags_export_window_finalize(GObject *gobject);
void ags_export_window_connect(AgsConnectable *connectable);
void ags_export_window_disconnect(AgsConnectable *connectable);
void ags_export_window_show(GtkWidget *widget);
gboolean ags_export_window_delete_event(GtkWidget *widget, GdkEventAny *event);

/**
 * SECTION:ags_export_window
 * @short_description: The export dialog
 * @title: AgsExportWindow
 * @section_id:
 * @include: ags/X/ags_export_window.h
 *
 * #AgsExportWindow lets you export to audio files.
 */

enum{
  PROP_0,
  PROP_APPLICATION_CONTEXT,
  PROP_MAIN_WINDOW,
};

static gpointer ags_export_window_parent_class = NULL;

GType
ags_export_window_get_type()
{
  static volatile gsize g_define_type_id__volatile = 0;

  if(g_once_init_enter (&g_define_type_id__volatile)){
    GType ags_type_export_window = 0;

    static const GTypeInfo ags_export_window_info = {
      sizeof (AgsExportWindowClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_export_window_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsExportWindow),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_export_window_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_export_window_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_export_window = g_type_register_static(GTK_TYPE_WINDOW,
						    "AgsExportWindow", &ags_export_window_info,
						    0);
    
    g_type_add_interface_static(ags_type_export_window,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);

    g_once_init_leave(&g_define_type_id__volatile, ags_type_export_window);
  }

  return g_define_type_id__volatile;
}

void
ags_export_window_class_init(AgsExportWindowClass *export_window)
{
  GObjectClass *gobject;
  GtkWidgetClass *widget;

  GParamSpec *param_spec;

  ags_export_window_parent_class = g_type_class_peek_parent(export_window);

  /* GObjectClass */
  gobject = (GObjectClass *) export_window;

  gobject->set_property = ags_export_window_set_property;
  gobject->get_property = ags_export_window_get_property;

  gobject->finalize = ags_export_window_finalize;

  /* properties */
  /**
   * AgsExportWindow:application-context:
   *
   * The assigned #AgsApplicationContext to give control of application.
   * 
   * Since: 2.0.0
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
   * AgsExportWindow:main-window:
   *
   * The assigned #AgsWindow.
   * 
   * Since: 2.0.0
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
  widget = (GtkWidgetClass *) export_window;

  widget->show = ags_export_window_show;
  widget->delete_event = ags_export_window_delete_event;
}

void
ags_export_window_connectable_interface_init(AgsConnectableInterface *connectable)
{
  connectable->is_ready = NULL;
  connectable->is_connected = NULL;
  connectable->connect = ags_export_window_connect;
  connectable->disconnect = ags_export_window_disconnect;
}

void
ags_export_window_init(AgsExportWindow *export_window)
{
  GtkVBox *vbox;
  GtkHBox *hbox;
  GtkTable *table;
  GtkLabel *label;
  GtkAlignment *alignment;
  
  gchar *str;
  
  export_window->flags = 0;

  g_object_set(export_window,
	       "title", i18n("export to audio data"),
	       NULL);

  export_window->application_context = NULL;

  export_window->main_window = NULL;

  /* pack */
  vbox = (GtkVBox *) gtk_vbox_new(FALSE,
				  0);
  gtk_container_add(GTK_CONTAINER(export_window),
		    GTK_WIDGET(vbox));

  /* live export */
  str = ags_config_get_value(ags_config_get_instance(),
			     AGS_CONFIG_GENERIC,
			     "disable-feature");
  
  if(str == NULL ||
     !g_ascii_strncasecmp(str,
			  "experimental",
			  13)){
    export_window->live_export = NULL;
  }else{
    export_window->live_export = (GtkCheckButton *) gtk_check_button_new_with_label(i18n("live export"));
    gtk_toggle_button_set_active((GtkToggleButton *) export_window->live_export,
				 TRUE);
    gtk_box_pack_start(GTK_BOX(vbox),
		       GTK_WIDGET(export_window->live_export),
		       FALSE, FALSE,
		       0);
  }

  g_free(str);
  
  /* exclude sequencer */
  export_window->exclude_sequencer = (GtkCheckButton *) gtk_check_button_new_with_label(i18n("exclude sequencers"));
  gtk_toggle_button_set_active((GtkToggleButton *) export_window->exclude_sequencer,
			       TRUE);
  gtk_box_pack_start(GTK_BOX(vbox),
		     GTK_WIDGET(export_window->exclude_sequencer),
		     FALSE, FALSE,
		     0);

  table = (GtkTable *) gtk_table_new(5, 2,
				     FALSE);
  gtk_box_pack_start(GTK_BOX(vbox),
		     GTK_WIDGET(table),
		     FALSE, FALSE,
		     0);

  /* mode */
  label = (GtkLabel *) gtk_label_new(i18n("mode"));
  g_object_set(G_OBJECT(label),
	       "xalign", 0.0,
	       NULL);
  gtk_table_attach(table,
		   GTK_WIDGET(label),
		   0, 1,
		   1, 2,
		   GTK_FILL|GTK_EXPAND, GTK_FILL|GTK_EXPAND,
		   0, 0);

  export_window->mode = (GtkComboBoxText *) gtk_combo_box_text_new();
  gtk_combo_box_text_append_text(export_window->mode,
				 "tact");
  //TODO:JK: uncomment me
  //  gtk_combo_box_text_append_text(export_window->mode,
  //				 "time");
  gtk_combo_box_set_active((GtkComboBox *) export_window->mode,
			   0);
  gtk_table_attach(table,
		   GTK_WIDGET(export_window->mode),
		   1, 2,
		   1, 2,
		   GTK_FILL, GTK_FILL,
		   0, 0);

  /* tact */
  label = (GtkLabel *) gtk_label_new(i18n("tact"));
  g_object_set(G_OBJECT(label),
	       "xalign", 0.0,
	       NULL);
  gtk_table_attach(table,
		   GTK_WIDGET(label),
		   0, 1,
		   2, 3,
		   GTK_FILL|GTK_EXPAND, GTK_FILL|GTK_EXPAND,
		   0, 0);

  export_window->tact = (GtkSpinButton *) gtk_spin_button_new_with_range(0.0, AGS_NOTATION_DEFAULT_LENGTH, 0.25);
  gtk_spin_button_set_digits(export_window->tact,
			     2);
  gtk_table_attach(table,
		   GTK_WIDGET(export_window->tact),
		   1, 2,
		   2, 3,
		   GTK_FILL, GTK_FILL,
		   0, 0);

  /* time */
  label = (GtkLabel *) gtk_label_new(i18n("time"));
  g_object_set(G_OBJECT(label),
	       "xalign", 0.0,
	       NULL);
  gtk_table_attach(table,
		   GTK_WIDGET(label),
		   0, 1,
		   3, 4,
		   GTK_FILL|GTK_EXPAND, GTK_FILL|GTK_EXPAND,
		   0, 0);

  /* duration */
  hbox = (GtkHBox *) gtk_hbox_new(FALSE,
				  0);
  gtk_table_attach(table,
		   GTK_WIDGET(hbox),
		   1, 2,
		   3, 4,
		   GTK_FILL|GTK_EXPAND, GTK_FILL|GTK_EXPAND,
		   0, 0);

  export_window->duration = (GtkLabel *) gtk_label_new(ags_navigation_tact_to_time_string(0.0,
											  AGS_SOUNDCARD_DEFAULT_BPM,
											  1.0));
  gtk_box_pack_start(GTK_BOX(hbox),
		     GTK_WIDGET(export_window->duration),
		     FALSE, FALSE,
		     0);

  /* export soundcard */
  export_window->export_soundcard = (GtkVBox *) gtk_vbox_new(FALSE,
							     0);
  gtk_box_pack_start(GTK_BOX(vbox),
		     GTK_WIDGET(export_window->export_soundcard),
		     FALSE, FALSE,
		     0);

  /* add */
  alignment = (GtkAlignment *) gtk_alignment_new(1.0, 0.5,
						 0.0, 0.0);
  gtk_box_pack_start(GTK_BOX(vbox),
		     GTK_WIDGET(alignment),
		     FALSE, FALSE,
		     0);
  
  export_window->add = (GtkButton *) gtk_button_new_from_stock(GTK_STOCK_ADD);
  gtk_container_add(GTK_CONTAINER(alignment),
		    GTK_WIDGET(export_window->add));
  
  /* export */
  hbox = (GtkHBox *) gtk_hbox_new(FALSE,
				  0);
  gtk_box_pack_start(GTK_BOX(vbox),
		     GTK_WIDGET(hbox),
		     FALSE, FALSE,
		     0);

  export_window->export = (GtkToggleButton *) gtk_toggle_button_new_with_label(i18n("export"));
  gtk_box_pack_start(GTK_BOX(hbox),
		     GTK_WIDGET(export_window->export),
		     FALSE, FALSE,
		     0);
}

void
ags_export_window_set_property(GObject *gobject,
			       guint prop_id,
			       const GValue *value,
			       GParamSpec *param_spec)
{
  AgsExportWindow *export_window;

  export_window = AGS_EXPORT_WINDOW(gobject);

  switch(prop_id){
  case PROP_APPLICATION_CONTEXT:
    {
      AgsApplicationContext *application_context;

      application_context = (AgsApplicationContext *) g_value_get_object(value);

      if((AgsApplicationContext *) export_window->application_context == application_context){
	return;
      }
      
      if(export_window->application_context != NULL){
	g_object_unref(export_window->application_context);
      }

      if(application_context != NULL){
	g_object_ref(application_context);
      }

      export_window->application_context = (GObject *) application_context;
    }
    break;
  case PROP_MAIN_WINDOW:
    {
      AgsWindow *main_window;

      main_window = (AgsWindow *) g_value_get_object(value);

      if((AgsWindow *) export_window->main_window == main_window){
	return;
      }

      if(export_window->main_window != NULL){
	g_object_unref(export_window->main_window);
      }

      if(main_window != NULL){
	g_object_ref(main_window);
      }

      export_window->main_window = (GObject *) main_window;
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_export_window_get_property(GObject *gobject,
			       guint prop_id,
			       GValue *value,
			       GParamSpec *param_spec)
{
  AgsExportWindow *export_window;

  export_window = AGS_EXPORT_WINDOW(gobject);

  switch(prop_id){
  case PROP_APPLICATION_CONTEXT:
    {
      g_value_set_object(value, export_window->application_context);
    }
    break;
  case PROP_MAIN_WINDOW:
    {
      g_value_set_object(value, export_window->main_window);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_export_window_connect(AgsConnectable *connectable)
{
  AgsExportWindow *export_window;

  GList *list, *list_start;

  export_window = AGS_EXPORT_WINDOW(connectable);

  if((AGS_EXPORT_WINDOW_CONNECTED & (export_window->flags)) != 0){
    return;
  }

  export_window->flags |= AGS_EXPORT_WINDOW_CONNECTED;

  g_signal_connect(G_OBJECT(export_window->add), "clicked",
		   G_CALLBACK(ags_export_window_add_export_soundcard_callback), export_window);

  g_signal_connect_after(G_OBJECT(export_window->tact), "value-changed",
			 G_CALLBACK(ags_export_window_tact_callback), export_window);

  g_signal_connect_after(G_OBJECT(export_window->export), "clicked",
			 G_CALLBACK(ags_export_window_export_callback), export_window);

  /* export soundcard */
  list_start =
    list = gtk_container_get_children(GTK_CONTAINER(export_window->export_soundcard));

  while(list != NULL){
    GList *child;

    child = gtk_container_get_children(GTK_CONTAINER(list->data));
    
    ags_connectable_connect(AGS_CONNECTABLE(child->data));

    g_list_free(child);

    /* iterate */
    list = list->next;
  }

  g_list_free(list_start);
}

void
ags_export_window_disconnect(AgsConnectable *connectable)
{
  AgsExportWindow *export_window;

  GList *list, *list_start;

  export_window = AGS_EXPORT_WINDOW(connectable);

  if((AGS_EXPORT_WINDOW_CONNECTED & (export_window->flags)) == 0){
    return;
  }

  export_window->flags &= (~AGS_EXPORT_WINDOW_CONNECTED);

  g_object_disconnect(G_OBJECT(export_window->add),
		      "any_signal::add",
		      G_CALLBACK(ags_export_window_add_export_soundcard_callback),
		      export_window,
		      NULL);

  g_object_disconnect(G_OBJECT(export_window->tact),
		      "any_signal::value-changed",
		      G_CALLBACK(ags_export_window_tact_callback),
		      export_window,
		      NULL);

  g_object_disconnect(G_OBJECT(export_window->export),
		      "any_signal::clicked",
		      G_CALLBACK(ags_export_window_export_callback),
		      export_window,
		      NULL);

  /* export soundcard */
  list_start =
    list = gtk_container_get_children(GTK_CONTAINER(export_window->export_soundcard));

  while(list != NULL){
    ags_connectable_disconnect(AGS_CONNECTABLE(list->data));
    
    list = list->next;
  }

  g_list_free(list_start);
}

void
ags_export_window_finalize(GObject *gobject)
{
  AgsExportWindow *export_window;

  export_window = (AgsExportWindow *) gobject;

  if(export_window->application_context != NULL){
    g_object_unref(export_window->application_context);
  }
  
  G_OBJECT_CLASS(ags_export_window_parent_class)->finalize(gobject);
}

void
ags_export_window_show(GtkWidget *widget)
{
  GTK_WIDGET_CLASS(ags_export_window_parent_class)->show(widget);
}

gboolean
ags_export_window_delete_event(GtkWidget *widget, GdkEventAny *event)
{
  gtk_widget_hide(widget);

  //  GTK_WIDGET_CLASS(ags_export_window_parent_class)->delete_event(widget, event);

  return(TRUE);
}

/**
 * ags_export_window_reload_soundcard_editor:
 * @export_window: the #AgsExportWindow
 * 
 * Reload soundcard editor.
 * 
 * Since: 2.0.0
 */
void
ags_export_window_reload_soundcard_editor(AgsExportWindow *export_window)
{
  AgsWindow *main_window;
  AgsExportSoundcard *export_soundcard;
  GtkHBox *hbox;
  GtkAlignment *alignment;
  GtkButton *remove_button;
  
  AgsApplicationContext *application_context;

  GList *start_list, *list;

  gchar *backend;
  gchar *str;
  
  guint i;
  
  /* retrieve main window and application context */
  main_window = export_window->main_window;

  application_context = export_window->application_context;

  if(application_context == NULL){
    return;
  }
  
  list =
    start_list = ags_sound_provider_get_soundcard(AGS_SOUND_PROVIDER(application_context));

  /* create export soundcard */
  for(i = 0; list != NULL; i++){
    guint soundcard_capability;

    soundcard_capability = ags_soundcard_get_capability(AGS_SOUNDCARD(list->data));
    
    if(soundcard_capability == AGS_SOUNDCARD_CAPABILITY_PLAYBACK){
      /* create GtkHBox */
      hbox = (GtkHBox *) gtk_hbox_new(FALSE,
				      0);
      gtk_box_pack_start((GtkBox *) export_window->export_soundcard,
			 (GtkWidget *) hbox,
			 FALSE, FALSE,
			 0);
    
      /* instantiate export soundcard */
      export_soundcard = (AgsExportSoundcard *) g_object_new(AGS_TYPE_EXPORT_SOUNDCARD,
							     "soundcard", list->data,
							     NULL);
      gtk_box_pack_start((GtkBox *) hbox,
			 (GtkWidget *) export_soundcard,
			 FALSE, FALSE,
			 0);
      ags_connectable_connect(AGS_CONNECTABLE(export_soundcard));
    
      /* remove button */
      alignment = (GtkAlignment *) gtk_alignment_new(0.5, 1.0,
						     0.0, 0.0);
      gtk_box_pack_start((GtkBox *) hbox,
			 (GtkWidget *) alignment,
			 FALSE, FALSE,
			 0);
    
      remove_button = (GtkButton *) gtk_button_new_from_stock(GTK_STOCK_REMOVE);
      gtk_container_add((GtkContainer *) alignment,
			(GtkWidget *) remove_button);
    
      g_signal_connect(G_OBJECT(remove_button), "clicked",
		       G_CALLBACK(ags_export_window_remove_export_soundcard_callback), export_window);
    
      /* set backend */
      backend = NULL;

      if(AGS_IS_DEVOUT(list->data)){
	if(ags_devout_test_flags(AGS_DEVOUT(list->data), AGS_DEVOUT_ALSA)){
	  backend = "alsa";
	}else if(ags_devout_test_flags(AGS_DEVOUT(list->data), AGS_DEVOUT_OSS)){
	  backend = "oss";
	}
      }else if(AGS_IS_JACK_DEVOUT(list->data)){
	backend = "jack";
      }else if(AGS_IS_PULSE_DEVOUT(list->data)){
	backend = "pulse";
      }else if(AGS_IS_CORE_AUDIO_DEVOUT(list->data)){
	backend = "core-audio";
      }

      ags_export_soundcard_set_backend(export_soundcard,
				       backend);
      ags_export_soundcard_refresh_card(export_soundcard);
    
      /* set card */
      str = ags_soundcard_get_device(AGS_SOUNDCARD(list->data));

      ags_export_soundcard_set_card(export_soundcard,
				    str);

      /* filename */
      str = g_strdup_printf("out-%d.wav",
			    i);
    
      ags_export_soundcard_set_filename(export_soundcard,
					str);
    
      g_free(str);

      /* set format */
      ags_export_soundcard_set_format(export_soundcard,
				      AGS_EXPORT_SOUNDCARD_FORMAT_WAV);

      /* show all */
      gtk_widget_show_all((GtkWidget *) hbox);
    }
    
    /* iterate */
    list = list->next;
  }

  g_list_free(start_list);
}

/**
 * ags_export_window_new:
 * 
 * Create a new instance of #AgsExportWindow
 * 
 * Returns: the #AgsExportWindow
 * 
 * Since: 2.0.0
 */
AgsExportWindow*
ags_export_window_new()
{
  AgsExportWindow *export_window;

  export_window = (AgsExportWindow *) g_object_new(AGS_TYPE_EXPORT_WINDOW,
						   NULL);

  return(export_window);
}
