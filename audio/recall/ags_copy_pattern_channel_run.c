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

#include <ags/audio/recall/ags_copy_pattern_audio.h>
#include <ags/audio/recall/ags_copy_pattern_audio_run.h>
#include <ags/audio/recall/ags_copy_pattern_channel.h>

#include <stdlib.h>

void ags_copy_pattern_channel_run_class_init(AgsCopyPatternChannelRunClass *copy_pattern_channel_run);
void ags_copy_pattern_channel_run_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_copy_pattern_channel_run_run_connectable_interface_init(AgsRunConnectableInterface *run_connectable);
void ags_copy_pattern_channel_run_init(AgsCopyPatternChannelRun *copy_pattern_channel_run);
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

void ags_copy_pattern_channel_run_run_order_changed(AgsRecallChannelRun *recall_channel_run,
						    guint nth_run);

void ags_copy_pattern_channel_run_tic_alloc_callback(AgsDelayAudioRun *delay_audio_run,
						     guint nth_run,
						     AgsCopyPatternChannelRun *copy_pattern_channel_run);


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
  AgsRecallChannelRunClass *recall_channel_run;
  GParamSpec *param_spec;

  ags_copy_pattern_channel_run_parent_class = g_type_class_peek_parent(copy_pattern_channel_run);

  /* GObjectClass */
  gobject = (GObjectClass *) copy_pattern_channel_run;

  gobject->finalize = ags_copy_pattern_channel_run_finalize;

  /* AgsRecallClass */
  recall = (AgsRecallClass *) copy_pattern_channel_run;

  recall->run_init_pre = ags_copy_pattern_channel_run_run_init_pre;
  recall->done = ags_copy_pattern_channel_run_done;
  recall->cancel = ags_copy_pattern_channel_run_cancel;
  recall->remove = ags_copy_pattern_channel_run_remove;
  recall->duplicate = ags_copy_pattern_channel_run_duplicate;

  /* AgsRecallChannelRunClass */
  recall_channel_run = (AgsRecallChannelRunClass *) copy_pattern_channel_run;
  recall_channel_run->run_order_changed = ags_copy_pattern_channel_run_run_order_changed;
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
  copy_pattern_channel_run->nth_run = 0;
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
  AgsCopyPatternAudioRun *copy_pattern_audio_run;
  AgsCopyPatternChannelRun *copy_pattern_channel_run;

  ags_copy_pattern_channel_run_parent_run_connectable_interface->connect(run_connectable);

  /* AgsCopyPatternChannelRun */
  copy_pattern_channel_run = AGS_COPY_PATTERN_CHANNEL_RUN(run_connectable);

  /* get AgsCopyPatternAudioRun */
  copy_pattern_audio_run = AGS_COPY_PATTERN_AUDIO_RUN(copy_pattern_channel_run->recall_channel_run.recall_audio_run);

  /* connect tic_alloc in AgsDelayAudioRun */
  g_object_ref(G_OBJECT(copy_pattern_audio_run->delay_audio_run));
  copy_pattern_channel_run->tic_alloc_handler =
    g_signal_connect(G_OBJECT(copy_pattern_audio_run->delay_audio_run), "tic_alloc_input\0",
		     G_CALLBACK(ags_copy_pattern_channel_run_tic_alloc_callback), copy_pattern_channel_run);
}

void
ags_copy_pattern_channel_run_run_disconnect(AgsRunConnectable *run_connectable)
{
  AgsCopyPatternAudioRun *copy_pattern_audio_run;
  AgsCopyPatternChannelRun *copy_pattern_channel_run;

  ags_copy_pattern_channel_run_parent_run_connectable_interface->disconnect(run_connectable);

  /* AgsCopyPatternChannelRun */
  copy_pattern_channel_run = AGS_COPY_PATTERN_CHANNEL_RUN(run_connectable);

  /* get AgsCopyPatternAudioRun */
  copy_pattern_audio_run = AGS_COPY_PATTERN_AUDIO_RUN(copy_pattern_channel_run->recall_channel_run.recall_audio_run);

  /* disconnect tic_alloc in AgsDelayAudioRun */
  g_signal_handler_disconnect(G_OBJECT(copy_pattern_audio_run->delay_audio_run),
			      copy_pattern_channel_run->tic_alloc_handler);
  g_object_ref(G_OBJECT(copy_pattern_audio_run->delay_audio_run));
}

void
ags_copy_pattern_channel_run_finalize(GObject *gobject)
{
  G_OBJECT_CLASS(ags_copy_pattern_channel_run_parent_class)->finalize(gobject);
}

