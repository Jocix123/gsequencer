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

#include <ags/audio/recall/ags_play_channel.h>

#include <ags/object/ags_connectable.h>
#include <ags/object/ags_run_connectable.h>

#include <ags/audio/ags_devout.h>
#include <ags/audio/ags_audio.h>
#include <ags/audio/ags_recycling.h>
#include <ags/audio/ags_audio_signal.h>
#include <ags/audio/ags_recall_id.h>

#include <ags/audio/task/ags_cancel_recall.h>

#include <ags/audio/recall/ags_play_recycling.h>

#include <stdlib.h>
#include <stdio.h>

void ags_play_channel_class_init(AgsPlayChannelClass *play_channel);
void ags_play_channel_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_play_channel_run_connectable_interface_init(AgsRunConnectableInterface *run_connectable);
void ags_play_channel_init(AgsPlayChannel *play_channel);
void ags_play_channel_connect(AgsConnectable *connectable);
void ags_play_channel_disconnect(AgsConnectable *connectable);
void ags_play_channel_run_connect(AgsRunConnectable *run_connectable);
void ags_play_channel_run_disconnect(AgsRunConnectable *run_connectable);
void ags_play_channel_finalize(GObject *gobject);

void ags_play_channel_run_init_pre(AgsPlayChannel *play_channel, guint audio_channel, gpointer data);

void ags_play_channel_done(AgsRecall *recall, gpointer data);
void ags_play_channel_remove(AgsRecall *recall, gpointer data);
void ags_play_channel_cancel(AgsRecall *recall, gpointer data);
AgsRecall* ags_play_channel_duplicate(AgsRecall *recall, AgsRecallID *recall_id);

void ags_play_channel_map_play_recycling(AgsPlayChannel *play_channel);
void ags_play_channel_remap_child_source(AgsPlayChannel *play_channel,
					 AgsRecycling *old_start_region, AgsRecycling *old_end_region,
					 AgsRecycling *new_start_region, AgsRecycling *new_end_region);


void ags_play_channel_source_recycling_changed_callback(AgsChannel *channel,
							AgsRecycling *old_start_region, AgsRecycling *old_end_region,
							AgsRecycling *new_start_region, AgsRecycling *new_end_region,
							AgsPlayChannel *play_channel);

static gpointer ags_play_channel_parent_class = NULL;
static AgsConnectableInterface *ags_play_channel_parent_connectable_interface;
static AgsRunConnectableInterface *ags_play_channel_parent_run_connectable_interface;

