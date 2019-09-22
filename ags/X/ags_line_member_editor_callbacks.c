/* GSequencer - Advanced GTK Sequencer
 * Copyright (C) 2005-2019 Joël Krähemann
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

#include <ags/X/ags_line_member_editor_callbacks.h>

#include <ags/libags.h>
#include <ags/libags-audio.h>
#include <ags/libags-gui.h>

#include <ags/X/ags_ui_provider.h>
#include <ags/X/ags_window.h>
#include <ags/X/ags_machine.h>
#include <ags/X/ags_pad.h>
#include <ags/X/ags_line.h>
#include <ags/X/ags_effect_bridge.h>
#include <ags/X/ags_effect_pad.h>
#include <ags/X/ags_effect_line.h>
#include <ags/X/ags_line_member.h>
#include <ags/X/ags_machine_editor.h>
#include <ags/X/ags_line_editor.h>
#include <ags/X/ags_lv2_browser.h>
#include <ags/X/ags_ladspa_browser.h>

#include <ags/X/thread/ags_gui_thread.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include <ladspa.h>

void
ags_line_member_editor_add_callback(GtkWidget *button,
				    AgsLineMemberEditor *line_member_editor)
{
  gtk_widget_show_all((GtkWidget *) line_member_editor->plugin_browser);
}

void
ags_line_member_editor_plugin_browser_response_callback(GtkDialog *dialog,
							gint response,
							AgsLineMemberEditor *line_member_editor)
{
  AgsWindow *window;
  AgsMachine *machine;
  AgsMachineEditor *machine_editor;
  AgsLineEditor *line_editor;
  
  AgsApplicationContext *application_context;
  
  GList *pad, *pad_start;
  GList *list, *list_start;

  gchar *filename, *effect;

  gboolean has_bridge;
  gboolean is_output;
  
  auto void ags_line_member_editor_plugin_browser_response_create_entry();
  
  void ags_line_member_editor_plugin_browser_response_create_entry(){
    GtkHBox *hbox;
    GtkCheckButton *check_button;
    GtkLabel *label;

    /* create entry */
    hbox = (GtkHBox *) gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(line_member_editor->line_member),
		       GTK_WIDGET(hbox),
		       FALSE, FALSE,
		       0);
      
    check_button = (GtkCheckButton *) gtk_check_button_new();
    gtk_box_pack_start(GTK_BOX(hbox),
		       GTK_WIDGET(check_button),
		       FALSE, FALSE,
		       0);

    //TODO:JK: ugly
    label = (GtkLabel *) gtk_label_new(g_strdup_printf("%s - %s",
						       filename,
						       effect));
    gtk_box_pack_start(GTK_BOX(hbox),
		       GTK_WIDGET(label),
		       FALSE, FALSE,
		       0);
    gtk_widget_show_all((GtkWidget *) hbox);
  }
  
  switch(response){
  case GTK_RESPONSE_ACCEPT:
    {
      machine_editor = (AgsMachineEditor *) gtk_widget_get_ancestor((GtkWidget *) line_member_editor,
								    AGS_TYPE_MACHINE_EDITOR);
      line_editor = (AgsLineEditor *) gtk_widget_get_ancestor((GtkWidget *) line_member_editor,
							      AGS_TYPE_LINE_EDITOR);

      machine = machine_editor->machine;

      window = (AgsWindow *) gtk_widget_get_toplevel((GtkWidget *) machine);

      application_context = (AgsApplicationContext *) window->application_context;

      if(AGS_IS_OUTPUT(line_editor->channel)){
	is_output = TRUE;
      }else{
	is_output = FALSE;
      }

      if(machine->bridge != NULL){
	has_bridge = TRUE;
      }else{
	has_bridge = FALSE;
      }
      
      if(!has_bridge){	
	AgsLine *line;
	
	/* find pad and line */
	line = NULL;
	
	if(is_output){
	  pad_start = 
	    pad = gtk_container_get_children((GtkContainer *) machine_editor->machine->output);
	}else{
	  pad_start = 
	    pad = gtk_container_get_children((GtkContainer *) machine_editor->machine->input);
	}

	pad = g_list_nth(pad,
			 line_editor->channel->pad);

	if(pad != NULL){
	  list_start =
	    list = gtk_container_get_children((GtkContainer *) AGS_PAD(pad->data)->expander_set);

	  while(list != NULL){
	    if(AGS_LINE(list->data)->channel == line_editor->channel){
	      break;
	    }

	    list = list->next;
	  }

	  if(list != NULL){
	    line = AGS_LINE(list->data);
	    g_list_free(list_start);
	  }
	}

	g_list_free(pad_start);

	/* retrieve plugin */
	filename = ags_plugin_browser_get_plugin_filename(line_member_editor->plugin_browser);
	effect = ags_plugin_browser_get_plugin_effect(line_member_editor->plugin_browser);

	if(line != NULL){
	  AgsAddEffect *add_effect;

	  GList *start_play, *start_recall;

	  g_object_get(line->channel,
		       "play", &start_play,
		       "recall", &start_recall,
		       NULL);
	  
	  if(ags_recall_find_recall_id_with_effect(start_play,
						   NULL,
						   filename,
						   effect) == NULL &&
	     ags_recall_find_recall_id_with_effect(start_recall,
						   NULL,
						   filename,
						   effect) == NULL){
	    ags_line_member_editor_plugin_browser_response_create_entry();
	    
	    /* add effect */
	    add_effect = ags_add_effect_new(line->channel,
					    filename,
					    effect);
	    ags_xorg_application_context_schedule_task(application_context,
						       (GObject *) add_effect);
	  }

	  g_list_free_full(start_play,
			   g_object_unref);
	  g_list_free_full(start_recall,
			   g_object_unref);
	}
      }else{
	AgsEffectBridge *effect_bridge;
	AgsEffectLine *effect_line;
	
	effect_bridge = (AgsEffectBridge *) machine->bridge;
	effect_line = NULL;
	
	/* find effect pad and effect line */
	if(is_output){
	  pad_start = 
	    pad = gtk_container_get_children((GtkContainer *) effect_bridge->output);
	}else{
	  pad_start = 
	    pad = gtk_container_get_children((GtkContainer *) effect_bridge->input);
	}

	pad = g_list_nth(pad,
			 line_editor->channel->pad);

	if(pad != NULL){
	  list_start =
	    list = gtk_container_get_children((GtkContainer *) AGS_EFFECT_PAD(pad->data)->table);

	  while(list != NULL){
	    if(AGS_EFFECT_LINE(list->data)->channel == line_editor->channel){
	      break;
	    }

	    list = list->next;
	  }

	  if(list != NULL){
	    effect_line = AGS_EFFECT_LINE(list->data);
	    g_list_free(list_start);
	  }
	}

	g_list_free(pad_start);

	/* retrieve plugin */
	filename = ags_plugin_browser_get_plugin_filename(line_member_editor->plugin_browser);
	effect = ags_plugin_browser_get_plugin_effect(line_member_editor->plugin_browser);

	if(effect_line != NULL){
	  AgsAddEffect *add_effect;
	  
	  GList *start_play, *start_recall;

	  g_object_get(effect_line->channel,
		       "play", &start_play,
		       "recall", &start_recall,
		       NULL);

	  if(ags_recall_find_recall_id_with_effect(start_play,
						   NULL,
						   filename,
						   effect) == NULL &&
	     ags_recall_find_recall_id_with_effect(start_recall,
						   NULL,
						   filename,
						   effect) == NULL){
	    ags_line_member_editor_plugin_browser_response_create_entry();

	    /* add effect */
	    add_effect = ags_add_effect_new(effect_line->channel,
					    filename,
					    effect);
	    ags_xorg_application_context_schedule_task(application_context,
						       (GObject *) add_effect);
	  }

	  g_list_free_full(start_play,
			   g_object_unref);
	  g_list_free_full(start_recall,
			   g_object_unref);
	}
      }
    }
    break;      
  }
}

