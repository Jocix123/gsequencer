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

#include <ags/X/machine/ags_live_dssi_bridge.h>
#include <ags/X/machine/ags_live_dssi_bridge_callbacks.h>

#include <ags/libags.h>
#include <ags/libags-audio.h>
#include <ags/libags-gui.h>

#include <ags/X/ags_window.h>
#include <ags/X/ags_effect_bridge.h>
#include <ags/X/ags_effect_bulk.h>
#include <ags/X/ags_bulk_member.h>

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include <ladspa.h>
#include <dssi.h>

#include <ags/i18n.h>

void ags_live_dssi_bridge_class_init(AgsLiveDssiBridgeClass *live_dssi_bridge);
void ags_live_dssi_bridge_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_live_dssi_bridge_plugin_interface_init(AgsPluginInterface *plugin);
void ags_live_dssi_bridge_init(AgsLiveDssiBridge *live_dssi_bridge);
void ags_live_dssi_bridge_set_property(GObject *gobject,
				       guint prop_id,
				       const GValue *value,
				       GParamSpec *param_spec);
void ags_live_dssi_bridge_get_property(GObject *gobject,
				       guint prop_id,
				       GValue *value,
				       GParamSpec *param_spec);
void ags_live_dssi_bridge_finalize(GObject *gobject);

void ags_live_dssi_bridge_connect(AgsConnectable *connectable);
void ags_live_dssi_bridge_disconnect(AgsConnectable *connectable);

gchar* ags_live_dssi_bridge_get_version(AgsPlugin *plugin);
void ags_live_dssi_bridge_set_version(AgsPlugin *plugin, gchar *version);
gchar* ags_live_dssi_bridge_get_build_id(AgsPlugin *plugin);
void ags_live_dssi_bridge_set_build_id(AgsPlugin *plugin, gchar *build_id);
gchar* ags_live_dssi_bridge_get_xml_type(AgsPlugin *plugin);
void ags_live_dssi_bridge_set_xml_type(AgsPlugin *plugin, gchar *xml_type);
void ags_live_dssi_bridge_read(AgsFile *file, xmlNode *node, AgsPlugin *plugin);
void ags_live_dssi_bridge_launch_task(AgsFileLaunch *file_launch, AgsLiveDssiBridge *live_dssi_bridge);
xmlNode* ags_live_dssi_bridge_write(AgsFile *file, xmlNode *parent, AgsPlugin *plugin);

void ags_live_dssi_bridge_resize_audio_channels(AgsMachine *machine,
						guint audio_channels, guint audio_channels_old,
						gpointer data);
void ags_live_dssi_bridge_resize_pads(AgsMachine *machine, GType type,
				      guint pads, guint pads_old,
				      gpointer data);

void ags_live_dssi_bridge_map_recall(AgsMachine *machine);

/**
 * SECTION:ags_live_dssi_bridge
 * @short_description: A composite widget to visualize a bunch of #AgsChannel
 * @title: AgsLiveDssiBridge
 * @section_id:
 * @include: ags/X/machine/ags_live_dssi_bridge.h
 *
 * #AgsLiveDssiBridge is a composite widget to visualize all #AgsChannel. It should be
 * packed by an #AgsMachine.
 */

enum{
  PROP_0,
  PROP_FILENAME,
  PROP_EFFECT,
  PROP_INDEX,
};

static gpointer ags_live_dssi_bridge_parent_class = NULL;
static AgsConnectableInterface* ags_live_dssi_bridge_parent_connectable_interface;
static AgsPluginInterface* ags_live_dssi_bridge_parent_plugin_interface;

extern GHashTable *ags_effect_bulk_indicator_queue_draw;

