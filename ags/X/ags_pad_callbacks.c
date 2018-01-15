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

#include <ags/X/ags_pad_callbacks.h>

#include <ags/libags.h>
#include <ags/libags-audio.h>

#include <ags/X/ags_window.h>
#include <ags/X/ags_machine.h>
#include <ags/X/ags_line_callbacks.h>

#include <ags/X/thread/ags_gui_thread.h>

void
ags_pad_group_clicked_callback(GtkWidget *widget, AgsPad *pad)
{
  AgsLine *line;
  GtkContainer *container;
  GList *list, *list_start;

  if(gtk_toggle_button_get_active(pad->group)){
    container = (GtkContainer *) pad->expander_set;

    list_start = 
      list = gtk_container_get_children(container);
    
    while(list != NULL){
      line = AGS_LINE(list->data);

      if(!gtk_toggle_button_get_active(line->group)){
	gtk_toggle_button_set_active(line->group, TRUE);
      }

      list = list->next;
    }

    g_list_free(list_start);
  }else{
    container = (GtkContainer *) pad->expander_set;

    list_start = 
      list = gtk_container_get_children(container);
    
    while(list != NULL){
      line = AGS_LINE(list->data);

      if(!gtk_toggle_button_get_active(line->group)){
	g_list_free(list_start);
	
	return;
      }

      list = list->next;
    }

    g_list_free(list_start);
    gtk_toggle_button_set_active(pad->group, TRUE);
  }
}

void
ags_pad_mute_clicked_callback(GtkWidget *widget, AgsPad *pad)
{
  AgsWindow *window;
  AgsMachine *machine;
  GtkContainer *container;

  AgsChannel *current, *next_pad;

  AgsSetMuted *set_muted;

  AgsMutexManager *mutex_manager;
  AgsThread *main_loop;
  AgsGuiThread *gui_thread;

  AgsApplicationContext *application_context;

  GList *list, *list_start, *tasks;

  gboolean is_output;
  
  pthread_mutex_t *application_mutex;
  pthread_mutex_t *current_mutex;

  machine = (AgsMachine *) gtk_widget_get_ancestor((GtkWidget *) pad,
						   AGS_TYPE_MACHINE);
  window = (AgsWindow *) gtk_widget_get_toplevel((GtkWidget *) machine);
  
  application_context = (AgsApplicationContext *) window->application_context;

  mutex_manager = ags_mutex_manager_get_instance();
  application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);

  /* get audio loop */
  pthread_mutex_lock(application_mutex);

  main_loop = (AgsThread *) application_context->main_loop;

  pthread_mutex_unlock(application_mutex);

  /* get task thread */
  gui_thread = (AgsGuiThread *) ags_thread_find_type(main_loop,
						       AGS_TYPE_GUI_THREAD);

  /*  */
  current = pad->channel;
  tasks = NULL;

  /* lookup current mutex */
  pthread_mutex_lock(application_mutex);

  current_mutex = ags_mutex_manager_lookup(mutex_manager,
					   (GObject *) current);
  
  pthread_mutex_unlock(application_mutex);

  if(gtk_toggle_button_get_active(pad->mute)){
    if(gtk_toggle_button_get_active(pad->solo)){
      gtk_toggle_button_set_active(pad->solo, FALSE);
    }
    
    /* mute */
    pthread_mutex_lock(current_mutex);

    next_pad = pad->channel->next_pad;

    pthread_mutex_unlock(current_mutex);

    while(current != next_pad){
      /* lookup current mutex */
      pthread_mutex_lock(application_mutex);

      current_mutex = ags_mutex_manager_lookup(mutex_manager,
					       (GObject *) current);
  
      pthread_mutex_unlock(application_mutex);

      /* instantiate set muted task */
      set_muted = ags_set_muted_new((GObject *) current,
				    TRUE);
      tasks = g_list_prepend(tasks,
			     set_muted);

      /* iterate */
      pthread_mutex_lock(current_mutex);
      
      current = current->next;
      
      pthread_mutex_unlock(current_mutex);
    }
  }else{
    if((AGS_MACHINE_SOLO & (machine->flags)) != 0){
      pthread_mutex_lock(current_mutex);

      is_output = (AGS_IS_OUTPUT(pad->channel))? TRUE: FALSE;
      
      pthread_mutex_unlock(current_mutex);

      container = (GtkContainer *) (is_output ? machine->output: machine->input);
      list_start = 
	list = gtk_container_get_children(container);

      while(!gtk_toggle_button_get_active(AGS_PAD(list->data)->solo)){
	list = list->next;
      }

      g_list_free(list_start);

      gtk_toggle_button_set_active(AGS_PAD(list->data)->solo, FALSE);

      machine->flags &= ~(AGS_MACHINE_SOLO);
    }

    /* unmute */
    pthread_mutex_lock(current_mutex);

    next_pad = pad->channel->next_pad;

    pthread_mutex_unlock(current_mutex);
    
    while(current != next_pad){
      /* lookup current mutex */
      pthread_mutex_lock(application_mutex);

      current_mutex = ags_mutex_manager_lookup(mutex_manager,
					       (GObject *) current);
  
      pthread_mutex_unlock(application_mutex);

      /* instantiate set muted task */
      set_muted = ags_set_muted_new((GObject *) current,
				    FALSE);
      tasks = g_list_prepend(tasks,
			     set_muted);

      /* iterate */
      pthread_mutex_lock(current_mutex);
      
      current = current->next;
      
      pthread_mutex_unlock(current_mutex);
    }
  }

  ags_gui_thread_schedule_task_list(gui_thread,
				    tasks);
}

