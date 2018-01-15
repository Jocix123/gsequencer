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

#include <ags/audio/recall/ags_play_notation_audio_run.h>
#include <ags/audio/recall/ags_play_notation_audio.h>

#include <ags/libags.h>

#include <ags/audio/ags_recall_id.h>
#include <ags/audio/ags_recall_container.h>

#include <ags/audio/recall/ags_delay_audio.h>
#include <ags/audio/recall/ags_delay_audio_run.h>

#include <ags/audio/thread/ags_audio_loop.h>
#include <ags/audio/thread/ags_soundcard_thread.h>

#include <ags/i18n.h>

void ags_play_notation_audio_run_class_init(AgsPlayNotationAudioRunClass *play_notation_audio_run);
void ags_play_notation_audio_run_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_play_notation_audio_run_dynamic_connectable_interface_init(AgsDynamicConnectableInterface *dynamic_connectable);
void ags_play_notation_audio_run_plugin_interface_init(AgsPluginInterface *plugin);
void ags_play_notation_audio_run_init(AgsPlayNotationAudioRun *play_notation_audio_run);
void ags_play_notation_audio_run_set_property(GObject *gobject,
					      guint prop_id,
					      const GValue *value,
					      GParamSpec *param_spec);
void ags_play_notation_audio_run_get_property(GObject *gobject,
					      guint prop_id,
					      GValue *value,
					      GParamSpec *param_spec);
void ags_play_notation_audio_run_dispose(GObject *gobject);
void ags_play_notation_audio_run_finalize(GObject *gobject);
void ags_play_notation_audio_run_connect(AgsConnectable *connectable);
void ags_play_notation_audio_run_disconnect(AgsConnectable *connectable);
void ags_play_notation_audio_run_connect_dynamic(AgsDynamicConnectable *dynamic_connectable);
void ags_play_notation_audio_run_disconnect_dynamic(AgsDynamicConnectable *dynamic_connectable); 
void ags_play_notation_audio_run_read(AgsFile *file, xmlNode *node, AgsPlugin *plugin);
xmlNode* ags_play_notation_audio_run_write(AgsFile *file, xmlNode *parent, AgsPlugin *plugin);

void ags_play_notation_audio_run_resolve_dependencies(AgsRecall *recall);
AgsRecall* ags_play_notation_audio_run_duplicate(AgsRecall *recall,
						 AgsRecallID *recall_id,
						 guint *n_params, GParameter *parameter);

void ags_play_notation_audio_run_alloc_input_callback(AgsDelayAudioRun *delay_audio_run,
						      guint nth_run,
						      gdouble delay, guint attack,
						      AgsPlayNotationAudioRun *play_notation_audio_run);

void ags_play_notation_audio_run_write_resolve_dependency(AgsFileLookup *file_lookup,
							GObject *recall);
void ags_play_notation_audio_run_read_resolve_dependency(AgsFileLookup *file_lookup,
						       GObject *recall);

/**
 * SECTION:ags_play_notation_audio_run
 * @short_description: play notation
 * @title: AgsPlayNotationAudioRun
 * @section_id:
 * @include: ags/audio/recall/ags_play_notation_audio_run.h
 *
 * The #AgsPlayNotationAudioRun class play notation.
 */

enum{
  PROP_0,
  PROP_DELAY_AUDIO_RUN,
  PROP_COUNT_BEATS_AUDIO_RUN,
  PROP_NOTATION,
};

static gpointer ags_play_notation_audio_run_parent_class = NULL;
static AgsConnectableInterface* ags_play_notation_audio_run_parent_connectable_interface;
static AgsDynamicConnectableInterface *ags_play_notation_audio_run_parent_dynamic_connectable_interface;
static AgsPluginInterface *ags_play_notation_audio_run_parent_plugin_interface;

