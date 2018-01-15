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

#include <ags/X/ags_notation_editor_callbacks.h>

#include <ags/libags.h>
#include <ags/libags-audio.h>
#include <ags/libags-gui.h>

void
ags_notation_editor_machine_changed_callback(AgsMachineSelector *machine_selector, AgsMachine *machine,
					     AgsNotationEditor *notation_editor)
{
  ags_notation_editor_machine_changed(notation_editor, machine);
}

void
ags_notation_editor_resize_audio_channels_callback(AgsMachine *machine, 
						   guint audio_channels, guint audio_channels_old,
						   AgsNotationEditor *notation_editor)
{
  guint i;
  
  if(audio_channels > audio_channels_old){
    GList *tab;
    
    for(i = audio_channels_old; i < audio_channels; i++){
      ags_notebook_insert_tab(notation_editor->notebook,
			      i);

      tab = notation_editor->notebook->tab;
      gtk_toggle_button_set_active(AGS_NOTEBOOK_TAB(tab->data)->toggle,
				   TRUE);
    }
  }else{
    /* shrink notebook */
    for(i = audio_channels; i < audio_channels_old; i++){
      ags_notebook_remove_tab(notation_editor->notebook,
			      audio_channels);
    }
  }
}

void
ags_notation_editor_resize_pads_callback(AgsMachine *machine, GType channel_type,
					 guint pads, guint pads_old,
					 AgsNotationEditor *notation_editor)
{
  AgsAudio *audio;
  
  AgsMutexManager *mutex_manager;

  pthread_mutex_t *application_mutex;
  pthread_mutex_t *audio_mutex;

  audio = machine->audio;
  
  mutex_manager = ags_mutex_manager_get_instance();
  application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);

  /* get audio mutex */
  pthread_mutex_lock(application_mutex);
  
  audio_mutex = ags_mutex_manager_lookup(mutex_manager,
					 (GObject *) audio);
  
  pthread_mutex_unlock(application_mutex);

  /* verify pads */
  pthread_mutex_lock(audio_mutex);
  
  if((AGS_AUDIO_NOTATION_DEFAULT & (audio->flags)) != 0){
    if(!g_type_is_a(channel_type,
		    AGS_TYPE_INPUT)){
      pthread_mutex_unlock(audio_mutex);
      
      return;
    }    
  }else{
    if(!g_type_is_a(channel_type,
		    AGS_TYPE_OUTPUT)){
      pthread_mutex_unlock(audio_mutex);
      
      return;
    }
  }
  
  pthread_mutex_unlock(audio_mutex);

  /*  */
  g_object_set(notation_editor->scrolled_piano->piano,
	       "key-count", pads,
	       NULL);
  gtk_widget_queue_draw(notation_editor->scrolled_piano->piano);

  /*  */
  gtk_widget_queue_draw(notation_editor->notation_edit);
}


void
ags_notation_editor_init_channel_launch_callback(AgsTask *task, AgsNote *note)
{
  AgsAudio *audio;
  AgsChannel *channel;
  AgsRecycling *recycling, *last_recycling;
  AgsRecallID *recall_id;
  
  AgsAddAudioSignal *add_audio_signal;

  AgsMutexManager *mutex_manager;

  AgsApplicationContext *application_context;
  AgsConfig *config;

  GObject *soundcard;
  
  GList *recall, *tmp;
  GList *delay_audio;
  
  gchar *str;
  gdouble notation_delay;
  guint samplerate;
  
  GValue value = {0,};

  pthread_mutex_t *application_mutex;
  pthread_mutex_t *soundcard_mutex;
  pthread_mutex_t *audio_mutex;
  pthread_mutex_t *channel_mutex;
  pthread_mutex_t *recycling_mutex;

  mutex_manager = ags_mutex_manager_get_instance();
  application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);

  channel = AGS_INIT_CHANNEL(task)->channel;

  /* */
  pthread_mutex_lock(application_mutex);

  channel_mutex = ags_mutex_manager_lookup(mutex_manager,
					   (GObject *) channel);

  pthread_mutex_unlock(application_mutex);

  /* */
  pthread_mutex_lock(channel_mutex);
  
  soundcard = channel->soundcard;
  audio = channel->audio;
  
  pthread_mutex_unlock(channel_mutex);

  /* */
  pthread_mutex_lock(application_mutex);

  soundcard_mutex = ags_mutex_manager_lookup(mutex_manager,
					     (GObject *) soundcard);

  pthread_mutex_unlock(application_mutex);

  /*  */
  pthread_mutex_lock(soundcard_mutex);

  ags_soundcard_get_presets(AGS_SOUNDCARD(soundcard),
			    NULL,
			    &samplerate,
			    NULL,
			    NULL);

  pthread_mutex_unlock(soundcard_mutex);
  
