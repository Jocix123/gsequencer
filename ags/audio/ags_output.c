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

#include <ags/audio/ags_output.h>

#include <ags/object/ags_connectable.h>
#include <ags/object/ags_soundcard.h>

#include <ags/audio/ags_audio.h>

void ags_output_class_init(AgsOutputClass *output_class);
void ags_output_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_output_init(AgsOutput *output);
void ags_output_finalize(GObject *gobject);
void ags_output_connect(AgsConnectable *connectable);
void ags_output_disconnect(AgsConnectable *connectable);

/**
 * SECTION:ags_output
 * @short_description: Output of #AgsAudio
 * @title: AgsOutput
 * @section_id:
 * @include: ags/audio/ags_output.h
 *
 * #AgsOutput represents an output channel of #AgsAudio.
 */

static gpointer ags_output_parent_class = NULL;
static AgsConnectableInterface *ags_output_parent_connectable_interface;

GType
ags_output_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;

  if(g_once_init_enter (&g_define_type_id__volatile)){
    GType ags_type_output;

    static const GTypeInfo ags_output_info = {
      sizeof (AgsOutputClass),
      (GBaseInitFunc) NULL, /* base_init */
      (GBaseFinalizeFunc) NULL, /* base_finalize */
      (GClassInitFunc) ags_output_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsOutput),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_output_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_output_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_output = g_type_register_static(AGS_TYPE_CHANNEL,
					     "AgsOutput",
					     &ags_output_info,
					     0);

    g_type_add_interface_static(ags_type_output,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);

    g_once_init_leave (&g_define_type_id__volatile, ags_type_output);
  }

  return g_define_type_id__volatile;
}

void
ags_output_class_init(AgsOutputClass *output)
{
  GObjectClass *gobject;

  ags_output_parent_class = g_type_class_peek_parent(output);

  /* GObjectClass */
  gobject = (GObjectClass *) output;
  
  gobject->finalize = ags_output_finalize;
}

void
ags_output_connectable_interface_init(AgsConnectableInterface *connectable)
{
  AgsConnectableInterface *ags_output_connectable_parent_interface;

  ags_output_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_output_connect;
  connectable->disconnect = ags_output_disconnect;
}

void
ags_output_init(AgsOutput *output)
{
  /* empty */
}

void
ags_output_finalize(GObject *gobject)
{
  G_OBJECT_CLASS(ags_output_parent_class)->finalize(gobject);
}

void
ags_output_connect(AgsConnectable *connectable)
{
  ags_output_parent_connectable_interface->connect(connectable);
}

void
ags_output_disconnect(AgsConnectable *connectable)
{
  ags_output_parent_connectable_interface->disconnect(connectable);
}

/**
 * ags_output_map_audio_signal:
 * @output: an #AgsOutput
 * @recall_id: the assigned #AgsRecallID
 *
 * Maps audio signal and assigning it to recall id.
 *
 * Returns: a new #GList containing #AgsAudioSignal
 *
 * Since: 1.0.0
 */
GList*
ags_output_map_audio_signal(AgsOutput *output, AgsRecallID *recall_id)
{
  AgsAudioSignal *audio_signal;
  
  GObject *soundcard;
  
  GList *list_destination;

  if(output != NULL){
    soundcard = AGS_CHANNEL(output)->soundcard;
    
    list_destination = g_list_alloc();
    
    goto ags_copy_pattern_map_destination0;
  }else{
    return(NULL);
  }

  while(output != NULL){
    list_destination->next = g_list_alloc();
    list_destination->next->prev = list_destination;
    list_destination = list_destination->next;
  ags_copy_pattern_map_destination0:
    g_message("ags_output_map_audio_signal\n");

    audio_signal = ags_audio_signal_new((GObject *) soundcard,
					(GObject *) output->channel.first_recycling,
					(GObject *) recall_id);
    ags_connectable_connect(AGS_CONNECTABLE(audio_signal));

    ags_recycling_add_audio_signal(output->channel.first_recycling,
				   audio_signal);
    audio_signal->stream_current = audio_signal->stream_beginning;
    list_destination->data = (gpointer) audio_signal;

    output = (AgsOutput *) output->channel.next_pad;
  }
  
  return(list_destination);
}


/**
 * ags_output_find_first_input_recycling:
 * @output: an #AgsOutput
 *
 * Retrieve first input recycling.
 *
 * Returns: the first #AgsRecycling of #AgsAudio
 *
 * Since: 1.0.0
 */
AgsRecycling*
ags_output_find_first_input_recycling(AgsOutput *output)
{
  AgsAudio *audio;
  AgsChannel *input;

  if(output == NULL)
    return(NULL);

  audio = AGS_AUDIO(AGS_CHANNEL(output)->audio);

  if((AGS_AUDIO_ASYNC & (audio->flags)) != 0){
    input = ags_channel_nth(audio->input, AGS_CHANNEL(output)->audio_channel);

    input = ags_channel_first_with_recycling(input);
  }else{
    input = ags_channel_nth(audio->input, AGS_CHANNEL(output)->line);
  }

  if(input != NULL){
    return(input->first_recycling);
  }else{
    return(NULL);
  }
}

/**
 * ags_output_find_last_input_recycling:
 * @output: an #AgsOutput
 *
 * Retrieve last input recycling.
 *
 * Returns: the last #AgsRecycling of #AgsAudio
 *
 * Since: 1.0.0
 */
AgsRecycling*
ags_output_find_last_input_recycling(AgsOutput *output)
{
  AgsAudio *audio;
  AgsChannel *input;

  if(output == NULL)
    return(NULL);

  audio = AGS_AUDIO(AGS_CHANNEL(output)->audio);

  if((AGS_AUDIO_ASYNC & (audio->flags)) != 0){
    input = ags_channel_nth(audio->input, AGS_CHANNEL(output)->audio_channel);

    input = ags_channel_last_with_recycling(input);
  }else{
    input = ags_channel_nth(audio->input, AGS_CHANNEL(output)->line);
  }

  if(input != NULL){
    return(input->last_recycling);
  }else{
    return(NULL);
  }
}

/**
 * ags_output_new:
 * @audio: the #AgsAudio
 *
 * Creates an #AgsOutput, linking tree to @audio.
 *
 * Returns: a new #AgsOutput
 *
 * Since: 1.0.0
 */
AgsOutput*
ags_output_new(GObject *audio)
{
  AgsOutput *output;

  output = (AgsOutput *) g_object_new(AGS_TYPE_OUTPUT,
				      "audio", audio,
				      NULL);

  return(output);
}
