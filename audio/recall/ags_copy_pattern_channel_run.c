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

#include <ags/audio/recall/ags_copy_pattern_channel_run.h>

#include <ags/object/ags_connectable.h>
#include <ags/object/ags_run_connectable.h>

#include <ags/audio/ags_audio.h>
#include <ags/audio/ags_recycling.h>
#include <ags/audio/ags_audio_signal.h>
#include <ags/audio/ags_recall_id.h>
#include <ags/audio/ags_recall_container.h>

#include <ags/audio/recall/ags_copy_pattern_audio.h>
#include <ags/audio/recall/ags_copy_pattern_audio_run.h>
#include <ags/audio/recall/ags_copy_pattern_channel.h>

#include <stdlib.h>

void ags_copy_pattern_channel_run_class_init(AgsCopyPatternChannelRunClass *copy_pattern_channel_run);
void ags_copy_pattern_channel_run_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_copy_pattern_channel_run_run_connectable_interface_init(AgsRunConnectableInterface *run_connectable);
void ags_copy_pattern_channel_run_init(AgsCopyPatternChannelRun *copy_pattern_channel_run);
void ags_copy_pattern_channel_run_set_property(GObject *gobject,
					       guint prop_id,
					       const GValue *value,
					       GParamSpec *param_spec);
void ags_copy_pattern_channel_run_get_property(GObject *gobject,
					       guint prop_id,
					       GValue *value,
					       GParamSpec *param_spec);
void ags_copy_pattern_channel_run_connect(AgsConnectable *connectable);
void ags_copy_pattern_channel_run_disconnect(AgsConnectable *connectable);
void ags_copy_pattern_channel_run_run_connect(AgsRunConnectable *run_connectable);
void ags_copy_pattern_channel_run_run_disconnect(AgsRunConnectable *run_connectable);
void ags_copy_pattern_channel_run_finalize(GObject *gobject);

void ags_copy_pattern_channel_run_run_init_pre(AgsRecall *recall);
void ags_copy_pattern_channel_run_done(AgsRecall *recall);
void ags_copy_pattern_channel_run_cancel(AgsRecall *recall);
void ags_copy_pattern_channel_run_remove(AgsRecall *recall);
AgsRecall* ags_copy_pattern_channel_run_duplicate(AgsRecall *recall, AgsRecallID *recall_id);
void ags_copy_pattern_channel_run_run_order_changed(AgsRecall *recall, guint nth_run);

void ags_copy_pattern_channel_run_tic_alloc_callback(AgsDelayAudioRun *delay_audio_run,
						     guint audio_channel,
						     AgsCopyPatternChannelRun *copy_pattern_channel_run);

enum{
  PROP_0,
  PROP_DELAY_AUDIO_RUN,
};

static gpointer ags_copy_pattern_channel_run_parent_class = NULL;
static AgsConnectableInterface* ags_copy_pattern_channel_run_parent_connectable_interface;
static AgsRunConnectableInterface *ags_copy_pattern_channel_run_parent_run_connectable_interface;

