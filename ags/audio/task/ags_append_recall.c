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

#include <ags/audio/task/ags_append_recall.h>

#include <ags/audio/ags_recall.h>
#include <ags/audio/ags_playback.h>

#include <ags/audio/thread/ags_audio_loop.h>

#include <ags/i18n.h>

void ags_append_recall_class_init(AgsAppendRecallClass *append_recall);
void ags_append_recall_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_append_recall_init(AgsAppendRecall *append_recall);
void ags_append_recall_set_property(GObject *gobject,
				    guint prop_id,
				    const GValue *value,
				    GParamSpec *param_spec);
void ags_append_recall_get_property(GObject *gobject,
				    guint prop_id,
				    GValue *value,
				    GParamSpec *param_spec);
void ags_append_recall_connect(AgsConnectable *connectable);
void ags_append_recall_disconnect(AgsConnectable *connectable);
void ags_append_recall_dispose(GObject *gobject);
void ags_append_recall_finalize(GObject *gobject);

void ags_append_recall_launch(AgsTask *task);

/**
 * SECTION:ags_append_recall
 * @short_description: append recall object to audio loop
 * @title: AgsAppendRecall
 * @section_id:
 * @include: ags/audio/task/ags_append_recall.h
 *
 * The #AgsAppendRecall task appends #AgsRecall to #AgsAudioLoop.
 */

static gpointer ags_append_recall_parent_class = NULL;
static AgsConnectableInterface *ags_append_recall_parent_connectable_interface;

enum{
  PROP_0,
  PROP_AUDIO_LOOP,
  PROP_PLAYBACK,
};

GType
ags_append_recall_get_type()
{
  static volatile gsize g_define_type_id__volatile = 0;

  if(g_once_init_enter (&g_define_type_id__volatile)){
    GType ags_type_append_recall;

    static const GTypeInfo ags_append_recall_info = {
      sizeof (AgsAppendRecallClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_append_recall_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsAppendRecall),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_append_recall_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_append_recall_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_append_recall = g_type_register_static(AGS_TYPE_TASK,
						  "AgsAppendRecall",
						  &ags_append_recall_info,
						  0);

    g_type_add_interface_static(ags_type_append_recall,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);

    g_once_init_leave (&g_define_type_id__volatile, ags_type_append_recall);
  }

  return g_define_type_id__volatile;
}

void
ags_append_recall_class_init(AgsAppendRecallClass *append_recall)
{
  GObjectClass *gobject;
  AgsTaskClass *task;
  GParamSpec *param_spec;
  
  ags_append_recall_parent_class = g_type_class_peek_parent(append_recall);

  /* GObjectClass */
  gobject = (GObjectClass *) append_recall;

  gobject->set_property = ags_append_recall_set_property;
  gobject->get_property = ags_append_recall_get_property;

  gobject->dispose = ags_append_recall_dispose;
  gobject->finalize = ags_append_recall_finalize;

  /* properties */
  /**
   * AgsAppendRecall:audio-loop:
   *
   * The assigned #AgsAudioLoop
   * 
   * Since: 1.0.0
   */
  param_spec = g_param_spec_object("audio-loop",
				   i18n_pspec("audio loop of append recall"),
				   i18n_pspec("The audio loop of append recall task"),
				   AGS_TYPE_AUDIO_LOOP,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_AUDIO_LOOP,
				  param_spec);

  /**
   * AgsAppendRecall:playback:
   *
   * The assigned #AgsPlayback
   * 
   * Since: 1.0.0
   */
  param_spec = g_param_spec_object("playback",
				   i18n_pspec("playback of append recall"),
				   i18n_pspec("The playback of append recall task"),
				   AGS_TYPE_PLAYBACK,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_PLAYBACK,
				  param_spec);
  
  /* AgsTaskClass */
  task = (AgsTaskClass *) append_recall;

  task->launch = ags_append_recall_launch;
}

void
ags_append_recall_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_append_recall_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_append_recall_connect;
  connectable->disconnect = ags_append_recall_disconnect;
}

void
ags_append_recall_init(AgsAppendRecall *append_recall)
{
  append_recall->audio_loop = NULL;
  append_recall->playback = NULL;
}

