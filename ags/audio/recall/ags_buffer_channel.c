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

#include <ags/audio/recall/ags_buffer_channel.h>

#include <ags/libags.h>

#include <ags/plugin/ags_plugin_port.h>

#include <ags/i18n.h>

void ags_buffer_channel_class_init(AgsBufferChannelClass *buffer_channel);
void ags_buffer_channel_mutable_interface_init(AgsMutableInterface *mutable);
void ags_buffer_channel_plugin_interface_init(AgsPluginInterface *plugin);
void ags_buffer_channel_init(AgsBufferChannel *buffer_channel);
void ags_buffer_channel_set_property(GObject *gobject,
				     guint prop_id,
				     const GValue *value,
				     GParamSpec *param_spec);
void ags_buffer_channel_get_property(GObject *gobject,
				     guint prop_id,
				     GValue *value,
				     GParamSpec *param_spec);
void ags_buffer_channel_dispose(GObject *gobject);
void ags_buffer_channel_finalize(GObject *gobject);

void ags_buffer_channel_set_ports(AgsPlugin *plugin, GList *port);

void ags_buffer_channel_set_muted(AgsMutable *mutable, gboolean muted);

static AgsPluginPort* ags_buffer_channel_get_muted_plugin_port();

/**
 * SECTION:ags_buffer_channel
 * @short_description: buffers channel
 * @title: AgsBufferChannel
 * @section_id:
 * @include: ags/audio/recall/ags_buffer_channel.h
 *
 * The #AgsBufferChannel class provides ports to the effect processor.
 */

enum{
  PROP_0,
  PROP_MUTED,
};

static gpointer ags_buffer_channel_parent_class = NULL;
static AgsMutableInterface *ags_buffer_channel_parent_mutable_interface;
static AgsPluginInterface *ags_buffer_channel_parent_plugin_interface;

static const gchar *ags_buffer_channel_plugin_name = "ags-buffer";
static const gchar *ags_buffer_channel_plugin_specifier[] = {
  "./muted[0]",
};
static const gchar *ags_buffer_channel_plugin_control_port[] = {
  "1/1",
};

