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

#include <ags/audio/recall/ags_volume_audio_signal.h>

#include <ags/object/ags_connectable.h>
#include <ags/object/ags_run_connectable.h>

#include <ags/audio/recall/ags_volume_channel.h>

void ags_volume_audio_signal_class_init(AgsVolumeAudioSignalClass *volume_audio_signal);
void ags_volume_audio_signal_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_volume_audio_signal_run_connectable_interface_init(AgsRunConnectableInterface *run_connectable);
void ags_volume_audio_signal_init(AgsVolumeAudioSignal *volume_audio_signal);
void ags_volume_audio_signal_set_property(GObject *gobject,
					  guint prop_id,
					  const GValue *value,
					  GParamSpec *param_spec);
void ags_volume_audio_signal_get_property(GObject *gobject,
					  guint prop_id,
					  GValue *value,
					  GParamSpec *param_spec);
void ags_volume_audio_signal_connect(AgsConnectable *connectable);
void ags_volume_audio_signal_disconnect(AgsConnectable *connectable);
void ags_volume_audio_signal_run_connect(AgsRunConnectable *run_connectable);
void ags_volume_audio_signal_run_disconnect(AgsRunConnectable *run_connectable);
void ags_volume_audio_signal_finalize(GObject *gobject);

void ags_volume_audio_signal_run_inter(AgsRecall *recall);
AgsRecall* ags_volume_audio_signal_duplicate(AgsRecall *recall, AgsRecallID *recall_id);

enum{
  PROP_0,
  PROP_AUDIO_SIGNAL,
  PROP_VOLUME,
};

static gpointer ags_volume_audio_signal_parent_class = NULL;
static AgsConnectableInterface *ags_volume_audio_signal_parent_connectable_interface;
static AgsRunConnectableInterface *ags_volume_audio_signal_parent_run_connectable_interface;

