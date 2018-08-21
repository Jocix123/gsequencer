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

#include <ags/X/machine/ags_drum_callbacks.h>
#include <ags/X/machine/ags_drum_input_pad.h>
#include <ags/X/ags_machine.h>
#include <ags/X/ags_machine_callbacks.h>

#include <ags/libags.h>
#include <ags/libags-audio.h>
#include <ags/libags-gui.h>

#include <ags/X/ags_ui_provider.h>
#include <ags/X/ags_window.h>
#include <ags/X/ags_pad.h>
#include <ags/X/ags_navigation.h>

#include <ags/X/thread/ags_gui_thread.h>

#include <stdlib.h>
#include <math.h>

#define AGS_AUDIO_FILE_DEVOUT "AgsAudioFileDevout"
#define AGS_DRUM_PLAY_RECALL "AgsDrumPlayRecall"

void ags_drum_open_response_callback(GtkDialog *dialog, gint response, AgsDrum *drum);

void
ags_drum_parent_set_callback(GtkWidget *widget, GtkObject *old_parent, AgsDrum *drum)
{
  AgsWindow *window;

  gchar *str;
  
  if(old_parent != NULL){
    return;
  }

  window = AGS_WINDOW(gtk_widget_get_ancestor((GtkWidget *) drum, AGS_TYPE_WINDOW));

  str = g_strdup_printf("Default %d",
			ags_window_find_machine_counter(window, AGS_TYPE_DRUM)->counter);

  g_object_set(AGS_MACHINE(drum),
	       "machine-name", str,
	       NULL);

  ags_window_increment_machine_counter(window,
				       AGS_TYPE_DRUM);
  g_free(str);
}

void
ags_drum_destroy_callback(GtkWidget *widget, AgsDrum *drum)
{
  GList *list, *list_start;

  if(drum->open_dialog != NULL){
    gtk_widget_destroy(drum->open_dialog);
  }
  
  list =
    list_start = gtk_container_get_children(AGS_MACHINE(drum)->input);

  while(list != NULL){

    if(AGS_DRUM_INPUT_PAD(list->data)->file_chooser != NULL){
      gtk_widget_destroy(GTK_WIDGET(AGS_DRUM_INPUT_PAD(list->data)->file_chooser));
    }

    list = list->next;
  }
}

void
ags_drum_open_callback(GtkWidget *toggle_button, AgsDrum *drum)
{
  GtkFileChooserDialog *file_chooser;
  GtkCheckButton *check_button;

  if(drum->open_dialog != NULL){
    return;
  }
  
  file_chooser = (GtkFileChooserDialog *) gtk_file_chooser_dialog_new(g_strdup("open audio files"),
								      (GtkWindow *) gtk_widget_get_toplevel((GtkWidget *) drum),
								      GTK_FILE_CHOOSER_ACTION_OPEN,
								      GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
								      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, 
								      NULL);
  drum->open_dialog = (GtkWidget *) file_chooser;
  gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(file_chooser), TRUE);

  check_button = (GtkCheckButton *) gtk_check_button_new_with_label(g_strdup("open in new channel"));
  gtk_toggle_button_set_active((GtkToggleButton *) check_button, TRUE);
  gtk_box_pack_start((GtkBox *) GTK_DIALOG(file_chooser)->vbox, (GtkWidget *) check_button, FALSE, FALSE, 0);
  g_object_set_data(G_OBJECT(file_chooser), "create", (gpointer) check_button);

  check_button = (GtkCheckButton *) gtk_check_button_new_with_label(g_strdup("overwrite existing links"));
  gtk_toggle_button_set_active((GtkToggleButton *) check_button, TRUE);
  gtk_box_pack_start((GtkBox *) GTK_DIALOG(file_chooser)->vbox, (GtkWidget *) check_button, FALSE, FALSE, 0);
  g_object_set_data(G_OBJECT(file_chooser), "overwrite", (gpointer) check_button);

  gtk_widget_show_all(GTK_WIDGET(file_chooser));

  g_signal_connect(G_OBJECT(file_chooser), "response",
		   G_CALLBACK(ags_drum_open_response_callback), drum);
  g_signal_connect(G_OBJECT(file_chooser), "response",
		   G_CALLBACK(ags_machine_open_response_callback), drum);
}

