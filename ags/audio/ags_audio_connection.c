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

#include <ags/audio/ags_audio_connection.h>

#include <ags/i18n.h>

void ags_audio_connection_class_init(AgsAudioConnectionClass *audio_connection);
void ags_audio_connection_init (AgsAudioConnection *audio_connection);
void ags_audio_connection_set_property(GObject *gobject,
				       guint prop_id,
				       const GValue *value,
				       GParamSpec *param_spec);
void ags_audio_connection_get_property(GObject *gobject,
				       guint prop_id,
				       GValue *value,
				       GParamSpec *param_spec);
void ags_audio_connection_finalize(GObject *gobject);

/**
 * SECTION:ags_audio_connection
 * @short_description: audio connections
 * @title: AgsAudioConnection
 * @section_id:
 * @include: ags/audio/ags_audio_connection.h
 *
 * The #AgsAudioConnection specifies your audio connections.
 */

static gpointer ags_audio_connection_parent_class = NULL;

enum{
  PROP_0,
  PROP_AUDIO,
  PROP_CHANNEL_TYPE,
  PROP_PAD,
  PROP_AUDIO_CHANNEL,
  PROP_LINE,
  PROP_MAPPED_LINE,
};

GType
ags_audio_connection_get_type (void)
{
  static GType ags_type_audio_connection = 0;

  if(!ags_type_audio_connection){
    static const GTypeInfo ags_audio_connection_info = {
      sizeof (AgsAudioConnectionClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_audio_connection_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsAudioConnection),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_audio_connection_init,
    };

    ags_type_audio_connection = g_type_register_static(AGS_TYPE_CONNECTION,
						       "AgsAudioConnection",
						       &ags_audio_connection_info,
						       0);
  }

  return (ags_type_audio_connection);
}

void
ags_audio_connection_class_init(AgsAudioConnectionClass *audio_connection)
{
  GObjectClass *gobject;
  GParamSpec *param_spec;
  
  ags_audio_connection_parent_class = g_type_class_peek_parent(audio_connection);

  /* GObjectClass */
  gobject = (GObjectClass *) audio_connection;

  gobject->set_property = ags_audio_connection_set_property;
  gobject->get_property = ags_audio_connection_get_property;

  gobject->finalize = ags_audio_connection_finalize;

  /* properties */
  /**
   * AgsAudioConnection:audio:
   *
   * The #AgsAudio belonging to.
   * 
   * Since: 1.0.0
   */
  param_spec = g_param_spec_object("audio",
				   i18n_pspec("audio"),
				   i18n_pspec("The audio belonging to"),
				   G_TYPE_OBJECT,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_AUDIO,
				  param_spec);

  /**
   * AgsAudioConnection:channel-type:
   *
   * The #GType of #AgsChannel belonging to.
   * 
   * Since: 1.0.0
   */
  param_spec = g_param_spec_ulong("channel-type",
				  i18n_pspec("channel type"),
				  i18n_pspec("The channel type belonging to"),
				  0, G_MAXULONG,
				  G_TYPE_NONE,
				  G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_CHANNEL_TYPE,
				  param_spec);

  /**
   * AgsAudioConnection:pad:
   *
   * The nth pad assigned with.
   * 
   * Since: 1.0.0
   */
  param_spec = g_param_spec_uint("pad",
				 i18n_pspec("pad"),
				 i18n_pspec("The pad assigned with"),
				 0,
				 G_MAXUINT32,
				 0,
				 G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_PAD,
				  param_spec);

  /**
   * AgsAudioConnection:audio-channel:
   *
   * The nth audio channel assigned with.
   * 
   * Since: 1.0.0
   */
  param_spec = g_param_spec_uint("audio-channel",
				 i18n_pspec("audio-channel"),
				 i18n_pspec("The audio channel assigned with"),
				 0,
				 G_MAXUINT32,
				 0,
				 G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_AUDIO_CHANNEL,
				  param_spec);

  /**
   * AgsAudioConnection:line:
   *
   * The nth line assigned with.
   * 
   * Since: 1.0.0
   */
  param_spec = g_param_spec_uint("line",
				 i18n_pspec("line"),
				 i18n_pspec("The line assigned with"),
				 0,
				 G_MAXUINT32,
				 0,
				 G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_LINE,
				  param_spec);

  /**
   * AgsAudioConnection:mapped-line:
   *
   * The nth mapped line assigned with.
   * 
   * Since: 1.0.0
   */
  param_spec = g_param_spec_uint("mapped-line",
				 i18n_pspec("mapped line"),
				 i18n_pspec("The mapped line of data object"),
				 0,
				 G_MAXUINT32,
				 0,
				 G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_MAPPED_LINE,
				  param_spec);
}

void
ags_audio_connection_init(AgsAudioConnection *audio_connection)
{
  audio_connection->flags = 0;
  
  audio_connection->audio = NULL;
  audio_connection->channel_type = G_TYPE_NONE;

  audio_connection->pad = 0;
  audio_connection->audio_channel = 0;
  audio_connection->line = 0;

  audio_connection->mapped_line = 0;
}

