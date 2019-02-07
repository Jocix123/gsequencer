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

#include <ags/audio/ags_playback_domain.h>

#include <ags/libags.h>

#include <ags/audio/ags_audio.h>
#include <ags/audio/ags_channel.h>
#include <ags/audio/ags_output.h>
#include <ags/audio/ags_input.h>
#include <ags/audio/ags_playback.h>

#include <ags/audio/thread/ags_audio_thread.h>

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <ags/i18n.h>

void ags_playback_domain_class_init(AgsPlaybackDomainClass *playback_domain);
void ags_playback_domain_init(AgsPlaybackDomain *playback_domain);
void ags_playback_domain_set_property(GObject *gobject,
				      guint prop_id,
				      const GValue *value,
				      GParamSpec *param_spec);
void ags_playback_domain_get_property(GObject *gobject,
				      guint prop_id,
				      GValue *value,
				      GParamSpec *param_spec);
void ags_playback_domain_dispose(GObject *gobject);
void ags_playback_domain_finalize(GObject *gobject);

/**
 * SECTION:ags_playback_domain
 * @short_description: Outputting to soundcard domain
 * @title: AgsPlaybackDomain
 * @section_id:
 * @include: ags/audio/ags_playback_domain.h
 *
 * #AgsPlaybackDomain represents a domain to output.
 */

static gpointer ags_playback_domain_parent_class = NULL;

static pthread_mutex_t ags_playback_domain_class_mutex = PTHREAD_MUTEX_INITIALIZER;

enum{
  PROP_0,
  PROP_AUDIO,
  PROP_OUTPUT_PLAYBACK,
  PROP_INPUT_PLAYBACK,
};

GType
ags_playback_domain_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;

  if(g_once_init_enter (&g_define_type_id__volatile)){
    GType ags_type_playback_domain = 0;

    static const GTypeInfo ags_playback_domain_info = {
      sizeof(AgsPlaybackDomainClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_playback_domain_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof(AgsPlaybackDomain),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_playback_domain_init,
    };

    ags_type_playback_domain = g_type_register_static(G_TYPE_OBJECT,
						      "AgsPlaybackDomain",
						      &ags_playback_domain_info,
						      0);

    g_once_init_leave(&g_define_type_id__volatile, ags_type_playback_domain);
  }

  return g_define_type_id__volatile;
}

void
ags_playback_domain_class_init(AgsPlaybackDomainClass *playback_domain)
{
  GObjectClass *gobject;
  GParamSpec *param_spec;

  ags_playback_domain_parent_class = g_type_class_peek_parent(playback_domain);

  /* GObjectClass */
  gobject = (GObjectClass *) playback_domain;

  gobject->set_property = ags_playback_domain_set_property;
  gobject->get_property = ags_playback_domain_get_property;

  gobject->dispose = ags_playback_domain_dispose;
  gobject->finalize = ags_playback_domain_finalize;

  /* properties */
  /**
   * AgsPlaybackDomain:audio:
   *
   * The assigned #AgsAudio.
   * 
   * Since: 2.0.0
   */
  param_spec = g_param_spec_object("audio",
				   i18n_pspec("assigned audio"),
				   i18n_pspec("The audio it is assigned with"),
				   G_TYPE_OBJECT,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_AUDIO,
				  param_spec);

  /**
   * AgsPlaybackDomain:output-playback:
   *
   * The assigned output playback.
   * 
   * Since: 2.0.0
   */
  param_spec = g_param_spec_pointer("output-playback",
				    i18n_pspec("assigned output playback"),
				    i18n_pspec("The output playback it is assigned with"),
				    G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_OUTPUT_PLAYBACK,
				  param_spec);

  /**
   * AgsPlaybackDomain:input-playback:
   *
   * The assigned input playback.
   * 
   * Since: 2.0.0
   */
  param_spec = g_param_spec_pointer("input-playback",
				    i18n_pspec("assigned input playback"),
				    i18n_pspec("The input playback it is assigned with"),
				    G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_INPUT_PLAYBACK,
				  param_spec);
}

