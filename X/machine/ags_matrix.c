/* AGS - Advanced GTK Sequencer
 * Copyright (C) 2005-2011 Joël Krähemann
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <ags/X/machine/ags_matrix.h>
#include <ags/X/machine/ags_matrix_callbacks.h>

#include <ags/object/ags_connectable.h>

#include <ags/audio/ags_audio.h>
#include <ags/audio/ags_channel.h>
#include <ags/audio/ags_input.h>
#include <ags/audio/ags_output.h>
#include <ags/audio/ags_pattern.h>
#include <ags/audio/ags_recall.h>

#include <ags/audio/recall/ags_delay_audio.h>
#include <ags/audio/recall/ags_delay_audio_run.h>
#include <ags/audio/recall/ags_copy_channel.h>
#include <ags/audio/recall/ags_stream_channel.h>
#include <ags/audio/recall/ags_copy_pattern_audio.h>
#include <ags/audio/recall/ags_copy_pattern_audio_run.h>
#include <ags/audio/recall/ags_copy_pattern_channel.h>
#include <ags/audio/recall/ags_copy_pattern_channel_run.h>

#define AGS_MATRIX_CELL_WIDTH   12
#define AGS_MATRIX_CELL_HEIGHT  10

void ags_matrix_class_init(AgsMatrixClass *matrix);
void ags_matrix_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_matrix_init(AgsMatrix *matrix);
void ags_matrix_connect(AgsConnectable *connectable);
void ags_matrix_destroy(GtkObject *object);
void ags_matrix_show(GtkWidget *widget);

void ags_matrix_set_audio_channels(AgsAudio *audio,
				   guint audio_channels, guint audio_channels_old,
				   gpointer data);
void ags_matrix_set_pads(AgsAudio *audio, GType type,
			 guint pads, guint pads_old,
			 gpointer data);

extern void ags_file_read_matrix(AgsFile *file, AgsMachine *machine);
extern void ags_file_write_matrix(AgsFile *file, AgsMachine *machine);

static AgsConnectableInterface *ags_matrix_parent_connectable_interface;

GtkStyle *matrix_style;

extern const char *AGS_COPY_PATTERN;
const char *AGS_MATRIX_INDEX = "AgsMatrixIndex\0";

GType
ags_matrix_get_type(void)
{
  static GType ags_type_matrix = 0;

  if(!ags_type_matrix){
    static const GTypeInfo ags_matrix_info = {
      sizeof(AgsMatrixClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_matrix_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof(AgsMatrix),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_matrix_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_matrix_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };
    
    ags_type_matrix = g_type_register_static(AGS_TYPE_MACHINE,
					    "AgsMatrix\0", &ags_matrix_info,
					    0);
    
    g_type_add_interface_static(ags_type_matrix,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
  }

  return(ags_type_matrix);
}

void
ags_matrix_class_init(AgsMatrixClass *matrix)
{
  GtkObjectClass *object = (GtkObjectClass *) matrix;
  GtkWidgetClass *widget = (GtkWidgetClass *) matrix;
  AgsMachineClass *machine = (AgsMachineClass *) matrix;

  //  object->destroy = ags_matrix_destroy;

  //  widget->show = ags_matrix_show;

  //  machine->read_file = ags_file_read_matrix;
  //  machine->write_file = ags_file_write_matrix;
}

void
ags_matrix_connectable_interface_init(AgsConnectableInterface *connectable)
{
  AgsConnectableInterface *ags_matrix_connectable_parent_interface;

  ags_matrix_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_matrix_connect;
}

void
ags_matrix_init(AgsMatrix *matrix)
{
  AgsAudio *audio;
  AgsDelayAudio *delay_audio;
  AgsDelayAudioRun *play_delay_audio_run, *recall_delay_audio_run;
  AgsCopyPatternAudio *copy_pattern_audio;
  AgsCopyPatternAudioRun *copy_pattern_audio_run;
  GtkFrame *frame;
  GtkTable *table;
  GtkToggleButton *button;
  GtkVScrollbar *vscrollbar;
  GtkVBox *vbox;
  GtkHBox *hbox;
  int i, j;

  g_signal_connect_after((GObject *) matrix, "parent_set\0",
			 G_CALLBACK(ags_matrix_parent_set_callback), (gpointer) matrix);

  audio = matrix->machine.audio;
  audio->flags |= (AGS_AUDIO_OUTPUT_HAS_RECYCLING |
		   AGS_AUDIO_SYNC |
		   AGS_AUDIO_ASYNC |
		   AGS_AUDIO_HAS_NOTATION);
  matrix->machine.audio->audio_channels = 1;

  matrix->flags = 0;

  /* create AgsDelayAudio in audio->play */
  delay_audio = ags_delay_audio_new(audio,
				    0);
  AGS_RECALL(delay_audio)->flags |= AGS_RECALL_TEMPLATE;

  audio->play = g_list_append(audio->play, (gpointer) delay_audio);
  //  ags_connectable_connect(AGS_CONNECTABLE(delay_audio));

  /* create AgsDelayAudioRun in audio->play */
  play_delay_audio_run = ags_delay_audio_run_new((AgsRecallAudio *) delay_audio);
  AGS_RECALL(play_delay_audio_run)->flags |= AGS_RECALL_TEMPLATE;

  audio->play = g_list_append(audio->play, (gpointer) play_delay_audio_run);
  ags_connectable_connect(AGS_CONNECTABLE(play_delay_audio_run));

  /* create AgsDelayAudio in audio->recall */
  delay_audio = ags_delay_audio_new(audio,
				    0);
  AGS_RECALL(delay_audio)->flags |= AGS_RECALL_TEMPLATE;

  audio->recall = g_list_append(audio->recall, (gpointer) delay_audio);
  //  ags_connectable_connect(AGS_CONNECTABLE(delay_audio));

  /* create AgsDelayAudioRun in audio->recall */
  recall_delay_audio_run = ags_delay_audio_run_new((AgsRecallAudio *) delay_audio);
  AGS_RECALL(recall_delay_audio_run)->flags = AGS_RECALL_TEMPLATE;

  audio->recall = g_list_append(audio->recall, (gpointer) recall_delay_audio_run);
  ags_connectable_connect(AGS_CONNECTABLE(recall_delay_audio_run));

  /* create AgsCopyPatternAudio in audio->play */
  copy_pattern_audio = ags_copy_pattern_audio_new(NULL, audio,
						  0, 0,
						  16, FALSE,
						  0);
  AGS_RECALL(copy_pattern_audio)->flags |= AGS_RECALL_TEMPLATE;

  audio->play = g_list_append(audio->play, (gpointer) copy_pattern_audio);
  //  ags_connectable_connect(AGS_CONNECTABLE(copy_pattern_audio));

  /* create AgsCopyPatternAudioRun in audio->play */
  copy_pattern_audio_run = ags_copy_pattern_audio_run_new((AgsRecallAudio *) copy_pattern_audio,
							  play_delay_audio_run, 0);
  AGS_RECALL(copy_pattern_audio_run)->flags |= AGS_RECALL_TEMPLATE;

  audio->play = g_list_append(audio->play, (gpointer) copy_pattern_audio_run);
  ags_connectable_connect(AGS_CONNECTABLE(copy_pattern_audio_run));

  /* create AgsCopyPatternAudio in audio->recall */
  copy_pattern_audio = ags_copy_pattern_audio_new(NULL, audio,
						  0, 0,
						  16, FALSE,
						  0);
  AGS_RECALL(copy_pattern_audio)->flags |= AGS_RECALL_TEMPLATE;

  audio->play = g_list_append(audio->play, (gpointer) copy_pattern_audio);
  //  ags_connectable_connect(AGS_CONNECTABLE(copy_pattern_audio));

  /* create AgsCopyPatternAudioRun in audio->recall */
  copy_pattern_audio_run = ags_copy_pattern_audio_run_new((AgsRecallAudio *) copy_pattern_audio,
							  recall_delay_audio_run, 0);
  AGS_RECALL(copy_pattern_audio_run)->flags |= AGS_RECALL_TEMPLATE;

  audio->recall = g_list_append(audio->recall, (gpointer) copy_pattern_audio_run);
  ags_connectable_connect(AGS_CONNECTABLE(copy_pattern_audio_run));


  /* create widgets */
  frame = (GtkFrame *) (gtk_container_get_children((GtkContainer *) matrix))->data;

  matrix->table = (GtkTable *) gtk_table_new(4, 4, FALSE);
  gtk_container_add((GtkContainer *) frame, (GtkWidget *) matrix->table);

  matrix->run = (GtkToggleButton *) gtk_toggle_button_new_with_label(g_strdup("run\0"));
  gtk_table_attach(matrix->table, (GtkWidget *) matrix->run,
		   0, 1, 0, 1,
		   GTK_FILL, GTK_FILL,
		   0, 0);

  table = (GtkTable *) gtk_table_new(3, 3, FALSE);
  gtk_table_attach(matrix->table, (GtkWidget *) table,
		   1, 2, 0, 1,
		   GTK_FILL, GTK_FILL,
		   0, 0);
  matrix->selected = NULL;

  for(i = 0; i < 3; i++){
    for(j = 0; j < 3; j++){
      matrix->index[i * 3 + j] =
	button = (GtkToggleButton *) gtk_toggle_button_new_with_label(g_strdup_printf("%d\0", i * 3 + j +1));
      g_object_set_data((GObject *) button, AGS_MATRIX_INDEX, GUINT_TO_POINTER(i * 3 + j));
      gtk_table_attach(table, (GtkWidget *) button,
		       j, j +1, i, i +1,
		       GTK_FILL, GTK_FILL,
		       0, 0);
    }
  }

  matrix->selected = matrix->index[0];
  gtk_toggle_button_set_active(matrix->selected, TRUE);

  table = (GtkTable *) gtk_table_new(2, 2, FALSE);
  gtk_table_attach(matrix->table, (GtkWidget *) table,
		   2, 3, 0, 1,
		   GTK_FILL, GTK_FILL,
		   0, 0);

  matrix->drawing_area = (GtkDrawingArea *) gtk_drawing_area_new();
  gtk_widget_set_size_request((GtkWidget *) matrix->drawing_area, 32 * AGS_MATRIX_CELL_WIDTH +1, 8 * AGS_MATRIX_CELL_HEIGHT +1);
  gtk_widget_set_style((GtkWidget *) matrix->drawing_area, matrix_style);
  gtk_table_attach(table, (GtkWidget *) matrix->drawing_area,
		   0, 1, 0, 1,
		   GTK_FILL, GTK_FILL,
		   0, 0);
  gtk_widget_set_events ((GtkWidget *) matrix->drawing_area,
                         GDK_EXPOSURE_MASK
                         | GDK_LEAVE_NOTIFY_MASK
                         | GDK_BUTTON_PRESS_MASK
                         | GDK_POINTER_MOTION_MASK
                         | GDK_POINTER_MOTION_HINT_MASK);


  matrix->adjustment = (GtkAdjustment *) gtk_adjustment_new(0.0, 0.0, 77.0, 1.0, 1.0, 8.0);

  vscrollbar = (GtkVScrollbar *) gtk_vscrollbar_new(matrix->adjustment);
  gtk_widget_set_style((GtkWidget *) vscrollbar, matrix_style);
  gtk_table_attach(table, (GtkWidget *) vscrollbar,
		   1, 2, 0, 1,
		   GTK_FILL, GTK_FILL,
		   0, 0);

  vbox = (GtkVBox *) gtk_vbox_new(FALSE, 0);
  gtk_table_attach(matrix->table, (GtkWidget *) vbox,
		   3, 4, 0, 1,
		   GTK_FILL, GTK_FILL,
		   0, 0);

  hbox = (GtkHBox *) gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start((GtkBox *) vbox, (GtkWidget *) hbox, FALSE, FALSE, 0);

  gtk_box_pack_start((GtkBox *) hbox, gtk_label_new(g_strdup("length\0")), FALSE, FALSE, 0);

  matrix->length_spin = (GtkSpinButton *) gtk_spin_button_new_with_range(1.0, 32.0, 1.0);
  matrix->length_spin->adjustment->value = 16.0;
  gtk_box_pack_start((GtkBox *) hbox, (GtkWidget *) matrix->length_spin, FALSE, FALSE, 0);

  matrix->tact = (GtkOptionMenu *) gtk_option_menu_new();
  gtk_box_pack_start((GtkBox *) vbox, (GtkWidget *) matrix->tact, FALSE, FALSE, 0);

  gtk_option_menu_set_menu(matrix->tact, (GtkWidget *) ags_tact_menu_new());
  gtk_option_menu_set_history(matrix->tact, 6);

  matrix->loop_button = (GtkCheckButton *) gtk_check_button_new_with_label(g_strdup("loop\0"));
  gtk_box_pack_start((GtkBox *) vbox, (GtkWidget *) matrix->loop_button, FALSE, FALSE, 0);
}

