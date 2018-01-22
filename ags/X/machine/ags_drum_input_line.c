/* GSequencer - Advanced GTK Sequencer
 * Copyright (C) 2005-2015 Joël Krähemann
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

#include <ags/X/machine/ags_drum_input_line.h>
#include <ags/X/machine/ags_drum_input_line_callbacks.h>

#include <ags/libags.h>
#include <ags/libags-audio.h>
#include <ags/libags-gui.h>

#include <ags/X/ags_window.h>
#include <ags/X/ags_line_callbacks.h>
#include <ags/X/ags_line_member.h>

#include <ags/X/machine/ags_drum.h>

#include <ags/object/ags_config.h>

void ags_drum_input_line_class_init(AgsDrumInputLineClass *drum_input_line);
void ags_drum_input_line_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_drum_input_line_plugin_interface_init(AgsPluginInterface *plugin);
void ags_drum_input_line_init(AgsDrumInputLine *drum_input_line);
void ags_drum_input_line_destroy(GtkObject *object);
void ags_drum_input_line_connect(AgsConnectable *connectable);
void ags_drum_input_line_disconnect(AgsConnectable *connectable);
gchar* ags_drum_input_line_get_name(AgsPlugin *plugin);
void ags_drum_input_line_set_name(AgsPlugin *plugin, gchar *name);
gchar* ags_drum_input_line_get_xml_type(AgsPlugin *plugin);
void ags_drum_input_line_set_xml_type(AgsPlugin *plugin, gchar *xml_type);
void ags_drum_input_line_read(AgsFile *file, xmlNode *node, AgsPlugin *plugin);
xmlNode* ags_drum_input_line_write(AgsFile *file, xmlNode *parent, AgsPlugin *plugin);

void ags_drum_input_line_set_channel(AgsLine *line, AgsChannel *channel);
void ags_drum_input_line_group_changed(AgsLine *line);
void ags_drum_input_line_map_recall(AgsLine *line,
				    guint output_pad_start);

/**
 * SECTION:ags_drum_input_line
 * @short_description: drum sequencer input line
 * @title: AgsDrumInputLine
 * @section_id:
 * @include: ags/X/machine/ags_drum_input_line.h
 *
 * The #AgsDrumInputLine is a composite widget to act as drum sequencer input line.
 */

static gpointer ags_drum_input_line_parent_class = NULL;
static AgsConnectableInterface *ags_drum_input_line_parent_connectable_interface;

extern GHashTable *ags_line_indicator_queue_draw;

