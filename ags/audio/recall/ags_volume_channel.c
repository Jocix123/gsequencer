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

#include <ags/audio/recall/ags_volume_channel.h>

#include <ags/plugin/ags_plugin_port.h>

#include <ags/i18n.h>

void ags_volume_channel_class_init(AgsVolumeChannelClass *volume_channel);
void ags_volume_channel_init(AgsVolumeChannel *volume_channel);
void ags_volume_channel_set_property(GObject *gobject,
				     guint prop_id,
				     const GValue *value,
				     GParamSpec *param_spec);
void ags_volume_channel_get_property(GObject *gobject,
				     guint prop_id,
				     GValue *value,
				     GParamSpec *param_spec);
void ags_volume_channel_dispose(GObject *gobject);
void ags_volume_channel_finalize(GObject *gobject);

static AgsPluginPort* ags_volume_channel_get_volume_plugin_port();

/**
 * SECTION:ags_volume_channel
 * @short_description: volumes channel
 * @title: AgsVolumeChannel
 * @section_id:
 * @include: ags/audio/recall/ags_volume_channel.h
 *
 * The #AgsVolumeChannel class provides ports to the effect processor.
 */

enum{
  PROP_0,
  PROP_VOLUME,
};

static gpointer ags_volume_channel_parent_class = NULL;

const gchar *ags_volume_channel_plugin_name = "ags-volume";
const gchar *ags_volume_channel_specifier[] = {
  "./volume[0]"
};
const gchar *ags_volume_channel_control_port[] = {
  "1/1"
};

GType
ags_volume_channel_get_type()
{
  static volatile gsize g_define_type_id__volatile = 0;

  if(g_once_init_enter (&g_define_type_id__volatile)){
    GType ags_type_volume_channel = 0;

    static const GTypeInfo ags_volume_channel_info = {
      sizeof (AgsVolumeChannelClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_volume_channel_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsVolumeChannel),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_volume_channel_init,
    };

    ags_type_volume_channel = g_type_register_static(AGS_TYPE_RECALL_CHANNEL,
						     "AgsVolumeChannel",
						     &ags_volume_channel_info,
						     0);
    
    g_once_init_leave(&g_define_type_id__volatile, ags_type_volume_channel);
  }

  return g_define_type_id__volatile;
}

void
ags_volume_channel_class_init(AgsVolumeChannelClass *volume_channel)
{
  GObjectClass *gobject;
  AgsRecallClass *recall;
  GParamSpec *param_spec;

  ags_volume_channel_parent_class = g_type_class_peek_parent(volume_channel);

  /* GObjectClass */
  gobject = (GObjectClass *) volume_channel;

  gobject->set_property = ags_volume_channel_set_property;
  gobject->get_property = ags_volume_channel_get_property;

  gobject->dispose = ags_volume_channel_dispose;
  gobject->finalize = ags_volume_channel_finalize;

  /* properties */
  /**
   * AgsVolumeChannel:volume:
   * 
   * The volume port.
   * 
   * Since: 3.0.0 
   */
  param_spec = g_param_spec_object("volume",
				   i18n_pspec("volume to apply"),
				   i18n_pspec("The volume to apply on the channel"),
				   AGS_TYPE_PORT,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_VOLUME,
				  param_spec);
}

void
ags_volume_channel_init(AgsVolumeChannel *volume_channel)
{
  GList *port;

  AGS_RECALL(volume_channel)->name = "ags-volume";
  AGS_RECALL(volume_channel)->version = AGS_RECALL_DEFAULT_VERSION;
  AGS_RECALL(volume_channel)->build_id = AGS_RECALL_DEFAULT_BUILD_ID;
  AGS_RECALL(volume_channel)->xml_type = "ags-volume-channel";

  /* initialize the port */
  port = NULL;

  /* volume */
  volume_channel->volume = g_object_new(AGS_TYPE_PORT,
					"plugin-name", "ags-volume",
					"specifier", "./volume[0]",
					"control-port", "1/1",
					"port-value-is-pointer", FALSE,
					"port-value-type", G_TYPE_FLOAT,
					"port-value-size", sizeof(gfloat),
					"port-value-length", 1,
					NULL);
  g_object_ref(volume_channel->volume);
  
  volume_channel->volume->port_value.ags_port_float = 1.0;

  /* port descriptor */
  g_object_set(volume_channel->volume,
	       "plugin-port", ags_volume_channel_get_volume_plugin_port(),
	       NULL);

  /* add to port */  
  port = g_list_prepend(port, volume_channel->volume);
  g_object_ref(volume_channel->volume);
  
  /* set port */
  AGS_RECALL(volume_channel)->port = port;
}