void
ags_matrix_connect(AgsConnectable *connectable)
{
  AgsMatrix *matrix;
  int i;

  ags_matrix_parent_connectable_interface->connect(connectable);

  /* AgsMatrix */
  matrix = AGS_MATRIX(connectable);

  g_signal_connect((GObject *) matrix, "destroy\0",
		   G_CALLBACK(ags_matrix_destroy_callback), (gpointer) matrix);

  g_signal_connect(G_OBJECT(matrix->run), "clicked\0",
		   G_CALLBACK(ags_matrix_run_callback), (gpointer) matrix);

  for(i  = 0; i < 9; i++){
    g_signal_connect (G_OBJECT (matrix->index[i]), "clicked\0",
		      G_CALLBACK (ags_matrix_index_callback), (gpointer) matrix);
  }

  g_signal_connect (G_OBJECT (matrix->drawing_area), "expose_event\0",
                    G_CALLBACK (ags_matrix_drawing_area_expose_callback), (gpointer) matrix);

  g_signal_connect (G_OBJECT (matrix->drawing_area), "button_press_event\0",
                    G_CALLBACK (ags_matrix_drawing_area_button_press_callback), (gpointer) matrix);

  g_signal_connect (G_OBJECT (matrix->adjustment), "value_changed\0",
                    G_CALLBACK (ags_matrix_adjustment_value_changed_callback), (gpointer) matrix);

  g_signal_connect_after((GObject *) matrix->length_spin, "value-changed\0",
			 G_CALLBACK(ags_matrix_length_spin_callback), (gpointer) matrix);

  g_signal_connect((GObject *) matrix->tact, "changed\0",
		   G_CALLBACK(ags_matrix_tact_callback), (gpointer) matrix);

  g_signal_connect((GObject *) matrix->loop_button, "clicked\0",
		   G_CALLBACK(ags_matrix_loop_button_callback), (gpointer) matrix);

  /* AgsAudio */
  g_signal_connect_after(G_OBJECT(matrix->machine.audio), "set_audio_channels\0",
			 G_CALLBACK(ags_matrix_set_audio_channels), NULL);

  g_signal_connect_after(G_OBJECT(matrix->machine.audio), "set_pads\0",
			 G_CALLBACK(ags_matrix_set_pads), NULL);
}