GType
ags_buffer_channel_get_type()
{
  static volatile gsize g_define_type_id__volatile = 0;

  if(g_once_init_enter (&g_define_type_id__volatile)){
    GType ags_type_buffer_channel = 0;

    static const GTypeInfo ags_buffer_channel_info = {
      sizeof (AgsBufferChannelClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_buffer_channel_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsBufferChannel),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_buffer_channel_init,
    };

    static const GInterfaceInfo ags_mutable_interface_info = {
      (GInterfaceInitFunc) ags_buffer_channel_mutable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    static const GInterfaceInfo ags_plugin_interface_info = {
      (GInterfaceInitFunc) ags_buffer_channel_plugin_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_buffer_channel = g_type_register_static(AGS_TYPE_RECALL_CHANNEL,
						     "AgsBufferChannel",
						     &ags_buffer_channel_info,
						     0);

    g_type_add_interface_static(ags_type_buffer_channel,
				AGS_TYPE_MUTABLE,
				&ags_mutable_interface_info);

    g_type_add_interface_static(ags_type_buffer_channel,
				AGS_TYPE_PLUGIN,
				&ags_plugin_interface_info);

    g_once_init_leave(&g_define_type_id__volatile, ags_type_buffer_channel);
  }

  return g_define_type_id__volatile;
}

void
ags_buffer_channel_mutable_interface_init(AgsMutableInterface *mutable)
{
  ags_buffer_channel_parent_mutable_interface = g_type_interface_peek_parent(mutable);

  mutable->set_muted = ags_buffer_channel_set_muted;
}

void
ags_buffer_channel_plugin_interface_init(AgsPluginInterface *plugin)
{
  ags_buffer_channel_parent_plugin_interface = g_type_interface_peek_parent(plugin);

  plugin->set_ports = ags_buffer_channel_set_ports;
}

void
ags_buffer_channel_class_init(AgsBufferChannelClass *buffer_channel)
{
  GObjectClass *gobject;
  AgsRecallClass *recall;
  GParamSpec *param_spec;

  ags_buffer_channel_parent_class = g_type_class_peek_parent(buffer_channel);

  /* GObjectClass */
  gobject = (GObjectClass *) buffer_channel;

  gobject->set_property = ags_buffer_channel_set_property;
  gobject->get_property = ags_buffer_channel_get_property;

  gobject->dispose = ags_buffer_channel_dispose;
  gobject->finalize = ags_buffer_channel_finalize;

  /* properties */
  /**
   * AgsBufferChannel:muted:
   *
   * The mute port.
   * 
   * Since: 2.0.0
   */
  param_spec = g_param_spec_object("muted",
				   i18n_pspec("mute channel"),
				   i18n_pspec("Mute the channel"),
				   AGS_TYPE_PORT,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_MUTED,
				  param_spec);
}

void
ags_buffer_channel_init(AgsBufferChannel *buffer_channel)
{
  GList *port;

  AGS_RECALL(buffer_channel)->name = "ags-buffer";
  AGS_RECALL(buffer_channel)->version = AGS_RECALL_DEFAULT_VERSION;
  AGS_RECALL(buffer_channel)->build_id = AGS_RECALL_DEFAULT_BUILD_ID;
  AGS_RECALL(buffer_channel)->xml_type = "ags-buffer-channel";

  port = NULL;

  buffer_channel->muted = g_object_new(AGS_TYPE_PORT,
				       "plugin-name", ags_buffer_channel_plugin_name,
				       "specifier", ags_buffer_channel_plugin_specifier[0],
				       "control-port", ags_buffer_channel_plugin_control_port[0],
				       "port-value-is-pointer", FALSE,
				       "port-value-type", G_TYPE_FLOAT,
				       "port-value-size", sizeof(gfloat),
				       "port-value-length", 1,
				       NULL);
  g_object_ref(buffer_channel->muted);
  buffer_channel->muted->port_value.ags_port_float = (float) FALSE;

  /* plugin port */
  g_object_set(buffer_channel->muted,
	       "plugin-port", ags_buffer_channel_get_muted_plugin_port(),
	       NULL);

  /* add to port */
  port = g_list_prepend(port, buffer_channel->muted);
  g_object_ref(buffer_channel->muted);

  /* set port */
  AGS_RECALL(buffer_channel)->port = port;

}


void
ags_buffer_channel_set_property(GObject *gobject,
				guint prop_id,
				const GValue *value,
				GParamSpec *param_spec)
{
  AgsBufferChannel *buffer_channel;

  pthread_mutex_t *recall_mutex;
  
  buffer_channel = AGS_BUFFER_CHANNEL(gobject);

  /* get recall mutex */
  recall_mutex = AGS_RECALL_GET_OBJ_MUTEX(buffer_channel);
  
  switch(prop_id){
  case PROP_MUTED:
    {
      AgsPort *port;

      port = (AgsPort *) g_value_get_object(value);

      pthread_mutex_lock(recall_mutex);
      
      if(port == buffer_channel->muted){
	pthread_mutex_unlock(recall_mutex);
	
	return;
      }

      if(buffer_channel->muted != NULL){
	g_object_unref(G_OBJECT(buffer_channel->muted));
      }
      
      if(port != NULL){
	g_object_ref(G_OBJECT(port));
      }

      buffer_channel->muted = port;

      pthread_mutex_unlock(recall_mutex);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }  
}

void
ags_buffer_channel_get_property(GObject *gobject,
				guint prop_id,
				GValue *value,
				GParamSpec *param_spec)
{
  AgsBufferChannel *buffer_channel;

  pthread_mutex_t *recall_mutex;
  
  buffer_channel = AGS_BUFFER_CHANNEL(gobject);

  /* get recall mutex */
  recall_mutex = AGS_RECALL_GET_OBJ_MUTEX(buffer_channel);

  switch(prop_id){
  case PROP_MUTED:
    {
      pthread_mutex_lock(recall_mutex);

      g_value_set_object(value, buffer_channel->muted);

      pthread_mutex_unlock(recall_mutex);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_buffer_channel_dispose(GObject *gobject)
{
  AgsBufferChannel *buffer_channel;

  buffer_channel = AGS_BUFFER_CHANNEL(gobject);

  if(buffer_channel->muted != NULL){
    g_object_unref(G_OBJECT(buffer_channel->muted));

    buffer_channel->muted = NULL;
  }

  /* call parent */
  G_OBJECT_CLASS(ags_buffer_channel_parent_class)->dispose(gobject);
}

void
ags_buffer_channel_finalize(GObject *gobject)
{
  AgsBufferChannel *buffer_channel;

  buffer_channel = AGS_BUFFER_CHANNEL(gobject);

  if(buffer_channel->muted != NULL){
    g_object_unref(G_OBJECT(buffer_channel->muted));
  }

  /* call parent */
  G_OBJECT_CLASS(ags_buffer_channel_parent_class)->finalize(gobject);
}

void
ags_buffer_channel_set_ports(AgsPlugin *plugin, GList *port)
{
  while(port != NULL){
    if(!strncmp(AGS_PORT(port->data)->specifier,
		"./muted[0]",
		9)){
      g_object_set(G_OBJECT(plugin),
		   "muted", AGS_PORT(port->data),
		   NULL);
    }

    port = port->next;
  }
}

void
ags_buffer_channel_set_muted(AgsMutable *mutable, gboolean is_muted)
{
  AgsPort *muted;
  
  GValue value = {0,};

  g_object_get(G_OBJECT(mutable),
	       "muted", &muted,
	       NULL);
  
  g_value_init(&value, G_TYPE_FLOAT);
  g_value_set_float(&value, (float) is_muted);

  ags_port_safe_write(muted, &value);

  g_value_unset(&value);
  g_object_unref(muted);
}

static AgsPluginPort*
ags_buffer_channel_get_muted_plugin_port()
{
  static AgsPluginPort *plugin_port = NULL;

  static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

  pthread_mutex_lock(&mutex);
  
  if(plugin_port == NULL){
    plugin_port = ags_plugin_port_new();
    g_object_ref(plugin_port);
    
    plugin_port->flags |= (AGS_PLUGIN_PORT_INPUT |
			   AGS_PLUGIN_PORT_CONTROL |
			   AGS_PLUGIN_PORT_TOGGLED);

    plugin_port->port_index = 0;

    /* range */
    g_value_init(plugin_port->default_value,
		 G_TYPE_FLOAT);
    g_value_init(plugin_port->lower_value,
		 G_TYPE_FLOAT);
    g_value_init(plugin_port->upper_value,
		 G_TYPE_FLOAT);

    g_value_set_float(plugin_port->default_value,
		      0.0);
    g_value_set_float(plugin_port->lower_value,
		      0.0);
    g_value_set_float(plugin_port->upper_value,
		      1.0);
  }
  
  pthread_mutex_unlock(&mutex);

  return(plugin_port);
}

/**
 * ags_buffer_channel_new:
 * @destination: the destination #AgsChannel
 * @source: the source #AgsChannel
 *
 * Create a new instance of #AgsBufferChannel
 *
 * Returns: the new #AgsBufferChannel
 *
 * Since: 2.0.0
 */
AgsBufferChannel*
ags_buffer_channel_new(AgsChannel *destination,
		       AgsChannel *source)
{
  AgsBufferChannel *buffer_channel;

  buffer_channel = (AgsBufferChannel *) g_object_new(AGS_TYPE_BUFFER_CHANNEL,
						     "destination", destination,
						     "source", source,
						     NULL);

  return(buffer_channel);
}