void
ags_line_member_editor_remove_callback(GtkWidget *button,
				       AgsLineMemberEditor *line_member_editor)
{
  AgsMachine *machine;
  AgsMachineEditor *machine_editor;
  AgsLineEditor *line_editor;

  GList *line_member, *line_member_next;
  GList *list, *list_start, *pad, *pad_start;
  GList *children;

  guint nth;
  gboolean has_bridge;
  gboolean is_output;
  
  auto void ags_line_member_editor_plugin_browser_response_destroy_entry();
  
  void ags_line_member_editor_plugin_browser_response_destroy_entry(){
    /* destroy line member editor entry */
    gtk_widget_destroy(GTK_WIDGET(line_member->data));
  }
  
  if(button == NULL ||
     line_member_editor == NULL){
    return;
  }

  machine_editor = (AgsMachineEditor *) gtk_widget_get_ancestor((GtkWidget *) line_member_editor,
								AGS_TYPE_MACHINE_EDITOR);
  line_editor = (AgsLineEditor *) gtk_widget_get_ancestor((GtkWidget *) line_member_editor,
							  AGS_TYPE_LINE_EDITOR);

  line_member = gtk_container_get_children(GTK_CONTAINER(line_member_editor->line_member));

  machine = machine_editor->machine;

  if(AGS_IS_OUTPUT(line_editor->channel)){
    is_output = TRUE;
  }else{
    is_output = FALSE;
  }

  if(machine->bridge != NULL){
    has_bridge = TRUE;
  }else{
    has_bridge = FALSE;
  }

  if(!has_bridge){	
    AgsLine *line;
    
    /* retrieve line and pad */
    line = NULL;

    if(AGS_IS_OUTPUT(line_editor->channel)){
      pad_start = 
	pad = gtk_container_get_children((GtkContainer *) machine->output);
    }else{
      pad_start = 
	pad = gtk_container_get_children((GtkContainer *) machine->input);
    }

    pad = g_list_nth(pad,
		     line_editor->channel->pad);

    if(pad != NULL){
      list_start =
	list = gtk_container_get_children((GtkContainer *) AGS_PAD(pad->data)->expander_set);

      while(list != NULL){
	if(AGS_LINE(list->data)->channel == line_editor->channel){
	  break;
	}

	list = list->next;
      }

      if(list != NULL){
	line = AGS_LINE(list->data);
	g_list_free(list_start);
      }
    }

    g_list_free(pad_start);

    /* iterate line member */
    if(line != NULL){
      for(nth = 0; line_member != NULL; nth++){
	line_member_next = line_member->next;
	
	children = gtk_container_get_children((GtkContainer *) GTK_CONTAINER(line_member->data));

	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(children->data))){
	  ags_line_member_editor_plugin_browser_response_destroy_entry();

	  /* remove effect */
	  ags_line_remove_effect(line,
				 nth);
	}
      
	line_member = line_member_next;
      }
    }
  }else{
    AgsEffectBridge *effect_bridge;
    AgsEffectLine *effect_line;
	
    effect_bridge = AGS_EFFECT_BRIDGE(machine->bridge);

    effect_line = NULL;
    
    /* retrieve effect line and effect pad */
    if(is_output){
      pad_start = 
	pad = gtk_container_get_children((GtkContainer *) effect_bridge->output);
    }else{
      pad_start = 
	pad = gtk_container_get_children((GtkContainer *) effect_bridge->input);
    }

    pad = g_list_nth(pad,
		     line_editor->channel->pad);

    if(pad != NULL){
      list_start =
	list = gtk_container_get_children((GtkContainer *) AGS_EFFECT_PAD(pad->data)->table);

      while(list != NULL){
	if(AGS_EFFECT_LINE(list->data)->channel == line_editor->channel){
	  break;
	}

	list = list->next;
      }

      if(list != NULL){
	effect_line = AGS_EFFECT_LINE(list->data);
	g_list_free(list_start);
      }
    }

    g_list_free(pad_start);

    /* iterate line member */
    if(effect_line != NULL){
      for(nth = 0; line_member != NULL; nth++){
	children = gtk_container_get_children(GTK_CONTAINER(line_member->data));

	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(children->data))){
	  ags_line_member_editor_plugin_browser_response_destroy_entry();
	
	  /* remove effect */
	  ags_effect_line_remove_effect(effect_line,
					nth);
	}

	g_list_free(children);
	
	line_member = line_member->next;
      }
    }
  }
}