void
ags_pad_solo_clicked_callback(GtkWidget *widget, AgsPad *pad)
{
  AgsMachine *machine;
  GtkContainer *container;

  GList *list, *list_start;

  machine = (AgsMachine *) gtk_widget_get_ancestor((GtkWidget *) pad, AGS_TYPE_MACHINE);

  if(gtk_toggle_button_get_active(pad->solo)){
    container = (GtkContainer *) (AGS_IS_OUTPUT(pad->channel) ? machine->output: machine->input);

    if(gtk_toggle_button_get_active(pad->mute))
      gtk_toggle_button_set_active(pad->mute, FALSE);

    list_start = 
      list = gtk_container_get_children(container);

    while(list != NULL){
      if(list->data == pad){
	list = list->next;
	continue;
      }

      gtk_toggle_button_set_active(AGS_PAD(list->data)->mute, TRUE);

      list = list->next;
    }

    g_list_free(list_start);
    machine->flags |= (AGS_MACHINE_SOLO);
  }else{
    machine->flags &= ~(AGS_MACHINE_SOLO);
  }
}

void
ags_pad_init_channel_launch_callback(AgsTask *task,
				     AgsPad *input_pad)
{ 
  AgsSoundcard *soundcard;
  AgsAudio *audio;
  AgsChannel *channel, *next_pad;
  AgsRecycling *last_recycling;
  AgsRecycling *recycling, *end_recycling;

  AgsAddAudioSignal *add_audio_signal;

  AgsMutexManager *mutex_manager;
  
  GList *recall;

  pthread_mutex_t *application_mutex;
  pthread_mutex_t *audio_mutex;
  pthread_mutex_t *channel_mutex;
  pthread_mutex_t *recycling_mutex;
 
  mutex_manager = ags_mutex_manager_get_instance();
  application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);

  /* get channel and its mutex */
  channel = input_pad->channel;

  pthread_mutex_lock(application_mutex);

  channel_mutex = ags_mutex_manager_lookup(mutex_manager,
					   (GObject *) channel);

  pthread_mutex_unlock(application_mutex);

  /* get audio and its audio mutex */
  pthread_mutex_lock(channel_mutex);

  audio = AGS_AUDIO(channel->audio);
  
  pthread_mutex_unlock(channel_mutex);

  pthread_mutex_lock(application_mutex);
  
  audio_mutex = ags_mutex_manager_lookup(mutex_manager,
					 (GObject *) input_pad->channel->audio);

  pthread_mutex_unlock(application_mutex);

  /* get soundcard */
  pthread_mutex_lock(audio_mutex);
  
  soundcard = AGS_SOUNDCARD(audio->soundcard);
  
  pthread_mutex_unlock(audio_mutex);
  
  /* get next pad */
  pthread_mutex_lock(channel_mutex);
  
  next_pad = channel->next_pad;

  pthread_mutex_unlock(channel_mutex);
  