void
ags_volume_channel_set_property(GObject *gobject,
				guint prop_id,
				const GValue *value,
				GParamSpec *param_spec)
{
  AgsVolumeChannel *volume_channel;

  GRecMutex *recall_mutex;

  volume_channel = AGS_VOLUME_CHANNEL(gobject);

  /* get recall mutex */
  recall_mutex = AGS_RECALL_GET_OBJ_MUTEX(volume_channel);
  
  switch(prop_id){
  case PROP_VOLUME:
    {
      AgsPort *port;

      port = (AgsPort *) g_value_get_object(value);

      g_rec_mutex_lock(recall_mutex);

      if(port == volume_channel->volume){      
	g_rec_mutex_unlock(recall_mutex);	

	return;
      }

      if(volume_channel->volume != NULL){
	g_object_unref(G_OBJECT(volume_channel->volume));
      }
      
      if(port != NULL){
	g_object_ref(G_OBJECT(port));
      }

      volume_channel->volume = port;
      
      g_rec_mutex_unlock(recall_mutex);	
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_volume_channel_get_property(GObject *gobject,
				guint prop_id,
				GValue *value,
				GParamSpec *param_spec)
{
  AgsVolumeChannel *volume_channel;

  GRecMutex *recall_mutex;

  volume_channel = AGS_VOLUME_CHANNEL(gobject);

  /* get recall mutex */
  recall_mutex = AGS_RECALL_GET_OBJ_MUTEX(volume_channel);

  switch(prop_id){
  case PROP_VOLUME:
    {
      g_rec_mutex_lock(recall_mutex);

      g_value_set_object(value, volume_channel->volume);
      
      g_rec_mutex_unlock(recall_mutex);	
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_volume_channel_dispose(GObject *gobject)
{
  AgsVolumeChannel *volume_channel;

  volume_channel = AGS_VOLUME_CHANNEL(gobject);

  /* volume */
  if(volume_channel->volume != NULL){
    g_object_unref(G_OBJECT(volume_channel->volume));
  }

  /* call parent */
  G_OBJECT_CLASS(ags_volume_channel_parent_class)->dispose(gobject);
}

void
ags_volume_channel_finalize(GObject *gobject)
{
  AgsVolumeChannel *volume_channel;

  volume_channel = AGS_VOLUME_CHANNEL(gobject);

  /* volume */
  if(volume_channel->volume != NULL){
    g_object_unref(G_OBJECT(volume_channel->volume));
  }

  /* call parent */
  G_OBJECT_CLASS(ags_volume_channel_parent_class)->finalize(gobject);
}

static AgsPluginPort*
ags_volume_channel_get_volume_plugin_port()
{
  static AgsPluginPort *plugin_port = NULL;

  static GMutex mutex;

  g_mutex_lock(&mutex);

  if(plugin_port == NULL){
    plugin_port = ags_plugin_port_new();
    g_object_ref(plugin_port);
    
    plugin_port->flags |= (AGS_PLUGIN_PORT_INPUT |
			   AGS_PLUGIN_PORT_CONTROL);

    plugin_port->port_index = 0;

    /* range */
    g_value_init(plugin_port->default_value,
		 G_TYPE_FLOAT);
    g_value_init(plugin_port->lower_value,
		 G_TYPE_FLOAT);
    g_value_init(plugin_port->upper_value,
		 G_TYPE_FLOAT);

    g_value_set_float(plugin_port->default_value,
		      1.0);
    g_value_set_float(plugin_port->lower_value,
		      0.0);
    g_value_set_float(plugin_port->upper_value,
		      2.0);
  }
  
  g_mutex_unlock(&mutex);

  return(plugin_port);
}

/**
 * ags_volume_channel_new:
 * @source: the #AgsAudioSignal
 *
 * Create a new instance of #AgsVolumeChannel
 *
 * Returns: the new #AgsVolumeChannel
 *
 * Since: 3.0.0
 */
AgsVolumeChannel*
ags_volume_channel_new(AgsChannel *source)
{
  AgsVolumeChannel *volume_channel;

  volume_channel = (AgsVolumeChannel *) g_object_new(AGS_TYPE_VOLUME_CHANNEL,
						     "source", source,
						     NULL);

  return(volume_channel);
}
