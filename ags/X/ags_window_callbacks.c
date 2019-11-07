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

#include <ags/X/ags_window_callbacks.h>

#include <ags/libags.h>
#include <ags/libags-audio.h>

#include <ags/X/file/ags_simple_file.h>

gboolean
ags_window_delete_event_callback(GtkWidget *widget, gpointer data)
{
  AgsWindow *window;
  GtkDialog *dialog;
  GtkWidget *cancel_button;

  AgsApplicationContext *application_context;
  
  gint response;
  
  window = AGS_WINDOW(widget);

  application_context = ags_application_context_get_instance();

  /* ask the user if he wants save to a file */
  dialog = (GtkDialog *) gtk_message_dialog_new(GTK_WINDOW(window),
						GTK_DIALOG_DESTROY_WITH_PARENT,
						GTK_MESSAGE_QUESTION,
						GTK_BUTTONS_YES_NO,
						"Do you want to save '%s'?", window->name);
  cancel_button = gtk_dialog_add_button(dialog,
					GTK_STOCK_CANCEL,
					GTK_RESPONSE_CANCEL);
  gtk_widget_grab_focus(cancel_button);

  response = gtk_dialog_run(dialog);

  if(response == GTK_RESPONSE_YES){
    if(g_strcmp0(ags_config_get_value(ags_config_get_instance(),
				      AGS_CONFIG_GENERIC,
				      "simple-file"),
		 "false")){
      AgsSimpleFile *simple_file;

      GError *error;
      
      simple_file = (AgsSimpleFile *) g_object_new(AGS_TYPE_SIMPLE_FILE,
						   "filename", window->name,
						   NULL);
      
      error = NULL;
      ags_simple_file_rw_open(simple_file,
			      TRUE,
			      &error);

      if(error != NULL){
	g_message("%s", error->message);

	g_error_free(error);
      }
      
      ags_simple_file_write(simple_file);
      ags_simple_file_close(simple_file);
      
      g_object_unref(G_OBJECT(simple_file));
    }else{
      AgsFile *file;

      GError *error;
      
      file = (AgsFile *) g_object_new(AGS_TYPE_FILE,
				      "filename", window->name,
				      NULL);
      
      error = NULL;
      ags_file_rw_open(file,
		       TRUE,
		       &error);

      if(error != NULL){
	g_message("%s", error->message);

	g_error_free(error);
      }
      
      ags_file_write(file);
      ags_file_close(file);
      
      g_object_unref(G_OBJECT(file));
    }
  }

  if(response != GTK_RESPONSE_CANCEL){
    ags_application_context_quit(application_context);
  }else{
    gtk_widget_destroy(GTK_WIDGET(dialog));
  }

  return(TRUE);
}

void
ags_window_button_press_event(GtkWidget *widget, GdkEventButton *event, AgsWindow *window)
{
  if(event->type == GDK_BUTTON_PRESS && event->button == 3){
    gtk_menu_popup(GTK_MENU(window->context_menu), NULL, NULL, NULL, NULL,
                   (event != NULL) ? event->button : 0,
                   gdk_event_get_time((GdkEvent*)event));
  }
}
