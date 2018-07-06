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

#include <ags/X/machine/ags_ffplayer.h>
#include <ags/X/machine/ags_ffplayer_callbacks.h>

#include <ags/libags.h>
#include <ags/libags-audio.h>
#include <ags/libags-gui.h>

#include <ags/X/file/ags_gui_file_xml.h>

#include <ags/X/machine/ags_ffplayer_bridge.h>

#include <math.h>

#include <ags/i18n.h>

void ags_ffplayer_class_init(AgsFFPlayerClass *ffplayer);
void ags_ffplayer_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_ffplayer_plugin_interface_init(AgsPluginInterface *plugin);
void ags_ffplayer_init(AgsFFPlayer *ffplayer);
void ags_ffplayer_connect(AgsConnectable *connectable);
void ags_ffplayer_disconnect(AgsConnectable *connectable);
void ags_ffplayer_finalize(GObject *gobject);
void ags_ffplayer_realize(GtkWidget *widget);
void ags_ffplayer_show(GtkWidget *widget);
void ags_ffplayer_map_recall(AgsMachine *machine);
gchar* ags_ffplayer_get_name(AgsPlugin *plugin);
void ags_ffplayer_set_name(AgsPlugin *plugin, gchar *name);
gchar* ags_ffplayer_get_xml_type(AgsPlugin *plugin);
void ags_ffplayer_set_xml_type(AgsPlugin *plugin, gchar *xml_type);
void ags_ffplayer_read(AgsFile *file, xmlNode *node, AgsPlugin *plugin);
void ags_ffplayer_read_resolve_audio(AgsFileLookup *file_lookup,
				     AgsMachine *machine);
void ags_ffplayer_launch_task(AgsFileLaunch *file_launch, AgsFFPlayer *ffplayer);
xmlNode* ags_ffplayer_write(AgsFile *file, xmlNode *parent, AgsPlugin *plugin);

void ags_ffplayer_resize_audio_channels(AgsMachine *machine,
				     guint audio_channels, guint audio_channels_old,
				     gpointer data);
void ags_ffplayer_resize_pads(AgsMachine *machine, GType type,
			      guint pads, guint pads_old,
			      gpointer data);

void ags_ffplayer_output_map_recall(AgsFFPlayer *ffplayer, guint output_pad_start);
void ags_ffplayer_input_map_recall(AgsFFPlayer *ffplayer, guint input_pad_start);
void ags_ffplayer_paint(AgsFFPlayer *ffplayer);

/**
 * SECTION:ags_ffplayer
 * @short_description: ffplayer notation
 * @title: AgsFFPlayer
 * @section_id:
 * @include: ags/X/machine/ags_ffplayer.h
 *
 * The #AgsFFPlayer is a composite widget to act as soundfont2 notation player.
 */

static gpointer ags_ffplayer_parent_class = NULL;
static AgsConnectableInterface *ags_ffplayer_parent_connectable_interface;

GtkStyle *ffplayer_style = NULL;