GType
ags_play_notation_audio_run_get_type()
{
  static GType ags_type_play_notation_audio_run = 0;

  if(!ags_type_play_notation_audio_run){
    static const GTypeInfo ags_play_notation_audio_run_info = {
      sizeof (AgsPlayNotationAudioRunClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_play_notation_audio_run_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsPlayNotationAudioRun),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_play_notation_audio_run_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_play_notation_audio_run_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    static const GInterfaceInfo ags_dynamic_connectable_interface_info = {
      (GInterfaceInitFunc) ags_play_notation_audio_run_dynamic_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    static const GInterfaceInfo ags_plugin_interface_info = {
      (GInterfaceInitFunc) ags_play_notation_audio_run_plugin_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_play_notation_audio_run = g_type_register_static(AGS_TYPE_RECALL_AUDIO_RUN,
							      "AgsPlayNotationAudioRun",
							      &ags_play_notation_audio_run_info,
							      0);

    g_type_add_interface_static(ags_type_play_notation_audio_run,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);

    g_type_add_interface_static(ags_type_play_notation_audio_run,
				AGS_TYPE_DYNAMIC_CONNECTABLE,
				&ags_dynamic_connectable_interface_info);

    g_type_add_interface_static(ags_type_play_notation_audio_run,
				AGS_TYPE_PLUGIN,
				&ags_plugin_interface_info);
  }

  return (ags_type_play_notation_audio_run);
}

void
ags_play_notation_audio_run_class_init(AgsPlayNotationAudioRunClass *play_notation_audio_run)
{
  GObjectClass *gobject;
  AgsRecallClass *recall;
  GParamSpec *param_spec;

  ags_play_notation_audio_run_parent_class = g_type_class_peek_parent(play_notation_audio_run);

  /* GObjectClass */
  gobject = (GObjectClass *) play_notation_audio_run;

  gobject->set_property = ags_play_notation_audio_run_set_property;
  gobject->get_property = ags_play_notation_audio_run_get_property;

  gobject->dispose = ags_play_notation_audio_run_dispose;
  gobject->finalize = ags_play_notation_audio_run_finalize;

  /* properties */
  /**
   * AgsPlayNotationAudioRun:delay-audio-run:
   *
   * The delay audio run dependency.
   * 
   * Since: 1.0.0
   */
  param_spec = g_param_spec_object("delay-audio-run",
				   i18n_pspec("assigned AgsDelayAudioRun"),
				   i18n_pspec("the AgsDelayAudioRun which emits notation_alloc_input signal"),
				   AGS_TYPE_DELAY_AUDIO_RUN,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_DELAY_AUDIO_RUN,
				  param_spec);

  /**
   * AgsPlayNotationAudioRun:count-beats-audio-run:
   *
   * The count beats audio run dependency.
   * 
   * Since: 1.0.0
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
   * AgsPlayNotationAudioRun:notation:
   *
   * The notation containing the notes.
   * 
   * Since: 1.0.0
   */
  param_spec = g_param_spec_object("notation",
				   i18n_pspec("assigned AgsNotation"),
				   i18n_pspec("The AgsNotation containing notes"),
				   AGS_TYPE_NOTATION,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_NOTATION,
				  param_spec);

  /* AgsRecallClass */
  recall = (AgsRecallClass *) play_notation_audio_run;

  recall->resolve_dependencies = ags_play_notation_audio_run_resolve_dependencies;
  recall->duplicate = ags_play_notation_audio_run_duplicate;
}

void
ags_play_notation_audio_run_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_play_notation_audio_run_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_play_notation_audio_run_connect;
  connectable->disconnect = ags_play_notation_audio_run_disconnect;
}

void
ags_play_notation_audio_run_dynamic_connectable_interface_init(AgsDynamicConnectableInterface *dynamic_connectable)
{
  ags_play_notation_audio_run_parent_dynamic_connectable_interface = g_type_interface_peek_parent(dynamic_connectable);

  dynamic_connectable->connect_dynamic = ags_play_notation_audio_run_connect_dynamic;
  dynamic_connectable->disconnect_dynamic = ags_play_notation_audio_run_disconnect_dynamic;
}

void
ags_play_notation_audio_run_plugin_interface_init(AgsPluginInterface *plugin)
{
  ags_play_notation_audio_run_parent_plugin_interface = g_type_interface_peek_parent(plugin);
  
  plugin->read = ags_play_notation_audio_run_read;
  plugin->write = ags_play_notation_audio_run_write;
}

void
ags_play_notation_audio_run_init(AgsPlayNotationAudioRun *play_notation_audio_run)
{
  AGS_RECALL(play_notation_audio_run)->name = "ags-play-notation";
  AGS_RECALL(play_notation_audio_run)->version = AGS_RECALL_DEFAULT_VERSION;
  AGS_RECALL(play_notation_audio_run)->build_id = AGS_RECALL_DEFAULT_BUILD_ID;
  AGS_RECALL(play_notation_audio_run)->xml_type = "ags-play-notation-audio-run";
  AGS_RECALL(play_notation_audio_run)->port = NULL;

  play_notation_audio_run->delay_audio_run = NULL;
  play_notation_audio_run->count_beats_audio_run = NULL;

  play_notation_audio_run->notation = NULL;

  play_notation_audio_run->timestamp = ags_timestamp_new();

  play_notation_audio_run->timestamp->flags &= (~AGS_TIMESTAMP_UNIX);
  play_notation_audio_run->timestamp->flags |= AGS_TIMESTAMP_OFFSET;

  play_notation_audio_run->timestamp->timer.ags_offset.offset = 0;
}

void
ags_play_notation_audio_run_set_property(GObject *gobject,
					 guint prop_id,
					 const GValue *value,
					 GParamSpec *param_spec)
{
  AgsPlayNotationAudioRun *play_notation_audio_run;

  play_notation_audio_run = AGS_PLAY_NOTATION_AUDIO_RUN(gobject);

  switch(prop_id){
  case PROP_DELAY_AUDIO_RUN:
    {
      AgsDelayAudioRun *delay_audio_run;
      gboolean is_template;

      delay_audio_run = g_value_get_object(value);

      if(delay_audio_run == play_notation_audio_run->delay_audio_run){
	return;
      }

      if(delay_audio_run != NULL &&
	 (AGS_RECALL_TEMPLATE & (AGS_RECALL(delay_audio_run)->flags)) != 0){
	is_template = TRUE;
      }else{
	is_template = FALSE;
      }

      if(play_notation_audio_run->delay_audio_run != NULL){
	if(is_template){
	  ags_recall_remove_dependency(AGS_RECALL(play_notation_audio_run),
				       (AgsRecall *) play_notation_audio_run->delay_audio_run);
	}else{
	  if((AGS_RECALL_DYNAMIC_CONNECTED & (AGS_RECALL(play_notation_audio_run)->flags)) != 0){
	    g_object_disconnect(G_OBJECT(play_notation_audio_run->delay_audio_run),
				"any_signal::notation-alloc-input",
				G_CALLBACK(ags_play_notation_audio_run_alloc_input_callback),
				play_notation_audio_run,
				NULL);
	  }
	}

	g_object_unref(G_OBJECT(play_notation_audio_run->delay_audio_run));
      }

      if(delay_audio_run != NULL){
	g_object_ref(delay_audio_run);

	if(is_template){
	  ags_recall_add_dependency(AGS_RECALL(play_notation_audio_run),
				    ags_recall_dependency_new((GObject *) delay_audio_run));
	}else{
	  if((AGS_RECALL_DYNAMIC_CONNECTED & (AGS_RECALL(play_notation_audio_run)->flags)) != 0){
	    g_signal_connect(G_OBJECT(delay_audio_run), "notation-alloc-input",
			     G_CALLBACK(ags_play_notation_audio_run_alloc_input_callback), play_notation_audio_run);
	  }
	}
      }

      play_notation_audio_run->delay_audio_run = delay_audio_run;
    }
    break;
  case PROP_COUNT_BEATS_AUDIO_RUN:
    {
      AgsCountBeatsAudioRun *count_beats_audio_run;
      gboolean is_template;

      count_beats_audio_run = g_value_get_object(value);

      if(count_beats_audio_run == play_notation_audio_run->count_beats_audio_run){
	return;
      }

      if(count_beats_audio_run != NULL &&
	 (AGS_RECALL_TEMPLATE & (AGS_RECALL(count_beats_audio_run)->flags)) != 0){
	is_template = TRUE;
      }else{
	is_template = FALSE;
      }

      if(play_notation_audio_run->count_beats_audio_run != NULL){
	if(is_template){
	  ags_recall_remove_dependency(AGS_RECALL(play_notation_audio_run),
				       (AgsRecall *) play_notation_audio_run->count_beats_audio_run);
	}

	g_object_unref(G_OBJECT(play_notation_audio_run->count_beats_audio_run));
      }

      if(count_beats_audio_run != NULL){
	g_object_ref(count_beats_audio_run);

	if(is_template){
	  ags_recall_add_dependency(AGS_RECALL(play_notation_audio_run),
				    ags_recall_dependency_new((GObject *) count_beats_audio_run));
	}
      }

      play_notation_audio_run->count_beats_audio_run = count_beats_audio_run;
    }
    break;
  case PROP_NOTATION:
    {
      AgsNotation *notation;

      notation = (AgsNotation *) g_value_get_object(value);

      if(play_notation_audio_run->notation == notation){
	return;
      }

      if(play_notation_audio_run->notation != NULL){
	g_object_unref(play_notation_audio_run->notation);
      }

      if(notation != NULL){
	g_object_ref(notation);
      }

      play_notation_audio_run->notation = notation;
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  };
}

void
ags_play_notation_audio_run_get_property(GObject *gobject,
					 guint prop_id,
					 GValue *value,
					 GParamSpec *param_spec)
{
  AgsPlayNotationAudioRun *play_notation_audio_run;
  
  play_notation_audio_run = AGS_PLAY_NOTATION_AUDIO_RUN(gobject);

  switch(prop_id){
  case PROP_DELAY_AUDIO_RUN:
    {
      g_value_set_object(value, G_OBJECT(play_notation_audio_run->delay_audio_run));
    }
    break;
  case PROP_COUNT_BEATS_AUDIO_RUN:
    {
      g_value_set_object(value, G_OBJECT(play_notation_audio_run->count_beats_audio_run));
    }
    break;
  case PROP_NOTATION:
    {
      g_value_set_object(value, play_notation_audio_run->notation);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  };
}

void
ags_play_notation_audio_run_dispose(GObject *gobject)
{
  AgsPlayNotationAudioRun *play_notation_audio_run;

  play_notation_audio_run = AGS_PLAY_NOTATION_AUDIO_RUN(gobject);

  /* delay audio run */
  if(play_notation_audio_run->delay_audio_run != NULL){
    g_object_unref(G_OBJECT(play_notation_audio_run->delay_audio_run));

    play_notation_audio_run->delay_audio_run = NULL;
  }

  /* count beats audio run */
  if(play_notation_audio_run->count_beats_audio_run != NULL){
    g_object_unref(G_OBJECT(play_notation_audio_run->count_beats_audio_run));

    play_notation_audio_run->count_beats_audio_run = NULL;
  }

  /* notation */
  if(play_notation_audio_run->notation != NULL){
    g_object_unref(G_OBJECT(play_notation_audio_run->notation));

    play_notation_audio_run->notation = NULL;
  }

  /* call parent */
  G_OBJECT_CLASS(ags_play_notation_audio_run_parent_class)->dispose(gobject);
}

void
ags_play_notation_audio_run_finalize(GObject *gobject)
{
  AgsPlayNotationAudioRun *play_notation_audio_run;

  play_notation_audio_run = AGS_PLAY_NOTATION_AUDIO_RUN(gobject);

  /* delay audio run */
  if(play_notation_audio_run->delay_audio_run != NULL){
    g_object_unref(G_OBJECT(play_notation_audio_run->delay_audio_run));
  }

  /* count beats audio run */
  if(play_notation_audio_run->count_beats_audio_run != NULL){
    g_object_unref(G_OBJECT(play_notation_audio_run->count_beats_audio_run));
  }

  /* notation */
  if(play_notation_audio_run->notation != NULL){
    g_object_unref(G_OBJECT(play_notation_audio_run->notation));
  }

  /* timestamp */
  if(play_notation_audio_run->timestamp != NULL){
    g_object_unref(G_OBJECT(play_notation_audio_run->timestamp));
  }

  /* call parent */
  G_OBJECT_CLASS(ags_play_notation_audio_run_parent_class)->finalize(gobject);
}

void
ags_play_notation_audio_run_connect(AgsConnectable *connectable)
{
  if((AGS_RECALL_CONNECTED & (AGS_RECALL(connectable)->flags)) != 0){
    return;
  }

  /* call parent */
  ags_play_notation_audio_run_parent_connectable_interface->connect(connectable);
}

void
ags_play_notation_audio_run_disconnect(AgsConnectable *connectable)
{
  if((AGS_RECALL_CONNECTED & (AGS_RECALL(connectable)->flags)) == 0){
    return;
  }

  /* call parent */
  ags_play_notation_audio_run_parent_connectable_interface->disconnect(connectable);
}

void
ags_play_notation_audio_run_connect_dynamic(AgsDynamicConnectable *dynamic_connectable)
{
  AgsPlayNotationAudioRun *play_notation_audio_run;

  if((AGS_RECALL_DYNAMIC_CONNECTED & (AGS_RECALL(dynamic_connectable)->flags)) != 0){
    return;
  }

  play_notation_audio_run = AGS_PLAY_NOTATION_AUDIO_RUN(dynamic_connectable);

  /* call parent */
  ags_play_notation_audio_run_parent_dynamic_connectable_interface->connect_dynamic(dynamic_connectable);

  /* connect */
  g_signal_connect(G_OBJECT(play_notation_audio_run->delay_audio_run), "notation-alloc-input",
		   G_CALLBACK(ags_play_notation_audio_run_alloc_input_callback), play_notation_audio_run);  
}

void
ags_play_notation_audio_run_disconnect_dynamic(AgsDynamicConnectable *dynamic_connectable)
{
  AgsPlayNotationAudioRun *play_notation_audio_run;

  if((AGS_RECALL_DYNAMIC_CONNECTED & (AGS_RECALL(dynamic_connectable)->flags)) == 0){
    return;
  }

  /* call parent */
  ags_play_notation_audio_run_parent_dynamic_connectable_interface->disconnect_dynamic(dynamic_connectable);

  play_notation_audio_run = AGS_PLAY_NOTATION_AUDIO_RUN(dynamic_connectable);

  if(play_notation_audio_run->delay_audio_run != NULL){
    g_object_disconnect(G_OBJECT(play_notation_audio_run->delay_audio_run),
			"any_signal::notation-alloc-input",
			G_CALLBACK(ags_play_notation_audio_run_alloc_input_callback),
			play_notation_audio_run,
			NULL);
  }
}

void
ags_play_notation_audio_run_read(AgsFile *file, xmlNode *node, AgsPlugin *plugin)
{
  AgsFileLookup *file_lookup;

  xmlNode *iter;

  /* read parent */
  ags_play_notation_audio_run_parent_plugin_interface->read(file, node, plugin);

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
			       G_CALLBACK(ags_play_notation_audio_run_read_resolve_dependency), G_OBJECT(plugin));
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
ags_play_notation_audio_run_write(AgsFile *file, xmlNode *parent, AgsPlugin *plugin)
{
  AgsFileLookup *file_lookup;

  xmlNode *node, *child;
  xmlNode *dependency_node;
  
  GList *list;

  gchar *id;

  /* write parent */
  node = ags_play_notation_audio_run_parent_plugin_interface->write(file, parent, plugin);

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
		     G_CALLBACK(ags_play_notation_audio_run_write_resolve_dependency), G_OBJECT(plugin));

    list = list->next;
  }

  return(node);
}

void
ags_play_notation_audio_run_resolve_dependencies(AgsRecall *recall)
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
ags_play_notation_audio_run_duplicate(AgsRecall *recall,
				      AgsRecallID *recall_id,
				      guint *n_params, GParameter *parameter)
{
  AgsPlayNotationAudioRun *copy;

  copy = AGS_PLAY_NOTATION_AUDIO_RUN(AGS_RECALL_CLASS(ags_play_notation_audio_run_parent_class)->duplicate(recall,
													   recall_id,
													   n_params, parameter));

  return((AgsRecall *) copy);
}

void
ags_play_notation_audio_run_alloc_input_callback(AgsDelayAudioRun *delay_audio_run,
						 guint nth_run,
						 gdouble delay, guint attack,
						 AgsPlayNotationAudioRun *play_notation_audio_run)
{
  GObject *soundcard;
  AgsAudio *audio;
  AgsChannel *output, *input;
  AgsChannel *selected_channel, *channel, *next_pad;
  AgsRecycling *recycling;
  AgsAudioSignal *audio_signal;
  AgsNotation *notation;
  AgsNote *note;

  AgsPlayNotationAudio *play_notation_audio;
  AgsDelayAudio *delay_audio;
  
  AgsMutexManager *mutex_manager;

  AgsConfig *config;
  
  GList *current_position;
  GList *list;

  gchar *str;

  guint notation_counter;
  guint input_pads;
  guint audio_channel;
  guint samplerate;
  guint i;
  
  pthread_mutex_t *application_mutex;
  pthread_mutex_t *audio_mutex;
  pthread_mutex_t *channel_mutex;
  pthread_mutex_t *recycling_mutex;

  auto void ags_play_notation_audio_run_alloc_input_callback_play_notation(AgsNotation *notation);

  void ags_play_notation_audio_run_alloc_input_callback_play_notation(AgsNotation *notation)
  {
    pthread_mutex_lock(audio_mutex);

    current_position = notation->notes;

    pthread_mutex_unlock(audio_mutex);
  
    while(current_position != NULL){
      AgsRecallID *child_recall_id;
      GList *list;

      guint note_x0;
    
      pthread_mutex_lock(audio_mutex);
      
      note = AGS_NOTE(current_position->data);
      note_x0 = note->x[0];
      
      pthread_mutex_unlock(audio_mutex);
  
      if(note_x0 == notation_counter){      
	if((AGS_AUDIO_REVERSE_MAPPING & (audio->flags)) != 0){
	  selected_channel = ags_channel_pad_nth(channel, input_pads - note->y - 1);
	}else{
	  selected_channel = ags_channel_pad_nth(channel, note->y);
	}

	if(selected_channel == NULL){
	  current_position = current_position->next;
	  continue;
	}

	/* get child recall id */
	child_recall_id = NULL;
	
	list = selected_channel->recall_id;

	while(list != NULL){
	  if(AGS_RECALL_ID(list->data)->recycling_context->parent == AGS_RECALL(delay_audio_run)->recall_id->recycling_context){
	    child_recall_id = (AgsRecallID *) list->data;
	    break;
	  }
	  
	  list = list->next;
	}

	/* lookup channel mutex */
	pthread_mutex_lock(application_mutex);

	channel_mutex = ags_mutex_manager_lookup(mutex_manager,
						 (GObject *) selected_channel);
	
	pthread_mutex_unlock(application_mutex);

	/* recycling */
	pthread_mutex_lock(channel_mutex);
	
	recycling = selected_channel->first_recycling;

	pthread_mutex_unlock(channel_mutex);
	
#ifdef AGS_DEBUG	
	g_message("playing[%u|%u]: %u | %u\n", audio_channel, selected_channel->pad, note->x[0], note->y);
#endif

	while(recycling != selected_channel->last_recycling->next){
	  /* lookup recycling mutex */
	  pthread_mutex_lock(application_mutex);

	  recycling_mutex = ags_mutex_manager_lookup(mutex_manager,
						     (GObject *) recycling);
	
	  pthread_mutex_unlock(application_mutex);

	  /* create audio signal */
	  audio_signal = ags_audio_signal_new((GObject *) soundcard,
					      (GObject *) recycling,
					      (GObject *) child_recall_id);
	  g_object_set(audio_signal,
		       "note", note,
		       NULL);
	  
	  if((AGS_AUDIO_PATTERN_MODE & (audio->flags)) != 0){
	    ags_recycling_create_audio_signal_with_defaults(recycling,
							    audio_signal,
							    0.0, 0);
	  }else{
	    gdouble notation_delay;

	    GValue value = {0,};

	    /* get notation delay */
	    g_value_init(&value,
			 G_TYPE_DOUBLE);
	    ags_port_safe_read(delay_audio->notation_delay,
			       &value);

	    notation_delay = g_value_get_double(&value);
	    g_value_unset(&value);

	    /* create audio signal with frame count */
	    ags_recycling_create_audio_signal_with_frame_count(recycling,
							       audio_signal,
							       (guint) (((gdouble) samplerate / notation_delay) * (gdouble) (note->x[1] - note->x[0])),
							       0.0, 0);
	  }
	  
	  ags_connectable_connect(AGS_CONNECTABLE(audio_signal));

	  audio_signal->stream_current = audio_signal->stream_beginning;

	  /* lock and add */
	  pthread_mutex_lock(recycling_mutex);

	  ags_recycling_add_audio_signal(recycling,
					 audio_signal);
	  //	g_object_unref(audio_signal);

	  /* iterate */
	  recycling = recycling->next;

	  pthread_mutex_unlock(recycling_mutex);
	}
      }else if(note_x0 > notation_counter){
	break;
      }
    
      pthread_mutex_lock(audio_mutex);
    
      current_position = current_position->next;

      pthread_mutex_unlock(audio_mutex);
    }
  }
  
  if(delay != 0.0){
    //    g_message("d %f", delay);
    return;
  }

  play_notation_audio = AGS_PLAY_NOTATION_AUDIO(AGS_RECALL_AUDIO_RUN(play_notation_audio_run)->recall_audio);

  delay_audio = AGS_DELAY_AUDIO(AGS_RECALL_AUDIO_RUN(delay_audio_run)->recall_audio);
  
  audio = AGS_RECALL_AUDIO(play_notation_audio)->audio;

  mutex_manager = ags_mutex_manager_get_instance();
  application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);

  config = ags_config_get_instance();
  
  /* read config and audio mutex */
  pthread_mutex_lock(application_mutex);
  
  /* samplerate */
  str = ags_config_get_value(config,
			     AGS_CONFIG_SOUNDCARD,
			     "samplerate");

  if(str == NULL){
    str = ags_config_get_value(config,
			       AGS_CONFIG_SOUNDCARD_0,
			       "samplerate");
  }
  
  if(str != NULL){  
    samplerate = g_ascii_strtoull(str,
				  NULL,
				  10);
    free(str);
  }else{
    samplerate = AGS_SOUNDCARD_DEFAULT_SAMPLERATE;
  }

  /*  */
  audio_mutex = ags_mutex_manager_lookup(mutex_manager,
					 (GObject *) audio);
  
  pthread_mutex_unlock(application_mutex);

  pthread_mutex_lock(audio_mutex);

  soundcard = (GObject *) audio->soundcard;
  list = audio->notation;

  pthread_mutex_unlock(audio_mutex);
  
  if(list == NULL){
    return;
  }

  /* lookup channel mutex */
  pthread_mutex_lock(application_mutex);

  channel = (AgsChannel *) AGS_RECYCLING(AGS_RECALL(delay_audio_run)->recall_id->recycling)->channel;
  channel_mutex = ags_mutex_manager_lookup(mutex_manager,
					   (GObject *) channel);
	
  pthread_mutex_unlock(application_mutex);

  /* get audio channel */
  pthread_mutex_lock(channel_mutex);

  output = audio->output;
  input = audio->input;
  
  audio_channel = channel->audio_channel;

  pthread_mutex_unlock(channel_mutex);

  /* get channel */
  if((AGS_AUDIO_NOTATION_DEFAULT & (audio->flags)) != 0){
    channel = ags_channel_nth(input,
			      audio_channel);
  }else{
    channel = ags_channel_nth(output,
			      audio_channel);
  }
  
  /* play notation */
  notation = NULL;
  
  pthread_mutex_lock(audio_mutex);

  notation_counter = play_notation_audio_run->count_beats_audio_run->notation_counter;

  input_pads = audio->input_pads;
  
  play_notation_audio_run->timestamp->timer.ags_offset.offset = AGS_NOTATION_DEFAULT_OFFSET * floor(notation_counter / AGS_NOTATION_DEFAULT_OFFSET);
  
  list = ags_notation_find_near_timestamp(audio->notation, audio_channel,
					  play_notation_audio_run->timestamp);
  
  if(list != NULL){
    notation = list->data;
  }
  
  pthread_mutex_unlock(audio_mutex);

  if(notation != NULL){
    ags_play_notation_audio_run_alloc_input_callback_play_notation(notation);
  }  
}

void
ags_play_notation_audio_run_write_resolve_dependency(AgsFileLookup *file_lookup,
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
ags_play_notation_audio_run_read_resolve_dependency(AgsFileLookup *file_lookup,
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
 * ags_play_notation_audio_run_new:
 *
 * Creates an #AgsPlayNotationAudioRun
 *
 * Returns: a new #AgsPlayNotationAudioRun
 *
 * Since: 1.0.0
 */
AgsPlayNotationAudioRun*
ags_play_notation_audio_run_new()
{
  AgsPlayNotationAudioRun *play_notation_audio_run;

  play_notation_audio_run = (AgsPlayNotationAudioRun *) g_object_new(AGS_TYPE_PLAY_NOTATION_AUDIO_RUN,
								     NULL);

  return(play_notation_audio_run);
}