void
ags_playback_domain_init(AgsPlaybackDomain *playback_domain)
{
  AgsConfig *config;

  gchar *thread_model, *super_threaded_scope;

  gboolean super_threaded_audio;
  guint i;

  pthread_mutex_t *config_mutex;
  pthread_mutex_t *mutex;
  pthread_mutexattr_t *attr;

  playback_domain->flags = 0;
  
  /* add playback domain mutex */
  playback_domain->obj_mutexattr = 
    attr = (pthread_mutexattr_t *) malloc(sizeof(pthread_mutexattr_t));
  pthread_mutexattr_init(attr);
  pthread_mutexattr_settype(attr,
			    PTHREAD_MUTEX_RECURSIVE);

#ifdef __linux__
  pthread_mutexattr_setprotocol(attr,
				PTHREAD_PRIO_INHERIT);
#endif

  playback_domain->obj_mutex = 
    mutex = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(mutex,
		     attr);

  /* config */
  config = ags_config_get_instance();

  pthread_mutex_lock(ags_config_get_class_mutex());
  
  config_mutex = config->obj_mutex;

  pthread_mutex_unlock(ags_config_get_class_mutex());

  /* thread model */
  super_threaded_audio = FALSE;
  
  pthread_mutex_lock(config_mutex);

  thread_model = ags_config_get_value(config,
				      AGS_CONFIG_THREAD,
				      "model");

  if(thread_model != NULL &&
     !g_ascii_strncasecmp(thread_model,
			  "super-threaded",
			  15)){
    super_threaded_scope = ags_config_get_value(config,
						AGS_CONFIG_THREAD,
						"super-threaded-scope");
    if(super_threaded_scope != NULL &&
       (!g_ascii_strncasecmp(super_threaded_scope,
			     "audio",
			     6) ||
	!g_ascii_strncasecmp(super_threaded_scope,
			     "channel",
			     8))){
      super_threaded_audio = TRUE;
    }
    
    g_free(super_threaded_scope);
  }

  g_free(thread_model);

  pthread_mutex_unlock(config_mutex);
    
  /* default flags */
  if(super_threaded_audio){
    playback_domain->flags |= AGS_PLAYBACK_DOMAIN_SUPER_THREADED_AUDIO;
  }

  /* domain */
  playback_domain->audio = NULL;

  /* super threaded audio */
  playback_domain->audio_thread = (AgsThread **) malloc(AGS_SOUND_SCOPE_LAST * sizeof(AgsThread *));

  for(i = 0; i < AGS_SOUND_SCOPE_LAST; i++){
    playback_domain->audio_thread[i] = NULL;
  }

  /* playback */
  playback_domain->output_playback = NULL;
  playback_domain->input_playback = NULL;
}