void
ags_drum_open_response_callback(GtkDialog *dialog, gint response, AgsDrum *drum)
{
  drum->open_dialog = NULL;
}

void
ags_drum_loop_button_callback(GtkWidget *button, AgsDrum *drum)
{
  AgsPort *port;
  
  AgsCountBeatsAudio *count_beats_audio;

  GList *start_list, *list;

  gboolean loop;

  loop = (GTK_TOGGLE_BUTTON(button)->active) ? TRUE: FALSE;

  /* play - count beats audio */
  g_object_get(AGS_MACHINE(drum)->audio,
	       "play", &start_list,
	       NULL);
  
  list = start_list;

  while((list = ags_recall_find_type(list,
				     AGS_TYPE_COUNT_BEATS_AUDIO)) != NULL){
    GValue value = {0,};
    
    count_beats_audio = AGS_COUNT_BEATS_AUDIO(list->data);
    g_object_get(count_beats_audio,
		 "sequencer-loop", &port,
		 NULL);

    g_value_init(&value,
		 G_TYPE_BOOLEAN);
    g_value_set_boolean(&value,
			loop);

    ags_port_safe_write(port,
			&value);

    g_value_unset(&value);
    
    /* iterate */
    list = list->next;
  }

  g_list_free(start_list);

  /* recall - count beats audio */
  g_object_get(AGS_MACHINE(drum)->audio,
	       "recall", &start_list,
	       NULL);

  list = start_list;

  while((list = ags_recall_find_type(list,
				     AGS_TYPE_COUNT_BEATS_AUDIO)) != NULL){
    GValue value = {0,};
    
    count_beats_audio = AGS_COUNT_BEATS_AUDIO(list->data);
    g_object_get(count_beats_audio,
		 "sequencer-loop", &port,
		 NULL);

    g_value_init(&value,
		 G_TYPE_BOOLEAN);
    g_value_set_boolean(&value,
			loop);

    ags_port_safe_write(port,
			&value);

    g_value_unset(&value);
    
    /* iterate */
    list = list->next;
  }

  g_list_free(start_list);  
}

void
ags_drum_length_spin_callback(GtkWidget *spin_button, AgsDrum *drum)
{
  AgsWindow *window;
  
  AgsGuiThread *gui_thread;

  AgsApplySequencerLength *apply_sequencer_length;

  AgsApplicationContext *application_context;
  
  gdouble length;

  /* get window and application_context  */
  window = (AgsWindow *) gtk_widget_get_toplevel((GtkWidget *) drum);
  
  application_context = (AgsApplicationContext *) window->application_context;

  gui_thread = ags_ui_provider_get_gui_thread(AGS_UI_PROVIDER(application_context));

  /* task - apply length */
  length = GTK_SPIN_BUTTON(spin_button)->adjustment->value;

  apply_sequencer_length = ags_apply_sequencer_length_new(AGS_MACHINE(drum)->audio,
							  length);

  ags_gui_thread_schedule_task(gui_thread,
			       apply_sequencer_length);
}