#ifdef AGS_DEBUG
  g_message("launch");
#endif

  pthread_mutex_lock(channel_mutex);

  if(AGS_PLAYBACK(channel->playback) == NULL ||
     AGS_PLAYBACK(channel->playback)->recall_id[0] == NULL){    

    pthread_mutex_unlock(channel_mutex);
    
    return;
  }

  recall_id = AGS_PLAYBACK(channel->playback)->recall_id[0];
  last_recycling = channel->last_recycling;
  
  /* connect done */
  recall = ags_recall_find_provider_with_recycling_context(channel->play,
							   G_OBJECT(channel),
							   G_OBJECT(recall_id->recycling_context));
  
  tmp = recall;
  recall = ags_recall_find_type(recall,
				AGS_TYPE_PLAY_CHANNEL_RUN);
  //FIXME:JK: below
  //    g_list_free(tmp);

  pthread_mutex_unlock(channel_mutex);

  /* */
  pthread_mutex_lock(application_mutex);

  audio_mutex = ags_mutex_manager_lookup(mutex_manager,
					 (GObject *) audio);

  pthread_mutex_unlock(application_mutex);

  /* read notation delay */
  pthread_mutex_lock(audio_mutex);

  delay_audio = audio->play;
  delay_audio = ags_recall_find_type(delay_audio,
				     AGS_TYPE_DELAY_AUDIO);
  
  if(delay_audio != NULL){
    g_value_init(&value,
		 G_TYPE_DOUBLE);
    ags_port_safe_read(AGS_DELAY_AUDIO(delay_audio->data)->notation_delay,
		       &value);
    notation_delay = g_value_get_double(&value);
  }else{
    notation_delay = 1.0;
  }
  
  pthread_mutex_unlock(audio_mutex);

  if(recall != NULL){
    AgsAudioSignal *audio_signal;
      
    /* add audio signal */
    recycling = channel->first_recycling;

    while(recycling != last_recycling->next){
      /* get recycling mutex */
      pthread_mutex_lock(application_mutex);
  
      recycling_mutex = ags_mutex_manager_lookup(mutex_manager,
						 (GObject *) recycling);
	
      pthread_mutex_unlock(application_mutex);
      
      if(!AGS_RECALL(recall->data)->rt_safe){
	audio_signal = ags_audio_signal_new((GObject *) soundcard,
					    (GObject *) recycling,
					    (GObject *) recall_id);
	g_object_set(audio_signal,
		     "note", note,
		     NULL);
	
	/* add audio signal */
	ags_recycling_create_audio_signal_with_frame_count(recycling,
							   audio_signal,
							   (note->x[1] - note->x[0]) * ((gdouble) samplerate / notation_delay),
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
						 recall_id);
	    
	if(list != NULL){
	  audio_signal = list->data;

	  g_object_set(audio_signal,
		       "note", note,
		       NULL);
	}

	note->rt_offset = 0;

	pthread_mutex_unlock(recycling_mutex);
      }
      
      pthread_mutex_lock(recycling_mutex);
      
      recycling = recycling->next;

      pthread_mutex_unlock(recycling_mutex);
    }    
  }
}
