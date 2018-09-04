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

#include <ags/audio/recall/ags_copy_pattern_audio.h>

#include <ags/libags.h>

#include <ags/plugin/ags_plugin_port.h>

#include <ags/audio/recall/ags_copy_pattern_audio_run.h>
#include <ags/audio/recall/ags_copy_pattern_channel.h>
#include <ags/audio/recall/ags_copy_pattern_channel_run.h>

#include <ags/i18n.h>

void ags_copy_pattern_audio_class_init(AgsCopyPatternAudioClass *copy_pattern_audio);
void ags_copy_pattern_audio_plugin_interface_init(AgsPluginInterface *plugin);
void ags_copy_pattern_audio_init(AgsCopyPatternAudio *copy_pattern_audio);
void ags_copy_pattern_audio_set_property(GObject *gobject,
					 guint prop_id,
					 const GValue *value,
					 GParamSpec *param_spec);
void ags_copy_pattern_audio_get_property(GObject *gobject,
					 guint prop_id,
					 GValue *value,
					 GParamSpec *param_spec);
void ags_copy_pattern_audio_dispose(GObject *gobject);
void ags_copy_pattern_audio_finalize(GObject *gobject);

void ags_copy_pattern_audio_set_ports(AgsPlugin *plugin, GList *port);

static AgsPluginPort* ags_copy_pattern_audio_get_bank_index_0_plugin_port();
static AgsPluginPort* ags_copy_pattern_audio_get_bank_index_1_plugin_port();

/**
 * SECTION:ags_copy_pattern_audio
 * @short_description: copy audio pattern
 * @title: AgsCopyPatternAudio
 * @section_id:
 * @include: ags/audio/recall/ags_copy_pattern_audio.h
 *
 * The #AgsCopyPatternAudio class provides ports to the effect processor.
 */

enum{
  PROP_0,
  PROP_BANK_INDEX_0,
  PROP_BANK_INDEX_1,
};

static gpointer ags_copy_pattern_audio_parent_class = NULL;
static AgsPluginInterface *ags_copy_pattern_audio_parent_plugin_interface;

static const gchar *ags_copy_pattern_audio_plugin_name = "ags-copy-pattern";
static const gchar *ags_copy_pattern_audio_specifier[] = {
  "./bank-index-0[0]",
  "./bank-index-1[0]"  
};
static const gchar *ags_copy_pattern_audio_control_port[] = {
  "1/2",
  "2/2"
};

