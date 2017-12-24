/* GSequencer - Advanced GTK Sequencer
 * Copyright (C) 2005-2015 Joël Krähemann
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

#include <ags/X/machine/ags_drum_input_pad_callbacks.h>
#include <ags/X/machine/ags_drum_input_line_callbacks.h>
#include <ags/X/machine/ags_drum.h>

#include <ags/libags.h>
#include <ags/libags-audio.h>
#include <ags/libags-gui.h>

#include <ags/X/ags_window.h>
#include <ags/X/ags_line_callbacks.h>

#include <ags/X/thread/ags_gui_thread.h>

#include <math.h>

#include <ags/i18n.h>

void ags_drum_input_pad_open_response_callback(GtkWidget *widget, gint response, AgsDrumInputPad *pad);

#define AGS_DRUM_INPUT_PAD_OPEN_AUDIO_FILE_NAME "AgsDrumInputPadOpenAudioFileName"
#define AGS_DRUM_INPUT_PAD_OPEN_SPIN_BUTTON "AgsDrumInputPadOpenSpinButton"

void
ags_drum_input_pad_open_callback(GtkWidget *widget, AgsDrumInputPad *drum_input_pad)
{
  GtkFileChooserDialog *file_chooser;
  GtkHBox *hbox;
  GtkLabel *label;
  GtkSpinButton *spin_button;
  GtkToggleButton *play;

  if(drum_input_pad->file_chooser != NULL)
    return;

  drum_input_pad->file_chooser =
    file_chooser = (GtkFileChooserDialog *) gtk_file_chooser_dialog_new(i18n("Open File"),
									(GtkWindow *) gtk_widget_get_toplevel((GtkWidget *) drum_input_pad),
									GTK_FILE_CHOOSER_ACTION_OPEN,
									GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
									GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
									NULL);
  gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(file_chooser),
				       FALSE);
  g_object_set_data((GObject *) file_chooser, (char *) g_type_name(AGS_TYPE_AUDIO_FILE), NULL);
  g_object_set_data((GObject *) file_chooser, AGS_DRUM_INPUT_PAD_OPEN_AUDIO_FILE_NAME, NULL);

  hbox = (GtkHBox *) gtk_hbox_new(FALSE, 0);
  gtk_file_chooser_set_extra_widget((GtkFileChooser *) file_chooser,
				    (GtkWidget *) hbox);
  
  label = (GtkLabel *) gtk_label_new(i18n("channel: "));
  gtk_box_pack_start((GtkBox *) hbox,
		     (GtkWidget *) label,
		     FALSE, FALSE,
		     0);

  spin_button = (GtkSpinButton *) gtk_spin_button_new_with_range(0.0, AGS_AUDIO(AGS_PAD(drum_input_pad)->channel->audio)->audio_channels - 1, 1.0);
  g_object_set_data((GObject *) file_chooser, AGS_DRUM_INPUT_PAD_OPEN_SPIN_BUTTON, spin_button);
  gtk_box_pack_start((GtkBox *) hbox,
		     (GtkWidget *) spin_button,
		     FALSE, FALSE,
		     0);

  if(drum_input_pad->pad.group->active){
    gtk_widget_set_sensitive((GtkWidget *) spin_button,
			     FALSE);
  }

#if 0
  play = (GtkToggleButton *) g_object_new(GTK_TYPE_TOGGLE_BUTTON,
  					  "label", GTK_STOCK_MEDIA_PLAY,
  					  "use-stock", TRUE,
  					  "use-underline", TRUE,
  					  NULL);
  gtk_box_pack_start((GtkBox *) GTK_DIALOG(file_chooser)->action_area, (GtkWidget *) play, FALSE, FALSE, 0);
  gtk_box_reorder_child((GtkBox *) GTK_DIALOG(file_chooser)->action_area, (GtkWidget *) play, 0);
#endif
  
  gtk_widget_show_all((GtkWidget *) file_chooser);

  g_signal_connect((GObject *) file_chooser, "response",
		   G_CALLBACK(ags_drum_input_pad_open_response_callback), (gpointer) drum_input_pad);
}

void
ags_drum_input_pad_open_response_callback(GtkWidget *widget, gint response, AgsDrumInputPad *drum_input_pad)
{
  AgsWindow *window;
  AgsDrum *drum;

  GtkFileChooserDialog *file_chooser;
  GtkSpinButton *spin_button;

  AgsAudioFile *audio_file;

  AgsOpenSingleFile *open_single_file;

  AgsMutexManager *mutex_manager;
  AgsThread *main_loop;
  AgsGuiThread *gui_thread;
  
  AgsApplicationContext *application_context;
  
  char *name0, *name1;

  pthread_mutex_t *application_mutex;

  drum = (AgsDrum *) gtk_widget_get_ancestor(GTK_WIDGET(drum_input_pad),
					     AGS_TYPE_DRUM);

  window = (AgsWindow *) gtk_widget_get_ancestor((GtkWidget *) drum_input_pad,
						 AGS_TYPE_WINDOW);

  file_chooser = drum_input_pad->file_chooser;

  
  application_context = (AgsApplicationContext *) window->application_context;

  mutex_manager = ags_mutex_manager_get_instance();
  application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);

  /* get main loop */
  pthread_mutex_lock(application_mutex);

  main_loop = (AgsThread *) application_context->main_loop;

  pthread_mutex_unlock(application_mutex);

  /* find task thread */
  gui_thread = (AgsGuiThread *) ags_thread_find_type(main_loop,
						       AGS_TYPE_GUI_THREAD);

  if(response == GTK_RESPONSE_ACCEPT){
    name0 = gtk_file_chooser_get_filename((GtkFileChooser *) file_chooser);
    name1 = (char *) g_object_get_data((GObject *) file_chooser, AGS_DRUM_INPUT_PAD_OPEN_AUDIO_FILE_NAME);

    spin_button = (GtkSpinButton *) g_object_get_data((GObject *) file_chooser, AGS_DRUM_INPUT_PAD_OPEN_SPIN_BUTTON);

    /* open audio file and read audio signal */
    if(g_strcmp0(name0, name1)){
      if(name1 != NULL){
	audio_file = (AgsAudioFile *) g_object_get_data((GObject *) file_chooser, g_type_name(AGS_TYPE_AUDIO_FILE));
	g_object_unref(G_OBJECT(audio_file));
      }
    }

    /* task */
    if(AGS_PAD(drum_input_pad)->group->active){
      open_single_file = ags_open_single_file_new(AGS_PAD(drum_input_pad)->channel,
						  AGS_AUDIO(AGS_MACHINE(drum)->audio)->soundcard,
						  name0,
						  0, AGS_AUDIO(AGS_MACHINE(drum)->audio)->audio_channels);
    }else{
      AgsLine *line;
      GList *list;
      
      list = gtk_container_get_children(GTK_CONTAINER(AGS_PAD(drum_input_pad)->expander_set));
      line = AGS_LINE(ags_line_find_next_grouped(list)->data);

      open_single_file = ags_open_single_file_new(line->channel,
						  AGS_AUDIO(AGS_MACHINE(drum)->audio)->soundcard,
						  name0,
						  (guint) spin_button->adjustment->value, 1);

      g_list_free(list);
    }

    ags_gui_thread_schedule_task(gui_thread,
				open_single_file);

    gtk_widget_destroy((GtkWidget *) file_chooser);
  }else if(response == GTK_RESPONSE_CANCEL){
    audio_file = (AgsAudioFile *) g_object_get_data((GObject *) file_chooser, g_type_name(AGS_TYPE_AUDIO_FILE));

    if(audio_file != NULL){
      g_object_unref(G_OBJECT(audio_file));
    }

    gtk_widget_destroy((GtkWidget *) file_chooser);
  }

  drum_input_pad->file_chooser = NULL;
}

