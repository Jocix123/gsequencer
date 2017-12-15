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

#include <ags/X/machine/ags_cell_pattern_callbacks.h>

#include <ags/libags.h>
#include <ags/libags-audio.h>

#include <ags/X/ags_window.h>
#include <ags/X/ags_machine.h>

#include <ags/X/thread/ags_gui_thread.h>

#include <gdk/gdkkeysyms.h>

#include <math.h>

void ags_cell_pattern_init_channel_launch_callback(AgsTask *task, gpointer data);

gboolean
ags_cell_pattern_focus_in_callback(GtkWidget *widget, GdkEvent *event, AgsCellPattern *cell_pattern)
{
  pthread_t thread;
  
  pthread_create(&thread, NULL,
		 ags_cell_pattern_blink_worker, cell_pattern);
  
  return(TRUE);
}

gboolean
ags_cell_pattern_drawing_area_configure_callback(GtkWidget *widget, GdkEventConfigure *event, AgsCellPattern *cell_pattern)
{
  ags_cell_pattern_paint(cell_pattern);
  
  return(FALSE);
}

gboolean
ags_cell_pattern_drawing_area_expose_callback(GtkWidget *widget, GdkEventExpose *event, AgsCellPattern *cell_pattern)
{
  ags_cell_pattern_paint(cell_pattern);

  return(FALSE);
}

gboolean
ags_cell_pattern_drawing_area_button_press_callback(GtkWidget *widget, GdkEventButton *event, AgsCellPattern *cell_pattern)
{
  if(event->button == 1){
    AgsMachine *machine;

    AgsAudio *audio;
    AgsChannel *input;
    AgsChannel *channel;

    AgsMutexManager *mutex_manager;

    guint input_lines;
    guint i, j;
    guint index1;
    
    pthread_mutex_t *application_mutex;
    pthread_mutex_t *audio_mutex;
    pthread_mutex_t *channel_mutex;

    mutex_manager = ags_mutex_manager_get_instance();
    application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);

    machine = (AgsMachine *) gtk_widget_get_ancestor((GtkWidget *) cell_pattern,
						     AGS_TYPE_MACHINE);

    audio = machine->audio;
    
    /* get audio mutex */
    pthread_mutex_lock(application_mutex);

    audio_mutex = ags_mutex_manager_lookup(mutex_manager,
					   (GObject *) audio);
  
    pthread_mutex_unlock(application_mutex);

    /* get some fields */
    pthread_mutex_lock(audio_mutex);

    input = audio->input;

    input_lines = audio->input_lines;
    
    pthread_mutex_unlock(audio_mutex);
    
    /* get pattern position */        
    i = (guint) floor((double) event->y / (double) cell_pattern->cell_height);
    j = (guint) floor((double) event->x / (double) cell_pattern->cell_width);

    index1 = machine->bank_1;

    channel = ags_channel_nth(input,
			      input_lines - ((guint) GTK_RANGE(cell_pattern->vscrollbar)->adjustment->value + i) - 1);

    /* get channel mutex */
    pthread_mutex_lock(application_mutex);

    channel_mutex = ags_mutex_manager_lookup(mutex_manager,
					     (GObject *) channel);
  
    pthread_mutex_unlock(application_mutex);

    /* toggle pattern */
    pthread_mutex_lock(channel_mutex);
    
    ags_pattern_toggle_bit(channel->pattern->data,
			   0, index1,
			   j);

    pthread_mutex_unlock(channel_mutex);

    /* queue draw */
    gtk_widget_queue_draw(cell_pattern->drawing_area);
  }

  return(FALSE);
}

