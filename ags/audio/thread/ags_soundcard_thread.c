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

#include <ags/audio/thread/ags_soundcard_thread.h>

#include <ags/libags.h>

#include <ags/audio/ags_devout.h>
#include <ags/audio/ags_devin.h>

#include <ags/audio/jack/ags_jack_devout.h>
#include <ags/audio/jack/ags_jack_devin.h>

#include <ags/audio/pulse/ags_pulse_devout.h>
#include <ags/audio/pulse/ags_pulse_devin.h>

#include <ags/audio/core-audio/ags_core_audio_devout.h>
#include <ags/audio/core-audio/ags_core_audio_devin.h>

#include <ags/audio/thread/ags_audio_loop.h>

#include <ags/i18n.h>

void ags_soundcard_thread_class_init(AgsSoundcardThreadClass *soundcard_thread);
void ags_soundcard_thread_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_soundcard_thread_init(AgsSoundcardThread *soundcard_thread);
void ags_soundcard_thread_set_property(GObject *gobject,
				       guint prop_id,
				       const GValue *value,
				       GParamSpec *param_spec);
void ags_soundcard_thread_get_property(GObject *gobject,
				       guint prop_id,
				       GValue *value,
				       GParamSpec *param_spec);
void ags_soundcard_thread_connect(AgsConnectable *connectable);
void ags_soundcard_thread_disconnect(AgsConnectable *connectable);
void ags_soundcard_thread_dispose(GObject *gobject);
void ags_soundcard_thread_finalize(GObject *gobject);

void ags_soundcard_thread_start(AgsThread *thread);
void ags_soundcard_thread_run(AgsThread *thread);
void ags_soundcard_thread_stop(AgsThread *thread);

void ags_soundcard_thread_dispatch_callback(AgsPollFd *poll_fd,
					    AgsSoundcardThread *soundcard_thread);
void ags_soundcard_thread_stopped_all_callback(AgsAudioLoop *audio_loop,
					       AgsSoundcardThread *soundcard_thread);

/**
 * SECTION:ags_soundcard_thread
 * @short_description: soundcard thread
 * @title: AgsSoundcardThread
 * @section_id:
 * @include: ags/audio/thread/ags_soundcard_thread.h
 *
 * The #AgsSoundcardThread acts as audio output thread to soundcard.
 */

static gpointer ags_soundcard_thread_parent_class = NULL;
static AgsConnectableInterface *ags_soundcard_thread_parent_connectable_interface;

enum{
  PROP_0,
  PROP_SOUNDCARD,
  PROP_SOUNDCARD_CAPABILITY,
};