void
ags_matrix_destroy(GtkObject *object)
{
}

void
ags_matrix_show(GtkWidget *widget)
{
}

void
ags_matrix_set_audio_channels(AgsAudio *audio,
			      guint audio_channels, guint audio_channels_old,
			      gpointer data)
{
  printf("AgsMatrix only pads can be adjusted\n\0");
  //  _ags_audio_set_audio_channels(audio, audio_channels);
}

void
ags_matrix_set_pads(AgsAudio *audio, GType type,
		    guint pads, guint pads_old,
		    gpointer data)
{
  AgsMatrix *matrix;
  AgsChannel *source, *destination;
  AgsAudioSignal *audio_signal;
  AgsCopyPatternChannel *copy_pattern_channel;
  gboolean grow;

  if(type == AGS_TYPE_INPUT && pads < 8){
    printf("AgsMatrix minimum input pad count 8\n\0");
    pads = 8;
  }

  matrix = (AgsMatrix *) audio->machine;

  if(pads_old == pads)
    return;
  if(pads_old < pads)
    grow = TRUE;
  else
    grow = FALSE;

  if(type == AGS_TYPE_INPUT){

    if(grow){
      source = ags_channel_nth(audio->input, pads_old);

      while(source != NULL){
	source->pattern = g_list_alloc();
	source->pattern->data = (gpointer) ags_pattern_new();
	ags_pattern_set_dim((AgsPattern *) source->pattern->data, 1, 9, 32);
	
	source = source->next;
      }

      /* depending on destination */
      ags_matrix_input_map_recall(matrix, 0);
    }
  }else{
    if(grow){
      AgsDelayAudio *delay_audio;
      AgsStreamChannel *stream_channel;
      GList *recall_shared;
      guint stop;

      source = ags_channel_nth(audio->output, pads_old);

      recall_shared = ags_recall_find_type(AGS_AUDIO(source->audio)->play,
					   AGS_TYPE_DELAY_AUDIO);
      
      if(recall_shared != NULL){
	delay_audio = (AgsDelayAudio *) recall_shared->data;
	stop = ((guint)matrix->length_spin->adjustment->value) * (delay_audio->delay + 1);
      }

      while(source != NULL){
	audio_signal = ags_audio_signal_get_template(source->first_recycling->audio_signal);
	ags_audio_signal_stream_resize(audio_signal, stop);

	/* AgsStreamChannel */
	stream_channel = ags_stream_channel_new(source);
	AGS_RECALL(stream_channel)->flags |= AGS_RECALL_TEMPLATE;
    
	source->play = g_list_append(source->play, (gpointer) stream_channel);
	ags_connectable_connect(AGS_CONNECTABLE(stream_channel));

	source = source->next;
      }

      /* depending on destination */
      ags_matrix_input_map_recall(matrix, pads_old);
    }
  }
}