gboolean
ags_cell_pattern_drawing_area_key_press_event(GtkWidget *widget, GdkEventKey *event, AgsCellPattern *cell_pattern)
{
  if(event->keyval == GDK_KEY_Tab){
    return(FALSE);
  }

  switch(event->keyval){
  case GDK_KEY_Control_L:
    {
      cell_pattern->key_mask |= AGS_CELL_PATTERN_KEY_L_CONTROL;
    }
    break;
  case GDK_KEY_Control_R:
    {
      cell_pattern->key_mask |= AGS_CELL_PATTERN_KEY_R_CONTROL;
    }
    break;
  case GDK_KEY_c:
    {
      /* copy notes */
      if((AGS_CELL_PATTERN_KEY_L_CONTROL & (cell_pattern->key_mask)) != 0 || (AGS_CELL_PATTERN_KEY_R_CONTROL & (cell_pattern->key_mask)) != 0){
	AgsMachine *machine;

	machine = (AgsMachine *) gtk_widget_get_ancestor((GtkWidget *) cell_pattern,
							 AGS_TYPE_MACHINE);
	
	ags_machine_copy_pattern(machine);
      }
    }
    break;
  }
  
  return(TRUE);
}

gboolean
ags_cell_pattern_drawing_area_key_release_event(GtkWidget *widget, GdkEventKey *event, AgsCellPattern *cell_pattern)
{
  AgsWindow *window;
  AgsMachine *machine;
      
  AgsGuiThread *gui_thread;

  AgsAudio *audio;
  AgsChannel *input;
  AgsChannel *channel;

  AgsMutexManager *mutex_manager;
  AgsThread *main_loop;

  guint input_lines;
  
  pthread_mutex_t *application_mutex;
  pthread_mutex_t *audio_mutex;
  pthread_mutex_t *channel_mutex;

  auto void ags_cell_pattern_drawing_area_key_release_event_play_channel(AgsChannel *channel);

  void ags_cell_pattern_drawing_area_key_release_event_play_channel(AgsChannel *channel){
    GObject *soundcard;
    AgsAudio *audio;
    
    AgsStartSoundcard *start_soundcard;
    AgsInitChannel *init_channel;
    AgsAppendChannel *append_channel;

    AgsMutexManager *mutex_manager;
    AgsThread *main_loop;
    AgsGuiThread *gui_thread;
    AgsSoundcardThread *soundcard_thread;

    AgsApplicationContext *application_context;
    
    GList *tasks;

    gboolean no_soundcard;

    pthread_mutex_t *application_mutex;
    pthread_mutex_t *audio_mutex;
    pthread_mutex_t *channel_mutex;

    application_context = (AgsApplicationContext *) window->application_context;
    
    mutex_manager = ags_mutex_manager_get_instance();
    application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);

    no_soundcard = FALSE;

    pthread_mutex_lock(application_mutex);

    if(ags_sound_provider_get_soundcard(AGS_SOUND_PROVIDER(application_context)) == NULL){
      no_soundcard = TRUE;
    }

    pthread_mutex_unlock(application_mutex);

    if(no_soundcard){
      g_message("No soundcard available");
      
      return;
    }

    /* lookup channel mutex */
    pthread_mutex_lock(application_mutex);

    channel_mutex = ags_mutex_manager_lookup(mutex_manager,
					     (GObject *) channel);

    pthread_mutex_unlock(application_mutex);

    pthread_mutex_lock(channel_mutex);
  
    soundcard = channel->soundcard;
    audio = (AgsAudio *) channel->audio;

    pthread_mutex_unlock(channel_mutex);
  
    /* lookup audio mutex */
    pthread_mutex_lock(application_mutex);

    audio_mutex = ags_mutex_manager_lookup(mutex_manager,
					   (GObject *) audio);
    
    pthread_mutex_unlock(application_mutex);

    pthread_mutex_lock(audio_mutex);
  
    main_loop = (AgsThread *) application_context->main_loop;
    
    gui_thread = (AgsGuiThread *) ags_thread_find_type(main_loop,
							 AGS_TYPE_GUI_THREAD);
    soundcard_thread = (AgsSoundcardThread *) ags_thread_find_type(main_loop,
								   AGS_TYPE_SOUNDCARD_THREAD);

    tasks = NULL;

    /* init channel for playback */
    init_channel = ags_init_channel_new(channel, FALSE,
					TRUE, FALSE, FALSE);
    g_signal_connect_after(G_OBJECT(init_channel), "launch",
			   G_CALLBACK(ags_cell_pattern_init_channel_launch_callback), NULL);
    tasks = g_list_prepend(tasks, init_channel);
    
    /* append channel for playback */
    append_channel = ags_append_channel_new(G_OBJECT(main_loop),
					    G_OBJECT(channel));
    tasks = g_list_prepend(tasks, append_channel);

    /* create start task */
    start_soundcard = ags_start_soundcard_new(application_context);
    tasks = g_list_prepend(tasks,
			   start_soundcard);

    /* perform playback */
    tasks = g_list_reverse(tasks);
    ags_gui_thread_schedule_task_list(gui_thread,
				      tasks);

    pthread_mutex_unlock(audio_mutex);
  }
  
  if(event->keyval == GDK_KEY_Tab){
    return(FALSE);
  }

  mutex_manager = ags_mutex_manager_get_instance();
  application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);

  machine = (AgsMachine *) gtk_widget_get_ancestor((GtkWidget *) cell_pattern,
						   AGS_TYPE_MACHINE);

  window = (AgsWindow *) gtk_widget_get_ancestor((GtkWidget *) cell_pattern,
						 AGS_TYPE_WINDOW);

  audio = machine->audio;

  /* get some fields */
  pthread_mutex_lock(application_mutex);
  
  main_loop = (AgsThread *) AGS_APPLICATION_CONTEXT(window->application_context)->main_loop;

  pthread_mutex_unlock(application_mutex);

  gui_thread = (AgsGuiThread *) ags_thread_find_type(main_loop,
						     AGS_TYPE_GUI_THREAD);

  /* get audio mutex */
  pthread_mutex_lock(application_mutex);

  audio_mutex = ags_mutex_manager_lookup(mutex_manager,
					 (GObject *) audio);
  
  pthread_mutex_unlock(application_mutex);

  /* get some fields */
  pthread_mutex_lock(audio_mutex);

  input = audio->input;

  input_lines = audio->input_lines;
    
  pthread_mutex_unlock(audio_mutex);

  switch(event->keyval){
  case GDK_KEY_Control_L:
    {
      cell_pattern->key_mask &= (~AGS_CELL_PATTERN_KEY_L_CONTROL);
    }
    break;
  case GDK_KEY_Control_R:
    {
      cell_pattern->key_mask &= (~AGS_CELL_PATTERN_KEY_R_CONTROL);
    }
    break;
  case GDK_KEY_Left:
  case GDK_KEY_leftarrow:
    {
      if(cell_pattern->cursor_x > 0){
	gboolean bit_is_on;
	
	cell_pattern->cursor_x -= 1;

	/* audible feedback */
	channel = ags_channel_nth(input,
				  input_lines - cell_pattern->cursor_y - 1);
	  
	/* get channel mutex */
	pthread_mutex_lock(application_mutex);

	channel_mutex = ags_mutex_manager_lookup(mutex_manager,
						 (GObject *) channel);
  
	pthread_mutex_unlock(application_mutex);

	/* check bit */
	pthread_mutex_lock(channel_mutex);

	bit_is_on = (ags_pattern_get_bit(channel->pattern->data,
					 0, machine->bank_1, cell_pattern->cursor_x)) ? TRUE: FALSE;
	
	pthread_mutex_unlock(channel_mutex);
	
	if(bit_is_on){
	  ags_cell_pattern_drawing_area_key_release_event_play_channel(channel);
	}
      }
    }
    break;
  case GDK_KEY_Right:
  case GDK_KEY_rightarrow:
    {
      if(cell_pattern->cursor_x < cell_pattern->n_cols){
	gboolean bit_is_on;
	
	cell_pattern->cursor_x += 1;

	/* audible feedback */
	channel = ags_channel_nth(input,
				  input_lines - cell_pattern->cursor_y - 1);

	/* get channel mutex */
	pthread_mutex_lock(application_mutex);

	channel_mutex = ags_mutex_manager_lookup(mutex_manager,
						 (GObject *) channel);
  
	pthread_mutex_unlock(application_mutex);

	/* check bit */
	if(ags_pattern_get_bit(channel->pattern->data,
			       0, machine->bank_1, cell_pattern->cursor_x)){
	  ags_cell_pattern_drawing_area_key_release_event_play_channel(channel);
	}
      }
    }
    break;
  case GDK_KEY_Up:
  case GDK_KEY_uparrow:
    {
      if(cell_pattern->cursor_y > 0){
	gboolean bit_is_on;
	
	cell_pattern->cursor_y -= 1;

	/* audible feedback */
	channel = ags_channel_nth(input,
				  input_lines - cell_pattern->cursor_y - 1);

	/* get channel mutex */
	pthread_mutex_lock(application_mutex);

	channel_mutex = ags_mutex_manager_lookup(mutex_manager,
						 (GObject *) channel);
  
	pthread_mutex_unlock(application_mutex);

	/* check bit */
	pthread_mutex_lock(channel_mutex);

	bit_is_on = (ags_pattern_get_bit(channel->pattern->data,
					 0, machine->bank_1, cell_pattern->cursor_x)) ? TRUE: FALSE;
	
	pthread_mutex_unlock(channel_mutex);
	
	if(bit_is_on){
	  ags_cell_pattern_drawing_area_key_release_event_play_channel(channel);
	}
      }

      if(cell_pattern->cursor_y < GTK_RANGE(cell_pattern->vscrollbar)->adjustment->value){
	gtk_range_set_value(GTK_RANGE(cell_pattern->vscrollbar),
			    GTK_RANGE(cell_pattern->vscrollbar)->adjustment->value - 1.0);
      }
    }
    break;
  case GDK_KEY_Down:
  case GDK_KEY_downarrow:
    {
      if(cell_pattern->cursor_y < cell_pattern->n_rows){
	gboolean bit_is_on;
	
	cell_pattern->cursor_y += 1;

	/* audible feedback */
	channel = ags_channel_nth(input,
				  input_lines - cell_pattern->cursor_y - 1);

	/* get channel mutex */
	pthread_mutex_lock(application_mutex);

	channel_mutex = ags_mutex_manager_lookup(mutex_manager,
						 (GObject *) channel);
  
	pthread_mutex_unlock(application_mutex);

	/* check bit */
	pthread_mutex_lock(channel_mutex);

	bit_is_on = (ags_pattern_get_bit(channel->pattern->data,
					 0, machine->bank_1, cell_pattern->cursor_x)) ? TRUE: FALSE;
	
	pthread_mutex_unlock(channel_mutex);
	
	if(bit_is_on){
	  ags_cell_pattern_drawing_area_key_release_event_play_channel(channel);
	}
      }

      if(cell_pattern->cursor_y >= GTK_RANGE(cell_pattern->vscrollbar)->adjustment->value + AGS_CELL_PATTERN_MAX_CONTROLS_SHOWN_VERTICALLY){
	gtk_range_set_value(GTK_RANGE(cell_pattern->vscrollbar),
			    GTK_RANGE(cell_pattern->vscrollbar)->adjustment->value + 1.0);
      }
    }
    break;
  case GDK_KEY_space:
    {
      guint i, j;
      guint index1;
      
      i = cell_pattern->cursor_y;
      j = cell_pattern->cursor_x;
      
      index1 = machine->bank_1;

      channel = ags_channel_nth(input,
				input_lines - i - 1);
      
      /* toggle pattern */
      pthread_mutex_lock(channel_mutex);
    
      ags_pattern_toggle_bit(channel->pattern->data,
			     0, index1,
			     j);

      pthread_mutex_unlock(channel_mutex);

      /* play pattern */
      if(!ags_pattern_get_bit(channel->pattern->data,
			      0, index1, j)){
	ags_cell_pattern_drawing_area_key_release_event_play_channel(channel);
      }

      /* queue draw */
      gtk_widget_queue_draw(cell_pattern->drawing_area);
    }
    break;
  }

  return(TRUE);
}

