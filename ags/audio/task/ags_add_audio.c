q/* GSequencer - Advanced GTK Sequencer
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

#include <ags/audio/task/ags_add_audio.h>

#include <ags/i18n.h>

void ags_add_audio_class_init(AgsAddAudioClass *add_audio);
void ags_add_audio_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_add_audio_init(AgsAddAudio *add_audio);
void ags_add_audio_set_property(GObject *gobject,
				guint prop_id,
				const GValue *value,
				GParamSpec *param_spec);
void ags_add_audio_get_property(GObject *gobject,
				guint prop_id,
				GValue *value,
				GParamSpec *param_spec);
void ags_add_audio_connect(AgsConnectable *connectable);
void ags_add_audio_disconnect(AgsConnectable *connectable);
void ags_add_audio_dispose(GObject *gobject);
void ags_add_audio_finalize(GObject *gobject);

void ags_add_audio_launch(AgsTask *task);

enum{
  PROP_0,
  PROP_SOUNDCARD,
  PROP_AUDIO,
};

/**
 * SECTION:ags_add_audio
 * @short_description: add audio object to soundcard
 * @title: AgsAddAudio
 * @section_id:
 * @include: ags/audio/task/ags_add_audio.h
 *
 * The #AgsAddAudio task adds #AgsAudio to #AgsSoundcard.
 */

static gpointer ags_add_audio_parent_class = NULL;
static AgsConnectableInterface *ags_add_audio_parent_connectable_interface;

GType
ags_add_audio_get_type()
{
  static volatile gsize g_define_type_id__volatile = 0;

  if(g_once_init_enter (&g_define_type_id__volatile)){
    GType ags_type_add_audio;

    static const GTypeInfo ags_add_audio_info = {
      sizeof (AgsAddAudioClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_add_audio_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsAddAudio),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_add_audio_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_add_audio_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };
    
    ags_type_add_audio = g_type_register_static(AGS_TYPE_TASK,
						"AgsAddAudio",
						&ags_add_audio_info,
						0);
    
    g_type_add_interface_static(ags_type_add_audio,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);

    g_once_init_leave (&g_define_type_id__volatile, ags_type_add_audio);
  }

  return g_define_type_id__volatile;
}

void
ags_add_audio_class_init(AgsAddAudioClass *add_audio)
{
  GObjectClass *gobject;
  AgsTaskClass *task;
  GParamSpec *param_spec;

  ags_add_audio_parent_class = g_type_class_peek_parent(add_audio);

  /* gobject */
  gobject = (GObjectClass *) add_audio;

  gobject->set_property = ags_add_audio_set_property;
  gobject->get_property = ags_add_audio_get_property;

  gobject->dispose = ags_add_audio_dispose;
  gobject->finalize = ags_add_audio_finalize;

  /* properties */
  /**
   * AgsAddAudio:soundcard:
   *
   * The assigned #AgsSoundcard
   * 
   * Since: 1.0.0
   */
  param_spec = g_param_spec_object("soundcard",
				   i18n_pspec("soundcard of add audio"),
				   i18n_pspec("The soundcard of add audio task"),
				   G_TYPE_OBJECT,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_SOUNDCARD,
				  param_spec);

  /**
   * AgsAddAudio:audio:
   *
   * The assigned #AgsAudio
   * 
   * Since: 1.0.0
   */
  param_spec = g_param_spec_object("audio",
				   i18n_pspec("audio of add audio"),
				   i18n_pspec("The audio of add audio task"),
				   AGS_TYPE_AUDIO,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_AUDIO,
				  param_spec);

  /* task */
  task = (AgsTaskClass *) add_audio;

  task->launch = ags_add_audio_launch;
}

void
ags_add_audio_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_add_audio_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_add_audio_connect;
  connectable->disconnect = ags_add_audio_disconnect;
}

void
ags_add_audio_init(AgsAddAudio *add_audio)
{
  add_audio->soundcard = NULL;
  add_audio->audio = NULL;
}

