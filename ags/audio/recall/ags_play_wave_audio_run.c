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

#include <ags/audio/recall/ags_play_wave_audio_run.h>
#include <ags/audio/recall/ags_play_wave_audio.h>

#include <ags/libags.h>

#include <ags/audio/ags_recall_id.h>
#include <ags/audio/ags_recall_container.h>

#include <ags/audio/recall/ags_delay_audio.h>
#include <ags/audio/recall/ags_delay_audio_run.h>

#include <ags/audio/thread/ags_audio_loop.h>
#include <ags/audio/thread/ags_soundcard_thread.h>

#include <ags/i18n.h>

void ags_play_wave_audio_run_class_init(AgsPlayWaveAudioRunClass *play_wave_audio_run);
void ags_play_wave_audio_run_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_play_wave_audio_run_dynamic_connectable_interface_init(AgsDynamicConnectableInterface *dynamic_connectable);
void ags_play_wave_audio_run_plugin_interface_init(AgsPluginInterface *plugin);
void ags_play_wave_audio_run_init(AgsPlayWaveAudioRun *play_wave_audio_run);
void ags_play_wave_audio_run_set_property(GObject *gobject,
					  guint prop_id,
					  const GValue *value,
					  GParamSpec *param_spec);
void ags_play_wave_audio_run_get_property(GObject *gobject,
					  guint prop_id,
					  GValue *value,
					  GParamSpec *param_spec);
void ags_play_wave_audio_run_dispose(GObject *gobject);
void ags_play_wave_audio_run_finalize(GObject *gobject);
void ags_play_wave_audio_run_connect(AgsConnectable *connectable);
void ags_play_wave_audio_run_disconnect(AgsConnectable *connectable);
void ags_play_wave_audio_run_connect_dynamic(AgsDynamicConnectable *dynamic_connectable);
void ags_play_wave_audio_run_disconnect_dynamic(AgsDynamicConnectable *dynamic_connectable); 
void ags_play_wave_audio_run_read(AgsFile *file, xmlNode *node, AgsPlugin *plugin);
xmlNode* ags_play_wave_audio_run_write(AgsFile *file, xmlNode *parent, AgsPlugin *plugin);

void ags_play_wave_audio_run_resolve_dependencies(AgsRecall *recall);
AgsRecall* ags_play_wave_audio_run_duplicate(AgsRecall *recall,
					     AgsRecallID *recall_id,
					     guint *n_params, GParameter *parameter);

void ags_play_wave_audio_run_alloc_input_callback(AgsDelayAudioRun *delay_audio_run,
						  guint nth_run,
						  gdouble delay, guint attack,
						  AgsPlayWaveAudioRun *play_wave_audio_run);

void ags_play_wave_audio_run_write_resolve_dependency(AgsFileLookup *file_lookup,
						      GObject *recall);
void ags_play_wave_audio_run_read_resolve_dependency(AgsFileLookup *file_lookup,
						     GObject *recall);

/**
 * SECTION:ags_play_wave_audio_run
 * @short_description: play wave
 * @title: AgsPlayWaveAudioRun
 * @section_id:
 * @include: ags/audio/recall/ags_play_wave_audio_run.h
 *
 * The #AgsPlayWaveAudioRun class play wave.
 */

enum{
  PROP_0,
  PROP_DELAY_AUDIO_RUN,
  PROP_COUNT_BEATS_AUDIO_RUN,
  PROP_WAVE,
};

static gpointer ags_play_wave_audio_run_parent_class = NULL;
static AgsConnectableInterface* ags_play_wave_audio_run_parent_connectable_interface;
static AgsDynamicConnectableInterface *ags_play_wave_audio_run_parent_dynamic_connectable_interface;
static AgsPluginInterface *ags_play_wave_audio_run_parent_plugin_interface;