GType
ags_ffplayer_get_type(void)
{
  static volatile gsize g_define_type_id__volatile = 0;

  if(g_once_init_enter (&g_define_type_id__volatile)){
    GType ags_type_ffplayer;

    static const GTypeInfo ags_ffplayer_info = {
      sizeof(AgsFFPlayerClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_ffplayer_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof(AgsFFPlayer),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_ffplayer_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_ffplayer_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    static const GInterfaceInfo ags_plugin_interface_info = {
      (GInterfaceInitFunc) ags_ffplayer_plugin_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_ffplayer = g_type_register_static(AGS_TYPE_MACHINE,
					       "AgsFFPlayer", &ags_ffplayer_info,
					       0);
    
    g_type_add_interface_static(ags_type_ffplayer,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);

    g_type_add_interface_static(ags_type_ffplayer,
				AGS_TYPE_PLUGIN,
				&ags_plugin_interface_info);

    g_once_init_leave (&g_define_type_id__volatile, ags_type_ffplayer);
  }

  return g_define_type_id__volatile;
}

void
ags_ffplayer_class_init(AgsFFPlayerClass *ffplayer)
{
  GObjectClass *gobject;
  GtkWidgetClass *widget;
  AgsMachineClass *machine;

  ags_ffplayer_parent_class = g_type_class_peek_parent(ffplayer);

  /* GObjectClass */
  gobject = (GObjectClass *) ffplayer;

  gobject->finalize = ags_ffplayer_finalize;

  /* GtkWidgetClass */
  widget = (GtkWidgetClass *) ffplayer;

  widget->realize = ags_ffplayer_realize;
  widget->show = ags_ffplayer_show;

  /* AgsMachineClass */
  machine = (AgsMachineClass *) ffplayer;

  machine->map_recall = ags_ffplayer_map_recall;
}

void
ags_ffplayer_connectable_interface_init(AgsConnectableInterface *connectable)
{
  AgsConnectableInterface *ags_ffplayer_connectable_parent_interface;

  ags_ffplayer_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_ffplayer_connect;
}

void
ags_ffplayer_plugin_interface_init(AgsPluginInterface *plugin)
{
  plugin->get_name = ags_ffplayer_get_name;
  plugin->set_name = ags_ffplayer_set_name;
  plugin->get_xml_type = ags_ffplayer_get_xml_type;
  plugin->set_xml_type = ags_ffplayer_set_xml_type;
  plugin->read = ags_ffplayer_read;
  plugin->write = ags_ffplayer_write;
}

void
ags_ffplayer_init(AgsFFPlayer *ffplayer)
{
  GtkVBox *vbox;
  GtkAlignment *alignment;
  GtkTable *table;
  GtkHBox *hbox;
  GtkVBox *piano_vbox;
  GtkLabel *label;
  
  PangoAttrList *attr_list;
  PangoAttribute *attr;

  AgsAudio *audio;
  
  g_signal_connect_after((GObject *) ffplayer, "parent_set",
			 G_CALLBACK(ags_ffplayer_parent_set_callback), (gpointer) ffplayer);

  audio = AGS_MACHINE(ffplayer)->audio;
  audio->flags |= (AGS_AUDIO_SYNC |
		   AGS_AUDIO_ASYNC |
		   AGS_AUDIO_OUTPUT_HAS_RECYCLING |
		   AGS_AUDIO_INPUT_HAS_RECYCLING |
		   AGS_AUDIO_INPUT_TAKES_FILE |
		   AGS_AUDIO_HAS_NOTATION | 
		   AGS_AUDIO_NOTATION_DEFAULT |
		   AGS_AUDIO_REVERSE_MAPPING);
  g_object_set(audio,
	       "audio-start-mapping", 0,
	       "audio-end-mapping", 128,
	       "midi-start-mapping", 0,
	       "midi-end-mapping", 128,
	       NULL);
  
  AGS_MACHINE(ffplayer)->flags |= (AGS_MACHINE_IS_SYNTHESIZER |
				   AGS_MACHINE_REVERSE_NOTATION);
  AGS_MACHINE(ffplayer)->file_input_flags |= AGS_MACHINE_ACCEPT_SOUNDFONT2;

  AGS_MACHINE(ffplayer)->input_pad_type = G_TYPE_NONE;
  AGS_MACHINE(ffplayer)->input_line_type = G_TYPE_NONE;
  AGS_MACHINE(ffplayer)->output_pad_type = G_TYPE_NONE;
  AGS_MACHINE(ffplayer)->output_line_type = G_TYPE_NONE;

  /* context menu */
  ags_machine_popup_add_connection_options(ffplayer,
  					   (AGS_MACHINE_POPUP_MIDI_DIALOG));

  /* audio resize */
  g_signal_connect_after(G_OBJECT(ffplayer), "resize-audio-channels",
			 G_CALLBACK(ags_ffplayer_resize_audio_channels), NULL);

  g_signal_connect_after(G_OBJECT(ffplayer), "resize-pads",
			 G_CALLBACK(ags_ffplayer_resize_pads), NULL);

  /* flags */
  ffplayer->flags = 0;

  /* mapped IO */
  ffplayer->mapped_input_pad = 0;
  ffplayer->mapped_output_pad = 0;

  /* context menu */
  ags_machine_popup_add_edit_options((AgsMachine *) ffplayer,
				     (AGS_MACHINE_POPUP_ENVELOPE));
  
  /* name and xml type */
  ffplayer->name = NULL;
  ffplayer->xml_type = "ags-ffplayer";

  /* create widgets */
  vbox = (GtkVBox *) gtk_vbox_new(FALSE, 0);
  gtk_container_add((GtkContainer *) (gtk_bin_get_child((GtkBin *) ffplayer)),
		    (GtkWidget *) vbox);

  alignment = (GtkAlignment *) g_object_new(GTK_TYPE_ALIGNMENT,
					    "xalign", 0.0,
					    NULL);
  gtk_box_pack_start((GtkBox *) vbox,
		     (GtkWidget *) alignment,
		     FALSE, FALSE,
		     0);
  
  table = (GtkTable *) gtk_table_new(3, 2, FALSE);
  gtk_container_add((GtkContainer *) alignment,
		    (GtkWidget *) table);
  
  hbox = (GtkHBox *) gtk_hbox_new(FALSE, 0);
  gtk_table_attach(table,
		   GTK_WIDGET(hbox),
		   0, 1,
		   1, 2,
		   GTK_FILL, GTK_FILL,
		   0, 0);

  label = (GtkLabel *) g_object_new(GTK_TYPE_LABEL,
				    "label", i18n("preset"),
				    "xalign", 0.0,
				    NULL);
  gtk_box_pack_start(GTK_BOX(hbox),
		     GTK_WIDGET(label),
		     FALSE, FALSE,
		     0);

  ffplayer->preset = (GtkComboBoxText *) gtk_combo_box_text_new();
  gtk_box_pack_start(GTK_BOX(hbox),
		     GTK_WIDGET(ffplayer->preset),
		     TRUE, FALSE,
		     0);

  label = (GtkLabel *) g_object_new(GTK_TYPE_LABEL,
				    "label", i18n("instrument"),
				    "xalign", 0.0,
				    NULL);
  gtk_box_pack_start(GTK_BOX(hbox),
		     GTK_WIDGET(label),
		     FALSE, FALSE,
		     0);

  ffplayer->instrument = (GtkComboBoxText *) gtk_combo_box_text_new();
  gtk_box_pack_start(GTK_BOX(hbox),
		     GTK_WIDGET(ffplayer->instrument),
		     TRUE, FALSE,
		     0);

  ffplayer->open = (GtkButton *) g_object_new(GTK_TYPE_BUTTON,
					      "label", GTK_STOCK_OPEN,
					      "use-stock", TRUE,
					      NULL);
  gtk_box_pack_start(GTK_BOX(hbox),
		     GTK_WIDGET(ffplayer->open),
		     FALSE, FALSE,
		     0);
  ffplayer->open_dialog = NULL;
  
  piano_vbox = (GtkVBox *) gtk_vbox_new(FALSE, 2);
  gtk_table_attach(table, (GtkWidget *) piano_vbox,
		   1, 2,
		   0, 3,
		   GTK_FILL, GTK_FILL,
		   0, 0);

  ffplayer->control_width = 12;
  ffplayer->control_height = 40;

  ffplayer->drawing_area = (GtkDrawingArea *) gtk_drawing_area_new();
  gtk_widget_set_size_request((GtkWidget *) ffplayer->drawing_area,
			      16 * ffplayer->control_width,
			      ffplayer->control_width * 8 + ffplayer->control_height);
  gtk_widget_set_events((GtkWidget *) ffplayer->drawing_area,
			GDK_EXPOSURE_MASK
			| GDK_LEAVE_NOTIFY_MASK
			| GDK_BUTTON_PRESS_MASK
			| GDK_POINTER_MOTION_MASK
			| GDK_POINTER_MOTION_HINT_MASK);
  gtk_box_pack_start((GtkBox *) piano_vbox,
		     (GtkWidget *) ffplayer->drawing_area,
		     FALSE, FALSE,
		     0);

  ffplayer->hadjustment = (GtkAdjustment *) gtk_adjustment_new(0.0,
							       0.0,
							       76 * ffplayer->control_width - GTK_WIDGET(ffplayer->drawing_area)->allocation.width,
							       1.0,
							       (double) ffplayer->control_width,
							       (double) (16 * ffplayer->control_width));
  ffplayer->hscrollbar = (GtkHScrollbar *) gtk_hscrollbar_new(ffplayer->hadjustment);
  gtk_box_pack_start((GtkBox *) piano_vbox,
		     (GtkWidget *) ffplayer->hscrollbar,
		     FALSE, FALSE,
		     0);

  /* effect bridge */
  AGS_MACHINE(ffplayer)->bridge = (GtkContainer *) ags_ffplayer_bridge_new(audio);
  gtk_box_pack_start((GtkBox *) vbox,
		     (GtkWidget *) AGS_MACHINE(ffplayer)->bridge,
		     FALSE, FALSE,
		     0);
}

void
ags_ffplayer_map_recall(AgsMachine *machine)
{
  AgsWindow *window;
  AgsFFPlayer *ffplayer;
  
  AgsAudio *audio;

  AgsDelayAudio *play_delay_audio;
  AgsDelayAudioRun *play_delay_audio_run;
  AgsCountBeatsAudio *play_count_beats_audio;
  AgsCountBeatsAudioRun *play_count_beats_audio_run;
  AgsRecordMidiAudio *recall_record_midi_audio;
  AgsRecordMidiAudioRun *recall_record_midi_audio_run;
  AgsPlayNotationAudio *recall_notation_audio;
  AgsPlayNotationAudioRun *recall_notation_audio_run;

  GList *list;

  if((AGS_MACHINE_MAPPED_RECALL & (machine->flags)) != 0 ||
     (AGS_MACHINE_PREMAPPED_RECALL & (machine->flags)) != 0){
    return;
  }

  ffplayer = AGS_FFPLAYER(machine);
  
  window = (AgsWindow *) gtk_widget_get_ancestor((GtkWidget *) machine,
						 AGS_TYPE_WINDOW);

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

  list = ags_recall_find_type(audio->play, AGS_TYPE_DELAY_AUDIO_RUN);

  if(list != NULL){
    play_delay_audio_run = AGS_DELAY_AUDIO_RUN(list->data);
    //    AGS_RECALL(play_delay_audio_run)->flags |= AGS_RECALL_PERSISTENT;
  }else{
    play_delay_audio_run = NULL;
  }
  
  /* ags-count-beats */
  ags_recall_factory_create(audio,
			    NULL, NULL,
			    "ags-count-beats",
			    0, 0,
			    0, 0,
			    (AGS_RECALL_FACTORY_OUTPUT |
			     AGS_RECALL_FACTORY_ADD |
			     AGS_RECALL_FACTORY_PLAY),
			    0);
  
  list = ags_recall_find_type(audio->play, AGS_TYPE_COUNT_BEATS_AUDIO_RUN);

  if(list != NULL){
    GValue value = {0,};

    play_count_beats_audio_run = AGS_COUNT_BEATS_AUDIO_RUN(list->data);

    /* set dependency */  
    g_object_set(G_OBJECT(play_count_beats_audio_run),
		 "delay-audio-run", play_delay_audio_run,
		 NULL);
    ags_seekable_seek(AGS_SEEKABLE(play_count_beats_audio_run),
		      window->navigation->position_tact->adjustment->value,
		      TRUE);

    g_value_init(&value, G_TYPE_BOOLEAN);
    g_value_set_boolean(&value, gtk_toggle_button_get_active((GtkToggleButton *) window->navigation->loop));
    ags_port_safe_write(AGS_COUNT_BEATS_AUDIO(AGS_RECALL_AUDIO_RUN(play_count_beats_audio_run)->recall_audio)->notation_loop,
			&value);
  }else{
    play_count_beats_audio_run = NULL;
  }

  /* ags-record-midi */
  ags_recall_factory_create(audio,
			    NULL, NULL,
			    "ags-record-midi",
			    0, 0,
			    0, 0,
			    (AGS_RECALL_FACTORY_INPUT |
			     AGS_RECALL_FACTORY_ADD |
			     AGS_RECALL_FACTORY_RECALL),
			    0);

  list = ags_recall_find_type(audio->recall, AGS_TYPE_RECORD_MIDI_AUDIO_RUN);

  if(list != NULL){
    recall_record_midi_audio_run = AGS_RECORD_MIDI_AUDIO_RUN(list->data);
    
    /* set dependency */
    g_object_set(G_OBJECT(recall_record_midi_audio_run),
		 "delay-audio-run", play_delay_audio_run,
		 NULL);

    /* set dependency */
    g_object_set(G_OBJECT(recall_record_midi_audio_run),
		 "count-beats-audio-run", play_count_beats_audio_run,
		 NULL);
  }  

  /* ags-play-notation */
  ags_recall_factory_create(audio,
			    NULL, NULL,
			    "ags-play-notation",
			    0, 0,
			    0, 0,
			    (AGS_RECALL_FACTORY_INPUT |
			     AGS_RECALL_FACTORY_ADD |
			     AGS_RECALL_FACTORY_RECALL),
			    0);

  list = ags_recall_find_type(audio->recall, AGS_TYPE_PLAY_NOTATION_AUDIO_RUN);

  if(list != NULL){
    recall_notation_audio_run = AGS_PLAY_NOTATION_AUDIO_RUN(list->data);
    
    /* set dependency */
    g_object_set(G_OBJECT(recall_notation_audio_run),
		 "delay-audio-run", play_delay_audio_run,
		 NULL);

    /* set dependency */
    g_object_set(G_OBJECT(recall_notation_audio_run),
		 "count-beats-audio-run", play_count_beats_audio_run,
		 NULL);
  }

  /* depending on destination */
  ags_ffplayer_input_map_recall(ffplayer, 0);

  /* depending on destination */
  ags_ffplayer_output_map_recall(ffplayer, 0);

  /* call parent */
  AGS_MACHINE_CLASS(ags_ffplayer_parent_class)->map_recall(machine);  
}

void
ags_ffplayer_finalize(GObject *gobject)
{
  G_OBJECT_CLASS(ags_ffplayer_parent_class)->finalize(gobject);
}

void
ags_ffplayer_connect(AgsConnectable *connectable)
{
  AgsWindow *window;
  AgsFFPlayer *ffplayer;

  GList *list;

  if((AGS_MACHINE_CONNECTED & (AGS_MACHINE(connectable)->flags)) != 0){
    return;
  }

  ags_ffplayer_parent_connectable_interface->connect(connectable);

  ffplayer = AGS_FFPLAYER(connectable);

  window = (AgsWindow *) gtk_widget_get_toplevel((GtkWidget *) ffplayer);
  
  g_signal_connect((GObject *) ffplayer, "destroy",
		   G_CALLBACK(ags_ffplayer_destroy_callback), (gpointer) ffplayer);
  
  /* AgsFFPlayer */
  g_signal_connect((GObject *) ffplayer->open, "clicked",
		   G_CALLBACK(ags_ffplayer_open_clicked_callback), (gpointer) ffplayer);

  g_signal_connect_after((GObject *) ffplayer->preset, "changed",
			 G_CALLBACK(ags_ffplayer_preset_changed_callback), (gpointer) ffplayer);

  g_signal_connect_after((GObject *) ffplayer->instrument, "changed",
			 G_CALLBACK(ags_ffplayer_instrument_changed_callback), (gpointer) ffplayer);


  g_signal_connect((GObject *) ffplayer->drawing_area, "expose_event",
                   G_CALLBACK(ags_ffplayer_drawing_area_expose_callback), (gpointer) ffplayer);

  g_signal_connect((GObject *) ffplayer->drawing_area, "button_press_event",
                   G_CALLBACK(ags_ffplayer_drawing_area_button_press_callback), (gpointer) ffplayer);

  g_signal_connect((GObject *) ffplayer->hadjustment, "value_changed",
		   G_CALLBACK(ags_ffplayer_hscrollbar_value_changed), (gpointer) ffplayer);

  /* AgsAudio */  
  //TODO:JK: magnify it
  if(!gtk_toggle_button_get_active((GtkToggleButton *) window->navigation->loop)){
    GList *list;

    list = ags_recall_find_type(ffplayer->machine.audio->play, AGS_TYPE_COUNT_BEATS_AUDIO_RUN);
  
    if(list != NULL){
      AgsCountBeatsAudioRun *play_count_beats_audio_run;
      GValue value = {0,};

      play_count_beats_audio_run = AGS_COUNT_BEATS_AUDIO_RUN(list->data);
      g_value_init(&value, G_TYPE_BOOLEAN);
      g_value_set_boolean(&value, FALSE);
      ags_port_safe_write(AGS_COUNT_BEATS_AUDIO(AGS_RECALL_AUDIO_RUN(play_count_beats_audio_run)->recall_audio)->notation_loop,
			  &value);
    }
  }
}

void
ags_ffplayer_disconnect(AgsConnectable *connectable)
{
  AgsFFPlayer *ffplayer;

  GList *list;

  if((AGS_MACHINE_CONNECTED & (AGS_MACHINE(connectable)->flags)) == 0){
    return;
  }

  ags_ffplayer_parent_connectable_interface->disconnect(connectable);

  ffplayer = AGS_FFPLAYER(connectable);

  g_object_disconnect((GObject *) ffplayer,
		      "any_signal::destroy",
		      G_CALLBACK(ags_ffplayer_destroy_callback),
		      (gpointer) ffplayer,
		      NULL);
  
  /* AgsFFPlayer */
  g_object_disconnect((GObject *) ffplayer->open,
		      "any_signal::clicked",
		      G_CALLBACK(ags_ffplayer_open_clicked_callback),
		      (gpointer) ffplayer,
		      NULL);

  g_object_disconnect((GObject *) ffplayer->preset,
		      "any_signal::changed",
		      G_CALLBACK(ags_ffplayer_preset_changed_callback),
		      (gpointer) ffplayer,
		      NULL);

  g_object_disconnect((GObject *) ffplayer->instrument,
		      "any_signal::changed",
		      G_CALLBACK(ags_ffplayer_instrument_changed_callback),
		      (gpointer) ffplayer,
		      NULL);

  g_object_disconnect((GObject *) ffplayer->drawing_area,
		      "any_signal::expose_event",
		      G_CALLBACK(ags_ffplayer_drawing_area_expose_callback),
		      (gpointer) ffplayer,
		      NULL);

  g_object_disconnect((GObject *) ffplayer->drawing_area,
		      "any_signal::button_press_event",
		      G_CALLBACK(ags_ffplayer_drawing_area_button_press_callback),
		      (gpointer) ffplayer,
		      NULL);

  g_object_disconnect((GObject *) ffplayer->hadjustment,
		      "any_signal::value_changed",
		      G_CALLBACK(ags_ffplayer_hscrollbar_value_changed),
		      (gpointer) ffplayer,
		      NULL);
}

void
ags_ffplayer_realize(GtkWidget *widget)
{
  AgsFFPlayer *ffplayer;

  ffplayer = widget;
  
  /* call parent */
  GTK_WIDGET_CLASS(ags_ffplayer_parent_class)->realize(widget);

  if(ffplayer_style == NULL){
    ffplayer_style = gtk_style_copy(gtk_widget_get_style(ffplayer));
  }
  
  gtk_widget_set_style((GtkWidget *) ffplayer->drawing_area,
		       ffplayer_style);

  gtk_widget_set_style((GtkWidget *) ffplayer->hscrollbar,
		       ffplayer_style);
}

void
ags_ffplayer_show(GtkWidget *widget)
{
  GTK_WIDGET_CLASS(ags_ffplayer_parent_class)->show(widget);
}

gchar*
ags_ffplayer_get_name(AgsPlugin *plugin)
{
  return(AGS_FFPLAYER(plugin)->name);
}

void
ags_ffplayer_set_name(AgsPlugin *plugin, gchar *name)
{
  AGS_FFPLAYER(plugin)->name = name;
}

gchar*
ags_ffplayer_get_xml_type(AgsPlugin *plugin)
{
  return(AGS_FFPLAYER(plugin)->xml_type);
}

void
ags_ffplayer_set_xml_type(AgsPlugin *plugin, gchar *xml_type)
{
  AGS_FFPLAYER(plugin)->xml_type = xml_type;
}

void
ags_ffplayer_read(AgsFile *file, xmlNode *node, AgsPlugin *plugin)
{
  AgsFFPlayer *gobject;
  AgsFileLookup *file_lookup;
  AgsFileLaunch *file_launch;
  GList *list;
  
  gobject = AGS_FFPLAYER(plugin);

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context", file->application_context,
				   "file", file,
				   "node", node,
				   "xpath", g_strdup_printf("xpath=//*[@id='%s']", xmlGetProp(node, AGS_FILE_ID_PROP)),
				   "reference", gobject,
				   NULL));

  /* lookup */
  list = file->lookup;

  while((list = ags_file_lookup_find_by_node(list,
					     node->parent)) != NULL){
    file_lookup = AGS_FILE_LOOKUP(list->data);
    
    if(g_signal_handler_find(list->data,
			     G_SIGNAL_MATCH_FUNC,
			     0,
			     0,
			     NULL,
			     ags_file_read_machine_resolve_audio,
			     NULL) != 0){
      g_signal_connect_after(G_OBJECT(file_lookup), "resolve",
			     G_CALLBACK(ags_ffplayer_read_resolve_audio), gobject);
      
      break;
    }

    list = list->next;
  }

  /* launch */
  file_launch = (AgsFileLaunch *) g_object_new(AGS_TYPE_FILE_LAUNCH,
					       "node", node,
					       NULL);
  g_signal_connect(G_OBJECT(file_launch), "start",
		   G_CALLBACK(ags_ffplayer_launch_task), gobject);
  ags_file_add_launch(file,
		      G_OBJECT(file_launch));
}

void
ags_ffplayer_read_resolve_audio(AgsFileLookup *file_lookup,
				AgsMachine *machine)
{
  AgsFFPlayer *ffplayer;

  ffplayer = AGS_FFPLAYER(machine);

  g_signal_connect_after(G_OBJECT(machine), "resize-audio-channels",
			 G_CALLBACK(ags_ffplayer_resize_audio_channels), ffplayer);

  g_signal_connect_after(G_OBJECT(machine), "resize-pads",
			 G_CALLBACK(ags_ffplayer_resize_pads), ffplayer);

  if((AGS_MACHINE_PREMAPPED_RECALL & (machine->flags)) == 0){
    /* ags-play-notation */
    ags_recall_factory_create(machine->audio,
			      NULL, NULL,
			      "ags-play-notation",
			      0, machine->audio->audio_channels,
			      0, 0,
			      (AGS_RECALL_FACTORY_INPUT |
			       AGS_RECALL_FACTORY_REMAP |
			       AGS_RECALL_FACTORY_RECALL),
			      0);

    ags_ffplayer_output_map_recall(ffplayer, 0);
    ags_ffplayer_input_map_recall(ffplayer, 0);
  }else{
    ffplayer->mapped_output_pad = machine->audio->output_pads;
    ffplayer->mapped_input_pad = machine->audio->input_pads;
  }
}

void
ags_ffplayer_launch_task(AgsFileLaunch *file_launch, AgsFFPlayer *ffplayer)
{
  AgsWindow *window;
  AgsFFPlayer *gobject;
  GtkTreeModel *list_store;
  GtkTreeIter iter;
  xmlNode *node;
  gchar *filename;
  gchar *preset, *instrument;

  window = AGS_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(ffplayer)));
  node = file_launch->node;

  filename = xmlGetProp(node,
			"filename");

  if(filename == NULL){
    return;
  }

  if(g_str_has_suffix(filename, ".sf2")){
    AgsIpatch *ipatch;
    AgsPlayable *playable;
    gchar **preset, **instrument, *selected;
    GError *error;

    /* clear preset, instrument and sample*/
    gtk_list_store_clear(GTK_LIST_STORE(gtk_combo_box_get_model(GTK_COMBO_BOX(ffplayer->instrument))));

    /* Ipatch related */
    ffplayer->ipatch =
      ipatch = g_object_new(AGS_TYPE_IPATCH,
			    "mode", AGS_IPATCH_READ,
			    "filename", filename,
			    NULL);
    ipatch->soundcard = window->soundcard;
    ags_playable_open(AGS_PLAYABLE(ipatch), filename);

    playable = AGS_PLAYABLE(ipatch);
      
    ags_playable_open(playable, filename);

    error = NULL;
    ipatch->nth_level = 0;
    ags_playable_level_select(playable,
			      0, filename,
			      &error);

    if(error != NULL){
      g_warning("%s", error->message);
    }
    
    /* select first preset */
    ipatch->nth_level = 1;
    preset = ags_playable_sublevel_names(playable);

    error = NULL;
    ags_playable_level_select(playable,
			      1, *preset,
			      &error);

    if(error != NULL){
      g_warning("%s", error->message);
    }
    
    /* fill ffplayer->preset */
    while(*preset != NULL){
      gtk_combo_box_text_append_text(ffplayer->preset,
				     *preset);

      preset++;
    }

    /* Get the first iter in the list */
    selected = xmlGetProp(node,
			  "preset");

    list_store = gtk_combo_box_get_model((GtkComboBox *) ffplayer->preset);

    if(gtk_tree_model_get_iter_first(list_store, &iter)){
      do{
	gchar *str;
	
	gtk_tree_model_get(list_store, &iter,
			   0, &str,
			   -1);
	if(!g_strcmp0(selected,
		      str)){
	  break;
	}
      }while(gtk_tree_model_iter_next(list_store, &iter));
      
      gtk_combo_box_set_active_iter(GTK_COMBO_BOX(ffplayer->preset),
				    &iter);
    }
  
    /* select first instrument */
    ipatch->nth_level = 2;
    instrument = ags_playable_sublevel_names(playable);

    error = NULL;
    ags_playable_level_select(playable,
			      2, *instrument,
			      &error);

    if(error != NULL){
      g_warning("%s", error->message);
    }
    
    /* fill ffplayer->instrument */
    while(*instrument != NULL){
      gtk_combo_box_text_append_text(ffplayer->instrument,
				     *instrument);

      instrument++;
    }

    /* Get the first iter in the list */
    selected = xmlGetProp(node,
			  "instrument");

    list_store = gtk_combo_box_get_model((GtkComboBox *) ffplayer->instrument);

    if(gtk_tree_model_get_iter_first(list_store, &iter)){
      do{
	gchar *str;

	gtk_tree_model_get(list_store, &iter,
			   0, &str,
			   -1);

	if(!g_strcmp0(selected,
		      str)){
	  break;
	}
      }while(gtk_tree_model_iter_next(list_store, &iter));

      gtk_combo_box_set_active_iter(GTK_COMBO_BOX(ffplayer->instrument),
				    &iter);
    }
  }
}