void
ags_add_audio_set_property(GObject *gobject,
			      guint prop_id,
			      const GValue *value,
			      GParamSpec *param_spec)
{
  AgsAddAudio *add_audio;

  add_audio = AGS_ADD_AUDIO(gobject);

  switch(prop_id){
  case PROP_SOUNDCARD:
    {
      GObject *soundcard;

      soundcard = (GObject *) g_value_get_object(value);

      if(add_audio->soundcard == (GObject *) soundcard){
	return;
      }

      if(add_audio->soundcard != NULL){
	g_object_unref(add_audio->soundcard);
      }

      if(soundcard != NULL){
	g_object_ref(soundcard);
      }

      add_audio->soundcard = (GObject *) soundcard;
    }
    break;
  case PROP_AUDIO:
    {
      AgsAudio *audio;

      audio = (AgsAudio *) g_value_get_object(value);

      if(add_audio->audio == (GObject *) audio){
	return;
      }

      if(add_audio->audio != NULL){
	g_object_unref(add_audio->audio);
      }

      if(audio != NULL){
	g_object_ref(audio);
      }

      add_audio->audio = (GObject *) audio;
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_add_audio_get_property(GObject *gobject,
			      guint prop_id,
			      GValue *value,
			      GParamSpec *param_spec)
{
  AgsAddAudio *add_audio;

  add_audio = AGS_ADD_AUDIO(gobject);

  switch(prop_id){
  case PROP_SOUNDCARD:
    {
      g_value_set_object(value, add_audio->soundcard);
    }
    break;
  case PROP_AUDIO:
    {
      g_value_set_object(value, add_audio->audio);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_add_audio_connect(AgsConnectable *connectable)
{
  ags_add_audio_parent_connectable_interface->connect(connectable);

  /* empty */
}

void
ags_add_audio_disconnect(AgsConnectable *connectable)
{
  ags_add_audio_parent_connectable_interface->disconnect(connectable);

  /* empty */
}

void
ags_add_audio_dispose(GObject *gobject)
{
  AgsAddAudio *add_audio;

  add_audio = AGS_ADD_AUDIO(gobject);

  if(add_audio->soundcard != NULL){
    g_object_unref(add_audio->soundcard);

    add_audio->soundcard = NULL;
  }

  if(add_audio->audio != NULL){
    g_object_unref(add_audio->audio);

    add_audio->audio = NULL;
  }
  
  /* call parent */
  G_OBJECT_CLASS(ags_add_audio_parent_class)->dispose(gobject);
}

void
ags_add_audio_finalize(GObject *gobject)
{
  AgsAddAudio *add_audio;

  add_audio = AGS_ADD_AUDIO(gobject);

  if(add_audio->soundcard != NULL){
    g_object_unref(add_audio->soundcard);
  }

  if(add_audio->audio != NULL){
    g_object_unref(add_audio->audio);
  }

  /* call parent */
  G_OBJECT_CLASS(ags_add_audio_parent_class)->finalize(gobject);
}

void
ags_add_audio_launch(AgsTask *task)
{
  AgsAddAudio *add_audio;
  
  AgsMutexManager *mutex_manager;
  
  GList *list;
  
  pthread_mutex_t *application_mutex;
  pthread_mutex_t *soundcard_mutex;

  /* get mutex manager and application mutex */
  mutex_manager = ags_mutex_manager_get_instance();
  application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);
  
  add_audio = AGS_ADD_AUDIO(task);

  //TODO:JK: use AgsSoundcard
  
  /* add audio */
  if(add_audio->soundcard != NULL){
    /* get soundcard mutex */
    pthread_mutex_lock(application_mutex);

    soundcard_mutex = ags_mutex_manager_lookup(mutex_manager,
					       (GObject *) add_audio->soundcard);

    pthread_mutex_unlock(application_mutex);

    /* ref audio */
    g_object_ref(G_OBJECT(add_audio->audio));

    /* add to soundcard */
    pthread_mutex_lock(soundcard_mutex);

    list = ags_soundcard_get_audio(AGS_SOUNDCARD(add_audio->soundcard));
    list = g_list_prepend(list,
			  add_audio->audio);
    ags_soundcard_set_audio(AGS_SOUNDCARD(add_audio->soundcard),
			    list);

    pthread_mutex_unlock(soundcard_mutex);
  }
  
  /* AgsAudio */
  ags_connectable_connect(AGS_CONNECTABLE(add_audio->audio));
}

/**
 * ags_add_audio_new:
 * @soundcard: the #AgsSoundcard
 * @audio: the #AgsAudio to add
 *
 * Creates an #AgsAddAudio.
 *
 * Returns: an new #AgsAddAudio.
 *
 * Since: 1.0.0
 */
AgsAddAudio*
ags_add_audio_new(GObject *soundcard,
		  AgsAudio *audio)
{
  AgsAddAudio *add_audio;

  add_audio = (AgsAddAudio *) g_object_new(AGS_TYPE_ADD_AUDIO,
					   "soundcard", soundcard,
					   "audio", audio,
					   NULL);

  return(add_audio);
}