GType
ags_copy_pattern_channel_run_get_type()
{
  static GType ags_type_copy_pattern_channel_run = 0;

  if(!ags_type_copy_pattern_channel_run){
    static const GTypeInfo ags_copy_pattern_channel_run_info = {
      sizeof (AgsCopyPatternChannelRunClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_copy_pattern_channel_run_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsCopyPatternChannelRun),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_copy_pattern_channel_run_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_copy_pattern_channel_run_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    static const GInterfaceInfo ags_run_connectable_interface_info = {
      (GInterfaceInitFunc) ags_copy_pattern_channel_run_run_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_copy_pattern_channel_run = g_type_register_static(AGS_TYPE_RECALL_CHANNEL_RUN,
							       "AgsCopyPatternChannelRun\0",
							       &ags_copy_pattern_channel_run_info,
							       0);
    
    g_type_add_interface_static(ags_type_copy_pattern_channel_run,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
    
    g_type_add_interface_static(ags_type_copy_pattern_channel_run,
				AGS_TYPE_RUN_CONNECTABLE,
				&ags_run_connectable_interface_info);
  }

  return(ags_type_copy_pattern_channel_run);
}

void
ags_copy_pattern_channel_run_class_init(AgsCopyPatternChannelRunClass *copy_pattern_channel_run)
{
  GObjectClass *gobject;
  AgsRecallClass *recall;
  GParamSpec *param_spec;

  ags_copy_pattern_channel_run_parent_class = g_type_class_peek_parent(copy_pattern_channel_run);

  /* GObjectClass */
  gobject = (GObjectClass *) copy_pattern_channel_run;

  gobject->set_property = ags_copy_pattern_channel_run_set_property;
  gobject->get_property = ags_copy_pattern_channel_run_get_property;

  gobject->finalize = ags_copy_pattern_channel_run_finalize;

  /* properties */
  param_spec = g_param_spec_object("delay_audio_run\0",
				   "assigned AgsDelayAudioRun\0",
				   "The AgsDelayAudioRun which emits tic_alloc signal\0",
				   AGS_TYPE_DELAY_AUDIO_RUN,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_DELAY_AUDIO_RUN,
				  param_spec);

  /* AgsRecallClass */
  recall = (AgsRecallClass *) copy_pattern_channel_run;

  recall->run_init_pre = ags_copy_pattern_channel_run_run_init_pre;
  recall->done = ags_copy_pattern_channel_run_done;
  recall->cancel = ags_copy_pattern_channel_run_cancel;
  recall->remove = ags_copy_pattern_channel_run_remove;
  recall->duplicate = ags_copy_pattern_channel_run_duplicate;
}

void
ags_copy_pattern_channel_run_connectable_interface_init(AgsConnectableInterface *connectable)
{
  AgsConnectableInterface *ags_copy_pattern_channel_run_connectable_parent_interface;

  ags_copy_pattern_channel_run_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_copy_pattern_channel_run_connect;
  connectable->disconnect = ags_copy_pattern_channel_run_disconnect;
}

void
ags_copy_pattern_channel_run_run_connectable_interface_init(AgsRunConnectableInterface *run_connectable)
{
  ags_copy_pattern_channel_run_parent_run_connectable_interface = g_type_interface_peek_parent(run_connectable);

  run_connectable->connect = ags_copy_pattern_channel_run_run_connect;
  run_connectable->disconnect = ags_copy_pattern_channel_run_run_disconnect;
}

void
ags_copy_pattern_channel_run_init(AgsCopyPatternChannelRun *copy_pattern_channel_run)
{
  copy_pattern_channel_run->delay_audio_run = NULL;
  copy_pattern_channel_run->nth_run = 0;
}

void
ags_copy_pattern_channel_run_set_property(GObject *gobject,
					  guint prop_id,
					  const GValue *value,
					  GParamSpec *param_spec)
{
  AgsCopyPatternChannelRun *copy_pattern_channel_run;

  copy_pattern_channel_run = AGS_COPY_PATTERN_CHANNEL_RUN(gobject);

  switch(prop_id){
  case PROP_DELAY_AUDIO_RUN:
    {
      AgsRecallContainer *recall_container;
      AgsCopyPatternAudioRun *copy_pattern_audio_run;
      AgsDelayAudioRun *delay_audio_run;
      GValue recall_container_value = {0,};
      GValue copy_pattern_audio_run_value = {0,};
      GValue delay_audio_run_value = {0,};

      /* get AgsRecallContainer */
      g_value_init(&recall_container_value, G_TYPE_OBJECT);
      g_object_get_property(G_OBJECT(copy_pattern_channel_run),
			    "recall_container\0",
			    &recall_container_value);
      recall_container = AGS_RECALL_CONTAINER(g_value_get_object(&recall_container_value));
      g_value_unset(&recall_container_value);

      printf("debug 1\n\0");

      /* get AgsCopyPatternAudioRun */
      g_value_init(&copy_pattern_audio_run_value, G_TYPE_OBJECT);
      g_object_get_property(G_OBJECT(recall_container),
			    "recall_audio_run\0",
			    &copy_pattern_audio_run_value);
      copy_pattern_audio_run = AGS_COPY_PATTERN_AUDIO_RUN(g_value_get_object(&copy_pattern_audio_run_value));
      g_value_unset(&copy_pattern_audio_run_value);

      printf("debug 2\n\0");

      /* AgsDelayAudioRun */
      g_value_init(&delay_audio_run_value, G_TYPE_OBJECT);
      g_object_get_property(G_OBJECT(copy_pattern_audio_run),
			    "delay_audio_run\0",
			    &delay_audio_run_value);
      delay_audio_run = (AgsDelayAudioRun *) g_value_get_object(&delay_audio_run_value);
      g_value_unset(&delay_audio_run_value);

      if(copy_pattern_channel_run->delay_audio_run == delay_audio_run)
	return;

      printf("debug 3\n\0");

      /* disconnect and unref */
      if(copy_pattern_channel_run->delay_audio_run != NULL){
	if((AGS_RECALL_RUN_INITIALIZED & (AGS_RECALL(gobject)->flags)) != 0)
	  g_signal_handler_disconnect(G_OBJECT(copy_pattern_audio_run->delay_audio_run),
				      copy_pattern_channel_run->tic_alloc_handler);

	g_object_unref(copy_pattern_channel_run->delay_audio_run);
      }

      /* ref and connect */
      if(delay_audio_run != NULL){
	g_object_ref(delay_audio_run);

	if((AGS_RECALL_RUN_INITIALIZED & (AGS_RECALL(gobject)->flags)) != 0)
	  copy_pattern_channel_run->tic_alloc_handler =
	    g_signal_connect(G_OBJECT(copy_pattern_audio_run->delay_audio_run), "tic_alloc\0",
			     G_CALLBACK(ags_copy_pattern_channel_run_tic_alloc_callback), copy_pattern_channel_run);
      }

      copy_pattern_channel_run->delay_audio_run = delay_audio_run;
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_copy_pattern_channel_run_get_property(GObject *gobject,
					  guint prop_id,
					  GValue *value,
					  GParamSpec *param_spec)
{
  AgsCopyPatternChannelRun *copy_pattern_channel_run;

  copy_pattern_channel_run = AGS_COPY_PATTERN_CHANNEL_RUN(gobject);

  switch(prop_id){
  case PROP_DELAY_AUDIO_RUN:
    {
      g_value_set_object(value, copy_pattern_channel_run->delay_audio_run);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_copy_pattern_channel_run_connect(AgsConnectable *connectable)
{
  AgsCopyPatternChannelRun *copy_pattern_channel_run;

  ags_copy_pattern_channel_run_parent_connectable_interface->connect(connectable);

  /* empty */
}

void
ags_copy_pattern_channel_run_disconnect(AgsConnectable *connectable)
{
  ags_copy_pattern_channel_run_parent_connectable_interface->disconnect(connectable);

  /* empty */
}

void
ags_copy_pattern_channel_run_run_connect(AgsRunConnectable *run_connectable)
{
  AgsRecallContainer *recall_container;
  AgsCopyPatternAudioRun *copy_pattern_audio_run;
  AgsCopyPatternChannelRun *copy_pattern_channel_run;
  GValue recall_container_value = {0,};
  GValue copy_pattern_audio_run_value = {0,};

  ags_copy_pattern_channel_run_parent_run_connectable_interface->connect(run_connectable);

  /* AgsCopyPatternChannelRun */
  copy_pattern_channel_run = AGS_COPY_PATTERN_CHANNEL_RUN(run_connectable);

  /* get AgsRecallContainer */
  g_value_init(&recall_container_value, G_TYPE_OBJECT);
  g_object_get_property(G_OBJECT(copy_pattern_channel_run),
			"container\0",
			&recall_container_value);
  recall_container = AGS_RECALL_CONTAINER(g_value_get_object(&recall_container_value));
  g_value_unset(&recall_container_value);

  /* get AgsCopyPatternAudioRun */
  g_value_init(&copy_pattern_audio_run_value, G_TYPE_OBJECT);
  g_object_get_property(G_OBJECT(recall_container),
			"recall_audio_run\0",
			&copy_pattern_audio_run_value);
  copy_pattern_audio_run = AGS_COPY_PATTERN_AUDIO_RUN(g_value_get_object(&copy_pattern_audio_run_value));
  g_value_unset(&copy_pattern_audio_run_value);

  /* connect tic_alloc in AgsDelayAudioRun */
  copy_pattern_channel_run->tic_alloc_handler =
    g_signal_connect(G_OBJECT(copy_pattern_audio_run->delay_audio_run), "tic_alloc\0",
		     G_CALLBACK(ags_copy_pattern_channel_run_tic_alloc_callback), copy_pattern_channel_run);
}

void
ags_copy_pattern_channel_run_run_disconnect(AgsRunConnectable *run_connectable)
{
  AgsRecallContainer *recall_container;
  AgsCopyPatternAudioRun *copy_pattern_audio_run;
  AgsCopyPatternChannelRun *copy_pattern_channel_run;
  GValue recall_container_value = {0,};
  GValue copy_pattern_audio_run_value = {0,};

  ags_copy_pattern_channel_run_parent_run_connectable_interface->disconnect(run_connectable);

  /* get AgsRecallContainer */
  g_value_init(&recall_container_value, G_TYPE_OBJECT);
  g_object_get_property(G_OBJECT(run_connectable),
			"container\0",
			&recall_container_value);
  recall_container = AGS_RECALL_CONTAINER(g_value_get_object(&recall_container_value));
  g_value_unset(&recall_container_value);

  /* get AgsCopyPatternAudioRun */
  g_value_init(&copy_pattern_audio_run_value, G_TYPE_OBJECT);
  g_object_get_property(G_OBJECT(recall_container),
			"recall_audio_run\0",
			&copy_pattern_audio_run_value);
  copy_pattern_audio_run = AGS_COPY_PATTERN_AUDIO_RUN(g_value_get_object(&copy_pattern_audio_run_value));
  g_value_unset(&copy_pattern_audio_run_value);

  /* disconnect tic_alloc in AgsDelayAudioRun */
  g_signal_handler_disconnect(G_OBJECT(copy_pattern_audio_run->delay_audio_run),
			      copy_pattern_channel_run->tic_alloc_handler);
}

void
ags_copy_pattern_channel_run_finalize(GObject *gobject)
{
  G_OBJECT_CLASS(ags_copy_pattern_channel_run_parent_class)->finalize(gobject);
}

void
ags_copy_pattern_channel_run_run_init_pre(AgsRecall *recall)
{
  AgsRecallContainer *recall_container;
  AgsCopyPatternAudio *copy_pattern_audio;
  AgsCopyPatternAudioRun *copy_pattern_audio_run;
  AgsCopyPatternChannel *copy_pattern_channel;
  AgsCopyPatternChannelRun *copy_pattern_channel_run;
  GValue recall_container_value = {0,};
  GValue copy_pattern_audio_value = {0,};
  GValue copy_pattern_audio_run_value = {0,};
  GValue copy_pattern_channel_value = {0,};

  AGS_RECALL_CLASS(ags_copy_pattern_channel_run_parent_class)->run_init_pre(recall);

  /* get AgsRecallContainer */
  g_value_init(&recall_container_value, G_TYPE_OBJECT);
  g_object_get_property(G_OBJECT(recall),
			"container\0",
			&recall_container_value);
  recall_container = AGS_RECALL_CONTAINER(g_value_get_object(&recall_container_value));
  g_value_unset(&recall_container_value);

  /* get AgsCopyPatternAudio */
  g_value_init(&copy_pattern_audio_value, G_TYPE_OBJECT);
  g_object_get_property(G_OBJECT(recall_container),
			"recall_audio\0",
			&copy_pattern_audio_value);
  copy_pattern_audio = AGS_COPY_PATTERN_AUDIO(g_value_get_object(&copy_pattern_audio_value));
  g_value_unset(&copy_pattern_audio_value);

  /* get AgsCopyPatternAudioRun */
  g_value_init(&copy_pattern_audio_run_value, G_TYPE_OBJECT);
  g_object_get_property(G_OBJECT(recall_container),
			"recall_audio_run\0",
			&copy_pattern_audio_run_value);
  copy_pattern_audio_run = AGS_COPY_PATTERN_AUDIO_RUN(g_value_get_object(&copy_pattern_audio_run_value));
  g_value_unset(&copy_pattern_audio_run_value);

  /* get AgsCopyPatternChannel */
  g_value_init(&copy_pattern_channel_value, G_TYPE_OBJECT);
  g_object_get_property(G_OBJECT(recall_container),
			"recall_audio\0",
			&copy_pattern_channel_value);
  copy_pattern_channel = AGS_COPY_PATTERN_CHANNEL(g_value_get_object(&copy_pattern_channel_value));
  g_value_unset(&copy_pattern_channel_value);

  if(copy_pattern_audio_run->delay_audio_run == NULL){
    AgsAudio *audio;
    AgsChannel *source;
    AgsRecallID *parent_recall_id;
    GList *delay_list;
    GValue source_value= {0,};

    /* get source */
    g_value_init(&source_value, G_TYPE_OBJECT);
    g_object_get_property(G_OBJECT(copy_pattern_channel),
			  "channel\0",
			  &source_value);
    source = AGS_CHANNEL(g_value_get_object(&source_value));
    g_value_unset(&source_value);

    /* find AgsRecallID */
    audio = AGS_AUDIO(source->audio);

    parent_recall_id = ags_recall_id_find_group_id(copy_pattern_channel->destination->recall_id,
						   AGS_RECALL(copy_pattern_channel_run)->recall_id->parent_group_id);

    /* find AgsDelayAudioRun */
    if(parent_recall_id->parent_group_id == 0)
      delay_list = ags_recall_find_type_with_group_id(audio->play, AGS_TYPE_DELAY_AUDIO_RUN, parent_recall_id->group_id);
    else
      delay_list = ags_recall_find_type_with_group_id(audio->recall, AGS_TYPE_DELAY_AUDIO_RUN, parent_recall_id->group_id);
    
    copy_pattern_audio_run->delay_audio_run = ((delay_list != NULL) ? AGS_DELAY_AUDIO_RUN(delay_list->data): NULL);
  }

  /* notify dependency */
  ags_recall_notify_dependency(AGS_RECALL(copy_pattern_audio_run->delay_audio_run),
			       AGS_RECALL_NOTIFY_CHANNEL_RUN, 1);
  copy_pattern_audio_run->recall_ref++;
}

void
ags_copy_pattern_channel_run_done(AgsRecall *recall)
{
  AgsCopyPatternChannelRun *copy_pattern_channel_run;

  AGS_RECALL_CLASS(ags_copy_pattern_channel_run_parent_class)->done(recall);

  copy_pattern_channel_run = AGS_COPY_PATTERN_CHANNEL_RUN(recall);

  //  copy_pattern_channel_run->shared_audio_run->delay->recall_ref--;
  //  copy_pattern_channel_run->shared_audio_run->recall_ref--;
}

void
ags_copy_pattern_channel_run_cancel(AgsRecall *recall)
{
  AgsRecallContainer *recall_container;
  AgsCopyPatternAudioRun *copy_pattern_audio_run;
  GValue recall_container_value = {0,};
  GValue copy_pattern_audio_run_value = {0,};

  AGS_RECALL_CLASS(ags_copy_pattern_channel_run_parent_class)->cancel(recall);

  /* get AgsRecallContainer */
  g_value_init(&recall_container_value, G_TYPE_OBJECT);
  g_object_get_property(G_OBJECT(recall),
			"container\0",
			&recall_container_value);
  recall_container = AGS_RECALL_CONTAINER(g_value_get_object(&recall_container_value));

  /* get AgsCopyPatternAudioRun */
  g_value_init(&copy_pattern_audio_run_value, G_TYPE_OBJECT);
  g_object_get_property(G_OBJECT(recall),
			"recall_audio_run\0",
			&copy_pattern_audio_run_value);
  copy_pattern_audio_run = AGS_COPY_PATTERN_AUDIO_RUN(g_value_get_object(&copy_pattern_audio_run_value));
  g_value_unset(&copy_pattern_audio_run_value);

  /* notify dependency */
  ags_recall_notify_dependency(AGS_RECALL(copy_pattern_audio_run->delay_audio_run),
			       AGS_RECALL_NOTIFY_CHANNEL_RUN, -1);
  //  copy_pattern_channel_run->shared_audio_run->recall_ref--;
}

void
ags_copy_pattern_channel_run_remove(AgsRecall *recall)
{

  AGS_RECALL_CLASS(ags_copy_pattern_channel_run_parent_class)->remove(recall);

  /* empty */
}

AgsRecall*
ags_copy_pattern_channel_run_duplicate(AgsRecall *recall, AgsRecallID *recall_id)
{
  AgsCopyPatternChannelRun *copy;

  copy = AGS_COPY_PATTERN_CHANNEL_RUN(AGS_RECALL_CLASS(ags_copy_pattern_channel_run_parent_class)->duplicate(recall, recall_id));

  /* empty */

  return((AgsRecall *) copy);
}

void
ags_copy_pattern_channel_run_run_order_changed(AgsRecall *recall, guint nth_run)
{
  GValue value = {0,};

  g_value_init(&value, G_TYPE_UINT);
  g_value_set_uint(&value, nth_run);
  g_object_set_property(G_OBJECT(recall),
			"nth_run\0",
			&value);
  g_value_unset(&value);
}

void
ags_copy_pattern_channel_run_tic_alloc_callback(AgsDelayAudioRun *delay_audio_run,
						guint nth_run,
						AgsCopyPatternChannelRun *copy_pattern_channel_run)
{
  AgsChannel *output, *source;
  AgsRecallContainer *recall_container;
  AgsCopyPatternAudio *copy_pattern_audio;
  AgsCopyPatternAudioRun *copy_pattern_audio_run;
  AgsCopyPatternChannel *copy_pattern_channel;
  //  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
  GValue recall_container_value = {0,};
  GValue copy_pattern_audio_value = {0,};
  GValue copy_pattern_audio_run_value = {0,};
  GValue copy_pattern_channel_value = {0,};

  //  pthread_mutex_lock(&mutex);
  if(copy_pattern_channel_run->nth_run != nth_run){
    return;
  }

  /* get AgsRecallContainer */
  g_value_init(&recall_container_value, G_TYPE_OBJECT);
  g_object_get_property(G_OBJECT(copy_pattern_channel_run),
			"container\0",
			&recall_container_value);
  recall_container = AGS_RECALL_CONTAINER(g_value_get_object(&recall_container_value));
  g_value_unset(&recall_container_value);

  /* get AgsCopyPatternAudio */
  g_value_init(&copy_pattern_audio_value, G_TYPE_OBJECT);
  g_object_get_property(G_OBJECT(recall_container),
			"recall_audio\0",
			&copy_pattern_audio_value);
  copy_pattern_audio = AGS_COPY_PATTERN_AUDIO(g_value_get_object(&copy_pattern_audio_value));
  g_value_unset(&copy_pattern_audio_value);

  /* get AgsCopyPatternAudioRun */
  g_value_init(&copy_pattern_audio_run_value, G_TYPE_OBJECT);
  g_object_get_property(G_OBJECT(recall_container),
			"recall_audio_run\0",
			&copy_pattern_audio_run_value);
  copy_pattern_audio_run = AGS_COPY_PATTERN_AUDIO_RUN(g_value_get_object(&copy_pattern_audio_run_value));
  g_value_unset(&copy_pattern_audio_run_value);

  /* get AgsCopyPatternChannel */
  g_value_init(&copy_pattern_channel_value, G_TYPE_OBJECT);
  g_object_get_property(G_OBJECT(recall_container),
			"recall_audio\0",
			&copy_pattern_channel_value);
  copy_pattern_channel = AGS_COPY_PATTERN_CHANNEL(g_value_get_object(&copy_pattern_channel_value));
  g_value_unset(&copy_pattern_channel_value);

  if(ags_pattern_get_bit((AgsPattern *) copy_pattern_channel->pattern,
			 copy_pattern_audio->i, copy_pattern_audio->j,
			 copy_pattern_audio_run->bit)){
    AgsRecycling *recycling;
    AgsAudioSignal *audio_signal;
    GValue source_value = {0,};

    /* get source */
    g_value_init(&source_value, G_TYPE_OBJECT);
    g_object_get_property(G_OBJECT(copy_pattern_channel),
			  "channel\0",
			  &source_value);
    source = AGS_CHANNEL(g_value_get_object(&source_value));
    g_value_unset(&source_value);
    fprintf(stdout, "ags_copy_pattern - playing: bit = %u; line = %u\n\0", copy_pattern_audio_run->bit, source->line);
    
    /* create new audio signals */
    recycling = source->first_recycling;
	
    if(recycling != NULL){
      while(recycling != source->last_recycling->next){
	audio_signal = ags_audio_signal_new((GObject *) copy_pattern_audio->devout,
					    (GObject *) recycling,
					    (GObject *) AGS_RECALL(copy_pattern_channel_run)->recall_id);
	ags_audio_signal_connect(audio_signal);
	
	ags_recycling_add_audio_signal(recycling,
				       audio_signal);
	
	recycling = recycling->next;
      }
    }
  }
  
      //      printf("%u\n\0", copy_pattern->shared_audio_run->bit);
      //      copy_pattern->shared_audio_run->bit++;
  //  }
}

AgsCopyPatternChannelRun*
ags_copy_pattern_channel_run_new(AgsDelayAudioRun *delay_audio_run)
{
  AgsCopyPatternChannelRun *copy_pattern_channel_run;

  copy_pattern_channel_run = (AgsCopyPatternChannelRun *) g_object_new(AGS_TYPE_COPY_PATTERN_CHANNEL_RUN,
								       "delay_audio_run\0", delay_audio_run,
								       NULL);

  return(copy_pattern_channel_run);
}