xmlNode*
ags_ffplayer_write(AgsFile *file, xmlNode *parent, AgsPlugin *plugin)
{
  AgsFFPlayer *ffplayer;

  xmlNode *node;

  gchar *id;

  ffplayer = AGS_FFPLAYER(plugin);

  id = ags_id_generator_create_uuid();
  
  node = xmlNewNode(NULL,
		    "ags-ffplayer");
  xmlNewProp(node,
	     AGS_FILE_ID_PROP,
	     id);

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context", file->application_context,
				   "file", file,
				   "node", node,
				   "xpath", g_strdup_printf("xpath=//*[@id='%s']", id),
				   "reference", ffplayer,
				   NULL));

  if(ffplayer->ipatch != NULL && ffplayer->ipatch->filename != NULL){
    xmlNewProp(node,
	       "filename",
	       g_strdup(ffplayer->ipatch->filename));

    xmlNewProp(node,
	       "preset",
	       g_strdup(gtk_combo_box_text_get_active_text((GtkComboBoxText *) ffplayer->preset)));

    xmlNewProp(node,
	       "instrument",
	       g_strdup(gtk_combo_box_text_get_active_text((GtkComboBoxText *) ffplayer->instrument)));
  }

  xmlAddChild(parent,
	      node);

  return(node);
}