GType
ags_drum_input_line_get_type()
{
  static GType ags_type_drum_input_line = 0;

  if(!ags_type_drum_input_line){
    static const GTypeInfo ags_drum_input_line_info = {
      sizeof(AgsDrumInputLineClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_drum_input_line_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof(AgsDrumInputLine),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_drum_input_line_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_drum_input_line_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    static const GInterfaceInfo ags_plugin_interface_info = {
      (GInterfaceInitFunc) ags_drum_input_line_plugin_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_drum_input_line = g_type_register_static(AGS_TYPE_LINE,
						      "AgsDrumInputLine", &ags_drum_input_line_info,
						      0);

    g_type_add_interface_static(ags_type_drum_input_line,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);

    g_type_add_interface_static(ags_type_drum_input_line,
				AGS_TYPE_PLUGIN,
				&ags_plugin_interface_info);
  }

  return(ags_type_drum_input_line);
}

void
ags_drum_input_line_class_init(AgsDrumInputLineClass *drum_input_line)
{
  AgsLineClass *line;

  ags_drum_input_line_parent_class = g_type_class_peek_parent(drum_input_line);

  /* AgsLineClass */
  line = AGS_LINE_CLASS(drum_input_line);

  line->set_channel = ags_drum_input_line_set_channel;
  
  line->group_changed = ags_drum_input_line_group_changed;

  line->map_recall = ags_drum_input_line_map_recall;
}

void
ags_drum_input_line_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_drum_input_line_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_drum_input_line_connect;
  connectable->disconnect = ags_drum_input_line_disconnect;
}

void
ags_drum_input_line_plugin_interface_init(AgsPluginInterface *plugin)
{
  plugin->get_name = ags_drum_input_line_get_name;
  plugin->set_name = ags_drum_input_line_set_name;
  plugin->get_xml_type = ags_drum_input_line_get_xml_type;
  plugin->set_xml_type = ags_drum_input_line_set_xml_type;
  plugin->read = ags_drum_input_line_read;
  plugin->write = ags_drum_input_line_write;
}

void
ags_drum_input_line_init(AgsDrumInputLine *drum_input_line)
{
  AgsLineMember *line_member;
  GtkWidget *widget;
  GtkAdjustment *adjustment;

  /* volume indicator */
  line_member = (AgsLineMember *) g_object_new(AGS_TYPE_LINE_MEMBER,
					       "widget-type", AGS_TYPE_VINDICATOR,
					       "plugin-name", "ags-peak",
					       "specifier", "./peak[0]",
					       "control-port", "1/1",
					       NULL);
  line_member->flags |= (AGS_LINE_MEMBER_PLAY_CALLBACK_WRITE |
			 AGS_LINE_MEMBER_RECALL_CALLBACK_WRITE);
  ags_expander_add(AGS_LINE(drum_input_line)->expander,
		   GTK_WIDGET(line_member),
		   0, 0,
		   1, 1);
  widget = gtk_bin_get_child(GTK_BIN(line_member));
  AGS_LINE(drum_input_line)->indicator = widget;
  g_hash_table_insert(ags_line_indicator_queue_draw,
		      widget, ags_line_indicator_queue_draw_timeout);
  g_timeout_add(1000 / 30, (GSourceFunc) ags_line_indicator_queue_draw_timeout, (gpointer) widget);

  adjustment = (GtkAdjustment *) gtk_adjustment_new(0.0, 0.0, 10.0, 1.0, 1.0, 10.0);
  g_object_set(widget,
	       "adjustment", adjustment,
	       NULL);

  gtk_widget_set_size_request(widget,
			      16, 100);
  gtk_widget_queue_draw(widget);

  //TODO:JK: fix me
  //  g_object_set(G_OBJECT(line_member),
  //	       "port-data", (gpointer) &(adjustment->value),
  //	       NULL);

  /* volume control */
  line_member = (AgsLineMember *) g_object_new(AGS_TYPE_LINE_MEMBER,
					       "widget-type", GTK_TYPE_VSCALE,
					       "plugin-name", "ags-volume",
					       "specifier", "./volume[0]",
					       "control-port", "1/1",
					       NULL);
  ags_expander_add(AGS_LINE(drum_input_line)->expander,
		   GTK_WIDGET(line_member),
		   1, 0,
		   1, 1);

  widget = gtk_bin_get_child(GTK_BIN(line_member));

  gtk_scale_set_digits(GTK_SCALE(widget),
		       3);

  gtk_range_set_range(GTK_RANGE(widget),
		      0.0, 2.00);
  gtk_range_set_increments(GTK_RANGE(widget),
			   0.025, 0.1);
  gtk_range_set_value(GTK_RANGE(widget),
		      1.0);
  gtk_range_set_inverted(GTK_RANGE(widget),
			 TRUE);

  gtk_widget_set_size_request(widget,
			      -1, 100);

  //  g_object_set(G_OBJECT(line_member),
  //	       "port-data", (gpointer) &(GTK_RANGE(widget)->adjustment->value),
  //	       NULL);
}

void
ags_drum_input_line_destroy(GtkObject *object)
{
}

void
ags_drum_input_line_connect(AgsConnectable *connectable)
{
  AgsDrumInputLine *drum_input_line;

  drum_input_line = AGS_DRUM_INPUT_LINE(connectable);

  if((AGS_LINE_CONNECTED & (AGS_LINE(drum_input_line)->flags)) != 0){
    return;
  }
  
  ags_drum_input_line_parent_connectable_interface->connect(connectable);

  /* empty */
}

void
ags_drum_input_line_disconnect(AgsConnectable *connectable)
{
  AgsDrumInputLine *drum_input_line;

  drum_input_line = AGS_DRUM_INPUT_LINE(connectable);

  if((AGS_LINE_CONNECTED & (AGS_LINE(drum_input_line)->flags)) != 0){
    return;
  }
  
  ags_drum_input_line_parent_connectable_interface->disconnect(connectable);

  /* empty */
}

gchar*
ags_drum_input_line_get_name(AgsPlugin *plugin)
{
  return(AGS_DRUM_INPUT_LINE(plugin)->name);
}

void
ags_drum_input_line_set_name(AgsPlugin *plugin, gchar *name)
{
  AGS_DRUM_INPUT_LINE(plugin)->name = name;
}

gchar*
ags_drum_input_line_get_xml_type(AgsPlugin *plugin)
{
  return(AGS_DRUM_INPUT_LINE(plugin)->xml_type);
}

void
ags_drum_input_line_set_xml_type(AgsPlugin *plugin, gchar *xml_type)
{
  AGS_DRUM_INPUT_LINE(plugin)->xml_type = xml_type;
}

void
ags_drum_input_line_set_channel(AgsLine *line, AgsChannel *channel)
{
  AgsChannel *old_channel;
  AgsRecycling *first_recycling;
  AgsAudioSignal *template;
  
  AgsMutexManager *mutex_manager;

  GObject *soundcard;

  guint nth_line;
  
  pthread_mutex_t *application_mutex;
  pthread_mutex_t *channel_mutex;
  pthread_mutex_t *recycling_mutex;

  mutex_manager = ags_mutex_manager_get_instance();
  application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);

  /* get channel mutex */
  pthread_mutex_lock(application_mutex);

  channel_mutex = ags_mutex_manager_lookup(mutex_manager,
					 (GObject *) channel);
  
  pthread_mutex_unlock(application_mutex);

  if(line->channel != NULL){
    old_channel = line->channel;
  }else{
    old_channel = NULL;
  }

  /* call parent */
  AGS_LINE_CLASS(ags_drum_input_line_parent_class)->set_channel(line,
								channel);

  if(channel != NULL){
    /* get some fields */
    pthread_mutex_lock(channel_mutex);

    soundcard = channel->soundcard;

    first_recycling = channel->first_recycling;

    nth_line = channel->line;
    
#ifdef AGS_DEBUG
    g_message("ags_drum_input_line_set_channel - channel: %u",
	      nth_line);
#endif

    pthread_mutex_unlock(channel_mutex);

    /* get recycling mutex */
    pthread_mutex_lock(application_mutex);
    
    recycling_mutex = ags_mutex_manager_lookup(mutex_manager,
					       (GObject *) first_recycling);
    
    pthread_mutex_unlock(application_mutex);

    /* get some fields */
    pthread_mutex_lock(recycling_mutex);

    template = ags_audio_signal_get_template(first_recycling->audio_signal);
    
    pthread_mutex_unlock(recycling_mutex);

    /* create audio signal */
    if(soundcard != NULL &&
       template == NULL){
      AgsAudioSignal *audio_signal;

      audio_signal = ags_audio_signal_new(soundcard,
					  (GObject *) first_recycling,
					  NULL);
      audio_signal->flags |= AGS_AUDIO_SIGNAL_TEMPLATE;
      ags_recycling_add_audio_signal(first_recycling,
				     audio_signal);
    }

    /* reset edit button */
    if(old_channel == NULL &&
       nth_line == 0){
      AgsDrum *drum;
      GtkToggleButton *selected_edit_button;

      GList *list;

      drum = (AgsDrum *) gtk_widget_get_ancestor(GTK_WIDGET(line),
						 AGS_TYPE_DRUM);

      if(drum != NULL){
	list = gtk_container_get_children((GtkContainer *) drum->input_pad);

	drum->selected_pad = AGS_DRUM_INPUT_PAD(list->data);
	drum->selected_edit_button = drum->selected_pad->edit;
	gtk_toggle_button_set_active((GtkToggleButton *) drum->selected_edit_button,
				     TRUE);
	
	g_list_free(list);
      }
    }
  }
}

void
ags_drum_input_line_group_changed(AgsLine *line)
{
  AgsDrum *drum;

  drum = (AgsDrum *) gtk_widget_get_ancestor(GTK_WIDGET(line), AGS_TYPE_DRUM);
  ags_pattern_box_set_pattern(drum->pattern_box);
}

void
ags_drum_input_line_map_recall(AgsLine *line,
			       guint output_pad_start)
{
  AgsLineMember *line_member;

  AgsAudio *audio;
  AgsChannel *source;
  AgsChannel *current;
  AgsPattern *pattern;
  AgsRecallHandler *recall_handler;

  AgsCopyPatternChannel *copy_pattern_channel;
  AgsPlayChannel *play_channel;
  AgsPlayChannelRun *play_channel_run;
  AgsPeakChannelRun *recall_peak_channel_run, *play_peak_channel_run;
  AgsStreamChannelRun *stream_channel_run;

  AgsMutexManager *mutex_manager;

  AgsConfig *config;
  
  GList *list;

  gchar *str;
  
  guint pad, audio_channel;
  guint i;
  gboolean rt_safe;

  pthread_mutex_t *application_mutex;
  pthread_mutex_t *source_mutex;

  if((AGS_LINE_MAPPED_RECALL & (line->flags)) != 0 ||
     (AGS_LINE_PREMAPPED_RECALL & (line->flags)) != 0){
    return;
  }

  config = ags_config_get_instance();
  
  mutex_manager = ags_mutex_manager_get_instance();
  application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);

  source = line->channel;
  
  /* lookup source mutex */
  pthread_mutex_lock(application_mutex);

  source_mutex = ags_mutex_manager_lookup(mutex_manager,
					 (GObject *) source);
  
  pthread_mutex_unlock(application_mutex);

  /* get some fields */
  pthread_mutex_lock(source_mutex);

  audio = (AgsAudio *) source->audio;

  pad = source->pad;
  audio_channel = source->audio_channel;

  pthread_mutex_unlock(source_mutex);

  /* ags-peak */
  ags_recall_factory_create(audio,
			    NULL, NULL,
			    "ags-peak",
			    audio_channel, audio_channel + 1, 
			    pad, pad + 1,
			    (AGS_RECALL_FACTORY_INPUT |
			     AGS_RECALL_FACTORY_PLAY |
			     AGS_RECALL_FACTORY_RECALL |
			     AGS_RECALL_FACTORY_ADD),
			    0);

  /* ags-copy-pattern */
  ags_recall_factory_create(audio,
			    NULL, NULL,
			    "ags-copy-pattern",
			    audio_channel, audio_channel + 1, 
			    pad, pad + 1,
			    (AGS_RECALL_FACTORY_INPUT |
			     AGS_RECALL_FACTORY_REMAP |
			     AGS_RECALL_FACTORY_RECALL),
			    0);

  /* set pattern object on port */
  pthread_mutex_lock(source_mutex);

  list = ags_recall_template_find_type(source->recall,
				       AGS_TYPE_COPY_PATTERN_CHANNEL);

  if(list != NULL){
    copy_pattern_channel = AGS_COPY_PATTERN_CHANNEL(list->data);
    list = source->pattern;
    
    pattern = AGS_PATTERN(list->data);
    copy_pattern_channel->pattern->port_value.ags_port_object = (GObject *) pattern;
  
    ags_portlet_set_port(AGS_PORTLET(pattern),
			 (GObject *) copy_pattern_channel->pattern);
  }

  pthread_mutex_unlock(source_mutex);

  /* ags-play */
  ags_recall_factory_create(audio,
			    NULL, NULL,
  			    "ags-play",
  			    audio_channel, audio_channel + 1, 
  			    pad, pad + 1,
  			    (AGS_RECALL_FACTORY_INPUT |
			     AGS_RECALL_FACTORY_PLAY |
  			     AGS_RECALL_FACTORY_ADD),
  			    0);

  pthread_mutex_lock(source_mutex);

  list = source->play;

  while((list = ags_recall_find_type(list, AGS_TYPE_PLAY_CHANNEL)) != NULL){
    GValue audio_channel_value = {0,};

    play_channel = AGS_PLAY_CHANNEL(list->data);    

    g_value_init(&audio_channel_value, G_TYPE_UINT64);
    g_value_set_uint64(&audio_channel_value,
		       audio_channel);
    ags_port_safe_write(play_channel->audio_channel,
			&audio_channel_value);
    
    list = list->next;
  }

  pthread_mutex_unlock(source_mutex);

  /* ags-volume */
  ags_recall_factory_create(audio,
			    NULL, NULL,
			    "ags-volume",
			    audio_channel, audio_channel + 1, 
			    pad, pad + 1,
			    (AGS_RECALL_FACTORY_INPUT |
			     AGS_RECALL_FACTORY_PLAY |
			     AGS_RECALL_FACTORY_RECALL |
			     AGS_RECALL_FACTORY_ADD),
			    0);

  /* ags-envelope */
  ags_recall_factory_create(audio,
			    NULL, NULL,
			    "ags-envelope",
			    audio_channel, audio_channel + 1, 
			    pad, pad + 1,
			    (AGS_RECALL_FACTORY_INPUT |
			     AGS_RECALL_FACTORY_PLAY |
			     AGS_RECALL_FACTORY_RECALL |
			     AGS_RECALL_FACTORY_ADD),
			    0);

  /* ags-stream */
  rt_safe = TRUE;

  str = ags_config_get_value(config,
			     AGS_CONFIG_GENERIC,
			     "rt-safe");

  if(str != NULL &&
     !g_ascii_strncasecmp(str,
			  "FALSE",
			  6)){
    rt_safe = FALSE;
  }

  if(!rt_safe){
    ags_recall_factory_create(audio,
			      NULL, NULL,
			      "ags-stream",
			      audio_channel, audio_channel + 1, 
			      pad, pad + 1,
			      (AGS_RECALL_FACTORY_INPUT |
			       AGS_RECALL_FACTORY_PLAY |
			       AGS_RECALL_FACTORY_RECALL | 
			       AGS_RECALL_FACTORY_ADD),
			      0);

    /* set up dependencies */
    pthread_mutex_lock(source_mutex);

    list = ags_recall_find_type(source->play,
				AGS_TYPE_PLAY_CHANNEL_RUN);
    play_channel_run = AGS_PLAY_CHANNEL_RUN(list->data);

    list = ags_recall_find_type(source->play,
				AGS_TYPE_STREAM_CHANNEL_RUN);
    stream_channel_run = AGS_STREAM_CHANNEL_RUN(list->data);

    g_object_set(G_OBJECT(play_channel_run),
		 "stream-channel-run", stream_channel_run,
		 NULL);

    pthread_mutex_unlock(source_mutex);
  }else{
    ags_recall_factory_create(audio,
			      NULL, NULL,
			      "ags-rt-stream",
			      audio_channel, audio_channel + 1, 
			      pad, pad + 1,
			      (AGS_RECALL_FACTORY_INPUT |
			       AGS_RECALL_FACTORY_PLAY |
			       AGS_RECALL_FACTORY_RECALL | 
			       AGS_RECALL_FACTORY_ADD),
			      0);
  }
  
  /* call parent */
  AGS_LINE_CLASS(ags_drum_input_line_parent_class)->map_recall(line,
							       output_pad_start);
}