void
ags_matrix_input_map_recall(AgsMatrix *matrix, guint output_pad_start)
{
  AgsAudio *audio;
  AgsChannel *destination, *destination_start, *source;
  AgsCopyPatternAudio *copy_pattern_audio;
  AgsCopyPatternAudioRun *copy_pattern_audio_run;
  AgsCopyPatternChannel *copy_pattern_channel;
  AgsCopyPatternChannelRun *copy_pattern_channel_run;
  AgsCopyChannel *copy_channel;
  GList *list;

  audio = matrix->machine.audio;
  source = audio->input;
  destination_start = ags_channel_nth(audio->output, output_pad_start);

  list = ags_recall_template_find_type(audio->play, AGS_TYPE_COPY_PATTERN_AUDIO);
  copy_pattern_audio = AGS_COPY_PATTERN_AUDIO(list->data);
  
  list = ags_recall_template_find_type(audio->play, AGS_TYPE_COPY_PATTERN_AUDIO_RUN);
  copy_pattern_audio_run = AGS_COPY_PATTERN_AUDIO_RUN(list->data);

  while(source != NULL){
    destination = destination_start;

    while(destination != NULL){
      /* AgsCopyChannel */
      g_object_ref(G_OBJECT(destination));
      
      copy_channel = ags_copy_channel_new(destination,
					  source,
					  (AgsDevout *) audio->devout);
      AGS_RECALL(copy_channel)->flags |= AGS_RECALL_TEMPLATE;
      
      source->recall = g_list_append(source->recall, (gpointer) copy_channel);
      ags_connectable_connect(AGS_CONNECTABLE(copy_channel));
	  
      /* AgsCopyPatternChannel */
      g_object_ref(G_OBJECT(destination));
	  
      copy_pattern_channel = ags_copy_pattern_channel_new((AgsRecallAudio *) copy_pattern_audio,
							  destination,
							  source, (AgsPattern *) source->pattern->data);
      AGS_RECALL(copy_pattern_channel)->flags |= AGS_RECALL_TEMPLATE;
      source->recall = g_list_append(source->recall, (gpointer) copy_pattern_channel);
      
      /* AgsCopyPatternChannelRun */
      copy_pattern_channel_run = ags_copy_pattern_channel_run_new((AgsRecallAudio *) copy_pattern_audio,
								  (AgsRecallAudioRun *) copy_pattern_audio_run,
								  (AgsRecallChannel *) copy_pattern_channel);
      
      AGS_RECALL(copy_pattern_channel_run)->flags |= AGS_RECALL_TEMPLATE;
      
      source->recall = g_list_append(source->recall, (gpointer) copy_pattern_channel_run);
      
      ags_connectable_connect(AGS_CONNECTABLE(copy_pattern_channel_run));
      
      g_signal_connect((GObject *) copy_pattern_channel_run, "done\0",
		       G_CALLBACK(ags_matrix_copy_pattern_done), matrix);
      
      g_signal_connect((GObject *) copy_pattern_channel_run, "cancel\0",
		       G_CALLBACK(ags_matrix_copy_pattern_cancel), matrix);
      
      g_signal_connect((GObject *) copy_pattern_channel_run, "loop\0",
		       G_CALLBACK(ags_matrix_copy_pattern_loop), matrix);
      
      destination = destination->next;
    }
    
    source = source->next;
  }
}