void
ags_ffplayer_resize_audio_channels(AgsMachine *machine,
				   guint audio_channels, guint audio_channels_old,
				   gpointer data)
{
  AgsAudio *audio;
  
  AgsMutexManager *mutex_manager;

  AgsConfig *config;

  gchar *str;

  guint input_pads;
  gboolean rt_safe;
  gboolean performance_mode;

  pthread_mutex_t *application_mutex;
  pthread_mutex_t *audio_mutex;

  mutex_manager = ags_mutex_manager_get_instance();
  application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);
  
  config = ags_config_get_instance();

  audio = machine->audio;

  /* get audio mutex */
  pthread_mutex_lock(application_mutex);

  audio_mutex = ags_mutex_manager_lookup(mutex_manager,
					 (GObject *) audio);
  
  pthread_mutex_unlock(application_mutex);

  /* get some fields */
  pthread_mutex_lock(audio_mutex);

  input_pads = audio->input_pads;
  
  pthread_mutex_unlock(audio_mutex);

  /*  */
  if(audio_channels > audio_channels_old){
    /* map dependending on output */
    str = ags_config_get_value(config,
			       AGS_CONFIG_GENERIC,
			       "engine-mode");
    rt_safe = TRUE;
    performance_mode = TRUE;

    str = ags_config_get_value(config,
			       AGS_CONFIG_GENERIC,
			       "rt-safe");

    if(str != NULL &&
       !g_ascii_strncasecmp(str,
			    "FALSE",
			    6)){
      rt_safe = FALSE;
    }

    str = ags_config_get_value(config,
			       AGS_CONFIG_GENERIC,
			       "engine-mode");

    if(str != NULL &&
       !g_ascii_strncasecmp(str,
			    "performance",
			    12)){
      performance_mode = TRUE;
    }else{
      performance_mode = FALSE;
    }
  
    if(rt_safe ||
       performance_mode){
      /* ags-copy */
      ags_recall_factory_create(audio,
				NULL, NULL,
				"ags-copy",
				audio_channels_old, audio_channels,
				0, input_pads,
				(AGS_RECALL_FACTORY_INPUT |
				 AGS_RECALL_FACTORY_RECALL |
				 AGS_RECALL_FACTORY_ADD),
				0);

      /* set performance mode */
      performance_mode = TRUE;
    }else{    
      /* ags-buffer */
      ags_recall_factory_create(audio,
				NULL, NULL,
				"ags-buffer",
				audio_channels_old, audio_channels,
				0, input_pads,
				(AGS_RECALL_FACTORY_INPUT |
				 AGS_RECALL_FACTORY_RECALL |
				 AGS_RECALL_FACTORY_ADD),
				0);
    }
    
    /* ags-play */
    ags_recall_factory_create(audio,
			      NULL, NULL,
			      "ags-play",
			      audio_channels_old, audio_channels, 
			      0, input_pads,
			      (AGS_RECALL_FACTORY_INPUT |
			       AGS_RECALL_FACTORY_PLAY |
			       AGS_RECALL_FACTORY_ADD),
			      0);

    /* ags-envelope */
    ags_recall_factory_create(audio,
			      NULL, NULL,
			      "ags-envelope",
			      audio_channels_old, audio_channels, 
			      0, input_pads,
			      (AGS_RECALL_FACTORY_INPUT |
			       AGS_RECALL_FACTORY_PLAY |
			       AGS_RECALL_FACTORY_RECALL | 
			       AGS_RECALL_FACTORY_ADD),
			      0);

    /* ags-stream */
    if(!rt_safe){
      ags_recall_factory_create(audio,
				NULL, NULL,
				"ags-stream",
				audio_channels_old, audio_channels, 
				0, input_pads,
				(AGS_RECALL_FACTORY_INPUT |
				 AGS_RECALL_FACTORY_PLAY |
				 AGS_RECALL_FACTORY_RECALL | 
				 AGS_RECALL_FACTORY_ADD),
				0);
    }else{
      ags_recall_factory_create(audio,
				NULL, NULL,
				"ags-rt-stream",
				audio_channels_old, audio_channels, 
				0, input_pads,
				(AGS_RECALL_FACTORY_INPUT |
				 AGS_RECALL_FACTORY_PLAY |
				 AGS_RECALL_FACTORY_RECALL | 
				 AGS_RECALL_FACTORY_ADD),
				0);
    }
    
    /* AgsOutput */
    /* ags-stream */
    if(!performance_mode){
      ags_recall_factory_create(audio,
				NULL, NULL,
				"ags-stream",
				audio_channels_old, audio_channels,
				0, audio->output_pads,
				(AGS_RECALL_FACTORY_OUTPUT |
				 AGS_RECALL_FACTORY_PLAY |
				 AGS_RECALL_FACTORY_ADD),
				0);
    }
  }
}