void
ags_drum_input_pad_play_callback(GtkToggleButton *toggle_button, AgsDrumInputPad *drum_input_pad)
{
  ags_pad_play(AGS_PAD(drum_input_pad));
}

void
ags_drum_input_pad_edit_callback(GtkWidget *toggle_button, AgsDrumInputPad *drum_input_pad)
{
  GtkToggleButton *toggle;
  AgsDrum *drum;

  drum = (AgsDrum *) gtk_widget_get_ancestor((GtkWidget *) drum_input_pad,
					     AGS_TYPE_DRUM);

  if(drum->selected_edit_button != NULL){
    if(GTK_TOGGLE_BUTTON(toggle_button) != drum->selected_edit_button){
      /* unset old */
      toggle = drum->selected_edit_button;

      drum->selected_edit_button = NULL;
      gtk_toggle_button_set_active((GtkToggleButton *) toggle, FALSE);

      /* apply new */
      drum->selected_edit_button = (GtkToggleButton *) toggle_button;
      drum->selected_pad = (AgsDrumInputPad *) gtk_widget_get_ancestor((GtkWidget *) toggle_button,
								       AGS_TYPE_DRUM_INPUT_PAD);

      AGS_MACHINE(drum)->selected_input_pad = (GtkWidget *) drum->selected_pad;
      
      ags_pattern_box_set_pattern(drum->pattern_box);
    }else{
      /* chain up */
      toggle = drum->selected_edit_button;
      
      drum->selected_edit_button = NULL;
      gtk_toggle_button_set_active((GtkToggleButton *) toggle, TRUE);

      /* reset */
      drum->selected_edit_button = toggle;
    }
  }
}