void
ags_matrix_draw_gutter(AgsMatrix *matrix)
{
  AgsChannel *channel;
  int i, j;

  gdk_draw_rectangle (GTK_WIDGET (matrix->drawing_area)->window,
                      GTK_WIDGET (matrix->drawing_area)->style->bg_gc[0],
                      TRUE,
                      0, 0,
                      288, 80);

  channel = ags_channel_nth(matrix->machine.audio->input, (guint) matrix->adjustment->value);

  for (i = 0; i < 8; i++){
    for (j = 0; j < 32; j++){
      gdk_draw_rectangle (GTK_WIDGET (matrix->drawing_area)->window,
                          GTK_WIDGET (matrix->drawing_area)->style->fg_gc[0],
                          FALSE,
                          j * 12, i * 10,
                          12, 10);

      ags_matrix_redraw_gutter_point (matrix, channel, j, i);
    }

    channel = channel->next;
  }
}

void
ags_matrix_draw_matrix(AgsMatrix *matrix)
{
  AgsChannel *channel;
  int i, j;

  channel = ags_channel_nth(matrix->machine.audio->input, (guint) matrix->adjustment->value);

  for (i = 0; i < 8; i++){
    for (j = 0; j < 32; j++)
      ags_matrix_redraw_gutter_point (matrix, channel, j, i);

    channel = channel->next;
  }
}