GType
ags_play_channel_get_type()
{
  static GType ags_type_play_channel = 0;

  if(!ags_type_play_channel){
    static const GTypeInfo ags_play_channel_info = {
      sizeof (AgsPlayChannelClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_play_channel_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsPlayChannel),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_play_channel_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_play_channel_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    static const GInterfaceInfo ags_run_connectable_interface_info = {
      (GInterfaceInitFunc) ags_play_channel_run_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_play_channel = g_type_register_static(AGS_TYPE_RECALL,
						   "AgsPlayChannel\0",
						   &ags_play_channel_info,
						   0);

    g_type_add_interface_static(ags_type_play_channel,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);

    g_type_add_interface_static(ags_type_play_channel,
				AGS_TYPE_RUN_CONNECTABLE,
				&ags_run_connectable_interface_info);
  }

  return(ags_type_play_channel);
}

void
ags_play_channel_class_init(AgsPlayChannelClass *play_channel)
{
  GObjectClass *gobject;
  AgsRecallClass *recall;

  ags_play_channel_parent_class = g_type_class_peek_parent(play_channel);

  gobject = (GObjectClass *) play_channel;

  gobject->finalize = ags_play_channel_finalize;

  recall = (AgsRecallClass *) play_channel;

  recall->duplicate = ags_play_channel_duplicate;
}

void
ags_play_channel_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_play_channel_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_play_channel_connect;
  connectable->disconnect = ags_play_channel_disconnect;
}

void
ags_play_channel_run_connectable_interface_init(AgsRunConnectableInterface *run_connectable)
{
  ags_play_channel_parent_run_connectable_interface = g_type_interface_peek_parent(run_connectable);

  run_connectable->connect = ags_play_channel_run_connect;
  run_connectable->disconnect = ags_play_channel_run_disconnect;
}

void
ags_play_channel_init(AgsPlayChannel *play_channel)
{
  //  play_channel->ref = 1;

  play_channel->devout = NULL;

  play_channel->source = NULL;
}

void
ags_play_channel_connect(AgsConnectable *connectable)
{
  AgsPlayChannel *play_channel;

  ags_play_channel_parent_connectable_interface->connect(connectable);

  /* AgsPlayChannel */
  play_channel = AGS_PLAY_CHANNEL(connectable);

  g_signal_connect((GObject *) play_channel, "run_init_pre\0",
		   G_CALLBACK(ags_play_channel_run_init_pre), NULL);

  g_signal_connect((GObject *) play_channel, "done\0",
		   G_CALLBACK(ags_play_channel_done), NULL);

  g_signal_connect((GObject *) play_channel, "remove\0",
		   G_CALLBACK(ags_play_channel_remove), NULL);

  g_signal_connect((GObject *) play_channel, "cancel\0",
		   G_CALLBACK(ags_play_channel_cancel), NULL);
}

void
ags_play_channel_disconnect(AgsConnectable *connectable)
{
  ags_play_channel_parent_connectable_interface->disconnect(connectable);
}

void
ags_play_channel_run_connect(AgsRunConnectable *run_connectable)
{
  AgsPlayChannel *play_channel;
  GObject *gobject;

  ags_play_channel_parent_run_connectable_interface->connect(run_connectable);

  /* AgsPlayChannel */
  play_channel = AGS_PLAY_CHANNEL(run_connectable);

  /* source */
  gobject = G_OBJECT(play_channel->source);

  play_channel->source_recycling_changed_handler =
    g_signal_connect(gobject, "recycling_changed\0",
		     G_CALLBACK(ags_play_channel_source_recycling_changed_callback), play_channel);
}

void
ags_play_channel_run_disconnect(AgsRunConnectable *run_connectable)
{
  AgsPlayChannel *play_channel;
  GObject *gobject;

  ags_play_channel_parent_run_connectable_interface->disconnect(run_connectable);

  /* AgsPlayChannel */
  play_channel = AGS_PLAY_CHANNEL(run_connectable);

  /* source */
  gobject = G_OBJECT(play_channel->source);

  g_signal_handler_disconnect(gobject, play_channel->source_recycling_changed_handler);
}

void
ags_play_channel_finalize(GObject *gobject)
{
  G_OBJECT_CLASS(ags_play_channel_parent_class)->finalize(gobject);
}

void
ags_play_channel_run_init_pre(AgsPlayChannel *play_channel, guint audio_channel, gpointer data)
{
  /* empty */
}

void
ags_play_channel_done(AgsRecall *recall, gpointer data)
{
  /* empty */
}

void 
ags_play_channel_cancel(AgsRecall *recall, gpointer data)
{
  /* empty */
}

void
ags_play_channel_remove(AgsRecall *recall, gpointer data)
{
  /* empty */
}

AgsRecall*
ags_play_channel_duplicate(AgsRecall *recall, AgsRecallID *recall_id)
{
  AgsPlayChannel *play_channel, *copy;
  
  play_channel = (AgsPlayChannel *) recall;
  copy = (AgsPlayChannel *) AGS_RECALL_CLASS(ags_play_channel_parent_class)->duplicate(recall, recall_id);

  copy->devout = play_channel->devout;
  copy->source = play_channel->source;

  ags_play_channel_map_play_recycling(copy);

  return((AgsRecall *) copy);
}

void
ags_play_channel_map_play_recycling(AgsPlayChannel *play_channel)
{
  AgsRecycling *source_recycling;
  AgsPlayRecycling *play_recycling;

  source_recycling = play_channel->source->first_recycling;

  if(source_recycling != NULL){
    while(source_recycling != play_channel->source->last_recycling->next){
      printf("mapping\n\0");

      play_recycling = ags_play_recycling_new(source_recycling, play_channel->source->audio_channel,
					      play_channel->devout);

      ags_recall_add_child(AGS_RECALL(play_channel), AGS_RECALL(play_recycling), play_channel->source->audio_channel);

      source_recycling = source_recycling->next;
    }
  }
}

void ags_play_channel_remap_child_source(AgsPlayChannel *play_channel,
					 AgsRecycling *old_start_region, AgsRecycling *old_end_region,
					 AgsRecycling *new_start_region, AgsRecycling *new_end_region)
{
  AgsRecycling *source_recycling;
  AgsPlayRecycling *play_recycling;
  GList *list;
  guint audio_channel;

  audio_channel = play_channel->source->audio_channel;

  /* remove old */
  if(old_start_region != NULL){
    AgsDevout *devout;
    AgsRecall *recall;
    AgsCancelRecall *cancel_recall;

    devout = AGS_DEVOUT(AGS_AUDIO(play_channel->source->audio)->devout);
    source_recycling = old_start_region;

    while(source_recycling != old_end_region->next){
      list = play_channel->recall.child;

      while(list != NULL){
	if(AGS_PLAY_RECYCLING(list->data)->source == source_recycling){
	  recall = AGS_RECALL(list->data);
	  
	  recall->flags |= AGS_RECALL_HIDE;
	  cancel_recall = ags_cancel_recall_new(recall, audio_channel,
						NULL);

	  ags_devout_append_task(devout, (AgsTask *) cancel_recall);
	}

	list = list->next;
      }

      source_recycling = source_recycling->next;
    }
  }

  /* remove new */
  if(new_start_region != NULL){
    source_recycling = new_start_region;

    while(source_recycling != new_end_region->next){
      play_recycling = ags_play_recycling_new(source_recycling, audio_channel,
					      play_channel->devout);

      ags_recall_add_child(AGS_RECALL(play_channel), AGS_RECALL(play_recycling), audio_channel);
      
      source_recycling = source_recycling->next;
    }
  }
}

void
ags_play_channel_source_recycling_changed_callback(AgsChannel *channel,
						   AgsRecycling *old_start_region, AgsRecycling *old_end_region,
						   AgsRecycling *new_start_region, AgsRecycling *new_end_region,
						   AgsPlayChannel *play_channel)
{
  ags_play_channel_remap_child_source(play_channel,
				      old_start_region, old_end_region,
				      new_start_region, new_end_region);
}

AgsPlayChannel*
ags_play_channel_new(AgsChannel *source,
		     AgsDevout *devout)
{
  AgsPlayChannel *play_channel;

  play_channel = (AgsPlayChannel *) g_object_new(AGS_TYPE_PLAY_CHANNEL, NULL);

  play_channel->devout = devout;

  play_channel->source = source;

  return(play_channel);
}