#ifdef AGS_DEBUG
  g_message("launch");
#endif
  
  while(channel != next_pad){
    pthread_mutex_lock(channel_mutex);
  
    if(AGS_PLAYBACK(channel->playback) == NULL ||
       AGS_PLAYBACK(channel->playback)->recall_id[0] == NULL){
      channel = channel->next;

      pthread_mutex_unlock(channel_mutex);
  
      continue;
    }

    /* connect done */
    recall = ags_recall_find_provider_with_recycling_context(channel->play,
							     G_OBJECT(channel),
							     G_OBJECT(AGS_PLAYBACK(channel->playback)->recall_id[0]->recycling_context));
    
    recall = ags_recall_find_type(recall,
				  AGS_TYPE_PLAY_CHANNEL_RUN);
      
    pthread_mutex_unlock(channel_mutex);
    
    if(recall != NULL){
      AgsAudioSignal *audio_signal;
      AgsRecallID *current_recall_id;
      AgsNote *note;
      
      /* add audio signal */
      pthread_mutex_lock(channel_mutex);
      
      recycling = channel->first_recycling;
      last_recycling = channel->last_recycling;

      note = AGS_PLAYBACK(channel->playback)->play_note;
      
      current_recall_id = AGS_RECALL(recall->data)->recall_id;
      
      pthread_mutex_unlock(channel_mutex);

      /* get recycling mutex */
      pthread_mutex_lock(application_mutex);
  
      recycling_mutex = ags_mutex_manager_lookup(mutex_manager,
						 (GObject *) last_recycling);
	
      pthread_mutex_unlock(application_mutex);

      pthread_mutex_lock(recycling_mutex);
      
      end_recycling = last_recycling->next;

      pthread_mutex_unlock(recycling_mutex);
      
      while(recycling != end_recycling){
	/* get recycling mutex */
	pthread_mutex_lock(application_mutex);
  
	recycling_mutex = ags_mutex_manager_lookup(mutex_manager,
						   (GObject *) recycling);
	
	pthread_mutex_unlock(application_mutex);

	if(!AGS_RECALL(recall->data)->rt_safe){
	  /* instantiate audio signal */
	  audio_signal = ags_audio_signal_new((GObject *) soundcard,
					      (GObject *) recycling,
					      (GObject *) current_recall_id);
	  g_object_set(audio_signal,
		       "note", note,
		       NULL);
	  
	  /* add audio signal */
	  ags_recycling_create_audio_signal_with_defaults(recycling,
							  audio_signal,
							  0.0, 0);
	  audio_signal->stream_current = audio_signal->stream_beginning;
	  ags_connectable_connect(AGS_CONNECTABLE(audio_signal));
  
	  /*
	   * emit add_audio_signal on AgsRecycling
	   */
	  ags_recycling_add_audio_signal(recycling,
					 audio_signal);
	}else{
	  GList *list;

	  pthread_mutex_lock(recycling_mutex);
	    
	  audio_signal = NULL;
	  list = ags_audio_signal_get_by_recall_id(recycling->audio_signal,
						   current_recall_id);
	    
	  if(list != NULL){
	    audio_signal = list->data;

	    g_object_set(audio_signal,
			 "note", note,
			 NULL);
	  }

	  note->rt_offset = 0;

	  pthread_mutex_unlock(recycling_mutex);
	}

	/* iterate recycling */
	pthread_mutex_lock(recycling_mutex);

	recycling = recycling->next;

	pthread_mutex_unlock(recycling_mutex);
      }    
    }

    /* iterate */
    pthread_mutex_lock(channel_mutex);

    channel = channel->next;

    pthread_mutex_unlock(channel_mutex);    
  }
}