void
ags_copy_pattern_channel_run_run_init_pre(AgsRecall *recall)
{
  AgsCopyPatternAudio *copy_pattern_audio;
  AgsCopyPatternAudioRun *copy_pattern_audio_run;
  AgsCopyPatternChannel *copy_pattern_channel;
  AgsCopyPatternChannelRun *copy_pattern_channel_run;

  AGS_RECALL_CLASS(ags_copy_pattern_channel_run_parent_class)->run_init_pre(recall);

  copy_pattern_channel_run = AGS_COPY_PATTERN_CHANNEL_RUN(recall);

  /* get AgsCopyPatternAudio */
  copy_pattern_audio = AGS_COPY_PATTERN_AUDIO(AGS_RECALL_CHANNEL_RUN(copy_pattern_channel_run)->recall_audio_run->recall_audio);

  /* get AgsCopyPatternAudioRun */
  copy_pattern_audio_run = AGS_COPY_PATTERN_AUDIO_RUN(AGS_RECALL_CHANNEL_RUN(copy_pattern_channel_run)->recall_audio_run);

  /* get AgsCopyPatternChannel */
  copy_pattern_channel = AGS_COPY_PATTERN_CHANNEL(copy_pattern_channel_run->recall_channel_run.recall_channel);

  if(copy_pattern_audio_run->delay_audio_run == NULL){
    AgsAudio *audio;
    AgsChannel *source;
    AgsRecallID *parent_recall_id;
    GList *delay_list;

    /* get source */
    source = copy_pattern_channel->recall_channel.channel;

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
  AgsCopyPatternChannelRun *copy_pattern_channel_run;
  AgsCopyPatternAudioRun *copy_pattern_audio_run;

  AGS_RECALL_CLASS(ags_copy_pattern_channel_run_parent_class)->cancel(recall);

  copy_pattern_channel_run = AGS_COPY_PATTERN_CHANNEL_RUN(recall);

  /* get AgsCopyPatternAudioRun */
  copy_pattern_audio_run = AGS_COPY_PATTERN_AUDIO_RUN(copy_pattern_channel_run->recall_channel_run.recall_audio_run);

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
ags_copy_pattern_channel_run_run_order_changed(AgsRecallChannelRun *recall_channel_run,
					       guint nth_run)
{
  AGS_COPY_PATTERN_CHANNEL_RUN(recall_channel_run)->nth_run = nth_run;
  /*
  GValue value = {0,};

  g_value_init(&value, G_TYPE_UINT);
  g_value_set_uint(&value, nth_run);
  g_object_set_property(G_OBJECT(recall_channel_run),
			"nth_run\0",
			&value);
  g_value_unset(&value);
  */
}

void
ags_copy_pattern_channel_run_tic_alloc_callback(AgsDelayAudioRun *delay_audio_run,
						guint nth_run,
						AgsCopyPatternChannelRun *copy_pattern_channel_run)
{
  AgsChannel *output, *source;
  AgsCopyPatternAudio *copy_pattern_audio;
  AgsCopyPatternAudioRun *copy_pattern_audio_run;
  AgsCopyPatternChannel *copy_pattern_channel;
  //  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

  //  pthread_mutex_lock(&mutex);
  if(copy_pattern_channel_run->nth_run != nth_run){
    return;
  }

  /* get AgsCopyPatternAudio */
  copy_pattern_audio = AGS_COPY_PATTERN_AUDIO(copy_pattern_channel_run->recall_channel_run.recall_audio_run->recall_audio);

  /* get AgsCopyPatternAudioRun */
  copy_pattern_audio_run = AGS_COPY_PATTERN_AUDIO_RUN(copy_pattern_channel_run->recall_channel_run.recall_audio_run);

  /* get AgsCopyPatternChannel */
  copy_pattern_channel = AGS_COPY_PATTERN_CHANNEL(copy_pattern_channel_run->recall_channel_run.recall_channel);

  if(ags_pattern_get_bit((AgsPattern *) copy_pattern_channel->pattern,
			 copy_pattern_audio->i, copy_pattern_audio->j,
			 copy_pattern_audio_run->bit)){
    AgsRecycling *recycling;
    AgsAudioSignal *audio_signal;
    printf("channel: %u\n\0", copy_pattern_channel->recall_channel.channel->line);

    /* get source */
    source = copy_pattern_channel->recall_channel.channel;
    
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
ags_copy_pattern_channel_run_new()
{
  AgsCopyPatternChannelRun *copy_pattern_channel_run;

  copy_pattern_channel_run = (AgsCopyPatternChannelRun *) g_object_new(AGS_TYPE_COPY_PATTERN_CHANNEL_RUN,
								       NULL);

  return(copy_pattern_channel_run);
}