void
ags_playback_domain_set_property(GObject *gobject,
				 guint prop_id,
				 const GValue *value,
				 GParamSpec *param_spec)
{
  AgsPlaybackDomain *playback_domain;
  
  pthread_mutex_t *playback_domain_mutex;

  playback_domain = AGS_PLAYBACK_DOMAIN(gobject);

  /* get playback_domain mutex */
  pthread_mutex_lock(ags_playback_domain_get_class_mutex());
  
  playback_domain_mutex = playback_domain->obj_mutex;
  
  pthread_mutex_unlock(ags_playback_domain_get_class_mutex());

  switch(prop_id){
  case PROP_AUDIO:
    {
      GObject *audio;

      audio = (GObject *) g_value_get_object(value);

      pthread_mutex_lock(playback_domain_mutex);

      if((GObject *) playback_domain->audio == audio){
	pthread_mutex_unlock(playback_domain_mutex);
	
	return;
      }

      if(playback_domain->audio != NULL){
	g_object_unref(G_OBJECT(playback_domain->audio));
      }

      if(audio != NULL){
	g_object_ref(G_OBJECT(audio));
      }

      playback_domain->audio = (GObject *) audio;
      
      pthread_mutex_unlock(playback_domain_mutex);
    }
    break;
  case PROP_OUTPUT_PLAYBACK:
    {
      AgsPlayback *output_playback;

      output_playback = (AgsPlayback *) g_value_get_pointer(value);

      pthread_mutex_lock(playback_domain_mutex);

      if(output_playback == NULL ||
	 g_list_find(playback_domain->output_playback,
		     output_playback) != NULL){
	pthread_mutex_unlock(playback_domain_mutex);
	
	return;
      }
      
      pthread_mutex_unlock(playback_domain_mutex);

      ags_playback_domain_add_playback(playback_domain,
				       (GObject *) output_playback, AGS_TYPE_OUTPUT);
    }
    break;
  case PROP_INPUT_PLAYBACK:
    {
      AgsPlayback *input_playback;

      input_playback = (AgsPlayback *) g_value_get_pointer(value);

      pthread_mutex_lock(playback_domain_mutex);

      if(input_playback == NULL ||
	 g_list_find(playback_domain->input_playback,
		     input_playback) != NULL){
	pthread_mutex_unlock(playback_domain_mutex);
	
	return;
      }
      
      pthread_mutex_unlock(playback_domain_mutex);

      ags_playback_domain_add_playback(playback_domain,
				       (GObject *) input_playback, AGS_TYPE_INPUT);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_playback_domain_get_property(GObject *gobject,
				 guint prop_id,
				 GValue *value,
				 GParamSpec *param_spec)
{
  AgsPlaybackDomain *playback_domain;

  pthread_mutex_t *playback_domain_mutex;

  playback_domain = AGS_PLAYBACK_DOMAIN(gobject);

  /* get playback_domain mutex */
  pthread_mutex_lock(ags_playback_domain_get_class_mutex());
  
  playback_domain_mutex = playback_domain->obj_mutex;
  
  pthread_mutex_unlock(ags_playback_domain_get_class_mutex());

  switch(prop_id){
  case PROP_AUDIO:
    {
      pthread_mutex_lock(playback_domain_mutex);

      g_value_set_object(value,
			 playback_domain->audio);
      
      pthread_mutex_unlock(playback_domain_mutex);
    }
    break;
  case PROP_OUTPUT_PLAYBACK:
    {
      pthread_mutex_lock(playback_domain_mutex);

      g_value_set_pointer(value,
			  g_list_copy_deep(playback_domain->output_playback,
					   (GCopyFunc) g_object_ref,
					   NULL));
      
      pthread_mutex_unlock(playback_domain_mutex);
    }
    break;
  case PROP_INPUT_PLAYBACK:
    {
      pthread_mutex_lock(playback_domain_mutex);

      g_value_set_pointer(value,
			  g_list_copy_deep(playback_domain->input_playback,
					   (GCopyFunc) g_object_ref,
					   NULL));
      
      pthread_mutex_unlock(playback_domain_mutex);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_playback_domain_dispose(GObject *gobject)
{
  AgsPlaybackDomain *playback_domain;

  guint i;
  
  playback_domain = AGS_PLAYBACK_DOMAIN(gobject);
  
  if(playback_domain->audio_thread != NULL){
    for(i = 0; i < AGS_SOUND_SCOPE_LAST; i++){
      if(playback_domain->audio_thread[i] != NULL){
	g_object_run_dispose((GObject *) playback_domain->audio_thread[i]);
	g_object_unref((GObject *) playback_domain->audio_thread[i]);

	playback_domain->audio_thread[i] = NULL;
      }
    }
  }

  /* domain */
  if(playback_domain->audio != NULL){
    g_object_unref(playback_domain->audio);

    playback_domain->audio = NULL;
  }

  /* output playback */
  if(playback_domain->output_playback != NULL){
    g_list_free_full(playback_domain->output_playback,
		     g_object_unref);
    
    playback_domain->output_playback = NULL;
  }

  /* input playback */
  if(playback_domain->input_playback != NULL){
    g_list_free_full(playback_domain->input_playback,
		     g_object_unref);
    
    playback_domain->input_playback = NULL;
  }
  
  /* call parent */
  G_OBJECT_CLASS(ags_playback_domain_parent_class)->dispose(gobject);
}

void
ags_playback_domain_finalize(GObject *gobject)
{
  AgsPlaybackDomain *playback_domain;

  guint i;
  
  playback_domain = AGS_PLAYBACK_DOMAIN(gobject);

  pthread_mutex_destroy(playback_domain->obj_mutex);
  free(playback_domain->obj_mutex);

  pthread_mutexattr_destroy(playback_domain->obj_mutexattr);
  free(playback_domain->obj_mutexattr);

  /* audio thread */
  if(playback_domain->audio_thread != NULL){
    for(i = 0; i < AGS_SOUND_SCOPE_LAST; i++){
      if(playback_domain->audio_thread[i] != NULL){
	g_object_run_dispose((GObject *) playback_domain->audio_thread[i]);
	g_object_unref((GObject *) playback_domain->audio_thread[i]);
      }
    }
    
    free(playback_domain->audio_thread);
  }

  /* domain */
  if(playback_domain->audio != NULL){
    g_object_unref(playback_domain->audio);
  }
  
  /* output playback */
  g_list_free_full(playback_domain->output_playback,
		   g_object_unref);

  /* input playback */
  g_list_free_full(playback_domain->input_playback,
		   g_object_unref);

  /* call parent */
  G_OBJECT_CLASS(ags_playback_domain_parent_class)->finalize(gobject);
}

/**
 * ags_playback_domain_get_class_mutex:
 * 
 * Use this function's returned mutex to access mutex fields.
 *
 * Returns: the class mutex
 * 
 * Since: 2.0.0
 */
pthread_mutex_t*
ags_playback_domain_get_class_mutex()
{
  return(&ags_playback_domain_class_mutex);
}

/**
 * ags_playback_domain_test_flags:
 * @playback_domain: the #AgsPlaybackDomain
 * @flags: the flags
 *
 * Test @flags to be set on @playback_domain.
 * 
 * Returns: %TRUE if flags are set, else %FALSE
 *
 * Since: 2.0.0
 */
gboolean
ags_playback_domain_test_flags(AgsPlaybackDomain *playback_domain, guint flags)
{
  gboolean retval;  
  
  pthread_mutex_t *playback_domain_mutex;

  if(!AGS_IS_PLAYBACK_DOMAIN(playback_domain)){
    return(FALSE);
  }

  /* get playback_domain mutex */
  pthread_mutex_lock(ags_playback_domain_get_class_mutex());
  
  playback_domain_mutex = playback_domain->obj_mutex;
  
  pthread_mutex_unlock(ags_playback_domain_get_class_mutex());

  /* test */
  pthread_mutex_lock(playback_domain_mutex);

  retval = (flags & (playback_domain->flags)) ? TRUE: FALSE;
  
  pthread_mutex_unlock(playback_domain_mutex);

  return(retval);
}

/**
 * ags_playback_domain_set_flags:
 * @playback_domain: the #AgsPlaybackDomain
 * @flags: the flags
 *
 * Set flags.
 * 
 * Since: 2.0.0
 */
void
ags_playback_domain_set_flags(AgsPlaybackDomain *playback_domain, guint flags)
{
  pthread_mutex_t *playback_domain_mutex;

  if(!AGS_IS_PLAYBACK_DOMAIN(playback_domain)){
    return;
  }

  /* get playback_domain mutex */
  pthread_mutex_lock(ags_playback_domain_get_class_mutex());
  
  playback_domain_mutex = playback_domain->obj_mutex;
  
  pthread_mutex_unlock(ags_playback_domain_get_class_mutex());

  /* set flags */
  pthread_mutex_lock(playback_domain_mutex);

  playback_domain->flags |= flags;

  pthread_mutex_unlock(playback_domain_mutex);
}

/**
 * ags_playback_domain_unset_flags:
 * @playback_domain: the #AgsPlaybackDomain
 * @flags: the flags
 *
 * Unset flags.
 * 
 * Since: 2.0.0
 */
void
ags_playback_domain_unset_flags(AgsPlaybackDomain *playback_domain, guint flags)
{
  pthread_mutex_t *playback_domain_mutex;

  if(!AGS_IS_PLAYBACK_DOMAIN(playback_domain)){
    return;
  }

  /* get playback_domain mutex */
  pthread_mutex_lock(ags_playback_domain_get_class_mutex());
  
  playback_domain_mutex = playback_domain->obj_mutex;
  
  pthread_mutex_unlock(ags_playback_domain_get_class_mutex());

  /* set flags */
  pthread_mutex_lock(playback_domain_mutex);

  playback_domain->flags &= (~flags);

  pthread_mutex_unlock(playback_domain_mutex);
}

/**
 * ags_playback_domain_set_audio_thread:
 * @playback_domain: the #AgsPlaybackDomain
 * @thread: the #AgsThread
 * @sound_scope: the thread's scope
 * 
 * Set audio thread to specified scope.
 * 
 * Since: 2.0.0
 */
void
ags_playback_domain_set_audio_thread(AgsPlaybackDomain *playback_domain,
				     AgsThread *thread,
				     gint sound_scope)
{
  pthread_mutex_t *playback_domain_mutex;

  if(!AGS_PLAYBACK_DOMAIN(playback_domain) ||
     sound_scope >= AGS_SOUND_SCOPE_LAST){
    return;
  }

  /* get playback domain mutex */
  pthread_mutex_lock(ags_playback_domain_get_class_mutex());
	  
  playback_domain_mutex = playback_domain->obj_mutex;
	  
  pthread_mutex_unlock(ags_playback_domain_get_class_mutex());

  /* set */
  pthread_mutex_lock(playback_domain_mutex);

  if(playback_domain->audio_thread[sound_scope] != NULL){
    if(ags_thread_is_running(playback_domain->audio_thread[sound_scope])){
      ags_thread_stop(playback_domain->audio_thread[sound_scope]);
    }
    
    g_object_run_dispose((GObject *) playback_domain->audio_thread[sound_scope]);
    g_object_unref((GObject *) playback_domain->audio_thread[sound_scope]);
  }

  if(thread != NULL){
    g_object_ref(thread);
  }
  
  playback_domain->audio_thread[sound_scope] = thread;

  pthread_mutex_unlock(playback_domain_mutex);
}

/**
 * ags_playback_domain_get_audio_thread:
 * @playback_domain: the #AgsPlaybackDomain
 * @sound_scope: the thread's scope
 * 
 * Get audio thread of specified scope.
 * 
 * Returns: the matching #AgsThread or %NULL
 * 
 * Since: 2.0.0
 */
AgsThread*
ags_playback_domain_get_audio_thread(AgsPlaybackDomain *playback_domain,
				     gint sound_scope)
{
  AgsThread *thread;
  
  pthread_mutex_t *playback_domain_mutex;

  if(!AGS_PLAYBACK_DOMAIN(playback_domain) ||
     playback_domain->audio_thread == NULL ||
     sound_scope >= AGS_SOUND_SCOPE_LAST){
    return(NULL);
  }

  /* get playback domain mutex */
  pthread_mutex_lock(ags_playback_domain_get_class_mutex());
	  
  playback_domain_mutex = playback_domain->obj_mutex;
	  
  pthread_mutex_unlock(ags_playback_domain_get_class_mutex());

  /* get */
  pthread_mutex_lock(playback_domain_mutex);

  thread = playback_domain->audio_thread[sound_scope];

  pthread_mutex_unlock(playback_domain_mutex);
  
  return(thread);
}


/**
 * ags_playback_domain_add_playback:
 * @playback_domain: the #AgsPlaybackDomain
 * @playback: the #AgsPlayback
 * @channel_type: either an AGS_TYPE_OUTPUT or AGS_TYPE_INPUT
 * 
 * Add @playback for @channel_type to @playback_domain.
 * 
 * Since: 2.0.0
 */
void
ags_playback_domain_add_playback(AgsPlaybackDomain *playback_domain,
				 GObject *playback, GType channel_type)
{
  pthread_mutex_t *playback_domain_mutex;
  
  if(!AGS_IS_PLAYBACK_DOMAIN(playback_domain) ||
     !AGS_IS_PLAYBACK(playback)){
    return;
  }

  /* get playback domain mutex */
  pthread_mutex_lock(ags_playback_domain_get_class_mutex());
	  
  playback_domain_mutex = playback_domain->obj_mutex;
	  
  pthread_mutex_unlock(ags_playback_domain_get_class_mutex());

  /* append */
  pthread_mutex_lock(playback_domain_mutex);
  
  //TODO:JK: rather use prepend but needs refactoring
  if(g_type_is_a(channel_type,
		 AGS_TYPE_OUTPUT)){
    g_object_ref(playback);
    playback_domain->output_playback = g_list_append(playback_domain->output_playback,
						     playback);
  }else if(g_type_is_a(channel_type,
		       AGS_TYPE_INPUT)){
    g_object_ref(playback);
    playback_domain->input_playback = g_list_append(playback_domain->input_playback,
						    playback);
  }

  pthread_mutex_unlock(playback_domain_mutex);
}

/**
 * ags_playback_domain_remove_playback:
 * @playback_domain: the #AgsPlaybackDomain
 * @playback: the #AgsPlayback
 * @channel_type: either an AGS_TYPE_OUTPUT or AGS_TYPE_INPUT
 * 
 * Remove @playback for @channel_type of @playback_domain.
 * 
 * Since: 2.0.0
 */
void
ags_playback_domain_remove_playback(AgsPlaybackDomain *playback_domain,
				    GObject *playback, GType channel_type)
{
  pthread_mutex_t *playback_domain_mutex;
  
  if(!AGS_IS_PLAYBACK_DOMAIN(playback_domain) ||
     !AGS_IS_PLAYBACK(playback)){
    return;
  }

  /* get playback domain mutex */
  pthread_mutex_lock(ags_playback_domain_get_class_mutex());
	  
  playback_domain_mutex = playback_domain->obj_mutex;
	  
  pthread_mutex_unlock(ags_playback_domain_get_class_mutex());

  /* remove */
  pthread_mutex_lock(playback_domain_mutex);
  
  if(g_type_is_a(channel_type,
		 AGS_TYPE_OUTPUT)){
    playback_domain->output_playback = g_list_remove(playback_domain->output_playback,
						     playback);
    g_object_unref(playback);
  }else if(g_type_is_a(channel_type,
		       AGS_TYPE_INPUT)){
    playback_domain->input_playback = g_list_remove(playback_domain->input_playback,
						    playback);
    g_object_unref(playback);
  }
  
  pthread_mutex_unlock(playback_domain_mutex);
}

/**
 * ags_playback_domain_new:
 * @audio: the #AgsAudio
 *
 * Creates an #AgsPlaybackDomain, refering to @application_context.
 *
 * Returns: a new #AgsPlaybackDomain
 *
 * Since: 2.0.0
 */
AgsPlaybackDomain*
ags_playback_domain_new(GObject *audio)
{
  AgsPlaybackDomain *playback_domain;

  playback_domain = (AgsPlaybackDomain *) g_object_new(AGS_TYPE_PLAYBACK_DOMAIN,
						       "audio", audio,
						       NULL);
  
  return(playback_domain);
}
