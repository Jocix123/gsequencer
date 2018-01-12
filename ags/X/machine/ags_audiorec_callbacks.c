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

#include <ags/X/machine/ags_audiorec_callbacks.h>

#include <ags/i18n.h>

void ags_audiorec_open_response_callback(GtkWidget *widget, gint response,
					 AgsAudiorec *audiorec);

void
ags_audiorec_open_callback(GtkWidget *button, AgsAudiorec *audiorec)
{
  GtkFileChooserDialog *dialog;

  if(audiorec->open_dialog != NULL){
    return;
  }
  
  audiorec->open_dialog = 
    dialog = gtk_file_chooser_dialog_new(i18n("Open audio files"),
					 (GtkWindow *) gtk_widget_get_toplevel((GtkWidget *) audiorec),
					 GTK_FILE_CHOOSER_ACTION_OPEN,
					 GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
					 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					 NULL);
  gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog),
				       FALSE);
  gtk_widget_show_all((GtkWidget *) dialog);

  g_signal_connect((GObject *) dialog, "response",
		   G_CALLBACK(ags_audiorec_open_response_callback), audiorec);
}

void
ags_audiorec_open_response_callback(GtkWidget *widget, gint response,
				    AgsAudiorec *audiorec)
{
  if(response == GTK_RESPONSE_ACCEPT){
    gchar *filename;

    filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widget));
    gtk_entry_set_text(audiorec->filename,
		       filename);
    ags_audiorec_open_filename(audiorec,
			       filename);
  }

  audiorec->open_dialog = NULL;
  gtk_widget_destroy(widget);
}

void
ags_audiorec_keep_data_callback(GtkWidget *button, AgsAudiorec *audiorec)
{
  //TODO:JK: implement me
}

void
ags_audiorec_replace_data_callback(GtkWidget *button, AgsAudiorec *audiorec)
{
  //TODO:JK: implement me
}

void
ags_audiorec_mix_data_callback(GtkWidget *button, AgsAudiorec *audiorec)
{
  //TODO:JK: implement me
}