void
ags_ffplayer_resize_pads(AgsMachine *machine, GType type,
			 guint pads, guint pads_old,
			 gpointer data)
{
  AgsFFPlayer *ffplayer;

  AgsAudio *audio;

  AgsMutexManager *mutex_manager;

  guint output_pads, input_pads;
  gboolean grow;

  pthread_mutex_t *application_mutex;
  pthread_mutex_t *audio_mutex;
  
  if(pads_old == pads){
    return;
  }
  
  mutex_manager = ags_mutex_manager_get_instance();
  application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);
  
  ffplayer = machine;

  audio = machine->audio;

  /* get audio mutex */
  pthread_mutex_lock(application_mutex);

  audio_mutex = ags_mutex_manager_lookup(mutex_manager,
					 (GObject *) audio);
  
  pthread_mutex_unlock(application_mutex);

  /* get some fields */
  pthread_mutex_lock(audio_mutex);

  output_pads = audio->output_pads;
  input_pads = audio->input_pads;
  
  pthread_mutex_unlock(audio_mutex);

  /* check grow */
  if(pads_old < pads){
    grow = TRUE;
  }else{
    grow = FALSE;
  }
  
  if(type == AGS_TYPE_INPUT){
    if(grow){
      /* depending on destination */
      ags_ffplayer_input_map_recall(ffplayer, pads_old);
    }else{
      ffplayer->mapped_input_pad = input_pads;
    }
  }else{
    if(grow){
      /* depending on destination */
      ags_ffplayer_output_map_recall(ffplayer, pads_old);
    }else{
      ffplayer->mapped_output_pad = output_pads;
    }
  }
}