void
ags_append_recall_set_property(GObject *gobject,
			      guint prop_id,
			      const GValue *value,
			      GParamSpec *param_spec)
{
  AgsAppendRecall *append_recall;

  append_recall = AGS_APPEND_RECALL(gobject);

  switch(prop_id){
  case PROP_AUDIO_LOOP:
    {
      AgsAudioLoop *audio_loop;

      audio_loop = (AgsAudioLoop *) g_value_get_object(value);

      if(append_recall->audio_loop == (GObject *) audio_loop){
	return;
      }

      if(append_recall->audio_loop != NULL){
	g_object_unref(append_recall->audio_loop);
      }

      if(audio_loop != NULL){
	g_object_ref(audio_loop);
      }

      append_recall->audio_loop = (GObject *) audio_loop;
    }
    break;
  case PROP_PLAYBACK:
    {
      AgsPlayback *playback;

      playback = (AgsPlayback *) g_value_get_object(value);

      if(append_recall->playback == (GObject *) playback){
	return;
      }

      if(append_recall->playback != NULL){
	g_object_unref(append_recall->playback);
      }

      if(playback != NULL){
	g_object_ref(playback);
      }

      append_recall->playback = (GObject *) playback;
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_append_recall_get_property(GObject *gobject,
			      guint prop_id,
			      GValue *value,
			      GParamSpec *param_spec)
{
  AgsAppendRecall *append_recall;

  append_recall = AGS_APPEND_RECALL(gobject);

  switch(prop_id){
  case PROP_AUDIO_LOOP:
    {
      g_value_set_object(value, append_recall->audio_loop);
    }
    break;
  case PROP_PLAYBACK:
    {
      g_value_set_object(value, append_recall->playback);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_append_recall_connect(AgsConnectable *connectable)
{
  ags_append_recall_parent_connectable_interface->connect(connectable);

  /* empty */
}

void
ags_append_recall_disconnect(AgsConnectable *connectable)
{
  ags_append_recall_parent_connectable_interface->disconnect(connectable);

  /* empty */
}

void
ags_append_recall_dispose(GObject *gobject)
{
  AgsAppendRecall *append_recall;

  append_recall = AGS_APPEND_RECALL(gobject);

  if(append_recall->audio_loop != NULL){
    g_object_unref(append_recall->audio_loop);

    append_recall->audio_loop = NULL;
  }

  if(append_recall->playback != NULL){
    g_object_unref(append_recall->playback);

    append_recall->playback = NULL;
  }
  
  /* call parent */
  G_OBJECT_CLASS(ags_append_recall_parent_class)->dispose(gobject);
}

void
ags_append_recall_finalize(GObject *gobject)
{
  AgsAppendRecall *append_recall;

  append_recall = AGS_APPEND_RECALL(gobject);

  if(append_recall->audio_loop != NULL){
    g_object_unref(append_recall->audio_loop);
  }

  if(append_recall->playback != NULL){
    g_object_unref(append_recall->playback);
  }

  /* call parent */
  G_OBJECT_CLASS(ags_append_recall_parent_class)->finalize(gobject);
}

void
ags_append_recall_launch(AgsTask *task)
{
  AgsAppendRecall *append_recall;

  AgsAudioLoop *audio_loop;

  append_recall = AGS_APPEND_RECALL(task);

  audio_loop = AGS_AUDIO_LOOP(append_recall->audio_loop);
  
  ags_audio_loop_add_recall(audio_loop,
			    append_recall->playback);
}

/**
 * ags_append_recall_new:
 * @audio_loop: the #AgsAudioLoop
 * @playback: the #AgsRecall to append
 *
 * Creates an #AgsAppendRecall.
 *
 * Returns: an new #AgsAppendRecall.
 *
 * Since: 1.0.0
 */
AgsAppendRecall*
ags_append_recall_new(GObject *audio_loop,
		      GObject *playback)
{
  AgsAppendRecall *append_recall;

  append_recall = (AgsAppendRecall *) g_object_new(AGS_TYPE_APPEND_RECALL,
						   "audio-loop", audio_loop,
						   "playback", playback,
						   NULL);

  return(append_recall);
}