void
ags_matrix_redraw_gutter_point (AgsMatrix *matrix, AgsChannel *channel, guint j, guint i)
{
  if(channel->pattern == NULL)
    return;

  if(ags_pattern_get_bit((AgsPattern *) channel->pattern->data, 0, strtol(matrix->selected->button.label_text, NULL, 10) -1, j))
    ags_matrix_highlight_gutter_point(matrix, j, i);
  else
    ags_matrix_unpaint_gutter_point(matrix, j, i);
}

void
ags_matrix_highlight_gutter_point(AgsMatrix *matrix, guint j, guint i)
{
  gdk_draw_rectangle (GTK_WIDGET (matrix->drawing_area)->window,
		      GTK_WIDGET (matrix->drawing_area)->style->fg_gc[0],
		      TRUE,
		      j * 12 +1, i * 10 +1,
		      11, 9);
}

void
ags_matrix_unpaint_gutter_point(AgsMatrix *matrix, guint j, guint i)
{
  gdk_draw_rectangle (GTK_WIDGET (matrix->drawing_area)->window,
		      GTK_WIDGET (matrix->drawing_area)->style->bg_gc[0],
		      TRUE,
		      j * 12 +1, i * 10 +1,
		      11, 9);
}

AgsMatrix*
ags_matrix_new(GObject *devout)
{
  AgsMatrix *matrix;
  GValue value;

  matrix = (AgsMatrix *) g_object_new(AGS_TYPE_MATRIX,
				      NULL);

  g_value_init(&value, G_TYPE_OBJECT);
  g_value_set_object(&value, devout);

  g_object_set_property(G_OBJECT(matrix->machine.audio),
			"devout\0", &value);

  return(matrix);
}