void
ags_cell_pattern_adjustment_value_changed_callback(GtkWidget *widget, AgsCellPattern *cell_pattern)
{
  ags_cell_pattern_paint(cell_pattern);
}

//TODO:JK: remove
#if 0
void
ags_cell_pattern_refresh_gui_callback(AgsTogglePatternBit *toggle_pattern_bit,
				      AgsCellPattern *cell_pattern)
{
  AgsMachine *machine;
  
  AgsChannel *channel;

  guint line;
  
  machine = (AgsMachine *) gtk_widget_get_ancestor((GtkWidget *) cell_pattern,
						   AGS_TYPE_MACHINE);
    
  channel = ags_channel_nth(machine->audio->input,
			    toggle_pattern_bit->line);
  line = machine->audio->input_pads - toggle_pattern_bit->line - (guint) GTK_RANGE(cell_pattern->vscrollbar)->adjustment->value - 1;

  ags_cell_pattern_redraw_gutter_point(cell_pattern,
				       channel,
				       toggle_pattern_bit->bit,
				       line);
  gtk_widget_queue_draw(cell_pattern);
}
#endif

void
ags_cell_pattern_init_channel_launch_callback(AgsTask *task, gpointer data)
{
  GObject *soundcard;
  AgsChannel *channel;
  AgsRecycling *recycling;

  AgsAddAudioSignal *add_audio_signal;

  AgsMutexManager *mutex_manager;
  AgsThread *main_loop;
  AgsGuiThread *gui_thread;

  AgsApplicationContext *application_context;
  
  GList *recall, *tmp;

  pthread_mutex_t *application_mutex;
  pthread_mutex_t *audio_mutex;

  channel = AGS_INIT_CHANNEL(task)->channel;
  soundcard = channel->soundcard;

  application_context = (AgsApplicationContext *) ags_soundcard_get_application_context(AGS_SOUNDCARD(soundcard));
  
  mutex_manager = ags_mutex_manager_get_instance();
  application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);

  /* get main loop and audio mutex */
  pthread_mutex_lock(application_mutex);

  main_loop = (AgsThread *) application_context->main_loop;

  audio_mutex = ags_mutex_manager_lookup(mutex_manager,
					 (GObject *) channel->audio);
  
  pthread_mutex_unlock(application_mutex);

  /* get gui trhead */
  gui_thread = (AgsGuiThread *) ags_thread_find_type(main_loop,
						     AGS_TYPE_GUI_THREAD);

