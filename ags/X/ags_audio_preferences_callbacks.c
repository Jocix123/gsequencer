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

#include <ags/X/ags_audio_preferences_callbacks.h>

#include <ags/libags.h>
#include <ags/libags-audio.h>

#include <ags/X/ags_xorg_application_context.h>
#include <ags/X/ags_window.h>
#include <ags/X/ags_preferences.h>
#include <ags/X/ags_soundcard_editor.h>

#include <ags/config.h>
#include <ags/i18n.h>

int
ags_audio_preferences_parent_set_callback(GtkWidget *widget, GtkObject *old_parent, AgsAudioPreferences *audio_preferences)
{  
  AgsPreferences *preferences;

  if(old_parent != NULL){
    return(0);
  }

  preferences = (AgsPreferences *) gtk_widget_get_ancestor(GTK_WIDGET(audio_preferences),
							   AGS_TYPE_PREFERENCES);

  audio_preferences->connect_sink = (GtkButton *) gtk_button_new_with_label(i18n("connect sink"));
  gtk_box_pack_end((GtkBox *) GTK_DIALOG(preferences)->action_area,
		   (GtkWidget *) audio_preferences->connect_sink,
		   TRUE, FALSE,
		   0);  

  audio_preferences->add = (GtkButton *) gtk_button_new_from_stock(GTK_STOCK_ADD);
  gtk_box_pack_end((GtkBox *) GTK_DIALOG(preferences)->action_area,
		   (GtkWidget *) audio_preferences->add,
		   TRUE, FALSE,
		   0);  

  return(0);
}

void
ags_audio_preferences_connect_sink_callback(GtkWidget *widget, AgsAudioPreferences *audio_preferences)
{
  AgsWindow *window;
  AgsPreferences *preferences;
  
  AgsApplicationContext *application_context;

  GObject *server;

  GList *start_list, *list;
  
  preferences = (AgsPreferences *) gtk_widget_get_ancestor(GTK_WIDGET(audio_preferences),
							   AGS_TYPE_PREFERENCES);
  window = (AgsWindow *) preferences->window;

  application_context = (AgsApplicationContext *) window->application_context;

  list = 
    start_list = ags_sound_provider_get_sound_server(AGS_SOUND_PROVIDER(application_context));

  while(list != NULL){
    server = list->data;

    if(AGS_IS_JACK_SERVER(server)){
      ags_jack_server_connect_client(server);
    }
    
    list = list->next;
  }

  g_list_free(start_list);
}

void
ags_audio_preferences_add_callback(GtkWidget *widget, AgsAudioPreferences *audio_preferences)
{
  AgsWindow *window;
  AgsPreferences *preferences;
  AgsSoundcardEditor *soundcard_editor;

  AgsSoundcardThread *soundcard_thread;

  AgsThread *main_loop;
  
  AgsApplicationContext *application_context;

  GObject *soundcard;

  GList *start_list, *list;

  preferences = (AgsPreferences *) gtk_widget_get_ancestor(GTK_WIDGET(audio_preferences),
							   AGS_TYPE_PREFERENCES);
  window = (AgsWindow *) preferences->window;

  application_context = (AgsApplicationContext *) window->application_context;

  g_object_get(application_context,
	       "main-loop", &main_loop,
	       NULL);
  
  /* retrieve first soundcard */
  soundcard = NULL;

  list =
    start_list = ags_sound_provider_get_soundcard(AGS_SOUND_PROVIDER(application_context));
  
  if(list != NULL){
    soundcard = list->data;
  }

  g_list_free(start_list);
  
  /* soundcard editor */
  soundcard_editor = ags_soundcard_editor_new();

  if(soundcard != NULL){
    soundcard_editor->soundcard = soundcard;
    soundcard_editor->soundcard_thread = (GObject *) ags_thread_find_type(main_loop,
									  AGS_TYPE_SOUNDCARD_THREAD);
   }
  
  list =
    start_list = gtk_container_get_children((GtkContainer *) audio_preferences->soundcard_editor);
  
  if(list != NULL){
    gtk_widget_set_sensitive((GtkWidget *) soundcard_editor->buffer_size,
			     FALSE);
  }

  g_list_free(start_list);
  
  gtk_box_pack_start((GtkBox *) audio_preferences->soundcard_editor,
		     (GtkWidget *) soundcard_editor,
		     FALSE, FALSE,
		     0);
  
  ags_applicable_reset(AGS_APPLICABLE(soundcard_editor));
  ags_connectable_connect(AGS_CONNECTABLE(soundcard_editor));
  g_signal_connect(soundcard_editor->remove, "clicked",
		   G_CALLBACK(ags_audio_preferences_remove_soundcard_editor_callback), audio_preferences);
  gtk_widget_show_all((GtkWidget *) soundcard_editor);

  /* reset default card */  
  g_object_set(window,
	       "soundcard", soundcard,
	       NULL);
}