GType
ags_play_wave_audio_run_get_type()
{
  static GType ags_type_play_wave_audio_run = 0;

  if(!ags_type_play_wave_audio_run){
    static const GTypeInfo ags_play_wave_audio_run_info = {
      sizeof (AgsPlayWaveAudioRunClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_play_wave_audio_run_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsPlayWaveAudioRun),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_play_wave_audio_run_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_play_wave_audio_run_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    static const GInterfaceInfo ags_dynamic_connectable_interface_info = {
      (GInterfaceInitFunc) ags_play_wave_audio_run_dynamic_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    static const GInterfaceInfo ags_plugin_interface_info = {
      (GInterfaceInitFunc) ags_play_wave_audio_run_plugin_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_play_wave_audio_run = g_type_register_static(AGS_TYPE_RECALL_AUDIO_RUN,
							  "AgsPlayWaveAudioRun",
							  &ags_play_wave_audio_run_info,
							  0);

    g_type_add_interface_static(ags_type_play_wave_audio_run,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);

    g_type_add_interface_static(ags_type_play_wave_audio_run,
				AGS_TYPE_DYNAMIC_CONNECTABLE,
				&ags_dynamic_connectable_interface_info);

    g_type_add_interface_static(ags_type_play_wave_audio_run,
				AGS_TYPE_PLUGIN,
				&ags_plugin_interface_info);
  }

  return (ags_type_play_wave_audio_run);
}

void
ags_play_wave_audio_run_class_init(AgsPlayWaveAudioRunClass *play_wave_audio_run)
{
  GObjectClass *gobject;
  AgsRecallClass *recall;
  GParamSpec *param_spec;

  ags_play_wave_audio_run_parent_class = g_type_class_peek_parent(play_wave_audio_run);

  /* GObjectClass */
  gobject = (GObjectClass *) play_wave_audio_run;

  gobject->set_property = ags_play_wave_audio_run_set_property;
  gobject->get_property = ags_play_wave_audio_run_get_property;

  gobject->dispose = ags_play_wave_audio_run_dispose;
  gobject->finalize = ags_play_wave_audio_run_finalize;

  /* properties */
  /**
   * AgsPlayWaveAudioRun:delay-audio-run:
   *
   * The delay audio run dependency.
   * 
   * Since: 1.5.0
   */
  param_spec = g_param_spec_object("delay-audio-run",
				   i18n_pspec("assigned AgsDelayAudioRun"),
				   i18n_pspec("the AgsDelayAudioRun which emits wave_alloc_input signal"),
				   AGS_TYPE_DELAY_AUDIO_RUN,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_DELAY_AUDIO_RUN,
				  param_spec);

  /**
   * AgsPlayWaveAudioRun:count-beats-audio-run:
   *
   * The count beats audio run dependency.
   * 
   * Since: 1.5.0
   */
  param_spec = g_param_spec_object("count-beats-audio-run",
				   i18n_pspec("assigned AgsCountBeatsAudioRun"),
				   i18n_pspec("the AgsCountBeatsAudioRun which just counts"),
				   AGS_TYPE_COUNT_BEATS_AUDIO_RUN,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_COUNT_BEATS_AUDIO_RUN,
				  param_spec);

  /**
   * AgsPlayWaveAudioRun:wave:
   *
   * The wave containing the notes.
   * 
   * Since: 1.5.0
   */
  param_spec = g_param_spec_object("wave",
				   i18n_pspec("assigned AgsWave"),
				   i18n_pspec("The AgsWave containing notes"),
				   AGS_TYPE_WAVE,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_WAVE,
				  param_spec);

  /* AgsRecallClass */
  recall = (AgsRecallClass *) play_wave_audio_run;

  recall->resolve_dependencies = ags_play_wave_audio_run_resolve_dependencies;
  recall->duplicate = ags_play_wave_audio_run_duplicate;
}

void
ags_play_wave_audio_run_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_play_wave_audio_run_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_play_wave_audio_run_connect;
  connectable->disconnect = ags_play_wave_audio_run_disconnect;
}

void
ags_play_wave_audio_run_dynamic_connectable_interface_init(AgsDynamicConnectableInterface *dynamic_connectable)
{
  ags_play_wave_audio_run_parent_dynamic_connectable_interface = g_type_interface_peek_parent(dynamic_connectable);

  dynamic_connectable->connect_dynamic = ags_play_wave_audio_run_connect_dynamic;
  dynamic_connectable->disconnect_dynamic = ags_play_wave_audio_run_disconnect_dynamic;
}

void
ags_play_wave_audio_run_plugin_interface_init(AgsPluginInterface *plugin)
{
  ags_play_wave_audio_run_parent_plugin_interface = g_type_interface_peek_parent(plugin);
  
  plugin->read = ags_play_wave_audio_run_read;
  plugin->write = ags_play_wave_audio_run_write;
}

void
ags_play_wave_audio_run_init(AgsPlayWaveAudioRun *play_wave_audio_run)
{
  AGS_RECALL(play_wave_audio_run)->name = "ags-play-wave";
  AGS_RECALL(play_wave_audio_run)->version = AGS_RECALL_DEFAULT_VERSION;
  AGS_RECALL(play_wave_audio_run)->build_id = AGS_RECALL_DEFAULT_BUILD_ID;
  AGS_RECALL(play_wave_audio_run)->xml_type = "ags-play-wave-audio-run";
  AGS_RECALL(play_wave_audio_run)->port = NULL;

  play_wave_audio_run->delay_audio_run = NULL;
  play_wave_audio_run->count_beats_audio_run = NULL;

  play_wave_audio_run->wave = NULL;

  play_wave_audio_run->timestamp = ags_timestamp_new();

  play_wave_audio_run->timestamp->flags &= (~AGS_TIMESTAMP_UNIX);
  play_wave_audio_run->timestamp->flags |= AGS_TIMESTAMP_OFFSET;

  play_wave_audio_run->timestamp->timer.ags_offset.offset = 0;
}

void
ags_play_wave_audio_run_set_property(GObject *gobject,
				     guint prop_id,
				     const GValue *value,
				     GParamSpec *param_spec)
{
  AgsPlayWaveAudioRun *play_wave_audio_run;

  play_wave_audio_run = AGS_PLAY_WAVE_AUDIO_RUN(gobject);

  switch(prop_id){
  case PROP_DELAY_AUDIO_RUN:
    {
      AgsDelayAudioRun *delay_audio_run;
      gboolean is_template;

      delay_audio_run = g_value_get_object(value);

      if(delay_audio_run == play_wave_audio_run->delay_audio_run){
	return;
      }

      if((AGS_RECALL_TEMPLATE & (AGS_RECALL(play_wave_audio_run)->flags)) != 0){
	is_template = TRUE;
      }else{
	is_template = FALSE;
      }

      if(play_wave_audio_run->delay_audio_run != NULL){
	if(is_template){
	  ags_recall_remove_dependency(AGS_RECALL(play_wave_audio_run),
				       (AgsRecall *) play_wave_audio_run->delay_audio_run);
	}else{
	  if((AGS_RECALL_DYNAMIC_CONNECTED & (AGS_RECALL(play_wave_audio_run)->flags)) != 0){
	    g_object_disconnect(G_OBJECT(play_wave_audio_run->delay_audio_run),
				"any_signal::wave-alloc-input",
				G_CALLBACK(ags_play_wave_audio_run_alloc_input_callback),
				play_wave_audio_run,
				NULL);
	  }
	}

	g_object_unref(G_OBJECT(play_wave_audio_run->delay_audio_run));
      }

      if(delay_audio_run != NULL){
	g_object_ref(delay_audio_run);

	if(is_template){
	  ags_recall_add_dependency(AGS_RECALL(play_wave_audio_run),
				    ags_recall_dependency_new((GObject *) delay_audio_run));
	}else{
	  if((AGS_RECALL_DYNAMIC_CONNECTED & (AGS_RECALL(play_wave_audio_run)->flags)) != 0){
	    g_signal_connect(G_OBJECT(delay_audio_run), "wave-alloc-input",
			     G_CALLBACK(ags_play_wave_audio_run_alloc_input_callback), play_wave_audio_run);
	  }
	}
      }

      play_wave_audio_run->delay_audio_run = delay_audio_run;
    }
    break;
  case PROP_COUNT_BEATS_AUDIO_RUN:
    {
      AgsCountBeatsAudioRun *count_beats_audio_run;
      gboolean is_template;

      count_beats_audio_run = g_value_get_object(value);

      if(count_beats_audio_run == play_wave_audio_run->count_beats_audio_run){
	return;
      }

      if((AGS_RECALL_TEMPLATE & (AGS_RECALL(play_wave_audio_run)->flags)) != 0){
	is_template = TRUE;
      }else{
	is_template = FALSE;
      }

      if(play_wave_audio_run->count_beats_audio_run != NULL){
	if(is_template){
	  ags_recall_remove_dependency(AGS_RECALL(play_wave_audio_run),
				       (AgsRecall *) play_wave_audio_run->count_beats_audio_run);
	}

	g_object_unref(G_OBJECT(play_wave_audio_run->count_beats_audio_run));
      }

      if(count_beats_audio_run != NULL){
	g_object_ref(count_beats_audio_run);

	if(is_template){
	  ags_recall_add_dependency(AGS_RECALL(play_wave_audio_run),
				    ags_recall_dependency_new((GObject *) count_beats_audio_run));
	}
      }

      play_wave_audio_run->count_beats_audio_run = count_beats_audio_run;
    }
    break;
  case PROP_WAVE:
    {
      AgsWave *wave;

      wave = (AgsWave *) g_value_get_object(value);

      if(play_wave_audio_run->wave == wave){
	return;
      }

      if(play_wave_audio_run->wave != NULL){
	g_object_unref(play_wave_audio_run->wave);
      }

      if(wave != NULL){
	g_object_ref(wave);
      }

      play_wave_audio_run->wave = wave;
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  };
}

void
ags_play_wave_audio_run_get_property(GObject *gobject,
				     guint prop_id,
				     GValue *value,
				     GParamSpec *param_spec)
{
  AgsPlayWaveAudioRun *play_wave_audio_run;
  
  play_wave_audio_run = AGS_PLAY_WAVE_AUDIO_RUN(gobject);

  switch(prop_id){
  case PROP_DELAY_AUDIO_RUN:
    {
      g_value_set_object(value, G_OBJECT(play_wave_audio_run->delay_audio_run));
    }
    break;
  case PROP_COUNT_BEATS_AUDIO_RUN:
    {
      g_value_set_object(value, G_OBJECT(play_wave_audio_run->count_beats_audio_run));
    }
    break;
  case PROP_WAVE:
    {
      g_value_set_object(value, play_wave_audio_run->wave);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  };
}

void
ags_play_wave_audio_run_dispose(GObject *gobject)
{
  AgsPlayWaveAudioRun *play_wave_audio_run;

  play_wave_audio_run = AGS_PLAY_WAVE_AUDIO_RUN(gobject);

  /* delay audio run */
  if(play_wave_audio_run->delay_audio_run != NULL){
    g_object_unref(G_OBJECT(play_wave_audio_run->delay_audio_run));

    play_wave_audio_run->delay_audio_run = NULL;
  }

  /* count beats audio run */
  if(play_wave_audio_run->count_beats_audio_run != NULL){
    g_object_unref(G_OBJECT(play_wave_audio_run->count_beats_audio_run));

    play_wave_audio_run->count_beats_audio_run = NULL;
  }

  /* wave */
  if(play_wave_audio_run->wave != NULL){
    g_object_unref(G_OBJECT(play_wave_audio_run->wave));

    play_wave_audio_run->wave = NULL;
  }

  /* call parent */
  G_OBJECT_CLASS(ags_play_wave_audio_run_parent_class)->dispose(gobject);
}

void
ags_play_wave_audio_run_finalize(GObject *gobject)
{
  AgsPlayWaveAudioRun *play_wave_audio_run;

  play_wave_audio_run = AGS_PLAY_WAVE_AUDIO_RUN(gobject);

  /* delay audio run */
  if(play_wave_audio_run->delay_audio_run != NULL){
    g_object_unref(G_OBJECT(play_wave_audio_run->delay_audio_run));
  }

  /* count beats audio run */
  if(play_wave_audio_run->count_beats_audio_run != NULL){
    g_object_unref(G_OBJECT(play_wave_audio_run->count_beats_audio_run));
  }

  /* wave */
  if(play_wave_audio_run->wave != NULL){
    g_object_unref(G_OBJECT(play_wave_audio_run->wave));
  }

  /* timestamp */
  if(play_wave_audio_run->timestamp != NULL){
    g_object_unref(G_OBJECT(play_wave_audio_run->timestamp));
  }

  /* call parent */
  G_OBJECT_CLASS(ags_play_wave_audio_run_parent_class)->finalize(gobject);
}

void
ags_play_wave_audio_run_connect(AgsConnectable *connectable)
{
  if((AGS_RECALL_CONNECTED & (AGS_RECALL(connectable)->flags)) != 0){
    return;
  }

  /* call parent */
  ags_play_wave_audio_run_parent_connectable_interface->connect(connectable);
}

void
ags_play_wave_audio_run_disconnect(AgsConnectable *connectable)
{
  if((AGS_RECALL_CONNECTED & (AGS_RECALL(connectable)->flags)) == 0){
    return;
  }

  /* call parent */
  ags_play_wave_audio_run_parent_connectable_interface->disconnect(connectable);
}

void
ags_play_wave_audio_run_connect_dynamic(AgsDynamicConnectable *dynamic_connectable)
{
  AgsPlayWaveAudioRun *play_wave_audio_run;

  if((AGS_RECALL_DYNAMIC_CONNECTED & (AGS_RECALL(dynamic_connectable)->flags)) != 0){
    return;
  }

  play_wave_audio_run = AGS_PLAY_WAVE_AUDIO_RUN(dynamic_connectable);

  /* call parent */
  ags_play_wave_audio_run_parent_dynamic_connectable_interface->connect_dynamic(dynamic_connectable);

  /* connect */
  g_signal_connect(G_OBJECT(play_wave_audio_run->delay_audio_run), "wave-alloc-input",
		   G_CALLBACK(ags_play_wave_audio_run_alloc_input_callback), play_wave_audio_run);  
}

void
ags_play_wave_audio_run_disconnect_dynamic(AgsDynamicConnectable *dynamic_connectable)
{
  AgsPlayWaveAudioRun *play_wave_audio_run;

  if((AGS_RECALL_DYNAMIC_CONNECTED & (AGS_RECALL(dynamic_connectable)->flags)) == 0){
    return;
  }

  /* call parent */
  ags_play_wave_audio_run_parent_dynamic_connectable_interface->disconnect_dynamic(dynamic_connectable);

  play_wave_audio_run = AGS_PLAY_WAVE_AUDIO_RUN(dynamic_connectable);

  if(play_wave_audio_run->delay_audio_run != NULL){
    g_object_disconnect(G_OBJECT(play_wave_audio_run->delay_audio_run),
			"any_signal::wave-alloc-input",
			G_CALLBACK(ags_play_wave_audio_run_alloc_input_callback),
			play_wave_audio_run,
			NULL);
  }
}

void
ags_play_wave_audio_run_read(AgsFile *file, xmlNode *node, AgsPlugin *plugin)
{
  AgsFileLookup *file_lookup;

  xmlNode *iter;

  /* read parent */
  ags_play_wave_audio_run_parent_plugin_interface->read(file, node, plugin);

  /* read depenendency */
  iter = node->children;

  while(iter != NULL){
    if(iter->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp(iter->name,
		     "ags-dependency-list",
		     19)){
	xmlNode *dependency_node;

	dependency_node = iter->children;

	while(dependency_node != NULL){
	  if(dependency_node->type == XML_ELEMENT_NODE){
	    if(!xmlStrncmp(dependency_node->name,
			   "ags-dependency",
			   15)){
	      file_lookup = (AgsFileLookup *) g_object_new(AGS_TYPE_FILE_LOOKUP,
							   "file", file,
							   "node", dependency_node,
							   "reference", G_OBJECT(plugin),
							   NULL);
	      ags_file_add_lookup(file, (GObject *) file_lookup);
	      g_signal_connect(G_OBJECT(file_lookup), "resolve",
			       G_CALLBACK(ags_play_wave_audio_run_read_resolve_dependency), G_OBJECT(plugin));
	    }
	  }
	  
	  dependency_node = dependency_node->next;
	}
      }
    }

    iter = iter->next;
  }
}

xmlNode*
ags_play_wave_audio_run_write(AgsFile *file, xmlNode *parent, AgsPlugin *plugin)
{
  AgsFileLookup *file_lookup;

  xmlNode *node, *child;
  xmlNode *dependency_node;
  
  GList *list;

  gchar *id;

  /* write parent */
  node = ags_play_wave_audio_run_parent_plugin_interface->write(file, parent, plugin);

  /* write dependencies */
  child = xmlNewNode(NULL,
		     "ags-dependency-list");

  xmlNewProp(child,
	     AGS_FILE_ID_PROP,
	     ags_id_generator_create_uuid());

  xmlAddChild(node,
	      child);

  list = AGS_RECALL(plugin)->dependencies;

  while(list != NULL){
    id = ags_id_generator_create_uuid();

    dependency_node = xmlNewNode(NULL,
				 "ags-dependency");

    xmlNewProp(dependency_node,
	       AGS_FILE_ID_PROP,
	       id);

    xmlAddChild(child,
		dependency_node);

    file_lookup = (AgsFileLookup *) g_object_new(AGS_TYPE_FILE_LOOKUP,
						 "file", file,
						 "node", dependency_node,
						 "reference", list->data,
						 NULL);
    ags_file_add_lookup(file, (GObject *) file_lookup);
    g_signal_connect(G_OBJECT(file_lookup), "resolve",
		     G_CALLBACK(ags_play_wave_audio_run_write_resolve_dependency), G_OBJECT(plugin));

    list = list->next;
  }

  return(node);
}

void
ags_play_wave_audio_run_resolve_dependencies(AgsRecall *recall)
{
  AgsRecall *template;
  AgsRecallID *recall_id;
  AgsRecallContainer *recall_container;

  AgsRecallDependency *recall_dependency;
  AgsDelayAudioRun *delay_audio_run;
  AgsCountBeatsAudioRun *count_beats_audio_run;

  GList *list;

  guint i, i_stop;
  
  recall_container = AGS_RECALL_CONTAINER(recall->container);
  
  list = ags_recall_find_template(recall_container->recall_audio_run);

  if(list != NULL){
    template = AGS_RECALL(list->data);
  }else{
    g_warning("AgsRecallClass::resolve - missing dependency");
    return;
  }

  list = template->dependencies;
  delay_audio_run = NULL;
  count_beats_audio_run = NULL;
  i_stop = 2;

  for(i = 0; i < i_stop && list != NULL;){
    recall_dependency = AGS_RECALL_DEPENDENCY(list->data);

    if(AGS_IS_DELAY_AUDIO_RUN(recall_dependency->dependency)){
      if(((AGS_RECALL_INPUT_ORIENTATED & (recall->flags)) != 0 &&
	  (AGS_RECALL_INPUT_ORIENTATED & (AGS_RECALL(recall_dependency->dependency)->flags)) != 0) ||
	 ((AGS_RECALL_OUTPUT_ORIENTATED & (recall->flags)) != 0 &&
	  (AGS_RECALL_OUTPUT_ORIENTATED & (AGS_RECALL(recall_dependency->dependency)->flags)) != 0)){
	recall_id = recall->recall_id;
      }else{
	recall_id = (AgsRecallID *) recall->recall_id->recycling_context->parent->recall_id;
      }

      delay_audio_run = (AgsDelayAudioRun *) ags_recall_dependency_resolve(recall_dependency, recall_id);

      i++;
    }else if(AGS_IS_COUNT_BEATS_AUDIO_RUN(recall_dependency->dependency)){
      if(((AGS_RECALL_INPUT_ORIENTATED & (recall->flags)) != 0 &&
	  (AGS_RECALL_INPUT_ORIENTATED & (AGS_RECALL(recall_dependency->dependency)->flags)) != 0) ||
	 ((AGS_RECALL_OUTPUT_ORIENTATED & (recall->flags)) != 0 &&
	  (AGS_RECALL_OUTPUT_ORIENTATED & (AGS_RECALL(recall_dependency->dependency)->flags)) != 0)){
	recall_id = recall->recall_id;
      }else{
	recall_id = (AgsRecallID *) recall->recall_id->recycling_context->parent->recall_id;
      }

      count_beats_audio_run = (AgsCountBeatsAudioRun *) ags_recall_dependency_resolve(recall_dependency, recall_id);

      i++;
    }

    list = list->next;
  }

  g_object_set(G_OBJECT(recall),
	       "delay-audio-run", delay_audio_run,
	       "count-beats-audio-run", count_beats_audio_run,
	       NULL);
}

AgsRecall*
ags_play_wave_audio_run_duplicate(AgsRecall *recall,
				  AgsRecallID *recall_id,
				  guint *n_params, GParameter *parameter)
{
  AgsPlayWaveAudioRun *copy;

  copy = AGS_PLAY_WAVE_AUDIO_RUN(AGS_RECALL_CLASS(ags_play_wave_audio_run_parent_class)->duplicate(recall,
												   recall_id,
												   n_params, parameter));

  return((AgsRecall *) copy);
}

void
ags_play_wave_audio_run_alloc_input_callback(AgsDelayAudioRun *delay_audio_run,
					     guint nth_run,
					     gdouble delay, guint attack,
					     AgsPlayWaveAudioRun *play_wave_audio_run)
{
  //TODO:JK: implement me
}

void
ags_play_wave_audio_run_write_resolve_dependency(AgsFileLookup *file_lookup,
						 GObject *recall)
{
  AgsFileIdRef *id_ref;
  gchar *id;

  id_ref = (AgsFileIdRef *) ags_file_find_id_ref_by_reference(file_lookup->file,
							      AGS_RECALL_DEPENDENCY(file_lookup->ref)->dependency);

  id = xmlGetProp(id_ref->node, AGS_FILE_ID_PROP);

  xmlNewProp(file_lookup->node,
	     "xpath",
  	     g_strdup_printf("xpath=//*[@id='%s']", id));
}

void
ags_play_wave_audio_run_read_resolve_dependency(AgsFileLookup *file_lookup,
						GObject *recall)
{
  AgsFileIdRef *id_ref;
  gchar *xpath;

  xpath = (gchar *) xmlGetProp(file_lookup->node,
			       "xpath");

  id_ref = (AgsFileIdRef *) ags_file_find_id_ref_by_xpath(file_lookup->file, xpath);

  if(AGS_IS_DELAY_AUDIO_RUN(id_ref->ref)){
    g_object_set(G_OBJECT(recall),
		 "delay-audio-run", id_ref->ref,
		 NULL);
  }else if(AGS_IS_COUNT_BEATS_AUDIO_RUN(id_ref->ref)){
    g_object_set(G_OBJECT(recall),
		 "count-beats-audio-run", id_ref->ref,
		 NULL);
  }
}

/**
 * ags_play_wave_audio_run_new:
 *
 * Creates an #AgsPlayWaveAudioRun
 *
 * Returns: a new #AgsPlayWaveAudioRun
 *
 * Since: 1.5.0
 */
AgsPlayWaveAudioRun*
ags_play_wave_audio_run_new()
{
  AgsPlayWaveAudioRun *play_wave_audio_run;

  play_wave_audio_run = (AgsPlayWaveAudioRun *) g_object_new(AGS_TYPE_PLAY_WAVE_AUDIO_RUN,
							     NULL);

  return(play_wave_audio_run);
}