void
ags_ffplayer_input_map_recall(AgsFFPlayer *ffplayer, guint input_pad_start)
{
  AgsAudio *audio;
  AgsChannel *source, *current;

  AgsMutexManager *mutex_manager;

  AgsConfig *config;

  GList *list;

  gchar *str;

  guint input_pads;
  guint audio_channels;
  gboolean rt_safe;
  gboolean performance_mode;

  pthread_mutex_t *application_mutex;
  pthread_mutex_t *audio_mutex;

  if(ffplayer->mapped_input_pad > input_pad_start){
    return;
  }

  mutex_manager = ags_mutex_manager_get_instance();
  application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);

  config = ags_config_get_instance();

  audio = AGS_MACHINE(ffplayer)->audio;

  /* get audio mutex */
  pthread_mutex_lock(application_mutex);

  audio_mutex = ags_mutex_manager_lookup(mutex_manager,
					 (GObject *) audio);
  
  pthread_mutex_unlock(application_mutex);

  /* get some fields */
  pthread_mutex_lock(audio_mutex);

  input_pads = audio->input_pads;

  audio_channels = audio->audio_channels;
  
  pthread_mutex_unlock(audio_mutex);

  /* map dependending on output */
    rt_safe = TRUE;
    performance_mode = TRUE;

    str = ags_config_get_value(config,
			       AGS_CONFIG_GENERIC,
			       "rt-safe");

    if(str != NULL &&
       !g_ascii_strncasecmp(str,
			    "FALSE",
			    6)){
      rt_safe = FALSE;
    }

    str = ags_config_get_value(config,
			       AGS_CONFIG_GENERIC,
			       "engine-mode");

    if(str != NULL &&
       !g_ascii_strncasecmp(str,
			    "performance",
			    12)){
      performance_mode = TRUE;
    }else{
      performance_mode = FALSE;
    }
  
  /* remap for input */
  if(rt_safe ||
     performance_mode){
    /* ags-copy */
    ags_recall_factory_create(audio,
			      NULL, NULL,
			      "ags-copy",
			      0, audio_channels,
			      input_pad_start, input_pads,
			      (AGS_RECALL_FACTORY_INPUT |
			       AGS_RECALL_FACTORY_RECALL |
			       AGS_RECALL_FACTORY_ADD),
			      0);
    
    /* set performance mode */
    performance_mode = TRUE;
  }else{
    /* ags-buffer */
    ags_recall_factory_create(audio,
			      NULL, NULL,
			      "ags-buffer",
			      0, audio_channels,
			      input_pad_start, input_pads,
			      (AGS_RECALL_FACTORY_INPUT |
			       AGS_RECALL_FACTORY_RECALL |
			       AGS_RECALL_FACTORY_ADD),
			      0);
  }
  
  /* ags-play */
  ags_recall_factory_create(audio,
			    NULL, NULL,
			    "ags-play",
			    0, audio_channels,
			    input_pad_start, input_pads,
			    (AGS_RECALL_FACTORY_INPUT |
			     AGS_RECALL_FACTORY_PLAY |
			     AGS_RECALL_FACTORY_ADD),
			    0);

  /* ags-feed */
  ags_recall_factory_create(audio,
			    NULL, NULL,
			    "ags-feed",
			    0, audio_channels,
			    input_pad_start, input_pads,
			    (AGS_RECALL_FACTORY_INPUT |
			     AGS_RECALL_FACTORY_PLAY |
			     AGS_RECALL_FACTORY_RECALL | 
			     AGS_RECALL_FACTORY_ADD),
			    0);

  /* ags-envelope */
  ags_recall_factory_create(audio,
			    NULL, NULL,
			    "ags-envelope",
			    0, audio_channels,
			    input_pad_start, input_pads,
			    (AGS_RECALL_FACTORY_INPUT |
			     AGS_RECALL_FACTORY_PLAY |
			     AGS_RECALL_FACTORY_RECALL | 
			     AGS_RECALL_FACTORY_ADD),
			    0);
  /* ags-stream */
  if(!rt_safe){
    ags_recall_factory_create(audio,
			      NULL, NULL,
			      "ags-stream",
			      0, audio_channels,
			      input_pad_start, input_pads,
			      (AGS_RECALL_FACTORY_INPUT |
			       AGS_RECALL_FACTORY_PLAY |
			       AGS_RECALL_FACTORY_RECALL | 
			       AGS_RECALL_FACTORY_ADD),
			      0);
  }else{
    ags_recall_factory_create(audio,
			      NULL, NULL,
			      "ags-rt-stream",
			      0, audio_channels,
			      input_pad_start, input_pads,
			      (AGS_RECALL_FACTORY_INPUT |
			       AGS_RECALL_FACTORY_PLAY |
			       AGS_RECALL_FACTORY_RECALL | 
			       AGS_RECALL_FACTORY_ADD),
			      0);
  }
  
  ffplayer->mapped_input_pad = input_pads;
}