GType
ags_copy_pattern_audio_get_type()
{
  static volatile gsize g_define_type_id__volatile = 0;

  if(g_once_init_enter (&g_define_type_id__volatile)){
    GType ags_type_copy_pattern_audio = 0;

    static const GTypeInfo ags_copy_pattern_audio_info = {
      sizeof(AgsCopyPatternAudioClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_copy_pattern_audio_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof(AgsCopyPatternAudio),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_copy_pattern_audio_init,
    };

    static const GInterfaceInfo ags_plugin_interface_info = {
      (GInterfaceInitFunc) ags_copy_pattern_audio_plugin_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_copy_pattern_audio = g_type_register_static(AGS_TYPE_RECALL_AUDIO,
							 "AgsCopyPatternAudio",
							 &ags_copy_pattern_audio_info,
							 0);

    g_type_add_interface_static(ags_type_copy_pattern_audio,
				AGS_TYPE_PLUGIN,
				&ags_plugin_interface_info);

    g_once_init_leave(&g_define_type_id__volatile, ags_type_copy_pattern_audio);
  }

  return g_define_type_id__volatile;
}

void
ags_copy_pattern_audio_class_init(AgsCopyPatternAudioClass *copy_pattern_audio)
{
  GObjectClass *gobject;

  GParamSpec *param_spec;

  ags_copy_pattern_audio_parent_class = g_type_class_peek_parent(copy_pattern_audio);

  /* GObjectClass */
  gobject = (GObjectClass *) copy_pattern_audio;

  gobject->set_property = ags_copy_pattern_audio_set_property;
  gobject->get_property = ags_copy_pattern_audio_get_property;

  gobject->dispose = ags_copy_pattern_audio_dispose;
  gobject->finalize = ags_copy_pattern_audio_finalize;

  /* properties */
  /**
   * AgsCopyPatternAudio:bank-index-0:
   *
   * The bank index 0 port.
   * 
   * Since: 2.0.0
   */
  param_spec = g_param_spec_object("bank-index-0",
				   i18n_pspec("current bank index 0"),
				   i18n_pspec("The current bank index 0 of the AgsPattern"),
				   AGS_TYPE_PORT,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_BANK_INDEX_0,
				  param_spec);

  /**
   * AgsCopyPatternAudio:bank-index-1:
   *
   * The bank index 1 port.
   * 
   * Since: 2.0.0
   */
  param_spec = g_param_spec_object("bank-index-1",
				   i18n_pspec("current bank index 1"),
				   i18n_pspec("The current bank index 1 of the AgsPattern"),
				   AGS_TYPE_PORT,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_BANK_INDEX_1,
				  param_spec);
}

void
ags_copy_pattern_audio_plugin_interface_init(AgsPluginInterface *plugin)
{
  ags_copy_pattern_audio_parent_plugin_interface = g_type_interface_peek_parent(plugin);

  plugin->set_ports = ags_copy_pattern_audio_set_ports;
}

void
ags_copy_pattern_audio_init(AgsCopyPatternAudio *copy_pattern_audio)
{
  GList *port;

  AGS_RECALL(copy_pattern_audio)->name = "ags-copy-pattern";
  AGS_RECALL(copy_pattern_audio)->version = AGS_RECALL_DEFAULT_VERSION;
  AGS_RECALL(copy_pattern_audio)->build_id = AGS_RECALL_DEFAULT_BUILD_ID;
  AGS_RECALL(copy_pattern_audio)->xml_type = "ags-copy-pattern-audio";

  port = NULL;

  /* bank index 0 */
  copy_pattern_audio->bank_index_0 = g_object_new(AGS_TYPE_PORT,
						  "plugin-name", ags_copy_pattern_audio_plugin_name,
						  "specifier", ags_copy_pattern_audio_specifier[0],
						  "control-port", ags_copy_pattern_audio_control_port[0],
						  "port-value-is-pointer", FALSE,
						  "port-value-type", G_TYPE_FLOAT,
						  "port-value-size", sizeof(gfloat),
						  "port-value-length", 1,
						  NULL);
  g_object_ref(copy_pattern_audio->bank_index_0);

  copy_pattern_audio->bank_index_0->port_value.ags_port_float = 0.0;

  /* plugin port */
#if 0
  g_object_set(copy_pattern_audio->bank_index_0,
	       "plugin-port", ags_copy_pattern_audio_get_bank_index_0_plugin_port(),
	       NULL);
#endif
  
  /* add to port */
  port = g_list_prepend(port, copy_pattern_audio->bank_index_0);
  g_object_ref(copy_pattern_audio->bank_index_0);

  /* bank index 1 */
  copy_pattern_audio->bank_index_1 = g_object_new(AGS_TYPE_PORT,
						  "plugin-name", ags_copy_pattern_audio_plugin_name,
						  "specifier", ags_copy_pattern_audio_specifier[1],
						  "control-port", ags_copy_pattern_audio_control_port[1],
						  "port-value-is-pointer", FALSE,
						  "port-value-type", G_TYPE_FLOAT,
						  "port-value-size", sizeof(gfloat),
						  "port-value-length", 1,
						  NULL);
  g_object_ref(copy_pattern_audio->bank_index_1);

  copy_pattern_audio->bank_index_1->port_value.ags_port_float = 0.0;

  /* plugin port */
#if 0
  g_object_set(copy_pattern_audio->bank_index_1,
	       "plugin-port", ags_copy_pattern_audio_get_bank_index_1_plugin_port(),
	       NULL);
#endif
  
  /* add to port */
  port = g_list_prepend(port, copy_pattern_audio->bank_index_1);
  g_object_ref(copy_pattern_audio->bank_index_1);

  /* set port */
  AGS_RECALL(copy_pattern_audio)->port = port;
}

void
ags_copy_pattern_audio_set_property(GObject *gobject,
				    guint prop_id,
				    const GValue *value,
				    GParamSpec *param_spec)
{
  AgsCopyPatternAudio *copy_pattern_audio;

  pthread_mutex_t *recall_mutex;
  
  copy_pattern_audio = AGS_COPY_PATTERN_AUDIO(gobject);

  /* get recall mutex */
  pthread_mutex_lock(ags_recall_get_class_mutex());
  
  recall_mutex = AGS_RECALL(gobject)->obj_mutex;

  pthread_mutex_unlock(ags_recall_get_class_mutex());

  switch(prop_id){
  case PROP_BANK_INDEX_0:
    {
      AgsPort *port;

      port = (AgsPort *) g_value_get_object(value);

      pthread_mutex_lock(recall_mutex);

      if(port == copy_pattern_audio->bank_index_0){
	pthread_mutex_unlock(recall_mutex);

	return;
      }

      if(copy_pattern_audio->bank_index_0 != NULL){
	g_object_unref(G_OBJECT(copy_pattern_audio->bank_index_0));
      }
      
      if(port != NULL){
	g_object_ref(G_OBJECT(port));
      }

      copy_pattern_audio->bank_index_0 = port;

      pthread_mutex_unlock(recall_mutex);
    }
    break;
  case PROP_BANK_INDEX_1:
    {
      AgsPort *port;

      port = (AgsPort *) g_value_get_object(value);

      pthread_mutex_lock(recall_mutex);

      if(port == copy_pattern_audio->bank_index_1){
	pthread_mutex_unlock(recall_mutex);

	return;
      }

      if(copy_pattern_audio->bank_index_1 != NULL){
	g_object_unref(G_OBJECT(copy_pattern_audio->bank_index_1));
      }
      
      if(port != NULL){
	g_object_ref(G_OBJECT(port));
      }

      copy_pattern_audio->bank_index_1 = port;

      pthread_mutex_unlock(recall_mutex);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  };
}

void
ags_copy_pattern_audio_get_property(GObject *gobject,
				    guint prop_id,
				    GValue *value,
				    GParamSpec *param_spec)
{
  AgsCopyPatternAudio *copy_pattern_audio;

  pthread_mutex_t *recall_mutex;
    
  copy_pattern_audio = AGS_COPY_PATTERN_AUDIO(gobject);

  /* get recall mutex */
  pthread_mutex_lock(ags_recall_get_class_mutex());
  
  recall_mutex = AGS_RECALL(gobject)->obj_mutex;

  pthread_mutex_unlock(ags_recall_get_class_mutex());

  switch(prop_id){
  case PROP_BANK_INDEX_0:
    {
      pthread_mutex_lock(recall_mutex);

      g_value_set_object(value, copy_pattern_audio->bank_index_0);

      pthread_mutex_unlock(recall_mutex);
    }
    break;
  case PROP_BANK_INDEX_1:
    {
      pthread_mutex_lock(recall_mutex);

      g_value_set_object(value, copy_pattern_audio->bank_index_1);

      pthread_mutex_unlock(recall_mutex);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  };
}

void
ags_copy_pattern_audio_dispose(GObject *gobject)
{
  AgsCopyPatternAudio *copy_pattern_audio;
  
  copy_pattern_audio = AGS_COPY_PATTERN_AUDIO(gobject);

  /* bank index 0 */
  if(copy_pattern_audio->bank_index_0 != NULL){
    g_object_unref(copy_pattern_audio->bank_index_0);

    copy_pattern_audio->bank_index_0 = NULL;
  }

  /* bank index 1 */
  if(copy_pattern_audio->bank_index_1 != NULL){
    g_object_unref(copy_pattern_audio->bank_index_1);

    copy_pattern_audio->bank_index_1 = NULL;
  }
  
  /* call parent */
  G_OBJECT_CLASS(ags_copy_pattern_audio_parent_class)->dispose(gobject);
}

void
ags_copy_pattern_audio_finalize(GObject *gobject)
{
  AgsCopyPatternAudio *copy_pattern_audio;
  
  copy_pattern_audio = AGS_COPY_PATTERN_AUDIO(gobject);

  /* bank index 0 */
  if(copy_pattern_audio->bank_index_0 != NULL){
    g_object_unref(copy_pattern_audio->bank_index_0);
  }

  /* bank index 1 */
  if(copy_pattern_audio->bank_index_1 != NULL){
    g_object_unref(copy_pattern_audio->bank_index_1);
  }
  
  /* call parent */
  G_OBJECT_CLASS(ags_copy_pattern_audio_parent_class)->finalize(gobject);
}

void
ags_copy_pattern_audio_set_ports(AgsPlugin *plugin, GList *port)
{
  while(port != NULL){
    if(!strncmp(AGS_PORT(port->data)->specifier,
		"./bank-index-0[0]",
		16)){
      g_object_set(G_OBJECT(plugin),
		   "bank-index-0", AGS_PORT(port->data),
		   NULL);
    }else if(!strncmp(AGS_PORT(port->data)->specifier,
		      "./bank-index-1[0]",
		      16)){
      g_object_set(G_OBJECT(plugin),
		   "bank-index-1", AGS_PORT(port->data),
		   NULL);
    }

    port = port->next;
  }
}

static AgsPluginPort*
ags_copy_pattern_audio_get_bank_index_0_plugin_port()
{
  static AgsPluginPort *plugin_port = NULL;

  static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

  pthread_mutex_lock(&mutex);
  
  if(plugin_port == NULL){
    plugin_port = ags_plugin_port_new();
    g_object_ref(plugin_port);
    
    plugin_port->flags |= (AGS_PLUGIN_PORT_INPUT |
			   AGS_PLUGIN_PORT_CONTROL |
			   AGS_PLUGIN_PORT_INTEGER);

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
		      AGS_COPY_PATTERN_AUDIO_MAX_BANK_INDEX_0);
  }
  
  pthread_mutex_unlock(&mutex);

  return(plugin_port);
}

static AgsPluginPort*
ags_copy_pattern_audio_get_bank_index_1_plugin_port()
{
  static AgsPluginPort *plugin_port = NULL;

  static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

  pthread_mutex_lock(&mutex);
  
  if(plugin_port == NULL){
    plugin_port = ags_plugin_port_new();
    g_object_ref(plugin_port);
    
    plugin_port->flags |= (AGS_PLUGIN_PORT_INPUT |
			   AGS_PLUGIN_PORT_CONTROL |
			   AGS_PLUGIN_PORT_INTEGER);

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
		      AGS_COPY_PATTERN_AUDIO_MAX_BANK_INDEX_1);
  }
  
  pthread_mutex_unlock(&mutex);

  return(plugin_port);
}

/**
 * ags_copy_pattern_audio_new:
 * @audio: the #AgsAudio
 * @bank_index_0: bank index 0
 * @bank_index_1: bank index 1
 *
 * Create a new instance of #AgsCopyPatternAudio
 *
 * Returns: the new #AgsCopyPatternAudio
 *
 * Since: 2.0.0
 */
AgsCopyPatternAudio*
ags_copy_pattern_audio_new(AgsAudio *audio,
			   guint bank_index_0,
			   guint bank_index_1)
{
  AgsCopyPatternAudio *copy_pattern_audio;
  AgsPort *port;
  
  GValue *value;
  
  copy_pattern_audio = (AgsCopyPatternAudio *) g_object_new(AGS_TYPE_COPY_PATTERN_AUDIO,
							    "audio", audio,
							    NULL);

  /* apply bank index */
  value = g_new0(GValue,
		 1);
  g_value_init(value, G_TYPE_UINT64);

  g_object_get(copy_pattern_audio,
	       "bank-index-0" , &port,
	       NULL);
  
  g_value_set_uint64(value, bank_index_0);
  ags_port_safe_write(port,
		      value);

  g_object_get(copy_pattern_audio,
	       "bank-index-1" , &port,
	       NULL);

  g_value_set_uint64(value, bank_index_1);
  ags_port_safe_write(copy_pattern_audio->bank_index_1,
		      value);

  g_value_unset(value);
  g_free(value);
  
  return(copy_pattern_audio);
}