void
ags_audio_connection_set_property(GObject *gobject,
				  guint prop_id,
				  const GValue *value,
				  GParamSpec *param_spec)
{
  AgsAudioConnection *audio_connection;

  audio_connection = AGS_AUDIO_CONNECTION(gobject);

  switch(prop_id){
  case PROP_AUDIO:
    {
      GObject *audio;

      audio = (GObject *) g_value_get_object(value);

      if(audio_connection->audio == audio){
	return;
      }

      if(audio_connection->audio != NULL){
	g_object_unref(audio_connection->audio);
      }

      if(audio != NULL){
	g_object_ref(audio);
      }

      audio_connection->audio = audio;
    }
    break;
  case PROP_CHANNEL_TYPE:
    {
      GType channel_type;

      audio_connection->channel_type = g_value_get_ulong(value);
    }
    break;
  case PROP_PAD:
    {
      audio_connection->pad = g_value_get_uint(value);
    }
    break;
  case PROP_AUDIO_CHANNEL:
    {
      audio_connection->audio_channel = g_value_get_uint(value);
    }
    break;
  case PROP_LINE:
    {
      audio_connection->line = g_value_get_uint(value);
    }
    break;
  case PROP_MAPPED_LINE:
    {
      AgsMessageDelivery *message_delivery;
      AgsMessageQueue *message_queue;

      audio_connection->mapped_line = g_value_get_uint(value);

      /* emit message */
      message_delivery = ags_message_delivery_get_instance();

      message_queue = ags_message_delivery_find_namespace(message_delivery,
							  "libags-audio");

      if(message_queue != NULL){
	AgsMessageEnvelope *message;

	xmlDoc *doc;
	xmlNode *root_node;

	/* specify message body */
	doc = xmlNewDoc("1.0");

	root_node = xmlNewNode(NULL,
			       "ags-command");
	xmlDocSetRootElement(doc, root_node);    

	xmlNewProp(root_node,
		   "method",
		   "GObject::notify::mapped-line");

	/* add message */
	message = ags_message_envelope_alloc(audio_connection,
					     NULL,
					     doc);

	/* set parameter */
	message->parameter = g_new0(GParameter,
				    1);
	message->n_params = 1;
    
	message->parameter[0].name = "mapped-line";
	g_value_init(&(message->parameter[0].value),
		     G_TYPE_UINT);
	g_value_set_uint(&(message->parameter[0].value),
			   audio_connection->mapped_line);

	/* add message */
	ags_message_delivery_add_message(message_delivery,
					 "libags-audio",
					 message);
      }
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
  }
}

void
ags_audio_connection_get_property(GObject *gobject,
				  guint prop_id,
				  GValue *value,
				  GParamSpec *param_spec)
{
  AgsAudioConnection *audio_connection;

  audio_connection = AGS_AUDIO_CONNECTION(gobject);

  switch(prop_id){
  case PROP_AUDIO:
    {
      g_value_set_object(value, audio_connection->audio);
    }
    break;
  case PROP_CHANNEL_TYPE:
    {
      g_value_set_ulong(value, audio_connection->channel_type);
    }
    break;
  case PROP_PAD:
    {
      g_value_set_uint(value, audio_connection->pad);
    }
    break;
  case PROP_AUDIO_CHANNEL:
    {
      g_value_set_uint(value, audio_connection->audio_channel);
    }
    break;
  case PROP_LINE:
    {
      g_value_set_uint(value, audio_connection->line);
    }
    break;
  case PROP_MAPPED_LINE:
    {
      g_value_set_uint(value, audio_connection->mapped_line);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
  }
}

void
ags_audio_connection_finalize(GObject *gobject)
{
  AgsAudioConnection *audio_connection;

  audio_connection = AGS_AUDIO_CONNECTION(gobject);

  if(audio_connection->audio != NULL){
    g_object_unref(audio_connection->audio);
  }
}

/**
 * ags_audio_connection_find:
 * @list: the #GList-struct containing #AgsAudioConnection
 * @channel_type: the channel type to match
 * @pad: the pad to match
 * @audio_channel: the audio channel to match
 * 
 * Find the specified audio connection matching @channel_type, @pad and @audio_channel.
 * 
 * Returns: the first matched #GList-struct, else %NULL if no match
 * 
 * Since: 1.0.0
 */
GList*
ags_audio_connection_find(GList *list,
			  GType channel_type,
			  guint pad,
			  guint audio_channel)
{
  while(list != NULL){
    pthread_mutex_lock(AGS_CONNECTION(list->data)->mutex);

    if(AGS_AUDIO_CONNECTION(list->data)->channel_type == channel_type &&
       AGS_AUDIO_CONNECTION(list->data)->pad == pad &&
       AGS_AUDIO_CONNECTION(list->data)->audio_channel == audio_channel){
      pthread_mutex_unlock(AGS_CONNECTION(list->data)->mutex);
      
      return(list);
    }

    pthread_mutex_unlock(AGS_CONNECTION(list->data)->mutex);

    list = list->next;
  }

  return(NULL);
}

/**
 * ags_audio_connection_new:
 *
 * Creates an #AgsAudioConnection
 *
 * Returns: a new #AgsAudioConnection
 *
 * Since: 1.0.0
 */
AgsAudioConnection*
ags_audio_connection_new()
{
  AgsAudioConnection *audio_connection;

  audio_connection = (AgsAudioConnection *) g_object_new(AGS_TYPE_AUDIO_CONNECTION,
							 NULL);

  return(audio_connection);
}