void
ags_ffplayer_output_map_recall(AgsFFPlayer *ffplayer, guint output_pad_start)
{
  AgsAudio *audio;

  AgsMutexManager *mutex_manager;

  AgsConfig *config;

  gchar *str;

  guint output_pads, input_pads;
  guint audio_channels;
  gboolean rt_safe;
  gboolean performance_mode;

  pthread_mutex_t *application_mutex;
  pthread_mutex_t *audio_mutex;

  if(ffplayer->mapped_output_pad > output_pad_start){
    return;
  }

  mutex_manager = ags_mutex_manager_get_instance();
  application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);

  config = ags_config_get_instance();

  audio = AGS_MACHINE(ffplayer)->audio;

  /* get audio mutex */
  pthread_mutex_lock(application_mutex);

  audio_mutex = ags_mutex_manager_lookup(mutex_manager,
					 (GObject *) audio);
  
  pthread_mutex_unlock(application_mutex);

  /* get some fields */
  pthread_mutex_lock(audio_mutex);

  output_pads = audio->output_pads;
  input_pads = audio->input_pads;

  audio_channels = audio->audio_channels;
  
  pthread_mutex_unlock(audio_mutex);
  
  /* map dependending on output */
  rt_safe = TRUE;
  performance_mode = TRUE;

  str = ags_config_get_value(config,
			     AGS_CONFIG_GENERIC,
			     "rt-safe");

  if(str != NULL &&
     !g_ascii_strncasecmp(str,
			  "FALSE",
			  6)){
    rt_safe = FALSE;
  }

  str = ags_config_get_value(config,
			     AGS_CONFIG_GENERIC,
			     "engine-mode");

  if(str != NULL &&
     !g_ascii_strncasecmp(str,
			  "performance",
			  12)){
    performance_mode = TRUE;
  }else{
    performance_mode = FALSE;
  }

  /* remap for input */
  if(rt_safe ||
     performance_mode){
    /* ags-copy */
    ags_recall_factory_create(audio,
			      NULL, NULL,
			      "ags-copy",
			      0, audio_channels, 
			      0, input_pads,
			      (AGS_RECALL_FACTORY_INPUT |
			       AGS_RECALL_FACTORY_RECALL |
			       AGS_RECALL_FACTORY_ADD),
			      0);
    
    /* set performance mode */
    performance_mode = TRUE;
  }else{
    /* ags-buffer */
    ags_recall_factory_create(audio,
			      NULL, NULL,
			      "ags-buffer",
			      0, audio_channels, 
			      0, input_pads,
			      (AGS_RECALL_FACTORY_INPUT |
			       AGS_RECALL_FACTORY_RECALL |
			       AGS_RECALL_FACTORY_ADD),
			      0);
  }

  if(!performance_mode){
    /* ags-stream */
    ags_recall_factory_create(audio,
			      NULL, NULL,
			      "ags-stream",
			      0, audio_channels,
			      output_pad_start, output_pads,
			      (AGS_RECALL_FACTORY_OUTPUT |
			       AGS_RECALL_FACTORY_PLAY |
			       AGS_RECALL_FACTORY_ADD),
			      0);
  }
  
  ffplayer->mapped_output_pad = output_pads;
}

