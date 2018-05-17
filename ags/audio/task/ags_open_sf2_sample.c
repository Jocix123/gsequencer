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

#include <ags/audio/task/ags_open_sf2_sample.h>

#include <ags/audio/ags_audio.h>
#include <ags/audio/ags_channel.h>
#include <ags/audio/ags_input.h>

#include <ags/audio/file/ags_audio_file_link.h>
#include <ags/audio/file/ags_audio_container.h>

#include <ags/config.h>

#include <ags/i18n.h>

void ags_open_sf2_sample_class_init(AgsOpenSf2SampleClass *open_sf2_sample);
void ags_open_sf2_sample_init(AgsOpenSf2Sample *open_sf2_sample);
void ags_open_sf2_sample_set_property(GObject *gobject,
				      guint prop_id,
				      const GValue *value,
				      GParamSpec *param_spec);
void ags_open_sf2_sample_get_property(GObject *gobject,
				      guint prop_id,
				      GValue *value,
				      GParamSpec *param_spec);
void ags_open_sf2_sample_dispose(GObject *gobject);
void ags_open_sf2_sample_finalize(GObject *gobject);

void ags_open_sf2_sample_launch(AgsTask *task);

/**
 * SECTION:ags_open_sf2_sample
 * @short_description: open Soundfont2 sample
 * @title: AgsOpenSf2Sample
 * @section_id:
 * @include: ags/audio/task/ags_open_sf2_sample.h
 *
 * The #AgsOpenSf2Sample task opens Soundfont2 samples.
 */

static gpointer ags_open_sf2_sample_parent_class = NULL;

enum{
  PROP_0,
  PROP_CHANNEL,
  PROP_FILENAME,
  PROP_PRESET,
  PROP_INSTRUMENT,
  PROP_SAMPLE,
  PROP_AUDIO_CHANNEL,
};