void
ags_audio_preferences_remove_soundcard_editor_callback(GtkWidget *button,
						       AgsAudioPreferences *audio_preferences)
{
  AgsWindow *window;
  AgsPreferences *preferences;
  AgsSoundcardEditor *soundcard_editor;

  AgsApplicationContext *application_context;

  GObject *soundcard;

  GList *start_list, *list;
  
  preferences = (AgsPreferences *) gtk_widget_get_ancestor(GTK_WIDGET(audio_preferences),
							   AGS_TYPE_PREFERENCES);
  window = (AgsWindow *) preferences->window;

  application_context = (AgsApplicationContext *) window->application_context;

  soundcard_editor = (AgsSoundcardEditor *) gtk_widget_get_ancestor(button,
								    AGS_TYPE_SOUNDCARD_EDITOR);

  if(!AGS_IS_JACK_DEVOUT(soundcard_editor->soundcard)){
    ags_soundcard_editor_remove_soundcard(soundcard_editor,
					  soundcard_editor->soundcard);
  }
  
  gtk_widget_destroy((GtkWidget *) soundcard_editor);

  /* reset default card */
#if 0
  soundcard = NULL;
  
  list =
    start_list = ags_sound_provider_get_soundcard(AGS_SOUND_PROVIDER(application_context));
  
  if(list != NULL){
    soundcard = list->data;
  }

  g_list_free(start_list);
  
  g_object_set(window,
	       "soundcard", soundcard,
	       NULL);
#endif
  
  /*  */
  list =
    start_list = gtk_container_get_children((GtkContainer *) audio_preferences->soundcard_editor);
  
  if(list != NULL){
    gtk_widget_set_sensitive((GtkWidget *) AGS_SOUNDCARD_EDITOR(list->data)->buffer_size,
			     TRUE);
  }

  g_list_free(start_list);
}

void
ags_audio_preferences_start_jack_callback(GtkButton *button,
					  AgsAudioPreferences *audio_preferences)
{
  AgsWindow *window;

  AgsJackServer *jack_server;
  
  AgsApplicationContext *application_context;

#ifdef AGS_WITH_JACK
  jackctl_driver_t *driver;
#endif

  GList *start_list, *list;

#ifdef AGS_WITH_JACK
  JSList *jslist;
#endif

  //  gchar *driver_name;
  
  pthread_mutex_t *application_mutex;

  window = AGS_WINDOW(AGS_PREFERENCES(gtk_widget_get_ancestor(GTK_WIDGET(audio_preferences),
							      AGS_TYPE_PREFERENCES))->window);

  application_context = (AgsApplicationContext *) window->application_context;

  //TODO:JK: improve me
  list =
    start_list = ags_sound_provider_get_sound_server(AGS_SOUND_PROVIDER(application_context));
  
  while(list != NULL){
    if(AGS_IS_JACK_SERVER(list->data)){
      jack_server = list->data;

      break;
    }
    
    list = list->next;
  }

  g_list_free(start_list);
  
#ifdef AGS_WITH_JACK
  /* find driver */
  driver = NULL;
  
  //  driver_name = gtk_combo_box_text_get_active_text(audio_preferences->jack_driver);

  jslist = NULL;
  //  jslist = jackctl_server_get_drivers_list(jack_server->jackctl);

  while(jslist != NULL){
    //    if(!g_ascii_strcasecmp(driver_name,
    //			   jackctl_driver_get_name(jslist->data))){
    //      driver = jslist->data;

    //      break;
    //    }
  }

  /* start server */
  if(jack_server != NULL){
    //    jackctl_server_start(jack_server->jackctl);
  }
#endif
}

void
ags_audio_preferences_stop_jack_callback(GtkButton *button,
					 AgsAudioPreferences *audio_preferences)
{
  AgsWindow *window;

  AgsJackServer *jack_server;
  
  AgsMutexManager *mutex_manager;

  AgsApplicationContext *application_context;

  GList *start_list, *list;

  window = AGS_WINDOW(AGS_PREFERENCES(gtk_widget_get_ancestor(GTK_WIDGET(audio_preferences),
									 AGS_TYPE_PREFERENCES))->window);

  application_context = (AgsApplicationContext *) window->application_context;

  //TODO:JK: improve me
  list =
    start_list = ags_sound_provider_get_sound_server(AGS_SOUND_PROVIDER(application_context));
  
  while(list != NULL){
    if(AGS_IS_JACK_SERVER(list->data)){
      jack_server = list->data;

      break;
    }
    
    list = list->next;
  }

  g_list_free(start_list);
  
  if(jack_server != NULL){
#ifdef AGS_WITH_JACK
    //    jackctl_server_stop(jack_server->jackctl);
#endif
  }
}
