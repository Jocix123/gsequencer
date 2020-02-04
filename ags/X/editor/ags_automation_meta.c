/* GSequencer - Advanced GTK Sequencer
 * Copyright (C) 2005-2020 Joël Krähemann
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

#include <ags/X/editor/ags_automation_meta.h>
#include <ags/X/editor/ags_automation_meta_callbacks.h>

#include <ags/X/ags_automation_editor.h>

#include <ags/i18n.h>

void ags_automation_meta_class_init(AgsAutomationMetaClass *automation_meta);
void ags_automation_meta_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_automation_meta_init(AgsAutomationMeta *automation_meta);

void ags_automation_meta_connect(AgsConnectable *connectable);
void ags_automation_meta_disconnect(AgsConnectable *connectable);

/**
 * SECTION:ags_automation_meta
 * @short_description: automation_meta
 * @title: AgsAutomationMeta
 * @section_id:
 * @include: ags/X/editor/ags_automation_meta.h
 *
 * The #AgsAutomationMeta provides you information about automation editor.
 */

GType
ags_automation_meta_get_type(void)
{
  static volatile gsize g_define_type_id__volatile = 0;

  if(g_once_init_enter (&g_define_type_id__volatile)){
    GType ags_type_automation_meta = 0;

    static const GTypeInfo ags_automation_meta_info = {
      sizeof (AgsAutomationMetaClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_automation_meta_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsAutomationMeta),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_automation_meta_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_automation_meta_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_automation_meta = g_type_register_static(GTK_TYPE_VBOX,
						      "AgsAutomationMeta", &ags_automation_meta_info,
						      0);
    
    g_type_add_interface_static(ags_type_automation_meta,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);

    g_once_init_leave(&g_define_type_id__volatile, ags_type_automation_meta);
  }

  return g_define_type_id__volatile;
}

void
ags_automation_meta_class_init(AgsAutomationMetaClass *automation_meta)
{
  /* empty */
}

void
ags_automation_meta_connectable_interface_init(AgsConnectableInterface *connectable)
{
  connectable->is_ready = NULL;
  connectable->is_connected = NULL;
  connectable->connect = ags_automation_meta_connect;
  connectable->disconnect = ags_automation_meta_disconnect;
}

void
ags_automation_meta_init(AgsAutomationMeta *automation_meta)
{
  GtkGrid *grid;
  GtkLabel *label;

  guint i;
  guint j;
  
  automation_meta->flags = 0;

  grid = gtk_grid_new();
  gtk_box_pack_start((GtkBox *) automation_meta,
		     (GtkWidget *) grid,
		     FALSE,
		     TRUE,
		     0);
  
  /* machine type */
  i = 0;
  
  label = (GtkLabel *) gtk_label_new(i18n("machine type: "));
  g_object_set(label,
	       "halign", GTK_ALIGN_START,
	       NULL);
  gtk_grid_attach(grid,
		  (GtkWidget *) label,
		  0, i,
		  1, 1);

  automation_meta->machine_type = (GtkLabel *) gtk_label_new("(null)");
  g_object_set(automation_meta->machine_type,
	       "halign", GTK_ALIGN_START,
	       NULL);
  gtk_grid_attach(grid,
		  (GtkWidget *) automation_meta->machine_type,
		  1, i,
		  1, 1);
  
  /* machine name */
  i++;
  
  label = (GtkLabel *) gtk_label_new(i18n("machine name: "));
  g_object_set(label,
	       "halign", GTK_ALIGN_START,
	       NULL);
  gtk_grid_attach(grid,
		  (GtkWidget *) label,
		  0, i,
		  1, 1);

  automation_meta->machine_name = (GtkLabel *) gtk_label_new("(null)");
  g_object_set(automation_meta->machine_name,
	       "halign", GTK_ALIGN_START,
	       NULL);
  gtk_grid_attach(grid,
		  (GtkWidget *) automation_meta->machine_name,
		  1, i,
		  1, 1);

  /* audio channels */
  i++;
  
  label = (GtkLabel *) gtk_label_new(i18n("audio channels: "));
  g_object_set(label,
	       "halign", GTK_ALIGN_START,
	       NULL);
  gtk_grid_attach(grid,
		  (GtkWidget *) label,
		  0, i,
		  1, 1);

  automation_meta->audio_channels = (GtkLabel *) gtk_label_new("-1");
  g_object_set(automation_meta->audio_channels,
	       "halign", GTK_ALIGN_START,
	       NULL);
  gtk_grid_attach(grid,
		  (GtkWidget *) automation_meta->audio_channels,
		  1, i,
		  1, 1);

  /* output pads */
  i++;
  
  label = (GtkLabel *) gtk_label_new(i18n("output pads: "));
  g_object_set(label,
	       "halign", GTK_ALIGN_START,
	       NULL);
  gtk_grid_attach(grid,
		  (GtkWidget *) label,
		  0, i,
		  1, 1);

  automation_meta->output_pads = (GtkLabel *) gtk_label_new("-1");
  g_object_set(automation_meta->output_pads,
	       "halign", GTK_ALIGN_START,
	       NULL);
  gtk_grid_attach(grid,
		  (GtkWidget *) automation_meta->output_pads,
		  1, i,
		  1, 1);

  /* input pads */
  i++;
  
  label = (GtkLabel *) gtk_label_new(i18n("input pads: "));
  g_object_set(label,
	       "halign", GTK_ALIGN_START,
	       NULL);
  gtk_grid_attach(grid,
		  (GtkWidget *) label,
		  0, i,
		  1, 1);

  automation_meta->input_pads = (GtkLabel *) gtk_label_new("-1");
  g_object_set(automation_meta->input_pads,
	       "halign", GTK_ALIGN_START,
	       NULL);
  gtk_grid_attach(grid,
		  (GtkWidget *) automation_meta->input_pads,
		  1, i,
		  1, 1);

  /* editor tool */
  i++;
  
  label = (GtkLabel *) gtk_label_new(i18n("editor tool: "));
  g_object_set(label,
	       "halign", GTK_ALIGN_START,
	       NULL);
  gtk_grid_attach(grid,
		  (GtkWidget *) label,
		  0, i,
		  1, 1);

  automation_meta->editor_tool = (GtkLabel *) gtk_label_new("(null)");
  g_object_set(automation_meta->editor_tool,
	       "halign", GTK_ALIGN_START,
	       NULL);
  gtk_grid_attach(grid,
		  (GtkWidget *) automation_meta->editor_tool,
		  1, i,
		  1, 1);

  /* active audio channel */
  i++;
  
  label = (GtkLabel *) gtk_label_new(i18n("active audio channel: "));
  g_object_set(label,
	       "halign", GTK_ALIGN_START,
	       "valign", GTK_ALIGN_START,
	       NULL);
  gtk_grid_attach(grid,
		  (GtkWidget *) label,
		  0, i,
		  1, 1);

  automation_meta->active_audio_channel = (GtkLabel *) gtk_label_new("(null)");
  g_object_set(automation_meta->active_audio_channel,
	       "halign", GTK_ALIGN_START,
	       NULL);
  gtk_grid_attach(grid,
		  (GtkWidget *) automation_meta->active_audio_channel,
		  1, i,
		  1, 1);

  /* active port */
  i++;
  
  label = (GtkLabel *) gtk_label_new(i18n("active port: "));
  g_object_set(label,
	       "halign", GTK_ALIGN_START,
	       "valign", GTK_ALIGN_START,
	       NULL);
  gtk_grid_attach(grid,
		  (GtkWidget *) label,
		  0, i,
		  1, 1);

  automation_meta->active_port = (GtkLabel *) gtk_label_new("(null)");
  g_object_set(automation_meta->active_port,
	       "halign", GTK_ALIGN_START,
	       NULL);
  gtk_grid_attach(grid,
		  (GtkWidget *) automation_meta->active_port,
		  1, i,
		  1, 1);

  /* focused port */
  i++;
  
  label = (GtkLabel *) gtk_label_new(i18n("focused port: "));
  g_object_set(label,
	       "halign", GTK_ALIGN_START,
	       NULL);
  gtk_grid_attach(grid,
		  (GtkWidget *) label,
		  0, i,
		  1, 1);

  automation_meta->focused_port = (GtkLabel *) gtk_label_new("(null)");
  g_object_set(automation_meta->focused_port,
	       "halign", GTK_ALIGN_START,
	       NULL);
  gtk_grid_attach(grid,
		  (GtkWidget *) automation_meta->focused_port,
		  1, i,
		  1, 1);

  /* range upper */
  i++;
  
  label = (GtkLabel *) gtk_label_new(i18n("range upper: "));
  g_object_set(label,
	       "halign", GTK_ALIGN_START,
	       NULL);
  gtk_grid_attach(grid,
		  (GtkWidget *) label,
		  0, i,
		  1, 1);

  automation_meta->range_upper = (GtkLabel *) gtk_label_new("(null)");
  g_object_set(automation_meta->range_upper,
	       "halign", GTK_ALIGN_START,
	       NULL);
  gtk_grid_attach(grid,
		  (GtkWidget *) automation_meta->range_upper,
		  1, i,
		  1, 1);

  /* range lower */
  i++;
  
  label = (GtkLabel *) gtk_label_new(i18n("range lower: "));
  g_object_set(label,
	       "halign", GTK_ALIGN_START,
	       NULL);
  gtk_grid_attach(grid,
		  (GtkWidget *) label,
		  0, i,
		  1, 1);

  automation_meta->range_lower = (GtkLabel *) gtk_label_new("(null)");
  g_object_set(automation_meta->range_lower,
	       "halign", GTK_ALIGN_START,
	       NULL);
  gtk_grid_attach(grid,
		  (GtkWidget *) automation_meta->range_lower,
		  1, i,
		  1, 1);

  /* cursor x-position */
  i++;
  
  label = (GtkLabel *) gtk_label_new(i18n("cursor x-position: "));
  g_object_set(label,
	       "halign", GTK_ALIGN_START,
	       NULL);
  gtk_grid_attach(grid,
		  (GtkWidget *) label,
		  0, i,
		  1, 1);

  automation_meta->cursor_x_position = (GtkLabel *) gtk_label_new("(null)");
  g_object_set(automation_meta->cursor_x_position,
	       "halign", GTK_ALIGN_START,
	       NULL);
  gtk_grid_attach(grid,
		  (GtkWidget *) automation_meta->cursor_x_position,
		  1, i,
		  1, 1);

  /* cursor y-position */
  i++;
  
  label = (GtkLabel *) gtk_label_new(i18n("cursor y-position: "));
  g_object_set(label,
	       "halign", GTK_ALIGN_START,
	       NULL);
  gtk_grid_attach(grid,
		  (GtkWidget *) label,
		  0, i,
		  1, 1);

  automation_meta->cursor_y_position = (GtkLabel *) gtk_label_new("(null)");
  g_object_set(automation_meta->cursor_y_position,
	       "halign", GTK_ALIGN_START,
	       NULL);
  gtk_grid_attach(grid,
		  (GtkWidget *) automation_meta->cursor_y_position,
		  1, i,
		  1, 1);

  /* current acceleration */
  i++;
  
  label = (GtkLabel *) gtk_label_new(i18n("current acceleration: "));
  g_object_set(label,
	       "halign", GTK_ALIGN_START,
	       "valign", GTK_ALIGN_START,
	       NULL);
  gtk_grid_attach(grid,
		  (GtkWidget *) label,
		  0, i,
		  1, 1);

  automation_meta->current_acceleration = (GtkLabel *) gtk_label_new("(null)");
  g_object_set(automation_meta->current_acceleration,
	       "halign", GTK_ALIGN_START,
	       NULL);
  gtk_grid_attach(grid,
		  (GtkWidget *) automation_meta->current_acceleration,
		  1, i,
		  1, 1);
}

void
ags_automation_meta_connect(AgsConnectable *connectable)
{
  AgsAutomationEditor *automation_editor;
  AgsAutomationMeta *automation_meta;
  
  automation_meta = AGS_AUTOMATION_META(connectable);

  if((AGS_AUTOMATION_META_CONNECTED & (automation_meta->flags)) != 0){
    return;
  }

  automation_meta->flags |= AGS_AUTOMATION_META_CONNECTED;

  automation_editor = gtk_widget_get_ancestor(automation_meta,
					      AGS_TYPE_AUTOMATION_EDITOR);

  if(automation_editor != NULL){
    g_signal_connect_after(automation_editor, "machine-changed",
			   G_CALLBACK(ags_automation_meta_machine_changed_callback), automation_meta);
  }
}

void
ags_automation_meta_disconnect(AgsConnectable *connectable)
{
  AgsAutomationEditor *automation_editor;
  AgsAutomationMeta *automation_meta;
  
  automation_meta = AGS_AUTOMATION_META(connectable);

  if((AGS_AUTOMATION_META_CONNECTED & (automation_meta->flags)) == 0){
    return;
  }

  automation_meta->flags &= (~AGS_AUTOMATION_META_CONNECTED);

  automation_editor = gtk_widget_get_ancestor(automation_meta,
					      AGS_TYPE_AUTOMATION_EDITOR);
  if(automation_editor != NULL){
    g_object_disconnect(automation_editor,
			"any_signal::machine-changed",
			G_CALLBACK(ags_automation_meta_machine_changed_callback),
			automation_meta,
			NULL);
  }
}

/**
 * ags_automation_meta_refresh:
 * @automation_meta: the #AgsAutomationMeta
 * 
 * Refresh @automation_meta.
 * 
 * Since: 3.1.0
 */
void
ags_automation_meta_refresh(AgsAutomationMeta *automation_meta)
{
  AgsAutomationEditor *automation_editor;
  
  if(!AGS_IS_AUTOMATION_META(automation_meta)){
    return;
  }
  
  automation_editor = gtk_widget_get_ancestor(automation_meta,
					    AGS_TYPE_AUTOMATION_EDITOR);

  if(automation_editor == NULL){
    return;
  }

  if(automation_editor->selected_machine == NULL){
    gtk_label_set_label(automation_meta->machine_type,
			"(null)"); 

    gtk_label_set_label(automation_meta->machine_name,
			"(null)"); 

    gtk_label_set_label(automation_meta->audio_channels,
			"-1");

    gtk_label_set_label(automation_meta->output_pads,
			"-1"); 

    gtk_label_set_label(automation_meta->input_pads,
			"-1"); 

    gtk_label_set_label(automation_meta->editor_tool,
			"(null)");
    
    gtk_label_set_label(automation_meta->active_audio_channel,
			"(null)"); 

    gtk_label_set_label(automation_meta->active_port,
			"(null)");
    
    gtk_label_set_label(automation_meta->focused_port,
			"(null)");

    gtk_label_set_label(automation_meta->range_upper,
			"0.0");

    gtk_label_set_label(automation_meta->range_lower,
			"0.0");

    gtk_label_set_label(automation_meta->cursor_x_position,
			"-1"); 

    gtk_label_set_label(automation_meta->cursor_y_position,
			"-1"); 

    gtk_label_set_label(automation_meta->current_acceleration,
			"(null)"); 
  }else{
    AgsNotebook *notebook;
      
    AgsTimestamp *timestamp;
    
    GList *start_port, *port;
    GList *start_automation, *automation;
    GList *start_acceleration, *acceleration;

    gchar **collected_specifier;
    
    gchar *str;
    
    guint audio_channels;
    guint output_pads, input_pads;
    gint active_start, active_end;
    gint position;
    guint length;
    guint x0, y0;
    guint x1, y1;
    guint i;
    
    gtk_label_set_label(automation_meta->machine_type,
			G_OBJECT_TYPE_NAME(automation_editor->selected_machine)); 

    gtk_label_set_label(automation_meta->machine_name,
			automation_editor->selected_machine->machine_name); 

    g_object_get(automation_editor->selected_machine->audio,
		 "audio-channels", &audio_channels,
		 "output-pads", &output_pads,
		 "input-pads", &input_pads,
		 NULL);
    
    /* audio channels */
    str = g_strdup_printf("%u", audio_channels);
    gtk_label_set_label(automation_meta->audio_channels,
			str); 

    g_free(str);
    
    /* output pads */
    str = g_strdup_printf("%u", output_pads);
    gtk_label_set_label(automation_meta->output_pads,
			str);

    g_free(str);

    /* input pads */
    str = g_strdup_printf("%u", input_pads);
    gtk_label_set_label(automation_meta->input_pads,
			str);

    g_free(str);

    str = NULL;

    if(automation_editor->automation_toolbar->selected_edit_mode == automation_editor->automation_toolbar->position){
      str = i18n("position");
    }else if(automation_editor->automation_toolbar->selected_edit_mode == automation_editor->automation_toolbar->edit){
      str = i18n("edit");
    }else if(automation_editor->automation_toolbar->selected_edit_mode == automation_editor->automation_toolbar->clear){
      str = i18n("clear");
    }else if(automation_editor->automation_toolbar->selected_edit_mode == automation_editor->automation_toolbar->select){
      str = i18n("select");
    }

    if(str != NULL){
      gtk_label_set_label(automation_meta->editor_tool,
			  str);
    }else{
      gtk_label_set_label(automation_meta->editor_tool,
			  "(null)");
    }

    /* active audio channels */
    notebook = NULL;

    if(gtk_notebook_get_current_page(automation_editor->notebook) == 1){
      notebook = automation_editor->output_notebook;
    }else if(gtk_notebook_get_current_page(automation_editor->notebook) == 2){
      notebook = automation_editor->input_notebook;
    }

    str = NULL;

    if(notebook != NULL){
      for(i = 0; i < audio_channels; i++){
	gchar *current_str;

	guint active_prev;
	gboolean found;
	
	active_start = -1;
	active_end = -1;

	active_prev = -1;
	
	position = i;

	current_str = NULL;

	found = FALSE;
	
	for(; (position = ags_notebook_next_active_tab(notebook, position)) != -1;){
	  if(position % audio_channels != i){
	    position++;
	  }else{	    
	    found = TRUE;
	    
	    if(active_start == -1){
	      active_start = position;
	      active_end = position;
	    }else{
	      if(active_prev != -1 &&
		 active_prev + audio_channels != position){
		if(current_str == NULL){
		  if(active_start == active_end){
		    current_str = g_strdup_printf("%d",
						  active_start);
		  }else{
		    current_str = g_strdup_printf("%d-%d",
						  active_start,
						  active_end);
		  }		  
		}else{
		  if(active_start == active_end){
		    gchar *tmp;
	    
		    tmp = g_strdup_printf("%s, %d",
					  current_str,
					  active_start);

		    g_free(current_str);

		    current_str = tmp;
		  }else{
		    gchar *tmp;
	    
		    tmp = g_strdup_printf("%s, %d-%d",
					  current_str,
					  active_start,
					  active_end);

		    g_free(current_str);

		    current_str = tmp;
		  }
		}

		active_start = position;
		active_end = position;
	      }
	    }
	  
	    active_prev = position;	    
	    active_end = position;
	  
	    position += audio_channels;
	  }
	}

	if(active_start == -1){
	  if(str == NULL){
	    str = g_strdup_printf("@audio_channel[%d] -> (null)",
				  i);
	  }else{
	    gchar *tmp;
	  
	    tmp = g_strdup_printf("%s,\n  @audio_channel[%d] -> (null)",
				  str,
				  i);

	    g_free(str);

	    str = tmp;
	  }
	}else{
	  if(active_start == active_end){
	    if(current_str == NULL){
	      if(str == NULL){
		str = g_strdup_printf("@audio_channel[%d] -> [%d]",
				      i,
				      active_start);
	      }else{
		gchar *tmp;
	  
		tmp = g_strdup_printf("%s,\n  @audio_channel[%d] -> [%d]",
				      str,
				      i,
				      active_start);

		g_free(str);

		str = tmp;
	      }
	    }else{
	      if(str == NULL){
		str = g_strdup_printf("@audio_channel[%d] -> [%s, %d]",
				      i,
				      current_str,
				      active_start);
	      }else{
		gchar *tmp;
	  
		tmp = g_strdup_printf("%s,\n  @audio_channel[%d] -> [%s, %d]",
				      str,
				      i,
				      current_str,
				      active_start);

		g_free(str);

		str = tmp;
	      }

	      g_free(current_str);
	    }
	  }else{
	    if(current_str == NULL){
	      if(str == NULL){
		str = g_strdup_printf("@audio_channel[%d] -> [%d-%d]",
				      i,
				      active_start,
				      active_end);
	      }else{
		gchar *tmp;
	  
		tmp = g_strdup_printf("%s,\n  @audio_channel[%d] -> [%d-%d]",
				      str,
				      i,
				      active_start,
				      active_end);

		g_free(str);

		str = tmp;
	      }
	    }else{
	      if(str == NULL){
		str = g_strdup_printf("@audio_channel[%d] -> [%s, %d-%d]",
				      i,
				      current_str,
				      active_start,
				      active_end);
	      }else{
		gchar *tmp;
	  
		tmp = g_strdup_printf("%s,\n  @audio_channel[%d] -> [%s, %d-%d]",
				      str,
				      i,
				      current_str,
				      active_start,
				      active_end);

		g_free(str);

		str = tmp;
	      }

	      g_free(current_str);
	    }
	  }
	}
      }
    }

    if(str == NULL){
      gtk_label_set_label(automation_meta->active_audio_channel,
			  "(null)");
    }else{
      gchar *tmp;
      
      tmp = g_strdup_printf("[%s]", str);
      gtk_label_set_label(automation_meta->active_audio_channel,
			  tmp);

      g_free(str);
      g_free(tmp);
    }

    /* active port */
    start_port = NULL;
  
    if(notebook == NULL){
      start_port = ags_audio_collect_all_audio_ports(automation_editor->selected_machine->audio);
    }else{
      AgsChannel *start_channel, *channel;

      if(notebook == automation_editor->output_notebook){      
	g_object_get(automation_editor->selected_machine->audio,
		     "output", &start_channel,
		     NULL);
      }else if(notebook == automation_editor->input_notebook){
	g_object_get(automation_editor->selected_machine->audio,
		     "input", &start_channel,
		     NULL);
      }

      channel = start_channel;

      start_port = NULL;
    
      while(channel != NULL){
	AgsChannel *next_channel;

	GList *list;
      
	list = ags_channel_collect_all_channel_ports(channel);

	if(start_port == NULL){
	  start_port = list;
	}else{
	  start_port = g_list_concat(start_port,
				     list);
	}
      
	/* iterate */
	next_channel = ags_channel_next(channel);

	g_object_unref(channel);

	channel = next_channel;
      }
    }

    port = start_port;

    collected_specifier = (gchar **) malloc(sizeof(gchar*));

    collected_specifier[0] = NULL;
    length = 1;

    str = NULL;
  
    while(port != NULL){
      AgsPluginPort *plugin_port;

      gchar *specifier;

      gboolean is_enabled;
      gboolean contains_control_name;

      g_object_get(port->data,
		   "specifier", &specifier,
		   "plugin-port", &plugin_port,
		   NULL);

      contains_control_name = g_strv_contains(collected_specifier,
					      specifier);
      
      if(plugin_port != NULL &&
	 !contains_control_name){
	is_enabled = FALSE;
      
	if(notebook == NULL){
	  is_enabled = (ags_machine_automation_port_find_channel_type_with_control_name(automation_editor->selected_machine->enabled_automation_port,
											G_TYPE_NONE,
											specifier)) ? TRUE: FALSE;
	}else if(notebook == automation_editor->output_notebook){
	  is_enabled = (ags_machine_automation_port_find_channel_type_with_control_name(automation_editor->selected_machine->enabled_automation_port,
											AGS_TYPE_OUTPUT,
											specifier)) ? TRUE: FALSE;
	}else if(notebook == automation_editor->input_notebook){
	  is_enabled = (ags_machine_automation_port_find_channel_type_with_control_name(automation_editor->selected_machine->enabled_automation_port,
											AGS_TYPE_INPUT,
											specifier)) ? TRUE: FALSE;
	}

	/* add to collected specifier */
	collected_specifier = (gchar **) realloc(collected_specifier,
						 (length + 1) * sizeof(gchar *));
	collected_specifier[length - 1] = g_strdup(specifier);
	collected_specifier[length] = NULL;

	length++;	  

	if(is_enabled){
	  if(str == NULL){
	    str = specifier;
	  }else{
	    gchar *tmp;

	    tmp = g_strdup_printf("%s, %s",
				  str,
				  specifier);

	    g_free(specifier);
	    g_free(str);

	    str = tmp;
	  }
	}
      }
      
      if(plugin_port != NULL){
	g_object_unref(plugin_port);
      }
      
      port = port->next;
    }

    if(str == NULL){
      gtk_label_set_label(automation_meta->active_port,
			  "(null)");
    }else{
      gchar *tmp;

      tmp = g_strdup_printf("(%s)",
			    str);
      
      gtk_label_set_label(automation_meta->active_port,
			  tmp);

      g_free(str);
      g_free(tmp);
    }

    /* focus related */
    if(automation_editor->focused_automation_edit == NULL){
      gtk_label_set_label(automation_meta->focused_port,
			  "(null)");

      gtk_label_set_label(automation_meta->range_upper,
			  "0.0");

      gtk_label_set_label(automation_meta->range_lower,
			  "0.0");

      gtk_label_set_label(automation_meta->cursor_x_position,
			  "-1"); 

      gtk_label_set_label(automation_meta->cursor_y_position,
			  "-1"); 

      gtk_label_set_label(automation_meta->current_acceleration,
			  "(null)"); 
    }else{
      /* focused port */
      gtk_label_set_label(automation_meta->focused_port,
			  automation_editor->focused_automation_edit->control_specifier);

      /* upper */
      str = g_strdup_printf("%f", automation_editor->focused_automation_edit->upper);
      
      gtk_label_set_label(automation_meta->range_upper,
			  str);

      g_free(str);

      /* lower */
      str = g_strdup_printf("%f", automation_editor->focused_automation_edit->lower);
      
      gtk_label_set_label(automation_meta->range_lower,
			  str);

      g_free(str);

      /* cursor x-position */
      str = g_strdup_printf("%f", (double) automation_editor->focused_automation_edit->cursor_position_x / (double) AGS_AUTOMATION_EDIT_DEFAULT_CONTROL_WIDTH);
      
      gtk_label_set_label(automation_meta->cursor_x_position,
			  str);

      g_free(str);

      /* cursor y-position */
      str = g_strdup_printf("%f", automation_editor->focused_automation_edit->cursor_position_y);

      gtk_label_set_label(automation_meta->cursor_y_position,
			  str); 

      g_free(str);
    }
  }
}

/**
 * ags_automation_meta_new:
 *
 * Create a new #AgsAutomationMeta.
 *
 * Returns: a new #AgsAutomationMeta
 *
 * Since: 3.1.0
 */
AgsAutomationMeta*
ags_automation_meta_new()
{
  AgsAutomationMeta *automation_meta;

  automation_meta = (AgsAutomationMeta *) g_object_new(AGS_TYPE_AUTOMATION_META,
						       NULL);

  return(automation_meta);
}