GType
ags_open_sf2_sample_get_type()
{
  static GType ags_type_open_sf2_sample = 0;

  if(!ags_type_open_sf2_sample){
    static const GTypeInfo ags_open_sf2_sample_info = {
      sizeof(AgsOpenSf2SampleClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_open_sf2_sample_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof(AgsOpenSf2Sample),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_open_sf2_sample_init,
    };

    ags_type_open_sf2_sample = g_type_register_static(AGS_TYPE_TASK,
						"AgsOpenSf2Sample",
						&ags_open_sf2_sample_info,
						0);
  }
  
  return(ags_type_open_sf2_sample);
}

void
ags_open_sf2_sample_class_init(AgsOpenSf2SampleClass *open_sf2_sample)
{
  GObjectClass *gobject;
  AgsTaskClass *task;

  GParamSpec *param_spec;
  
  ags_open_sf2_sample_parent_class = g_type_class_peek_parent(open_sf2_sample);

  /* GObject */
  gobject = (GObjectClass *) open_sf2_sample;

  gobject->set_property = ags_open_sf2_sample_set_property;
  gobject->get_property = ags_open_sf2_sample_get_property;

  gobject->dispose = ags_open_sf2_sample_dispose;
  gobject->finalize = ags_open_sf2_sample_finalize;

  /* properties */
  /**
   * AgsOpenSf2Sample:channel:
   *
   * The assigned #AgsChannel
   * 
   * Since: 2.0.0
   */
  param_spec = g_param_spec_object("channel",
				   i18n_pspec("channel of open sf2 sample"),
				   i18n_pspec("The channel of open sf2 sample task"),
				   AGS_TYPE_CHANNEL,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_CHANNEL,
				  param_spec);
  
  /**
   * AgsOpenSf2Sample:filename:
   *
   * The assigned filename.
   * 
   * Since: 2.0.0
   */
  param_spec = g_param_spec_string("filename",
				   i18n_pspec("the filename"),
				   i18n_pspec("The filename containing the sf2 sample"),
				   NULL,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_FILENAME,
				  param_spec);

  /**
   * AgsOpenSf2Sample:preset:
   *
   * The assigned preset.
   * 
   * Since: 2.0.0
   */
  param_spec = g_param_spec_string("preset",
				   i18n_pspec("the preset"),
				   i18n_pspec("The preset containing the sf2 sample"),
				   NULL,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_PRESET,
				  param_spec);

  /**
   * AgsOpenSf2Sample:instrument:
   *
   * The assigned instrument.
   * 
   * Since: 2.0.0
   */
  param_spec = g_param_spec_string("instrument",
				   i18n_pspec("the instrument"),
				   i18n_pspec("The instrument containing the sf2 sample"),
				   NULL,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_INSTRUMENT,
				  param_spec);

  /**
   * AgsOpenSf2Sample:sample:
   *
   * The assigned sample.
   * 
   * Since: 2.0.0
   */
  param_spec = g_param_spec_string("sample",
				   i18n_pspec("the sample"),
				   i18n_pspec("The sf2 sample"),
				   NULL,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_SAMPLE,
				  param_spec);

  /**
   * AgsOpenSf2Sample:audio-channel:
   *
   * The nth audio channel.
   * 
   * Since: 2.0.0
   */
  param_spec =  g_param_spec_uint("audio-channel",
				  i18n_pspec("nth audio channel"),
				  i18n_pspec("The nth audio channel"),
				  0,
				  G_MAXUINT32,
				  0,
				  G_PARAM_READABLE);
  g_object_class_install_property(gobject,
				  PROP_AUDIO_CHANNEL,
				  param_spec);

  /* AgsTask */
  task = (AgsTaskClass *) open_sf2_sample;

  task->launch = ags_open_sf2_sample_launch;
}

void
ags_open_sf2_sample_init(AgsOpenSf2Sample *open_sf2_sample)
{
  open_sf2_sample->channel = NULL;

  open_sf2_sample->filename = NULL;

  open_sf2_sample->preset = NULL;
  open_sf2_sample->instrument = NULL;
  open_sf2_sample->sample = NULL;

  open_sf2_sample->audio_channel = 0;
}

void
ags_open_sf2_sample_set_property(GObject *gobject,
			       guint prop_id,
			       const GValue *value,
			       GParamSpec *param_spec)
{
  AgsOpenSf2Sample *open_sf2_sample;

  open_sf2_sample = AGS_OPEN_SF2_SAMPLE(gobject);

  switch(prop_id){
  case PROP_CHANNEL:
    {
      AgsChannel *channel;

      channel = (AgsChannel *) g_value_get_object(value);

      if(open_sf2_sample->channel == (GObject *) channel){
	return;
      }

      if(open_sf2_sample->channel != NULL){
	g_object_unref(open_sf2_sample->channel);
      }

      if(channel != NULL){
	g_object_ref(channel);
      }

      open_sf2_sample->channel = (GObject *) channel;
    }
    break;
  case PROP_FILENAME:
    {
      gchar *filename;

      filename = g_value_get_string(value);

      if(open_sf2_sample->filename == filename){
	return;
      }

      if(open_sf2_sample->filename != NULL){
        g_free(open_sf2_sample->filename);
      }

      open_sf2_sample->filename = g_strdup(filename);
    }
    break;
  case PROP_PRESET:
    {
      gchar *preset;

      preset = g_value_get_string(value);

      if(open_sf2_sample->preset == preset){
	return;
      }

      if(open_sf2_sample->preset != NULL){
        g_free(open_sf2_sample->preset);
      }

      open_sf2_sample->preset = g_strdup(preset);
    }
    break;
  case PROP_INSTRUMENT:
    {
      gchar *instrument;

      instrument = g_value_get_string(value);

      if(open_sf2_sample->instrument == instrument){
	return;
      }

      if(open_sf2_sample->instrument != NULL){
        g_free(open_sf2_sample->instrument);
      }

      open_sf2_sample->instrument = g_strdup(instrument);
    }
    break;
  case PROP_SAMPLE:
    {
      gchar *sample;

      sample = g_value_get_string(value);

      if(open_sf2_sample->sample == sample){
	return;
      }

      if(open_sf2_sample->sample != NULL){
        g_free(open_sf2_sample->sample);
      }

      open_sf2_sample->sample = g_strdup(sample);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_open_sf2_sample_get_property(GObject *gobject,
				 guint prop_id,
				 GValue *value,
				 GParamSpec *param_spec)
{
  AgsOpenSf2Sample *open_sf2_sample;

  open_sf2_sample = AGS_OPEN_SF2_SAMPLE(gobject);

  switch(prop_id){
  case PROP_CHANNEL:
    {
      g_value_set_object(value, open_sf2_sample->channel);
    }
    break;
  case PROP_FILENAME:
    {
      g_value_set_string(value, open_sf2_sample->filename);
    }
    break;
  case PROP_PRESET:
    {
      g_value_set_string(value, open_sf2_sample->preset);
    }
    break;
  case PROP_INSTRUMENT:
    {
      g_value_set_string(value, open_sf2_sample->instrument);
    }
    break;
  case PROP_SAMPLE:
    {
      g_value_set_string(value, open_sf2_sample->sample);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_open_sf2_sample_connect(AgsConnectable *connectable)
{
  ags_open_sf2_sample_parent_connectable_interface->connect(connectable);

  /* empty */
}

void
ags_open_sf2_sample_disconnect(AgsConnectable *connectable)
{
  ags_open_sf2_sample_parent_connectable_interface->disconnect(connectable);

  /* empty */
}

void
ags_open_sf2_sample_dispose(GObject *gobject)
{
  AgsOpenSf2Sample *open_sf2_sample;

  open_sf2_sample = AGS_OPEN_SF2_SAMPLE(gobject);

  if(open_sf2_sample->channel != NULL){
    g_object_unref(open_sf2_sample->channel);
  }

  if(open_sf2_sample->filename != NULL){
    g_free(open_sf2_sample->filename);

    open_sf2_sample->filename = NULL;
  }

  if(open_sf2_sample->preset != NULL){
    g_free(open_sf2_sample->preset);

    open_sf2_sample->preset = NULL;
  }

  if(open_sf2_sample->instrument != NULL){
    g_free(open_sf2_sample->instrument);

    open_sf2_sample->instrument = NULL;
  }

  if(open_sf2_sample->sample != NULL){
    g_free(open_sf2_sample->sample);

    open_sf2_sample->sample = NULL;
  }
  
  /* call parent */
  G_OBJECT_CLASS(ags_open_sf2_sample_parent_class)->dispose(gobject);
}

void
ags_open_sf2_sample_finalize(GObject *gobject)
{
  AgsOpenSf2Sample *open_sf2_sample;

  open_sf2_sample = AGS_OPEN_SF2_SAMPLE(gobject);

  if(open_sf2_sample->channel != NULL){
    g_object_unref(open_sf2_sample->channel);
  }
  
  g_free(open_sf2_sample->filename);
  g_free(open_sf2_sample->preset);
  g_free(open_sf2_sample->instrument);
  g_free(open_sf2_sample->sample);

  /* call parent */
  G_OBJECT_CLASS(ags_open_sf2_sample_parent_class)->finalize(gobject);
}

void
ags_open_sf2_sample_launch(AgsTask *task)
{
  //TODO:JK implement me
}

/**
 * ags_open_sf2_sample_new:
 * @channel: the #AgsChannel
 * @filename: the Soundfont2 file
 * @preset: the preset
 * @instrument: the instrument
 * @sample: the sample
 * @audio_channel: the audio channel
 *
 * Creates an #AgsOpenSf2Sample.
 *
 * Returns: an new #AgsOpenSf2Sample.
 *
 * Since: 2.0.0
 */
AgsOpenSf2Sample*
ags_open_sf2_sample_new(AgsChannel *channel,
			gchar *filename,
			gchar *preset,
			gchar *instrument,
			gchar *sample,
			guint audio_channel)
{
  AgsOpenSf2Sample *open_sf2_sample;

  open_sf2_sample = (AgsOpenSf2Sample *) g_object_new(AGS_TYPE_OPEN_SF2_SAMPLE,
						      "channel", channel,
						      "filename", filename,
						      "preset", preset,
						      "instrument", instrument,
						      "sample", sample,
						      "audio-channel", audio_channel,
						      NULL);

  return(open_sf2_sample);
}
