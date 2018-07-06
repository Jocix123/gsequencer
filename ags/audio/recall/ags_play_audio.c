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

#include <ags/audio/recall/ags_play_audio.h>

void ags_play_audio_class_init(AgsPlayAudioClass *play_audio);
void ags_play_audio_init(AgsPlayAudio *play_audio);
void ags_play_audio_finalize(GObject *gobject);

/**
 * SECTION:ags_play_audio
 * @short_description: play audio 
 * @title: AgsPlayAudio
 * @section_id:
 * @include: ags/audio/recall/ags_play_audio.h
 *
 * The #AgsPlayAudio class provides ports to the effect processor.
 */

static gpointer ags_play_audio_parent_class = NULL;

GType
ags_play_audio_get_type()
{
  static volatile gsize g_define_type_id__volatile = 0;

  if(g_once_init_enter (&g_define_type_id__volatile)){
    GType ags_type_play_audio;

    static const GTypeInfo ags_play_audio_info = {
      sizeof (AgsPlayAudioClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_play_audio_class_init,
      NULL, /* class_finalize */
      NULL, /* class_audio */
      sizeof (AgsPlayAudio),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_play_audio_init,
    };

    ags_type_play_audio = g_type_register_static(AGS_TYPE_RECALL_AUDIO,
						 "AgsPlayAudio",
						 &ags_play_audio_info,
						 0);

    g_once_init_leave (&g_define_type_id__volatile, ags_type_play_audio);
  }

  return g_define_type_id__volatile;
}

void
ags_play_audio_class_init(AgsPlayAudioClass *play_audio)
{
  GObjectClass *gobject;
  GParamSpec *param_spec;

  ags_play_audio_parent_class = g_type_class_peek_parent(play_audio);

  /* GObjectClass */
  gobject = (GObjectClass *) play_audio;

  gobject->finalize = ags_play_audio_finalize;
}

void
ags_play_audio_init(AgsPlayAudio *play_audio)
{
  AGS_RECALL(play_audio)->name = "ags-play";
  AGS_RECALL(play_audio)->version = AGS_RECALL_DEFAULT_VERSION;
  AGS_RECALL(play_audio)->build_id = AGS_RECALL_DEFAULT_BUILD_ID;
  AGS_RECALL(play_audio)->xml_type = "ags-play-audio";
}

void
ags_play_audio_finalize(GObject *gobject)
{
  /* empty */

  /* call parent */
  G_OBJECT_CLASS(ags_play_audio_parent_class)->finalize(gobject);
}

/**
 * ags_play_audio_new:
 *
 * Creates an #AgsPlayAudio
 *
 * Returns: a new #AgsPlayAudio
 *
 * Since: 1.0.0
 */
AgsPlayAudio*
ags_play_audio_new()
{
  AgsPlayAudio *play_audio;

  play_audio = (AgsPlayAudio *) g_object_new(AGS_TYPE_PLAY_AUDIO,
					     NULL);

  return(play_audio);
}