void
ags_drum_index0_callback(GtkWidget *widget, AgsDrum *drum)
{
  if(drum->selected0 != NULL){
    GtkToggleButton *toggle_button;

    if(GTK_TOGGLE_BUTTON(widget) != drum->selected0){
      AgsPort *port;
      
      AgsCopyPatternAudio *copy_pattern_audio;
      
      GList *start_list, *list;

      guint64 index0;
      
      toggle_button = drum->selected0;
      drum->selected0 = NULL;

      gtk_toggle_button_set_active(toggle_button,
				   FALSE);

      drum->selected0 = (GtkToggleButton*) widget;

      /* calculate index 0 */
      AGS_MACHINE(drum)->bank_0 = 
	index0 = ((guint) drum->selected0->button.label_text[0] - 'a');

      /* play - set port */
      g_object_get(AGS_MACHINE(drum)->audio,
		   "play", &start_list,
		   NULL);
      
      list = ags_recall_find_type(start_list,
				  AGS_TYPE_COPY_PATTERN_AUDIO);

      if(list != NULL){
	GValue value = {0,};

	g_value_init(&value,
		     G_TYPE_UINT64);
	
	g_value_set_uint64(&value,
			   index0);

	copy_pattern_audio = AGS_COPY_PATTERN_AUDIO(list->data);
	g_object_get(copy_pattern_audio,
		     "bank-index-0", &port,
		     NULL);
	
	ags_port_safe_write(port,
			    &value);

	g_value_unset(&value);
      }

      g_list_free(start_list);
      
      /* recall - set port */
      g_object_get(AGS_MACHINE(drum)->audio,
		   "recall", &start_list,
		   NULL);
      
      list = ags_recall_find_type(start_list,
				  AGS_TYPE_COPY_PATTERN_AUDIO);

      if(list != NULL){
	GValue value = {0,};

	g_value_init(&value,
		     G_TYPE_UINT64);
	
	g_value_set_uint64(&value,
			   index0);

	copy_pattern_audio = AGS_COPY_PATTERN_AUDIO(list->data);
	g_object_get(copy_pattern_audio,
		     "bank-index-0", &port,
		     NULL);

	ags_port_safe_write(port,
			    &value);

	g_value_unset(&value);
      }
      
      g_list_free(start_list);
    }else if(! gtk_toggle_button_get_active(drum->selected0)){
      drum->selected0 = NULL;

      gtk_toggle_button_set_active((GtkToggleButton *) widget,
				   TRUE);

      drum->selected0 = (GtkToggleButton*) widget;
    }

    ags_pattern_box_set_pattern(drum->pattern_box);
  }
}

void
ags_drum_index1_callback(GtkWidget *widget, AgsDrum *drum)
{
  if(drum->selected1 != NULL){
    GtkToggleButton *toggle_button;

    if(GTK_TOGGLE_BUTTON(widget) != drum->selected1){
      AgsPort *port;
      
      AgsCopyPatternAudio *copy_pattern_audio;

      GList *start_list, *list;

      guint64 index1;
      
      toggle_button = drum->selected1;
      drum->selected1 = NULL;

      gtk_toggle_button_set_active(toggle_button,
				   FALSE);

      drum->selected1 = (GtkToggleButton*) widget;

      /* calculate index 1 */
      AGS_MACHINE(drum)->bank_1 = 
	index1 = ((guint) g_ascii_strtoull(drum->selected1->button.label_text, NULL, 10)) - 1;

      /* play - set port */
      g_object_get(AGS_MACHINE(drum)->audio,
		   "play", &start_list,
		   NULL);
      
      list = ags_recall_find_type(start_list,
				  AGS_TYPE_COPY_PATTERN_AUDIO);

      if(list != NULL){
	GValue value = {0,};

	g_value_init(&value,
		     G_TYPE_UINT64);
	
	g_value_set_uint64(&value,
			   index1);

	copy_pattern_audio = AGS_COPY_PATTERN_AUDIO(list->data);
	g_object_get(copy_pattern_audio,
		     "bank-index-1", &port,
		     NULL);

	ags_port_safe_write(port,
			    &value);

	g_value_unset(&value);
      }


      g_list_free(start_list);
      
      /* recall - set port */
      g_object_get(AGS_MACHINE(drum)->audio,
		   "recall", &start_list,
		   NULL);
      
      list = ags_recall_find_type(start_list,
				  AGS_TYPE_COPY_PATTERN_AUDIO);

      if(list != NULL){
	GValue value = {0,};

	g_value_init(&value,
		     G_TYPE_UINT64);
	g_value_set_uint64(&value,
			   index1);

	copy_pattern_audio = AGS_COPY_PATTERN_AUDIO(list->data);
	g_object_get(copy_pattern_audio,
		     "bank-index-1", &port,
		     NULL);
	
	ags_port_safe_write(port,
			    &value);

	g_value_unset(&value);
      }
    }else if(!gtk_toggle_button_get_active(drum->selected1)){
      drum->selected1 = NULL;

      gtk_toggle_button_set_active((GtkToggleButton *) widget,
				   TRUE);

      drum->selected1 = (GtkToggleButton*) widget;
    }

    ags_pattern_box_set_pattern(drum->pattern_box);
  }
}

void
ags_drum_stop_callback(AgsDrum *drum,
		       GList *recall_id, gint sound_scope,
		       gpointer data)
{
  if(sound_scope != AGS_SOUND_SCOPE_SEQUENCER){
    return;
  }
  
  /* all done */
  ags_led_array_unset_all(drum->pattern_box->hled_array);
}