void
ags_drum_input_line_read(AgsFile *file, xmlNode *node, AgsPlugin *plugin)
{
  AgsDrumInputLine *gobject;

  gobject = AGS_DRUM_INPUT_LINE(plugin);

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context", file->application_context,
				   "file", file,
				   "node", node,
				   "xpath", g_strdup_printf("xpath=//*[@id='%s']", xmlGetProp(node, AGS_FILE_ID_PROP)),
				   "reference", gobject,
				   NULL));
}

xmlNode*
ags_drum_input_line_write(AgsFile *file, xmlNode *parent, AgsPlugin *plugin)
{
  AgsDrumInputLine *drum_input_line;
  xmlNode *node;
  gchar *id;

  drum_input_line = AGS_DRUM_INPUT_LINE(plugin);

  id = ags_id_generator_create_uuid();
  
  node = xmlNewNode(NULL,
		    "ags-drum-input-line");
  xmlNewProp(node,
	     AGS_FILE_ID_PROP,
	     id);

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context", file->application_context,
				   "file", file,
				   "node", node,
				   "xpath", g_strdup_printf("xpath=//*[@id='%s']", id),
				   "reference", drum_input_line,
				   NULL));

  return(node);
}

/**
 * ags_drum_input_line_new:
 * @channel: the assigned channel
 *
 * Creates an #AgsDrumInputLine
 *
 * Returns: a new #AgsDrumInputLine
 *
 * Since: 1.0.0
 */
AgsDrumInputLine*
ags_drum_input_line_new(AgsChannel *channel)
{
  AgsDrumInputLine *drum_input_line;

  drum_input_line = (AgsDrumInputLine *) g_object_new(AGS_TYPE_DRUM_INPUT_LINE,
						      "channel", channel,
						      NULL);

  return(drum_input_line);
}