GType
ags_live_dssi_bridge_get_type(void)
{
  static volatile gsize g_define_type_id__volatile = 0;

  if(g_once_init_enter (&g_define_type_id__volatile)){
    GType ags_type_live_dssi_bridge = 0;

    static const GTypeInfo ags_live_dssi_bridge_info = {
      sizeof(AgsLiveDssiBridgeClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_live_dssi_bridge_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof(AgsLiveDssiBridge),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_live_dssi_bridge_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_live_dssi_bridge_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    static const GInterfaceInfo ags_plugin_interface_info = {
      (GInterfaceInitFunc) ags_live_dssi_bridge_plugin_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_live_dssi_bridge = g_type_register_static(AGS_TYPE_MACHINE,
						       "AgsLiveDssiBridge", &ags_live_dssi_bridge_info,
						       0);

    g_type_add_interface_static(ags_type_live_dssi_bridge,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);

    g_type_add_interface_static(ags_type_live_dssi_bridge,
				AGS_TYPE_PLUGIN,
				&ags_plugin_interface_info);

    g_once_init_leave(&g_define_type_id__volatile, ags_type_live_dssi_bridge);
  }

  return g_define_type_id__volatile;
}

void
ags_live_dssi_bridge_class_init(AgsLiveDssiBridgeClass *live_dssi_bridge)
{
  AgsMachineClass *machine;
  GObjectClass *gobject;
  GParamSpec *param_spec;

  ags_live_dssi_bridge_parent_class = g_type_class_peek_parent(live_dssi_bridge);

  /* GObjectClass */
  gobject = G_OBJECT_CLASS(live_dssi_bridge);

  gobject->set_property = ags_live_dssi_bridge_set_property;
  gobject->get_property = ags_live_dssi_bridge_get_property;

  gobject->finalize = ags_live_dssi_bridge_finalize;
  
  /* properties */
  /**
   * AgsRecallLiveDssi:filename:
   *
   * The plugins filename.
   * 
   * Since: 2.0.0
   */
  param_spec =  g_param_spec_string("filename",
				    i18n_pspec("the object file"),
				    i18n_pspec("The filename as string of object file"),
				    NULL,
				    G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_FILENAME,
				  param_spec);

  /**
   * AgsRecallLiveDssi:effect:
   *
   * The effect's name.
   * 
   * Since: 2.0.0
   */
  param_spec =  g_param_spec_string("effect",
				    i18n_pspec("the effect"),
				    i18n_pspec("The effect's string representation"),
				    NULL,
				    G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_EFFECT,
				  param_spec);

  /**
   * AgsRecallLiveDssi:index:
   *
   * The effect's index.
   * 
   * Since: 2.0.0
   */
  param_spec =  g_param_spec_uint("index",
				  i18n_pspec("index of effect"),
				  i18n_pspec("The numerical index of effect"),
				  0,
				  65535,
				  0,
				  G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_INDEX,
				  param_spec);

  /* AgsMachine */
  machine = (AgsMachineClass *) live_dssi_bridge;

  machine->map_recall = ags_live_dssi_bridge_map_recall;
}

void
ags_live_dssi_bridge_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_live_dssi_bridge_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->is_ready = NULL;
  connectable->is_connected = NULL;
  connectable->connect = ags_live_dssi_bridge_connect;
  connectable->disconnect = ags_live_dssi_bridge_disconnect;
}

void
ags_live_dssi_bridge_plugin_interface_init(AgsPluginInterface *plugin)
{
  plugin->get_name = NULL;
  plugin->set_name = NULL;
  plugin->get_version = ags_live_dssi_bridge_get_version;
  plugin->set_version = ags_live_dssi_bridge_set_version;
  plugin->get_build_id = ags_live_dssi_bridge_get_build_id;
  plugin->set_build_id = ags_live_dssi_bridge_set_build_id;
  plugin->get_xml_type = ags_live_dssi_bridge_get_xml_type;
  plugin->set_xml_type = ags_live_dssi_bridge_set_xml_type;
  plugin->read = ags_live_dssi_bridge_read;
  plugin->write = ags_live_dssi_bridge_write;
  plugin->get_ports = NULL;
  plugin->set_ports = NULL;
}

void
ags_live_dssi_bridge_init(AgsLiveDssiBridge *live_dssi_bridge)
{
  GtkVBox *vbox;
  GtkHBox *hbox;
  GtkLabel *label;
  GtkTable *table;

  AgsAudio *audio;

  g_signal_connect_after((GObject *) live_dssi_bridge, "parent-set",
			 G_CALLBACK(ags_live_dssi_bridge_parent_set_callback), (gpointer) live_dssi_bridge);

  audio = AGS_MACHINE(live_dssi_bridge)->audio;
  ags_audio_set_flags(audio, (AGS_AUDIO_SYNC |
			      AGS_AUDIO_ASYNC |
			      AGS_AUDIO_OUTPUT_HAS_RECYCLING |
			      AGS_AUDIO_INPUT_HAS_RECYCLING |
			      AGS_AUDIO_SKIP_INPUT));
  ags_audio_set_ability_flags(audio, (AGS_SOUND_ABILITY_NOTATION));
  ags_audio_set_behaviour_flags(audio, (AGS_SOUND_BEHAVIOUR_REVERSE_MAPPING));
  //  audio->flags &= (~AGS_AUDIO_NOTATION_DEFAULT);
  
  g_object_set(audio,
	       "audio-start-mapping", 0,
	       "audio-end-mapping", 128,
	       "midi-start-mapping", 0,
	       "midi-end-mapping", 128,
	       NULL);
  
  AGS_MACHINE(live_dssi_bridge)->flags |= (AGS_MACHINE_IS_SYNTHESIZER |
					   AGS_MACHINE_REVERSE_NOTATION);

  ags_machine_popup_add_connection_options((AgsMachine *) live_dssi_bridge,
					   (AGS_MACHINE_POPUP_MIDI_DIALOG));

  g_signal_connect_after(G_OBJECT(live_dssi_bridge), "resize-audio-channels",
			 G_CALLBACK(ags_live_dssi_bridge_resize_audio_channels), NULL);

  g_signal_connect_after(G_OBJECT(live_dssi_bridge), "resize-pads",
			 G_CALLBACK(ags_live_dssi_bridge_resize_pads), NULL);
    
  live_dssi_bridge->flags = 0;

  live_dssi_bridge->name = NULL;

  live_dssi_bridge->version = AGS_LIVE_DSSI_BRIDGE_DEFAULT_VERSION;
  live_dssi_bridge->build_id = AGS_LIVE_DSSI_BRIDGE_DEFAULT_BUILD_ID;

  live_dssi_bridge->xml_type = "ags-live-dssi-bridge";
  
  live_dssi_bridge->mapped_output_pad = 0;
  live_dssi_bridge->mapped_input_pad = 0;
  
  live_dssi_bridge->filename = NULL;
  live_dssi_bridge->effect = NULL;
  live_dssi_bridge->effect_index = 0;

  live_dssi_bridge->port_values = NULL;
  live_dssi_bridge->dssi_descriptor = NULL;
  
  vbox = (GtkVBox *) gtk_vbox_new(FALSE, 0);
  gtk_container_add((GtkContainer *) gtk_bin_get_child((GtkBin *) live_dssi_bridge),
		    (GtkWidget *) vbox);

  /* program */
  hbox = (GtkHBox *) gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start((GtkBox *) vbox,
		     (GtkWidget *) hbox,
		     FALSE, FALSE,
		     0);

  label = (GtkLabel *) gtk_label_new(i18n("program"));
  gtk_box_pack_start((GtkBox *) hbox,
		     (GtkWidget *) label,
		     FALSE, FALSE,
		     0);

  live_dssi_bridge->program = (GtkComboBoxText *) gtk_combo_box_text_new();
  gtk_box_pack_start((GtkBox *) hbox,
		     (GtkWidget *) live_dssi_bridge->program,
		     FALSE, FALSE,
		     0);

  /* effect bridge */
  AGS_MACHINE(live_dssi_bridge)->bridge = (GtkContainer *) ags_effect_bridge_new(audio);
  gtk_box_pack_start((GtkBox *) vbox,
		     (GtkWidget *) AGS_MACHINE(live_dssi_bridge)->bridge,
		     FALSE, FALSE,
		     0);
  
  table = (GtkTable *) gtk_table_new(1, 2, FALSE);
  gtk_box_pack_start((GtkBox *) AGS_EFFECT_BRIDGE(AGS_MACHINE(live_dssi_bridge)->bridge),
		     (GtkWidget *) table,
		     FALSE, FALSE,
		     0);

  AGS_EFFECT_BRIDGE(AGS_MACHINE(live_dssi_bridge)->bridge)->bulk_output = (GtkWidget *) ags_effect_bulk_new(audio,
													    AGS_TYPE_OUTPUT);
  AGS_EFFECT_BULK(AGS_EFFECT_BRIDGE(AGS_MACHINE(live_dssi_bridge)->bridge)->bulk_output)->flags |= (AGS_EFFECT_BULK_HIDE_BUTTONS |
												    AGS_EFFECT_BULK_HIDE_ENTRIES |
												    AGS_EFFECT_BULK_SHOW_LABELS);
  gtk_table_attach(table,
		   AGS_EFFECT_BRIDGE(AGS_MACHINE(live_dssi_bridge)->bridge)->bulk_output,
		   0, 1,
		   0, 1,
		   GTK_FILL, GTK_FILL,
		   0, 0);
}

void
ags_live_dssi_bridge_set_property(GObject *gobject,
				  guint prop_id,
				  const GValue *value,
				  GParamSpec *param_spec)
{
  AgsLiveDssiBridge *live_dssi_bridge;

  live_dssi_bridge = AGS_LIVE_DSSI_BRIDGE(gobject);

  switch(prop_id){
  case PROP_FILENAME:
    {
      gchar *str;
      gchar *filename;

      filename = g_value_get_string(value);

      if(filename == live_dssi_bridge->filename){
	return;
      }

      if(live_dssi_bridge->filename != NULL){
	g_free(live_dssi_bridge->filename);
      }

      if(filename != NULL){
	if(!g_file_test(filename,
			G_FILE_TEST_EXISTS)){
	  AgsWindow *window;

	  window = (AgsWindow *) gtk_widget_get_toplevel((GtkWidget *) live_dssi_bridge);

	  str = g_strdup_printf("%s %s",
				i18n("Plugin file not present"),
				filename);
	  ags_window_show_error(window,
				str);

	  g_free(str);
	}
      }

      live_dssi_bridge->filename = g_strdup(filename);
    }
    break;
  case PROP_EFFECT:
    {
      gchar *effect;
      
      effect = g_value_get_string(value);

      if(effect == live_dssi_bridge->effect){
	return;
      }

      if(live_dssi_bridge->effect != NULL){
	g_free(live_dssi_bridge->effect);
      }

      live_dssi_bridge->effect = g_strdup(effect);
    }
    break;
  case PROP_INDEX:
    {
      unsigned long effect_index;
      
      effect_index = (unsigned long) g_value_get_uint(value);

      if(effect_index == live_dssi_bridge->effect_index){
	return;
      }

      live_dssi_bridge->effect_index = effect_index;
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_live_dssi_bridge_get_property(GObject *gobject,
				  guint prop_id,
				  GValue *value,
				  GParamSpec *param_spec)
{
  AgsLiveDssiBridge *live_dssi_bridge;

  live_dssi_bridge = AGS_LIVE_DSSI_BRIDGE(gobject);

  switch(prop_id){
  case PROP_FILENAME:
    {
      g_value_set_string(value, live_dssi_bridge->filename);
    }
    break;
  case PROP_EFFECT:
    {
      g_value_set_string(value, live_dssi_bridge->effect);
    }
    break;
  case PROP_INDEX:
    {
      g_value_set_uint(value, (guint) live_dssi_bridge->effect_index);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_live_dssi_bridge_finalize(GObject *gobject)
{
  AgsLiveDssiBridge *live_dssi_bridge;

  live_dssi_bridge = (AgsLiveDssiBridge *) gobject;
  
  g_object_disconnect(G_OBJECT(live_dssi_bridge),
		      "any_signal::resize-audio-channels",
		      G_CALLBACK(ags_live_dssi_bridge_resize_audio_channels),
		      NULL,
		      "any_signal::resize-pads",
		      G_CALLBACK(ags_live_dssi_bridge_resize_pads),
		      NULL,
		      NULL);

  g_free(live_dssi_bridge->filename);
  g_free(live_dssi_bridge->effect);
  
  /* call parent */
  G_OBJECT_CLASS(ags_live_dssi_bridge_parent_class)->finalize(gobject);
}

void
ags_live_dssi_bridge_connect(AgsConnectable *connectable)
{
  AgsLiveDssiBridge *live_dssi_bridge;

  if((AGS_MACHINE_CONNECTED & (AGS_MACHINE(connectable)->flags)) != 0){
    return;
  }

  ags_live_dssi_bridge_parent_connectable_interface->connect(connectable);

  live_dssi_bridge = AGS_LIVE_DSSI_BRIDGE(connectable);

  g_signal_connect_after(G_OBJECT(live_dssi_bridge->program), "changed",
			 G_CALLBACK(ags_live_dssi_bridge_program_changed_callback), live_dssi_bridge);
}

void
ags_live_dssi_bridge_disconnect(AgsConnectable *connectable)
{
  AgsLiveDssiBridge *live_dssi_bridge;

  if((AGS_MACHINE_CONNECTED & (AGS_MACHINE(connectable)->flags)) == 0){
    return;
  }

  ags_live_dssi_bridge_parent_connectable_interface->connect(connectable);

  live_dssi_bridge = AGS_LIVE_DSSI_BRIDGE(connectable);

  g_object_disconnect(G_OBJECT(live_dssi_bridge->program),
		      "any_signal::changed",
		      G_CALLBACK(ags_live_dssi_bridge_program_changed_callback),
		      live_dssi_bridge,
		      NULL);
}

gchar*
ags_live_dssi_bridge_get_version(AgsPlugin *plugin)
{
  return(AGS_LIVE_DSSI_BRIDGE(plugin)->version);
}

void
ags_live_dssi_bridge_set_version(AgsPlugin *plugin, gchar *version)
{
  AgsLiveDssiBridge *live_dssi_bridge;

  live_dssi_bridge = AGS_LIVE_DSSI_BRIDGE(plugin);

  live_dssi_bridge->version = version;
}

gchar*
ags_live_dssi_bridge_get_build_id(AgsPlugin *plugin)
{
  return(AGS_LIVE_DSSI_BRIDGE(plugin)->build_id);
}

void
ags_live_dssi_bridge_set_build_id(AgsPlugin *plugin, gchar *build_id)
{
  AgsLiveDssiBridge *live_dssi_bridge;

  live_dssi_bridge = AGS_LIVE_DSSI_BRIDGE(plugin);

  live_dssi_bridge->build_id = build_id;
}

gchar*
ags_live_dssi_bridge_get_xml_type(AgsPlugin *plugin)
{
  return(AGS_LIVE_DSSI_BRIDGE(plugin)->xml_type);
}

void
ags_live_dssi_bridge_set_xml_type(AgsPlugin *plugin, gchar *xml_type)
{
  AGS_LIVE_DSSI_BRIDGE(plugin)->xml_type = xml_type;
}

void
ags_live_dssi_bridge_read(AgsFile *file, xmlNode *node, AgsPlugin *plugin)
{
  AgsLiveDssiBridge *gobject;
  AgsFileLaunch *file_launch;

  gobject = AGS_LIVE_DSSI_BRIDGE(plugin);

  g_object_set(gobject,
	       "filename", xmlGetProp(node,
					"filename"),
	       "effect", xmlGetProp(node,
				      "effect"),
	       NULL);

  /* launch */
  file_launch = (AgsFileLaunch *) g_object_new(AGS_TYPE_FILE_LAUNCH,
					       "node", node,
					       NULL);
  g_signal_connect(G_OBJECT(file_launch), "start",
		   G_CALLBACK(ags_live_dssi_bridge_launch_task), gobject);
  ags_file_add_launch(file,
		      G_OBJECT(file_launch));
}

void
ags_live_dssi_bridge_launch_task(AgsFileLaunch *file_launch, AgsLiveDssiBridge *live_dssi_bridge)
{
  GtkTreeModel *model;

  GtkTreeIter iter;

  GList *list, *list_start;
  GList *recall;

  gchar *str;
  
  ags_live_dssi_bridge_load(live_dssi_bridge);

  /* block update bulk port */
  list_start = 
    list = gtk_container_get_children((GtkContainer *) AGS_EFFECT_BULK(AGS_EFFECT_BRIDGE(AGS_MACHINE(live_dssi_bridge)->bridge)->bulk_output)->table);

  while(list != NULL){
    if(AGS_IS_BULK_MEMBER(list->data)){
      AGS_BULK_MEMBER(list->data)->flags |= AGS_BULK_MEMBER_NO_UPDATE;
    }

    list = list->next;
  }
  
  /* update program */
  str = xmlGetProp(file_launch->node,
		   "program");

  model = gtk_combo_box_get_model((GtkComboBox *) live_dssi_bridge->program);

  if(gtk_tree_model_get_iter_first(model, &iter)){
    gchar *value;
    
    do{
      gtk_tree_model_get(model, &iter,
			 0, &value,
			 -1);

      if(!g_strcmp0(str,
		    value)){
	break;
      }
    }while(gtk_tree_model_iter_next(model,
				    &iter));

    gtk_combo_box_set_active_iter((GtkComboBox *) live_dssi_bridge->program,
				  &iter);
  }

  /* update value and unblock update bulk port */
  recall = NULL;
  
  if(AGS_MACHINE(live_dssi_bridge)->audio->play != NULL){
    recall = AGS_MACHINE(live_dssi_bridge)->audio->play;
    
    while((recall = ags_recall_template_find_type(recall, AGS_TYPE_PLAY_DSSI_AUDIO)) != NULL){
      if(!g_strcmp0(AGS_RECALL(recall->data)->filename,
		    live_dssi_bridge->filename) &&
	 !g_strcmp0(AGS_RECALL(recall->data)->effect,
		    live_dssi_bridge->effect)){
	break;
      }

      recall = recall->next;
    }
  }

  while(list != NULL){
    if(AGS_IS_BULK_MEMBER(list->data)){
      GtkWidget *child_widget;
      
      GList *port;

      child_widget = gtk_bin_get_child((GtkBin *) list->data);
      
      if(recall != NULL){
	port = AGS_RECALL(recall->data)->port;

	while(port != port->next){
	  if(!g_strcmp0(AGS_BULK_MEMBER(list->data)->specifier,
			AGS_PORT(port->data)->specifier)){
	    if(AGS_IS_DIAL(child_widget)){
	      gtk_adjustment_set_value(AGS_DIAL(child_widget)->adjustment,
				       AGS_PORT(port->data)->port_value.ags_port_ladspa);
	      ags_dial_draw((AgsDial *) child_widget);
	    }else if(GTK_IS_TOGGLE_BUTTON(child_widget)){
	      gtk_toggle_button_set_active((GtkToggleButton *) child_widget,
					   ((AGS_PORT(port->data)->port_value.ags_port_ladspa != 0.0) ? TRUE: FALSE));
	    }

	    break;
	  }

	  port = port->next;
	}
      }
     
      AGS_BULK_MEMBER(list->data)->flags &= (~AGS_BULK_MEMBER_NO_UPDATE);
    }
    
    list = list->next;
  }

  g_list_free(list_start);
}

xmlNode*
ags_live_dssi_bridge_write(AgsFile *file, xmlNode *parent, AgsPlugin *plugin)
{
  AgsLiveDssiBridge *live_dssi_bridge;

  GtkTreeIter iter;
  xmlNode *node;
  
  gchar *id;
  gchar *program;
  
  live_dssi_bridge = AGS_LIVE_DSSI_BRIDGE(plugin);

  id = ags_id_generator_create_uuid();
    
  node = xmlNewNode(NULL,
		    "ags-live_dssi-bridge");
  xmlNewProp(node,
	     AGS_FILE_ID_PROP,
	     id);

  xmlNewProp(node,
	     "filename",
	     g_strdup(live_dssi_bridge->filename));

  xmlNewProp(node,
	     "effect",
	     g_strdup(live_dssi_bridge->effect));

  if((program = gtk_combo_box_text_get_active_text(live_dssi_bridge->program)) != NULL){
    xmlNewProp(node,
	       "program",
	       g_strdup(program));
  }

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context", file->application_context,
				   "file", file,
				   "node", node,
				   "xpath", g_strdup_printf("xpath=//*[@id='%s']", id),
				   "reference", live_dssi_bridge,
				   NULL));

  xmlAddChild(parent,
	      node);

  return(node);
}

void
ags_live_dssi_bridge_resize_audio_channels(AgsMachine *machine,
					   guint audio_channels, guint audio_channels_old,
					   gpointer data)
{
  AgsLiveDssiBridge *live_dssi_bridge;

  AgsAudio *audio;
  AgsChannel *output, *input;
  AgsChannel *channel, *next_pad;
  AgsRecycling *first_recycling;
  AgsAudioSignal *audio_signal;  

  GObject *output_soundcard;
  
  guint output_pads, input_pads;
  
  live_dssi_bridge = (AgsLiveDssiBridge *) machine;

  audio = machine->audio;

  /* get some fields */
  g_object_get(audio,
	       "output", &output,
	       "input", &input,
	       "output-pads", &output_pads,
	       "input-pads", &input_pads,
	       NULL);

  if(input_pads == 0 &&
     output_pads == 0){
    return;
  }  

  if(audio_channels > audio_channels_old){
    /* AgsInput */
    channel = input;
    
    while(channel != NULL){
      /* get some fields */
      g_object_get(channel,
		   "next-pad", &next_pad,
		   NULL);
      
      channel = ags_channel_nth(channel,
				audio_channels_old);

      while(channel != next_pad){
	ags_channel_set_ability_flags(channel, (AGS_SOUND_ABILITY_NOTATION));

	/* get some fields */
	g_object_get(channel,
		     "output-soundcard", &output_soundcard,
		     "first-recycling", &first_recycling,
		     NULL);

	/* audio signal */
	audio_signal = ags_audio_signal_new(output_soundcard,
					    (GObject *) first_recycling,
					    NULL);
	audio_signal->flags |= AGS_AUDIO_SIGNAL_TEMPLATE;
	ags_audio_signal_stream_resize(audio_signal,
				       1);
	ags_recycling_add_audio_signal(first_recycling,
				       audio_signal);

	/* iterate */
	g_object_get(channel,
		     "next", &channel,
		     NULL);
      }
    }

    /* AgsOutput */
    channel = output;

    while(channel != NULL){
      /* get some fields */
      g_object_get(channel,
		   "next-pad", &next_pad,
		   NULL);

      channel = ags_channel_pad_nth(channel,
				    audio_channels_old);

      while(channel != next_pad){
	/* get some fields */
	g_object_get(channel,
		     "output-soundcard", &output_soundcard,
		     "first-recycling", &first_recycling,
		     NULL);

	/* audio signal */
	audio_signal = ags_audio_signal_new(output_soundcard,
					    (GObject *) first_recycling,
					    NULL);
	audio_signal->flags |= AGS_AUDIO_SIGNAL_TEMPLATE;
	ags_audio_signal_stream_resize(audio_signal,
				       3);
	ags_recycling_add_audio_signal(first_recycling,
				       audio_signal);
	
	/* iterate */
	g_object_get(channel,
		     "next", &channel,
		     NULL);
      }
    }

    /* recall */
    if((AGS_MACHINE_MAPPED_RECALL & (machine->flags)) != 0){
      ags_live_dssi_bridge_input_map_recall(live_dssi_bridge,
					    audio_channels_old,
					    0);

      ags_live_dssi_bridge_output_map_recall(live_dssi_bridge,
					     audio_channels_old,
					     0);
    }
  }
}

void
ags_live_dssi_bridge_resize_pads(AgsMachine *machine, GType type,
				 guint pads, guint pads_old,
				 gpointer data)
{
  AgsLiveDssiBridge *live_dssi_bridge;

  AgsAudio *audio;
  AgsChannel *output, *input;
  AgsChannel *channel;
  AgsRecycling *first_recycling;
  AgsAudioSignal *audio_signal;
  
  GObject *output_soundcard;
  
  guint audio_channels;
  gboolean grow;

  live_dssi_bridge = (AgsLiveDssiBridge *) machine;

  audio = machine->audio;

  /* get some fields */
  g_object_get(audio,
	       "output", &output,
	       "input", &input,
	       "audio-channels", &audio_channels,
	       NULL);

  if(pads == pads_old ||
     audio_channels == 0){
    return;
  }

  if(pads_old < pads){
    grow = TRUE;
  }else{
    grow = FALSE;
  }
  
  if(g_type_is_a(type, AGS_TYPE_INPUT)){
    if(grow){
      /* AgsInput */
      channel = ags_channel_pad_nth(input,
				    pads_old);

      while(channel != NULL){
	/* get some fields */
	g_object_get(channel,
		     "output-soundcard", &output_soundcard,
		     "first-recycling", &first_recycling,
		     NULL);

	/* audio signal */
	audio_signal = ags_audio_signal_new(output_soundcard,
					    (GObject *) first_recycling,
					    NULL);
	audio_signal->flags |= AGS_AUDIO_SIGNAL_TEMPLATE;
	ags_audio_signal_stream_resize(audio_signal,
				       1);
	ags_recycling_add_audio_signal(first_recycling,
				       audio_signal);
	
	/* iterate */
	g_object_get(channel,
		     "next", &channel,
		     NULL);
      }

      /* recall */
      if((AGS_MACHINE_MAPPED_RECALL & (machine->flags)) != 0){
	ags_live_dssi_bridge_input_map_recall(live_dssi_bridge,
					      0,
					      pads_old);
      }
    }else{
      live_dssi_bridge->mapped_input_pad = pads;
    }
  }else{
    if(grow){
      /* AgsOutput */
      channel = ags_channel_pad_nth(output,
				    pads_old);

      while(channel != NULL){
	ags_channel_set_ability_flags(channel, (AGS_SOUND_ABILITY_NOTATION));

	/* get some fields */
	g_object_get(channel,
		     "output-soundcard", &output_soundcard,
		     "first-recycling", &first_recycling,
		     NULL);

	/* audio signal */
	audio_signal = ags_audio_signal_new(output_soundcard,
					    (GObject *) first_recycling,
					    NULL);
	audio_signal->flags |= AGS_AUDIO_SIGNAL_TEMPLATE;
	ags_audio_signal_stream_resize(audio_signal,
				       3);
	ags_recycling_add_audio_signal(first_recycling,
				       audio_signal);
	
	/* iterate */
	g_object_get(channel,
		     "next", &channel,
		     NULL);
      }

      /* recall */
      if((AGS_MACHINE_MAPPED_RECALL & (machine->flags)) != 0){
	ags_live_dssi_bridge_output_map_recall(live_dssi_bridge,
					       0,
					       pads_old);
      }
    }else{
      live_dssi_bridge->mapped_output_pad = pads;
    }
  }
}

void
ags_live_dssi_bridge_map_recall(AgsMachine *machine)
{  
  AgsWindow *window;
  AgsLiveDssiBridge *live_dssi_bridge;
  
  AgsAudio *audio;

  AgsDelayAudio *play_delay_audio;
  AgsDelayAudioRun *play_delay_audio_run;
  AgsCountBeatsAudio *play_count_beats_audio;
  AgsCountBeatsAudioRun *play_count_beats_audio_run;
  AgsRecordMidiAudio *recall_record_midi_audio;
  AgsRecordMidiAudioRun *recall_record_midi_audio_run;
  AgsPlayDssiAudio *play_dssi_audio;
  AgsPlayDssiAudioRun *play_dssi_audio_run;

  GList *start_play, *play;
  GList *start_recall, *recall;
  
  if((AGS_MACHINE_MAPPED_RECALL & (machine->flags)) != 0 ||
     (AGS_MACHINE_PREMAPPED_RECALL & (machine->flags)) != 0){
    return;
  }

  window = (AgsWindow *) gtk_widget_get_ancestor((GtkWidget *) machine,
						 AGS_TYPE_WINDOW);

  live_dssi_bridge = (AgsLiveDssiBridge *) machine;

  audio = machine->audio;

  /* ags-delay */
  ags_recall_factory_create(audio,
			    NULL, NULL,
			    "ags-delay",
			    0, 0,
			    0, 0,
			    (AGS_RECALL_FACTORY_OUTPUT |
			     AGS_RECALL_FACTORY_ADD |
			     AGS_RECALL_FACTORY_PLAY),
			    0);

  g_object_get(audio,
	       "play", &start_play,
	       NULL);

  play = ags_recall_find_type(start_play,
			      AGS_TYPE_DELAY_AUDIO_RUN);

  if(play != NULL){
    play_delay_audio_run = AGS_DELAY_AUDIO_RUN(play->data);
    //    AGS_RECALL(play_delay_audio_run)->flags |= AGS_RECALL_PERSISTENT;
  }else{
    play_delay_audio_run = NULL;
  }

  g_list_free(start_play);
  
  /* ags-count-beats */
  ags_recall_factory_create(audio,
			    NULL, NULL,
			    "ags-count-beats",
			    0, 0,
			    0, 0,
			    (AGS_RECALL_FACTORY_OUTPUT |
			     AGS_RECALL_FACTORY_ADD |
			     AGS_RECALL_FACTORY_PLAY |
			     AGS_RECALL_FACTORY_PLAY),
			    0);

  g_object_get(audio,
	       "play", &start_play,
	       NULL);

  play = ags_recall_find_type(start_play,
			      AGS_TYPE_COUNT_BEATS_AUDIO_RUN);

  if(play != NULL){
    play_count_beats_audio_run = AGS_COUNT_BEATS_AUDIO_RUN(play->data);

    /* set dependency */  
    g_object_set(G_OBJECT(play_count_beats_audio_run),
		 "delay-audio-run", play_delay_audio_run,
		 NULL);
    ags_seekable_seek(AGS_SEEKABLE(play_count_beats_audio_run),
		      window->navigation->position_tact->adjustment->value,
		      TRUE);
  }else{
    play_count_beats_audio_run = NULL;
  }

  g_list_free(start_play);
  
  /* ags-record-midi */
  ags_recall_factory_create(audio,
			    NULL, NULL,
			    "ags-record-midi",
			    0, 0,
			    0, 0,
			    (AGS_RECALL_FACTORY_INPUT |
			     AGS_RECALL_FACTORY_ADD |
			     AGS_RECALL_FACTORY_PLAY),
			    0);

  g_object_get(audio,
	       "play", &start_play,
	       NULL);

  play = ags_recall_find_type(start_play,
			      AGS_TYPE_RECORD_MIDI_AUDIO_RUN);

  if(play != NULL){
    recall_record_midi_audio_run = AGS_RECORD_MIDI_AUDIO_RUN(play->data);
    
    /* set dependency */
    g_object_set(G_OBJECT(recall_record_midi_audio_run),
		 "delay-audio-run", play_delay_audio_run,
		 NULL);

    /* set dependency */
    g_object_set(G_OBJECT(recall_record_midi_audio_run),
		 "count-beats-audio-run", play_count_beats_audio_run,
		 NULL);
  }  

  g_list_free(start_play);
  
  /* ags-play-dssi */
  ags_recall_factory_create(audio,
			    NULL, NULL,
			    "ags-play-dssi",
			    0, 0,
			    0, 0,
			    (AGS_RECALL_FACTORY_OUTPUT |
			     AGS_RECALL_FACTORY_ADD |
			     AGS_RECALL_FACTORY_PLAY |
			     AGS_RECALL_FACTORY_BULK),
			    0);

  g_object_get(audio,
	       "play", &start_play,
	       NULL);

  play = ags_recall_find_type(start_play,
			      AGS_TYPE_PLAY_DSSI_AUDIO);
  
  if(play != NULL){
    play_dssi_audio = AGS_PLAY_DSSI_AUDIO(play->data);
    
    g_object_set(play_dssi_audio,
		 "filename", live_dssi_bridge->filename,
		 "effect", live_dssi_bridge->effect,
		 NULL);

    ags_play_dssi_audio_load(play_dssi_audio);
    ags_play_dssi_audio_load_ports(play_dssi_audio);
  }

  play = ags_recall_find_type(start_play,
			      AGS_TYPE_PLAY_DSSI_AUDIO_RUN);

  if(play != NULL){
    play_dssi_audio_run = AGS_PLAY_DSSI_AUDIO_RUN(play->data);

    /* set dependency */
    g_object_set(G_OBJECT(play_dssi_audio_run),
		 "delay-audio-run", play_delay_audio_run,
		 NULL);

    /* set dependency */
    g_object_set(G_OBJECT(play_dssi_audio_run),
		 "count-beats-audio-run", play_count_beats_audio_run,
		 NULL);
  }

  g_list_free(start_play);

  /* depending on destination */
  ags_live_dssi_bridge_input_map_recall(live_dssi_bridge,
					0,
					0);

  /* depending on destination */
  ags_live_dssi_bridge_output_map_recall(live_dssi_bridge,
					 0,
					 0);

  /* call parent */
  AGS_MACHINE_CLASS(ags_live_dssi_bridge_parent_class)->map_recall(machine);
}

void
ags_live_dssi_bridge_input_map_recall(AgsLiveDssiBridge *live_dssi_bridge,
				      guint audio_channel_start,
				      guint input_pad_start)
{
  AgsAudio *audio;

  audio = AGS_MACHINE(live_dssi_bridge)->audio;

  if(live_dssi_bridge->mapped_input_pad > input_pad_start){
    return;
  }
  
  live_dssi_bridge->mapped_input_pad = audio->input_pads;
}

void
ags_live_dssi_bridge_output_map_recall(AgsLiveDssiBridge *live_dssi_bridge,
				       guint audio_channel_start,
				       guint output_pad_start)
{
  AgsAudio *audio;

  audio = AGS_MACHINE(live_dssi_bridge)->audio;

  if(live_dssi_bridge->mapped_output_pad > output_pad_start){
    return;
  }
  
  live_dssi_bridge->mapped_output_pad = audio->output_pads;
}

void
ags_live_dssi_bridge_load(AgsLiveDssiBridge *live_dssi_bridge)
{
  AgsEffectBulk *effect_bulk;
  AgsBulkMember *bulk_member;
  GtkListStore *model;

  GtkTreeIter iter;

  AgsDssiPlugin *dssi_plugin;

  void *plugin_so;
  DSSI_Descriptor_Function dssi_descriptor;
  DSSI_Descriptor *plugin_descriptor;
  DSSI_Program_Descriptor *program_descriptor;
  LADSPA_PortDescriptor *port_descriptor;
  LADSPA_PortRangeHintDescriptor hint_descriptor;

  GList *start_list, *list;
  GList *start_plugin_port, *plugin_port;
  
  unsigned long samplerate;
  unsigned long effect_index;
  gdouble step;
  unsigned long port_count;
  gboolean has_output_port;

  guint x, y;
  unsigned long i, j;
  guint k;

  samplerate = ags_soundcard_helper_config_get_samplerate(ags_config_get_instance());

  g_message("ags_live_dssi_bridge.c - load %s %s", live_dssi_bridge->filename, live_dssi_bridge->effect);
 
  /* load plugin */
  dssi_plugin = ags_dssi_manager_find_dssi_plugin(ags_dssi_manager_get_instance(),
						  live_dssi_bridge->filename,
						  live_dssi_bridge->effect);

  plugin_so = AGS_BASE_PLUGIN(dssi_plugin)->plugin_so;

  /*  */
  gtk_list_store_clear(GTK_LIST_STORE(gtk_combo_box_get_model(GTK_COMBO_BOX(live_dssi_bridge->program))));
  
  /*  */
  effect_index = AGS_BASE_PLUGIN(dssi_plugin)->effect_index;

  /* load ports */
  model = gtk_list_store_new(3,
			     G_TYPE_STRING,
			     G_TYPE_ULONG,
			     G_TYPE_ULONG);

  if(effect_index != -1 &&
     plugin_so){
    dssi_descriptor = (DSSI_Descriptor_Function) dlsym(plugin_so,
						       "dssi_descriptor");

    if(dlerror() == NULL && dssi_descriptor){
      live_dssi_bridge->dssi_descriptor = 
	plugin_descriptor = dssi_descriptor(effect_index);

      live_dssi_bridge->ladspa_handle = plugin_descriptor->LADSPA_Plugin->instantiate(plugin_descriptor->LADSPA_Plugin,
										      samplerate);
      port_count = plugin_descriptor->LADSPA_Plugin->PortCount;
      port_descriptor = plugin_descriptor->LADSPA_Plugin->PortDescriptors;

      g_object_get(dssi_plugin,
		   "plugin-port", &start_plugin_port,
		   NULL);
  
      live_dssi_bridge->port_values = (LADSPA_Data *) malloc(plugin_descriptor->LADSPA_Plugin->PortCount * sizeof(LADSPA_Data));
      
      for(i = 0; i < port_count; i++){
	if(LADSPA_IS_PORT_CONTROL(port_descriptor[i])){
	  if(LADSPA_IS_PORT_INPUT(port_descriptor[i]) ||
	     LADSPA_IS_PORT_OUTPUT(port_descriptor[i])){
	    AgsDssiPlugin *dssi_plugin;

	    gchar *specifier;
	    
	    dssi_plugin = ags_dssi_manager_find_dssi_plugin(ags_dssi_manager_get_instance(),
							    live_dssi_bridge->filename, live_dssi_bridge->effect);

	    plugin_port = start_plugin_port;
 	    specifier = plugin_descriptor->LADSPA_Plugin->PortNames[i];

	    while(plugin_port != NULL){
	      if(!g_strcmp0(specifier,
			    AGS_PLUGIN_PORT(plugin_port->data)->port_name)){
		live_dssi_bridge->port_values[i] = g_value_get_float(AGS_PLUGIN_PORT(plugin_port->data)->default_value);

		break;
	      }

	      plugin_port = plugin_port->next;
	    }
	    
	    plugin_descriptor->LADSPA_Plugin->connect_port(live_dssi_bridge->ladspa_handle,
							   i,
							   &(live_dssi_bridge->port_values[i]));
	  }
	}
      }

      if(plugin_descriptor->get_program != NULL){
	for(i = 0; (program_descriptor = plugin_descriptor->get_program(live_dssi_bridge->ladspa_handle, i)) != NULL; i++){
	  gtk_list_store_append(model, &iter);
	  gtk_list_store_set(model, &iter,
			     0, program_descriptor->Name,
			     1, program_descriptor->Bank,
			     2, program_descriptor->Program,
			     -1);
	}
      }

      g_list_free(start_plugin_port);
    }
  }
  
  gtk_combo_box_set_model(GTK_COMBO_BOX(live_dssi_bridge->program),
			  GTK_TREE_MODEL(model));

  effect_bulk = AGS_EFFECT_BRIDGE(AGS_MACHINE(live_dssi_bridge)->bridge)->bulk_output;

  /* retrieve position within table  */
  x = 0;
  y = 0;
  
  list = effect_bulk->table->children;

  while(list != NULL){
    if(y <= ((GtkTableChild *) list->data)->top_attach){
      y = ((GtkTableChild *) list->data)->top_attach + 1;
    }

    list = list->next;
  }

  /* load ports */
  g_object_get(dssi_plugin,
	       "plugin-port", &start_plugin_port,
	       NULL);
  
  plugin_port = start_plugin_port;

  port_count = g_list_length(start_plugin_port);
  k = 0;

  while(plugin_port != NULL){
    if(ags_plugin_port_test_flags(plugin_port->data, AGS_PLUGIN_PORT_CONTROL)){
      GtkWidget *child_widget;

      AgsLadspaConversion *ladspa_conversion;
      
      GType widget_type;

      gchar *plugin_name;
      gchar *control_port;
      
      guint step_count;
      gboolean disable_seemless;

      disable_seemless = FALSE;
      
      if(x == AGS_EFFECT_BULK_COLUMNS_COUNT){
	x = 0;
	y++;
	gtk_table_resize(effect_bulk->table,
			 y + 1, AGS_EFFECT_BULK_COLUMNS_COUNT);
      }

      if(ags_plugin_port_test_flags(plugin_port->data, AGS_PLUGIN_PORT_TOGGLED)){
	if(ags_plugin_port_test_flags(plugin_port->data, AGS_PLUGIN_PORT_OUTPUT)){
	  widget_type = AGS_TYPE_LED;
	}else{
	  widget_type = GTK_TYPE_TOGGLE_BUTTON;
	}
      }else{
	if(ags_plugin_port_test_flags(plugin_port->data, AGS_PLUGIN_PORT_OUTPUT)){
	  widget_type = AGS_TYPE_HINDICATOR;
	}else{
	  widget_type = AGS_TYPE_DIAL;
	}
      }

      step_count = AGS_DIAL_DEFAULT_PRECISION;

      if(ags_plugin_port_test_flags(plugin_port->data, AGS_PLUGIN_PORT_INTEGER)){
	step_count = AGS_PLUGIN_PORT(plugin_port->data)->scale_steps;

	disable_seemless = TRUE;	
      }

      /* add bulk member */
      plugin_name = g_strdup_printf("dssi-%u",
				    dssi_plugin->unique_id);
      control_port = g_strdup_printf("%u/%u",
				     k,
				     port_count);
      bulk_member = (AgsBulkMember *) g_object_new(AGS_TYPE_BULK_MEMBER,
						   "widget-type", widget_type,
						   "widget-label", AGS_PLUGIN_PORT(plugin_port->data)->port_name,
						   "plugin-name", plugin_name,
						   "filename", live_dssi_bridge->filename,
						   "effect", live_dssi_bridge->effect,
						   "specifier", AGS_PLUGIN_PORT(plugin_port->data)->port_name,
						   "control-port", control_port,
						   "steps", step_count,
						   NULL);
      child_widget = ags_bulk_member_get_widget(bulk_member);

      g_free(plugin_name);
      g_free(control_port);
      
      /* ladspa conversion */
      ladspa_conversion = NULL;

      if(ags_plugin_port_test_flags(plugin_port->data, AGS_PLUGIN_PORT_BOUNDED_BELOW)){
	if(ladspa_conversion == NULL ||
	   !AGS_IS_LADSPA_CONVERSION(ladspa_conversion)){
	  ladspa_conversion = ags_ladspa_conversion_new();
	}

	ladspa_conversion->flags |= AGS_LADSPA_CONVERSION_BOUNDED_BELOW;
      }

      if(ags_plugin_port_test_flags(plugin_port->data, AGS_PLUGIN_PORT_BOUNDED_ABOVE)){
	if(ladspa_conversion == NULL ||
	   !AGS_IS_LADSPA_CONVERSION(ladspa_conversion)){
	  ladspa_conversion = ags_ladspa_conversion_new();
	}

	ladspa_conversion->flags |= AGS_LADSPA_CONVERSION_BOUNDED_ABOVE;
      }
      
      if(ags_plugin_port_test_flags(plugin_port->data, AGS_PLUGIN_PORT_SAMPLERATE)){
	if(ladspa_conversion == NULL ||
	   !AGS_IS_LADSPA_CONVERSION(ladspa_conversion)){
	  ladspa_conversion = ags_ladspa_conversion_new();
	}

	ladspa_conversion->flags |= AGS_LADSPA_CONVERSION_SAMPLERATE;
      }

      if(ags_plugin_port_test_flags(plugin_port->data, AGS_PLUGIN_PORT_LOGARITHMIC)){
	if(ladspa_conversion == NULL ||
	   !AGS_IS_LADSPA_CONVERSION(ladspa_conversion)){
	  ladspa_conversion = ags_ladspa_conversion_new();
	}
    
	ladspa_conversion->flags |= AGS_LADSPA_CONVERSION_LOGARITHMIC;
      }

      bulk_member->conversion = (AgsConversion *) ladspa_conversion;

      /* child widget */
      if(ags_plugin_port_test_flags(plugin_port->data, AGS_PLUGIN_PORT_TOGGLED)){
	bulk_member->port_flags = AGS_BULK_MEMBER_PORT_BOOLEAN;
      }
      
      if(ags_plugin_port_test_flags(plugin_port->data, AGS_PLUGIN_PORT_INTEGER)){
	bulk_member->port_flags = AGS_BULK_MEMBER_PORT_INTEGER;
      }

      if(AGS_IS_DIAL(child_widget)){
	AgsDial *dial;
	GtkAdjustment *adjustment;

	LADSPA_Data lower_bound, upper_bound;
	LADSPA_Data default_value;
	
	dial = (AgsDial *) child_widget;

	if(disable_seemless){
	  dial->flags &= (~AGS_DIAL_SEEMLESS_MODE);
	}

	/* add controls of ports and apply range  */
	lower_bound = g_value_get_float(AGS_PLUGIN_PORT(plugin_port->data)->lower_value);
	upper_bound = g_value_get_float(AGS_PLUGIN_PORT(plugin_port->data)->upper_value);

	adjustment = (GtkAdjustment *) gtk_adjustment_new(0.0, 0.0, 1.0, 0.1, 0.1, 0.0);
	g_object_set(dial,
		     "adjustment", adjustment,
		     NULL);

	if(upper_bound >= 0.0 && lower_bound >= 0.0){
	  step = (upper_bound - lower_bound) / step_count;
	}else if(upper_bound < 0.0 && lower_bound < 0.0){
	  step = -1.0 * (lower_bound - upper_bound) / step_count;
	}else{
	  step = (upper_bound - lower_bound) / step_count;
	}

	gtk_adjustment_set_step_increment(adjustment,
					  step);
	gtk_adjustment_set_lower(adjustment,
				 lower_bound);
	gtk_adjustment_set_upper(adjustment,
				 upper_bound);

	default_value = (LADSPA_Data) g_value_get_float(AGS_PLUGIN_PORT(plugin_port->data)->default_value);

	if(ladspa_conversion != NULL){
	  //	  default_value = ags_ladspa_conversion_convert(ladspa_conversion,
	  //						default_value,
	  //						TRUE);
	}
	
	gtk_adjustment_set_value(adjustment,
				 default_value);

#ifdef AGS_DEBUG
	g_message("dssi bounds: %f %f", lower_bound, upper_bound);
#endif
      }else if(AGS_IS_INDICATOR(child_widget) ||
	       AGS_IS_LED(child_widget)){
	g_hash_table_insert(ags_effect_bulk_indicator_queue_draw,
			    child_widget, ags_effect_bulk_indicator_queue_draw_timeout);
	effect_bulk->queued_drawing = g_list_prepend(effect_bulk->queued_drawing,
						     child_widget);
	g_timeout_add(1000 / 30, (GSourceFunc) ags_effect_bulk_indicator_queue_draw_timeout, (gpointer) child_widget);
      }

      gtk_table_attach(effect_bulk->table,
		       (GtkWidget *) bulk_member,
		       x, x + 1,
		       y, y + 1,
		       GTK_FILL, GTK_FILL,
		       0, 0);
      ags_connectable_connect(AGS_CONNECTABLE(bulk_member));
      gtk_widget_show_all((GtkWidget *) effect_bulk->table);

      x++;
    }

    plugin_port = plugin_port->next;    
    k++;
  }

  g_list_free(start_plugin_port);
}

/**
 * ags_live_dssi_bridge_new:
 * @soundcard: the assigned soundcard.
 * @filename: the plugin.so
 * @effect: the effect
 *
 * Create a new instance of #AgsLiveDssiBridge
 *
 * Returns: the new #AgsLiveDssiBridge
 *
 * Since: 2.0.0
 */
AgsLiveDssiBridge*
ags_live_dssi_bridge_new(GObject *soundcard,
			 gchar *filename,
			 gchar *effect)
{
  AgsLiveDssiBridge *live_dssi_bridge;

  live_dssi_bridge = (AgsLiveDssiBridge *) g_object_new(AGS_TYPE_LIVE_DSSI_BRIDGE,
							NULL);

  if(soundcard != NULL){
    g_object_set(G_OBJECT(AGS_MACHINE(live_dssi_bridge)->audio),
		 "output-soundcard", soundcard,
		 NULL);
  }

  g_object_set(live_dssi_bridge,
	       "filename", filename,
	       "effect", effect,
	       NULL);

  return(live_dssi_bridge);
}