GType
ags_volume_audio_signal_get_type()
{
  static GType ags_type_volume_audio_signal = 0;

  if(!ags_type_volume_audio_signal){
    static const GTypeInfo ags_volume_audio_signal_info = {
      sizeof (AgsVolumeAudioSignalClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_volume_audio_signal_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsVolumeAudioSignal),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_volume_audio_signal_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_volume_audio_signal_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    static const GInterfaceInfo ags_run_connectable_interface_info = {
      (GInterfaceInitFunc) ags_volume_audio_signal_run_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_volume_audio_signal = g_type_register_static(AGS_TYPE_RECALL,
							  "AgsVolumeAudioSignal\0",
							  &ags_volume_audio_signal_info,
							  0);

    g_type_add_interface_static(ags_type_volume_audio_signal,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);

    g_type_add_interface_static(ags_type_volume_audio_signal,
				AGS_TYPE_RUN_CONNECTABLE,
				&ags_run_connectable_interface_info);
  }

  return (ags_type_volume_audio_signal);
}

void
ags_volume_audio_signal_class_init(AgsVolumeAudioSignalClass *volume_audio_signal)
{
  GObjectClass *gobject;
  AgsRecallClass *recall;
  GParamSpec *param_spec;

  ags_volume_audio_signal_parent_class = g_type_class_peek_parent(volume_audio_signal);

  /* GObjectClass */
  gobject = (GObjectClass *) volume_audio_signal;

  gobject->set_property = ags_volume_audio_signal_set_property;
  gobject->get_property = ags_volume_audio_signal_get_property;

  gobject->finalize = ags_volume_audio_signal_finalize;

  /* properties */
  param_spec = g_param_spec_object("audio_signal\0",
				   "volume AgsAudioSignal\0",
				   "The AgsAudioSignal to apply volume\0",
				   AGS_TYPE_CHANNEL,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_AUDIO_SIGNAL,
				  param_spec);

  param_spec = g_param_spec_pointer("volume\0",
				    "volume to apply\0",
				    "The volume to apply on the audio signal\0",
				    G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_VOLUME,
				  param_spec);

  /* AgsRecallClass */
  recall = (AgsRecallClass *) volume_audio_signal;

  recall->run_inter = ags_volume_audio_signal_run_inter;
  recall->duplicate = ags_volume_audio_signal_duplicate;
}

void
ags_volume_audio_signal_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_volume_audio_signal_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_volume_audio_signal_connect;
  connectable->disconnect = ags_volume_audio_signal_disconnect;
}

void
ags_volume_audio_signal_run_connectable_interface_init(AgsRunConnectableInterface *run_connectable)
{
  ags_volume_audio_signal_parent_run_connectable_interface = g_type_interface_peek_parent(run_connectable);

  run_connectable->connect = ags_volume_audio_signal_run_connect;
  run_connectable->disconnect = ags_volume_audio_signal_run_disconnect;
}

void
ags_volume_audio_signal_init(AgsVolumeAudioSignal *volume_audio_signal)
{
  volume_audio_signal->audio_signal = NULL;
  volume_audio_signal->volume = NULL;
}


void
ags_volume_audio_signal_set_property(GObject *gobject,
				     guint prop_id,
				     const GValue *value,
				     GParamSpec *param_spec)
{
  AgsVolumeAudioSignal *volume_audio_signal;

  volume_audio_signal = AGS_VOLUME_AUDIO_SIGNAL(gobject);

  switch(prop_id){
  case PROP_AUDIO_SIGNAL:
    {
      AgsAudioSignal *audio_signal;

      audio_signal = (AgsAudioSignal *) g_value_get_object(value);

      if(volume_audio_signal->audio_signal != NULL){
	g_object_unref(G_OBJECT(volume_audio_signal->audio_signal));
      }

      if(audio_signal != NULL){
	g_object_ref(G_OBJECT(audio_signal));
      }

      volume_audio_signal->audio_signal = audio_signal;
    }
    break;
  case PROP_VOLUME:
    {
      volume_audio_signal->volume = g_value_get_pointer(value);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_volume_audio_signal_get_property(GObject *gobject,
				     guint prop_id,
				     GValue *value,
				     GParamSpec *param_spec)
{
  AgsVolumeAudioSignal *volume_audio_signal;

  volume_audio_signal = AGS_VOLUME_AUDIO_SIGNAL(gobject);

  switch(prop_id){
  case PROP_AUDIO_SIGNAL:
    {
      g_value_set_object(value, volume_audio_signal->audio_signal);
    }
    break;
  case PROP_VOLUME:
    {
      g_value_set_pointer(value, volume_audio_signal->volume);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_volume_audio_signal_finalize(GObject *gobject)
{
  AgsVolumeAudioSignal *volume_audio_signal;

  volume_audio_signal = AGS_VOLUME_AUDIO_SIGNAL(gobject);

  if(volume_audio_signal->audio_signal != NULL)
    g_object_unref(G_OBJECT(volume_audio_signal->audio_signal));

  /* call parent */
  G_OBJECT_CLASS(ags_volume_audio_signal_parent_class)->finalize(gobject);
}

void
ags_volume_audio_signal_connect(AgsConnectable *connectable)
{
  ags_volume_audio_signal_parent_connectable_interface->connect(connectable);

  /* empty */
}

void
ags_volume_audio_signal_disconnect(AgsConnectable *connectable)
{
  ags_volume_audio_signal_parent_connectable_interface->disconnect(connectable);

  /* empty */
}

void
ags_volume_audio_signal_run_connect(AgsRunConnectable *run_connectable)
{
  ags_volume_audio_signal_parent_run_connectable_interface->connect(run_connectable);

  /* empty */
}

void
ags_volume_audio_signal_run_disconnect(AgsRunConnectable *run_connectable)
{
  ags_volume_audio_signal_parent_run_connectable_interface->disconnect(run_connectable);

  /* empty */
}

void
ags_volume_audio_signal_run_inter(AgsRecall *recall)
{
  AgsVolumeAudioSignal *volume_audio_signal;

  AGS_RECALL_CLASS(ags_volume_audio_signal_parent_class)->run_inter(recall);

  volume_audio_signal = AGS_VOLUME_AUDIO_SIGNAL(recall);

  if(volume_audio_signal->audio_signal->stream_current != NULL){
    AgsDevout *devout;
    short *buffer;
    guint i;

    devout = AGS_DEVOUT(volume_audio_signal->audio_signal->devout);
    buffer = (short *) volume_audio_signal->audio_signal->stream_current->data;

    for(i = 0; i < devout->buffer_size; i++){
      buffer[i] = (short) ((0xffff) & (int)((gdouble)volume_audio_signal->volume[0] * (gdouble)buffer[i]));
    }
  }else{
    ags_recall_done(recall);
  }
}

AgsRecall*
ags_volume_audio_signal_duplicate(AgsRecall *recall, AgsRecallID *recall_id)
{
  AgsVolumeAudioSignal *volume_audio_signal, *copy;

  volume_audio_signal = (AgsVolumeAudioSignal *) recall;
  copy = (AgsVolumeAudioSignal *) AGS_RECALL_CLASS(ags_volume_audio_signal_parent_class)->duplicate(recall, recall_id);

  copy->audio_signal = volume_audio_signal->audio_signal;

  return((AgsRecall *) copy);
}

AgsVolumeAudioSignal*
ags_volume_audio_signal_new(AgsAudioSignal *audio_signal, gdouble *volume)
{
  AgsVolumeAudioSignal *volume_audio_signal;

  volume_audio_signal = (AgsVolumeAudioSignal *) g_object_new(AGS_TYPE_VOLUME_AUDIO_SIGNAL,
							      "audio_signal\0", audio_signal,
							      "volume\0", volume,
							      NULL);

  return(volume_audio_signal);
}