GType
ags_soundcard_thread_get_type()
{
  static GType ags_type_soundcard_thread = 0;

  if(!ags_type_soundcard_thread){
    static const GTypeInfo ags_soundcard_thread_info = {
      sizeof (AgsSoundcardThreadClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_soundcard_thread_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsSoundcardThread),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_soundcard_thread_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_soundcard_thread_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_soundcard_thread = g_type_register_static(AGS_TYPE_THREAD,
						       "AgsSoundcardThread",
						       &ags_soundcard_thread_info,
						       0);
    
    g_type_add_interface_static(ags_type_soundcard_thread,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
  }
  
  return (ags_type_soundcard_thread);
}

void
ags_soundcard_thread_class_init(AgsSoundcardThreadClass *soundcard_thread)
{
  GObjectClass *gobject;
  AgsThreadClass *thread;
  GParamSpec *param_spec;
  
  ags_soundcard_thread_parent_class = g_type_class_peek_parent(soundcard_thread);

  /* GObject */
  gobject = (GObjectClass *) soundcard_thread;

  gobject->set_property = ags_soundcard_thread_set_property;
  gobject->get_property = ags_soundcard_thread_get_property;

  gobject->dispose = ags_soundcard_thread_dispose;
  gobject->finalize = ags_soundcard_thread_finalize;

  /**
   * AgsSoundcardThread:soundcard:
   *
   * The assigned #AgsSoundcard.
   * 
   * Since: 2.0.0
   */
  param_spec = g_param_spec_object("soundcard",
				   i18n_pspec("soundcard assigned to"),
				   i18n_pspec("The AgsSoundcard it is assigned to"),
				   G_TYPE_OBJECT,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_SOUNDCARD,
				  param_spec);

  /**
   * AgsSoundcardThread:soundcard-capability:
   *
   * The soundcard capability.
   * 
   * Since: 2.0.0
   */
  param_spec =  g_param_spec_uint("soundcard-capability",
				  i18n_pspec("soundcard capability"),
				  i18n_pspec("The soundcard capability"),
				  0,
				  G_MAXINT32,
				  0,
				  G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_SOUNDCARD_CAPABILITY,
				  param_spec);

  /* AgsThread */
  thread = (AgsThreadClass *) soundcard_thread;

  thread->start = ags_soundcard_thread_start;
  thread->run = ags_soundcard_thread_run;
  thread->stop = ags_soundcard_thread_stop;
}

void
ags_soundcard_thread_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_soundcard_thread_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_soundcard_thread_connect;
  connectable->disconnect = ags_soundcard_thread_disconnect;
}

void
ags_soundcard_thread_init(AgsSoundcardThread *soundcard_thread)
{
  AgsThread *thread;

  AgsConfig *config;
  
  guint samplerate;
  guint buffer_size;
  
  thread = (AgsThread *) soundcard_thread;

  g_atomic_int_or(&(thread->flags),
		  (AGS_THREAD_START_SYNCED_FREQ |
		   AGS_THREAD_INTERMEDIATE_POST_SYNC));  
  
  //  g_atomic_int_or(&(thread->flags),
  //		  AGS_THREAD_TIMING);

  config = ags_config_get_instance();
  
  samplerate = (guint) ags_soundcard_helper_config_get_samplerate(config);
  buffer_size = (guint) ags_soundcard_helper_config_get_buffer_size(config);

  thread->freq = ceil((gdouble) samplerate / (gdouble) buffer_size) + AGS_SOUNDCARD_DEFAULT_OVERCLOCK;

  /*  */
  soundcard_thread->soundcard_capability = 0;

  soundcard_thread->soundcard = NULL;
  soundcard_thread->error = NULL;
}

void
ags_soundcard_thread_set_property(GObject *gobject,
				  guint prop_id,
				  const GValue *value,
				  GParamSpec *param_spec)
{
  AgsSoundcardThread *soundcard_thread;

  soundcard_thread = AGS_SOUNDCARD_THREAD(gobject);

  switch(prop_id){
  case PROP_SOUNDCARD:
    {
      GObject *soundcard;

      guint samplerate;
      guint buffer_size;

      soundcard = (GObject *) g_value_get_object(value);

      if(soundcard_thread->soundcard != NULL){
	g_object_unref(G_OBJECT(soundcard_thread->soundcard));
      }

      if(soundcard != NULL){
	g_object_ref(G_OBJECT(soundcard));

	ags_soundcard_get_presets(AGS_SOUNDCARD(soundcard),
				  NULL,
				  &samplerate,
				  &buffer_size,
				  NULL);
	
	g_object_set(soundcard_thread,
		     "frequency", ceil((gdouble) samplerate / (gdouble) buffer_size) + AGS_SOUNDCARD_DEFAULT_OVERCLOCK,
		     NULL);

	/* playback */
	if(AGS_IS_DEVOUT(soundcard)){
	  g_atomic_int_or(&(AGS_THREAD(soundcard_thread)->flags),
			  (AGS_THREAD_INTERMEDIATE_POST_SYNC));
	}else if(AGS_IS_JACK_DEVOUT(soundcard) ||
		 AGS_IS_PULSE_DEVOUT(soundcard)){
	  g_atomic_int_or(&(AGS_THREAD(soundcard_thread)->flags),
			  (AGS_THREAD_INTERMEDIATE_POST_SYNC));
	}else if(AGS_IS_CORE_AUDIO_DEVOUT(soundcard)){
	  g_atomic_int_or(&(AGS_THREAD(soundcard_thread)->flags),
	  		  (AGS_THREAD_INTERMEDIATE_POST_SYNC));
	}

	/* capture */
	if(AGS_IS_DEVIN(soundcard)){
	  g_atomic_int_or(&(AGS_THREAD(soundcard_thread)->flags),
			  (AGS_THREAD_INTERMEDIATE_PRE_SYNC));
	}else if(AGS_IS_JACK_DEVIN(soundcard) ||
		 AGS_IS_PULSE_DEVIN(soundcard)){
	  g_atomic_int_or(&(AGS_THREAD(soundcard_thread)->flags),
			  (AGS_THREAD_INTERMEDIATE_PRE_SYNC));
	}else if(AGS_IS_CORE_AUDIO_DEVIN(soundcard)){
	  g_atomic_int_or(&(AGS_THREAD(soundcard_thread)->flags),
	  		  (AGS_THREAD_INTERMEDIATE_PRE_SYNC));
	}

	/* duplex */
	//TODO:JK: implement me
      }

      soundcard_thread->soundcard = G_OBJECT(soundcard);
    }
    break;
  case PROP_SOUNDCARD_CAPABILITY:
    {
      soundcard_thread->soundcard_capability = g_value_get_uint(value);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_soundcard_thread_get_property(GObject *gobject,
				  guint prop_id,
				  GValue *value,
				  GParamSpec *param_spec)
{
  AgsSoundcardThread *soundcard_thread;

  soundcard_thread = AGS_SOUNDCARD_THREAD(gobject);

  switch(prop_id){
  case PROP_SOUNDCARD:
    {
      g_value_set_object(value, G_OBJECT(soundcard_thread->soundcard));
    }
    break;
  case PROP_SOUNDCARD_CAPABILITY:
    {
      g_value_set_uint(value,
		       soundcard_thread->soundcard_capability);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_soundcard_thread_connect(AgsConnectable *connectable)
{
  AgsThread *audio_loop, *soundcard_thread;

  soundcard_thread = AGS_THREAD(connectable);

  if((AGS_THREAD_CONNECTED & (g_atomic_int_get(&(soundcard_thread->flags)))) != 0){
    return;
  }  

  ags_soundcard_thread_parent_connectable_interface->connect(connectable);

  audio_loop = ags_thread_get_toplevel(soundcard_thread);
  g_signal_connect((GObject *) audio_loop, "stopped-all",
		   G_CALLBACK(ags_soundcard_thread_stopped_all_callback), soundcard_thread);    
}

void
ags_soundcard_thread_disconnect(AgsConnectable *connectable)
{
  ags_soundcard_thread_parent_connectable_interface->disconnect(connectable);

  /* empty */
}

void
ags_soundcard_thread_dispose(GObject *gobject)
{
  AgsSoundcardThread *soundcard_thread;

  soundcard_thread = AGS_SOUNDCARD_THREAD(gobject);

  /* soundcard */
  if(soundcard_thread->soundcard != NULL){
    g_object_unref(soundcard_thread->soundcard);

    soundcard_thread->soundcard = NULL;
  }

  /* call parent */
  G_OBJECT_CLASS(ags_soundcard_thread_parent_class)->dispose(gobject);
}

void
ags_soundcard_thread_finalize(GObject *gobject)
{
  AgsSoundcardThread *soundcard_thread;

  soundcard_thread = AGS_SOUNDCARD_THREAD(gobject);

  /* soundcard */
  if(soundcard_thread->soundcard != NULL){
    g_object_unref(soundcard_thread->soundcard);
  }

  /* call parent */
  G_OBJECT_CLASS(ags_soundcard_thread_parent_class)->finalize(gobject);
}

void
ags_soundcard_thread_start(AgsThread *thread)
{
  AgsSoundcardThread *soundcard_thread;

  AgsThread *main_loop;
  AgsPollingThread *polling_thread;
  
  GObject *soundcard;

  GList *poll_fd;
  
  GError *error;

  soundcard_thread = AGS_SOUNDCARD_THREAD(thread);
  main_loop = ags_thread_get_toplevel(thread);
  
  soundcard = soundcard_thread->soundcard;

  /* disable timing */
  g_atomic_int_and(&(thread->flags),
		   (~AGS_THREAD_TIMING));
  
  /* find polling thread */
  polling_thread = (AgsPollingThread *) ags_thread_find_type(main_loop,
							     AGS_TYPE_POLLING_THREAD);
    
  /* add poll fd and connect dispatch */
  poll_fd = ags_soundcard_get_poll_fd(AGS_SOUNDCARD(soundcard));
    
  while(poll_fd != NULL){
    if(polling_thread != NULL){
      gint position;
      
      ags_polling_thread_add_poll_fd(polling_thread,
				     poll_fd->data);
      g_signal_connect(G_OBJECT(poll_fd->data), "dispatch",
		       G_CALLBACK(ags_soundcard_thread_dispatch_callback), soundcard_thread);

      position = ags_polling_thread_fd_position(polling_thread,
						AGS_POLL_FD(poll_fd->data)->fd);
      
      if(position != -1){
	polling_thread->fds[position].events = POLLOUT;
      }
    }
    
    poll_fd = poll_fd->next;
  }

  if((AGS_THREAD_SINGLE_LOOP & (g_atomic_int_get(&(thread->flags)))) == 0){
    AGS_THREAD_CLASS(ags_soundcard_thread_parent_class)->start(thread);
  }
}

void
ags_soundcard_thread_run(AgsThread *thread)
{
  AgsSoundcardThread *soundcard_thread;

  GObject *soundcard;

  GList *poll_fd;
  
  gboolean is_playing, is_recording;
  
  GError *error;

  soundcard_thread = AGS_SOUNDCARD_THREAD(thread);

  soundcard = soundcard_thread->soundcard;

  /* real-time setup */
#ifdef AGS_WITH_RT
  if((AGS_THREAD_RT_SETUP & (g_atomic_int_get(&(thread->flags)))) == 0){
    struct sched_param param;
    
    /* Declare ourself as a real time task */
    param.sched_priority = AGS_RT_PRIORITY;
      
    if(sched_setscheduler(0, SCHED_FIFO, &param) == -1) {
      perror("sched_setscheduler failed");
    }

    g_atomic_int_or(&(thread->flags),
		    AGS_THREAD_RT_SETUP);
  }
#endif

  /* playback */
  if((AGS_SOUNDCARD_CAPABILITY_PLAYBACK & (soundcard_thread->soundcard_capability)) != 0){
    is_playing = ags_soundcard_is_playing(AGS_SOUNDCARD(soundcard));
  
    if(is_playing){
      error = NULL;
      ags_soundcard_play(AGS_SOUNDCARD(soundcard),
			 &error);

      if(error != NULL){
	//TODO:JK: implement me

	g_warning("%s",
		  error->message);
      }
    }
  }

  /* capture */
  if((AGS_SOUNDCARD_CAPABILITY_CAPTURE & (soundcard_thread->soundcard_capability)) != 0){
    is_recording = ags_soundcard_is_recording(AGS_SOUNDCARD(soundcard));
  
    if(is_recording){
      error = NULL;
      ags_soundcard_record(AGS_SOUNDCARD(soundcard),
			   &error);

      if(error != NULL){
	//TODO:JK: implement me

	g_warning("%s",
		  error->message);
      }
    }
  }

  /* duplex */
  if((AGS_SOUNDCARD_CAPABILITY_DUPLEX & (soundcard_thread->soundcard_capability)) != 0){
    //TODO:JK: implement me
  }
}

void
ags_soundcard_thread_stop(AgsThread *thread)
{
  AgsSoundcardThread *soundcard_thread;

  AgsThread *main_loop;
  AgsPollingThread *polling_thread;
  
  GObject *soundcard;

  GList *poll_fd;
    
  soundcard_thread = AGS_SOUNDCARD_THREAD(thread);
  main_loop = ags_thread_get_toplevel(thread);

  soundcard = soundcard_thread->soundcard;

  /* stop thread and soundcard */
  AGS_THREAD_CLASS(ags_soundcard_thread_parent_class)->stop(thread);

  //FIXME:JK: is this safe?
  ags_soundcard_stop(AGS_SOUNDCARD(soundcard));

  g_atomic_int_or(&(thread->flags),
		  AGS_THREAD_TIMING);

  /* find polling thread */
  polling_thread = (AgsPollingThread *) ags_thread_find_type(main_loop,
							     AGS_TYPE_POLLING_THREAD);
    
  /* remove poll fd */
  poll_fd = ags_soundcard_get_poll_fd(AGS_SOUNDCARD(soundcard));
    
  while(poll_fd != NULL){
    if(polling_thread != NULL){
      gint position;
      
      g_object_disconnect(G_OBJECT(poll_fd->data),
			  "dispatch",
			  G_CALLBACK(ags_soundcard_thread_dispatch_callback),
			  soundcard_thread,
			  NULL);
      
      ags_polling_thread_remove_poll_fd(polling_thread,
					poll_fd->data);
    }
    
    poll_fd = poll_fd->next;
  }
}

void
ags_soundcard_thread_dispatch_callback(AgsPollFd *poll_fd,
				       AgsSoundcardThread *soundcard_thread)
{
  AgsAudioLoop *audio_loop;

  AgsPollingThread *polling_thread;
  
  guint time_spent;

  audio_loop = (AgsAudioLoop *) ags_thread_get_toplevel((AgsThread *) soundcard_thread);

  if(ags_soundcard_is_available(AGS_SOUNDCARD(soundcard_thread->soundcard))){
    pthread_mutex_lock(audio_loop->timing_mutex);

    g_atomic_int_set(&(audio_loop->time_spent),
		     audio_loop->time_cycle);

    pthread_mutex_unlock(audio_loop->timing_mutex);

    //    ags_main_loop_interrupt(AGS_MAIN_LOOP(audio_loop),
    //			    AGS_THREAD_SUSPEND_SIG,
    //			    0, &time_spent);

    polling_thread = ags_thread_find_type(audio_loop,
					  AGS_TYPE_POLLING_THREAD);

    if(polling_thread != NULL){
      g_atomic_int_or(&(polling_thread->flags),
		      AGS_POLLING_THREAD_OMIT);
      g_atomic_int_inc(&(polling_thread->omit_count));
    }
  }
}

void
ags_soundcard_thread_stopped_all_callback(AgsAudioLoop *audio_loop,
				   AgsSoundcardThread *soundcard_thread)
{
  AgsSoundcard *soundcard;
  
  soundcard = AGS_SOUNDCARD(soundcard_thread->soundcard);

  if(ags_soundcard_is_playing(soundcard)){
    ags_thread_stop((AgsThread *) soundcard_thread);
  }
}

/**
 * ags_soundcard_thread_find_soundcard:
 * @soundcard_thread: the #AgsSoundcardThread
 * @soundcard: the #AgsSoundcard to find
 * 
 * Returns: the matching #AgsSoundcardThread, if not
 * found %NULL.
 * 
 * Since: 2.0.0
 */
AgsSoundcardThread*
ags_soundcard_thread_find_soundcard(AgsSoundcardThread *soundcard_thread,
				    GObject *soundcard)
{
  if(soundcard_thread == NULL ||
     !AGS_IS_SOUNDCARD_THREAD(soundcard_thread)){
    return(NULL);
  }
  
  while(soundcard_thread != NULL){
    if(AGS_IS_SOUNDCARD_THREAD(soundcard_thread) &&
       soundcard_thread->soundcard == soundcard){
      return(soundcard_thread);
    }
    
    soundcard_thread = g_atomic_pointer_get(&(((AgsThread *) soundcard_thread)->next));
  }
  
  return(NULL);
}

/**
 * ags_soundcard_thread_new:
 * @soundcard: the #AgsSoundcard
 * @soundcard_capability: see #AgsSoundcardCapability-enum
 *
 * Create a new instance of #AgsSoundcardThread.
 *
 * Returns: the new #AgsSoundcardThread
 *
 * Since: 2.0.0
 */
AgsSoundcardThread*
ags_soundcard_thread_new(GObject *soundcard,
			 guint soundcard_capability)
{
  AgsSoundcardThread *soundcard_thread;

  soundcard_thread = (AgsSoundcardThread *) g_object_new(AGS_TYPE_SOUNDCARD_THREAD,
							 "soundcard", soundcard,
							 "soundcard-capability", soundcard_capability,
							 NULL);


  return(soundcard_thread);
}