#ifdef AGS_DEBUG
  g_message("launch");
#endif
  
  pthread_mutex_lock(audio_mutex);
  
  if(AGS_PLAYBACK(channel->playback) == NULL ||
     AGS_PLAYBACK(channel->playback)->recall_id[0] == NULL){    
    pthread_mutex_unlock(audio_mutex);
  
    return;
  }

  /* connect done */
  recall = ags_recall_find_provider_with_recycling_context(channel->play,
							   G_OBJECT(channel),
							   G_OBJECT(AGS_PLAYBACK(channel->playback)->recall_id[0]->recycling_context));
  
  tmp = recall;
  recall = ags_recall_find_type(recall,
				AGS_TYPE_PLAY_CHANNEL_RUN);
  //TODO:JK: fix me
  //    g_list_free(tmp);

  if(recall != NULL){
    AgsAudioSignal *audio_signal;
      
    /* add audio signal */
    recycling = channel->first_recycling;

    while(recycling != channel->last_recycling->next){
      audio_signal = ags_audio_signal_new((GObject *) soundcard,
					  (GObject *) recycling,
					  (GObject *) AGS_RECALL(recall->data)->recall_id);
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

      recycling = recycling->next;
    }    
  }

  pthread_mutex_unlock(audio_mutex);
}