void
ags_ffplayer_paint(AgsFFPlayer *ffplayer)
{
  GtkWidget *widget;

  GtkStyle *ffplayer_style;
  
  cairo_t *cr;

  double semi_key_height;
  guint bitmap;
  guint x[2];
  guint i, i_stop, j, j0;
  
  static const gdouble white_gc = 65535.0;
    
  widget = (GtkWidget *) ffplayer->drawing_area;
  ffplayer_style = gtk_widget_get_style(widget);

  semi_key_height = 2.0 / 3.0 * (double) ffplayer->control_height;
  bitmap = 0x52a52a; // description of the keyboard

  j = (guint) ceil(ffplayer->hadjustment->value / (double) ffplayer->control_width);
  j = j % 12;

  x[0] = (guint) round(ffplayer->hadjustment->value) % ffplayer->control_width;

  if(x[0] != 0){
    x[0] = ffplayer->control_width - x[0];
  }

  x[1] = ((guint) widget->allocation.width - x[0]) % ffplayer->control_width;
  i_stop = (widget->allocation.width - x[0] - x[1]) / ffplayer->control_width;

  cr = gdk_cairo_create(widget->window);

  /* clear with background color */
  cairo_set_source_rgb(cr,
		       ffplayer_style->bg[0].red / white_gc,
		       ffplayer_style->bg[0].green / white_gc,
		       ffplayer_style->bg[0].blue / white_gc);
  cairo_rectangle(cr, 0.0, 0.0, (double) widget->allocation.width, (double) widget->allocation.height);
  cairo_fill(cr);

  /* draw piano */
  cairo_set_line_width(cr, 1.0);

  cairo_set_source_rgb(cr,
		       ffplayer_style->fg[0].red / white_gc,
		       ffplayer_style->fg[0].green / white_gc,
		       ffplayer_style->fg[0].blue / white_gc);

  if(x[0] != 0){
    j0 = (j != 0) ? j -1: 11;

    if(((1 << j0) & bitmap) != 0){
      cairo_rectangle(cr, 0.0, 0.0, x[0], (double) semi_key_height);
      cairo_fill(cr); 	

      if(x[0] > ffplayer->control_width / 2){
	cairo_move_to(cr, (double) (x[0] - ffplayer->control_width / 2),  semi_key_height);
	cairo_line_to(cr, (double) (x[0] - ffplayer->control_width / 2), (double) ffplayer->control_height);
	cairo_stroke(cr);
      }

      cairo_move_to(cr, 0.0, ffplayer->control_height);
      cairo_line_to(cr, (double) x[0], ffplayer->control_height);
      cairo_stroke(cr);
    }else{
      if(((1 << (j0 + 1)) & bitmap) == 0){
	cairo_move_to(cr, (double) x[0], 0.0);
	cairo_line_to(cr, (double) x[0], ffplayer->control_height);
	cairo_stroke(cr);
      }

      cairo_move_to(cr, 0.0, ffplayer->control_height);
      cairo_line_to(cr, (double) x[0], ffplayer->control_height);
      cairo_stroke(cr);
    }
  }

  for(i = 0; i < i_stop; i++){
    if(((1 << j) & bitmap) != 0){
      // draw semi tone key
      cairo_rectangle(cr, (double) (i * ffplayer->control_width + x[0]), 0.0, (double) ffplayer->control_width, semi_key_height);
      cairo_fill(cr); 	

      cairo_move_to(cr, (double) (i * ffplayer->control_width + x[0] + ffplayer->control_width / 2), semi_key_height);
      cairo_line_to(cr, (double) (i * ffplayer->control_width + x[0] + ffplayer->control_width / 2), ffplayer->control_height);
      cairo_stroke(cr);

      cairo_move_to(cr, (double) (i * ffplayer->control_width + x[0]), ffplayer->control_height);
      cairo_line_to(cr, (double) (i * ffplayer->control_width + x[0] + ffplayer->control_width), ffplayer->control_height);
      cairo_stroke(cr);
    }else{
      // no semi tone key
      if(((1 << (j + 1)) & bitmap) == 0){
	cairo_move_to(cr, (double) (i * ffplayer->control_width + x[0] + ffplayer->control_width), 0.0);
	cairo_line_to(cr, (double) (i * ffplayer->control_width + x[0] + ffplayer->control_width), ffplayer->control_height);
	cairo_stroke(cr);
      }

      cairo_move_to(cr, (double) (i * ffplayer->control_width + x[0]), ffplayer->control_height);
      cairo_line_to(cr, (double) (i * ffplayer->control_width + x[0] + ffplayer->control_width), ffplayer->control_height);
      cairo_stroke(cr);
    }

    if(j == 11)
      j = 0;
    else
      j++;
  }

  if(x[1] != 0){
    j0 = j;

    if(((1 << j0) & bitmap) != 0){
      cairo_rectangle(cr, (double) (widget->allocation.width - x[1]), 0.0, (double) x[1], semi_key_height);
      cairo_fill(cr); 	

      if(x[1] > ffplayer->control_width / 2){
	cairo_move_to(cr, (double) (widget->allocation.width - x[1] + ffplayer->control_width / 2), semi_key_height);
	cairo_line_to(cr, (double) (widget->allocation.width - x[1] + ffplayer->control_width / 2), ffplayer->control_height);
	cairo_stroke(cr);
      }

      cairo_move_to(cr, (double) (widget->allocation.width - x[1]), ffplayer->control_height);
      cairo_line_to(cr, (double) widget->allocation.width, ffplayer->control_height);
      cairo_stroke(cr);
    }else{
      cairo_move_to(cr, (double) (widget->allocation.width - x[1]), ffplayer->control_height);
      cairo_line_to(cr, (double) widget->allocation.width, ffplayer->control_height);
      cairo_stroke(cr);
    }
  }
  
  cairo_surface_mark_dirty(cairo_get_target(cr));
  cairo_destroy(cr);
}

void
ags_ffplayer_open_filename(AgsFFPlayer *ffplayer,
			   gchar *filename)
{
  if(filename == NULL){
    return;
  }
  
  if(g_str_has_suffix(filename, ".sf2")){
    AgsWindow *window;
    
    AgsIpatch *ipatch;
    AgsPlayable *playable;

    GError *error;

    window = (AgsWindow *) gtk_widget_get_toplevel((GtkWidget *) ffplayer);
    
    /* clear preset and instrument */
    gtk_list_store_clear(GTK_LIST_STORE(gtk_combo_box_get_model(GTK_COMBO_BOX(ffplayer->preset))));
    gtk_list_store_clear(GTK_LIST_STORE(gtk_combo_box_get_model(GTK_COMBO_BOX(ffplayer->instrument))));

    /* Ipatch related */
    ipatch = g_object_new(AGS_TYPE_IPATCH,
			  "mode", AGS_IPATCH_READ,
			  "filename", filename,
			  NULL);
    ffplayer->ipatch = ipatch;
    ipatch->soundcard = window->soundcard;

    playable = AGS_PLAYABLE(ipatch);

    error = NULL;
    ags_playable_level_select(playable,
			      0, filename,
			      &error);

    ags_ffplayer_load_preset(ffplayer);
    ags_ffplayer_load_instrument(ffplayer);
  }
}

void
ags_ffplayer_load_preset(AgsFFPlayer *ffplayer)
{
  AgsPlayable *playable;

  gchar **preset;

  GError *error;

  playable = AGS_PLAYABLE(ffplayer->ipatch);

  /* select first preset */
  ffplayer->ipatch->nth_level = 1;
  preset = ags_playable_sublevel_names(playable);

  error = NULL;
  ags_playable_level_select(playable,
			    1, *preset,
			    &error);

  /* fill ffplayer->preset */
  while(preset != NULL && preset[0] != NULL){
    gtk_combo_box_text_append_text(ffplayer->preset,
				   *preset);
    
    preset++;
  }
}

void
ags_ffplayer_load_instrument(AgsFFPlayer *ffplayer)
{
  AgsPlayable *playable;

  gchar **instrument;

  GError *error;
  
  playable = AGS_PLAYABLE(ffplayer->ipatch);

  ffplayer->ipatch->nth_level = 2;
  instrument = ags_playable_sublevel_names(playable);
  
  error = NULL;
  ags_playable_level_select(playable,
			    2, *instrument,
			    &error);
  
  /* fill ffplayer->instrument */
  while(instrument != NULL && instrument[0] != NULL){
    gtk_combo_box_text_append_text(ffplayer->instrument,
				   *instrument);

    instrument++;
  }
}

/**
 * ags_ffplayer_new:
 * @soundcard: the assigned soundcard.
 *
 * Creates an #AgsFFPlayer
 *
 * Returns: a new #AgsFFPlayer
 *
 * Since: 1.0.0
 */
AgsFFPlayer*
ags_ffplayer_new(GObject *soundcard)
{
  AgsFFPlayer *ffplayer;
  GValue value = {0,};

  ffplayer = (AgsFFPlayer *) g_object_new(AGS_TYPE_FFPLAYER,
					  NULL);

  if(soundcard != NULL){
    g_value_init(&value, G_TYPE_OBJECT);
    g_value_set_object(&value, soundcard);
    g_object_set_property(G_OBJECT(ffplayer->machine.audio),
			  "soundcard", &value);
    g_value_unset(&value);
  }

  return(ffplayer);
}
