/* GSequencer - Advanced GTK Sequencer
 * Copyright (C) 2005-2019 Joël Krähemann
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

#include <ags/audio/recall/ags_copy_pattern_channel_run.h>

#include <ags/audio/ags_audio.h>
#include <ags/audio/ags_recycling.h>
#include <ags/audio/ags_audio_signal.h>
#include <ags/audio/ags_preset.h>
#include <ags/audio/ags_pattern.h>
#include <ags/audio/ags_recall_id.h>
#include <ags/audio/ags_recall_container.h>

#include <ags/audio/recall/ags_copy_pattern_audio.h>
#include <ags/audio/recall/ags_copy_pattern_audio_run.h>
#include <ags/audio/recall/ags_copy_pattern_channel.h>
#include <ags/audio/recall/ags_delay_audio_run.h>
#include <ags/audio/recall/ags_count_beats_audio_run.h>

#include <stdlib.h>

void ags_copy_pattern_channel_run_class_init(AgsCopyPatternChannelRunClass *copy_pattern_channel_run);
void ags_copy_pattern_channel_run_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_copy_pattern_channel_run_init(AgsCopyPatternChannelRun *copy_pattern_channel_run);
void ags_copy_pattern_channel_run_dispose(GObject *gobject);
void ags_copy_pattern_channel_run_finalize(GObject *gobject);

void ags_copy_pattern_channel_run_notify_recall_audio_run(GObject *gobject,
							  GParamSpec *pspec,
							  gpointer user_data);
void ags_copy_pattern_channel_run_notify_recall_audio_run_after(GObject *gobject,
								GParamSpec *pspec,
								gpointer user_data);
void ags_copy_pattern_channel_run_notify_delay_audio_run(GObject *gobject,
							 GParamSpec *pspec,
							 AgsCopyPatternChannelRun *copy_pattern_channel_run);
void ags_copy_pattern_channel_run_notify_delay_audio_run_after(GObject *gobject,
							       GParamSpec *pspec,
							       AgsCopyPatternChannelRun *copy_pattern_channel_run);

void ags_copy_pattern_channel_run_connect(AgsConnectable *connectable);
void ags_copy_pattern_channel_run_disconnect(AgsConnectable *connectable);
void ags_copy_pattern_channel_run_connect_connection(AgsConnectable *connectable, GObject *connection);
void ags_copy_pattern_channel_run_disconnect_connection(AgsConnectable *connectable, GObject *connection);

void ags_copy_pattern_channel_run_run_init_pre(AgsRecall *recall);
void ags_copy_pattern_channel_run_done(AgsRecall *recall);

void ags_copy_pattern_channel_run_sequencer_alloc_callback(AgsDelayAudioRun *delay_audio_run,
							   guint run_order,
							   gdouble delay, guint attack,
							   AgsCopyPatternChannelRun *copy_pattern_channel_run);

/**
 * SECTION:ags_copy_pattern_channel_run
 * @short_description: copys pattern
 * @title: AgsCopyPatternChannelRun
 * @section_id:
 * @include: ags/audio/recall/ags_copy_pattern_channel_run.h
 *
 * The #AgsCopyPatternChannelRun class copys pattern.
 */

static gpointer ags_copy_pattern_channel_run_parent_class = NULL;
static AgsConnectableInterface* ags_copy_pattern_channel_run_parent_connectable_interface;

GType
ags_copy_pattern_channel_run_get_type()
{
  static volatile gsize g_define_type_id__volatile = 0;

  if(g_once_init_enter (&g_define_type_id__volatile)){
    GType ags_type_copy_pattern_channel_run = 0;

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

    ags_type_copy_pattern_channel_run = g_type_register_static(AGS_TYPE_RECALL_CHANNEL_RUN,
							       "AgsCopyPatternChannelRun",
							       &ags_copy_pattern_channel_run_info,
							       0);
    
    g_type_add_interface_static(ags_type_copy_pattern_channel_run,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);

    g_once_init_leave(&g_define_type_id__volatile, ags_type_copy_pattern_channel_run);
  }

  return g_define_type_id__volatile;
}

void
ags_copy_pattern_channel_run_class_init(AgsCopyPatternChannelRunClass *copy_pattern_channel_run)
{
  GObjectClass *gobject;
  AgsRecallClass *recall;

  ags_copy_pattern_channel_run_parent_class = g_type_class_peek_parent(copy_pattern_channel_run);

  /* GObjectClass */
  gobject = (GObjectClass *) copy_pattern_channel_run;

  gobject->dispose = ags_copy_pattern_channel_run_dispose;
  gobject->finalize = ags_copy_pattern_channel_run_finalize;

  /* AgsRecallClass */
  recall = (AgsRecallClass *) copy_pattern_channel_run;

  recall->run_init_pre = ags_copy_pattern_channel_run_run_init_pre;
  recall->done = ags_copy_pattern_channel_run_done;
}

void
ags_copy_pattern_channel_run_connectable_interface_init(AgsConnectableInterface *connectable)
{
  AgsConnectableInterface *ags_copy_pattern_channel_run_connectable_parent_interface;

  ags_copy_pattern_channel_run_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_copy_pattern_channel_run_connect;
  connectable->disconnect = ags_copy_pattern_channel_run_disconnect;

  connectable->connect_connection = ags_copy_pattern_channel_run_connect_connection;
  connectable->disconnect_connection = ags_copy_pattern_channel_run_disconnect_connection;
}

void
ags_copy_pattern_channel_run_init(AgsCopyPatternChannelRun *copy_pattern_channel_run)
{
  ags_recall_set_ability_flags((AgsRecall *) copy_pattern_channel_run, (AGS_SOUND_ABILITY_SEQUENCER));

  //  g_signal_connect(copy_pattern_channel_run, "notify::recall-audio-run",
  //		   G_CALLBACK(ags_copy_pattern_channel_run_notify_recall_audio_run), NULL);
  
  g_signal_connect_after(copy_pattern_channel_run, "notify::recall-audio-run",
			 G_CALLBACK(ags_copy_pattern_channel_run_notify_recall_audio_run_after), NULL);

  /*  */
  AGS_RECALL(copy_pattern_channel_run)->name = "ags-copy-pattern";
  AGS_RECALL(copy_pattern_channel_run)->version = AGS_RECALL_DEFAULT_VERSION;
  AGS_RECALL(copy_pattern_channel_run)->build_id = AGS_RECALL_DEFAULT_BUILD_ID;
  AGS_RECALL(copy_pattern_channel_run)->xml_type = "ags-copy-pattern-channel-run";
  AGS_RECALL(copy_pattern_channel_run)->port = NULL;

  AGS_RECALL(copy_pattern_channel_run)->child_type = G_TYPE_NONE;

  copy_pattern_channel_run->note = NULL;
}

void
ags_copy_pattern_channel_run_dispose(GObject *gobject)
{
  AgsCopyPatternChannelRun *copy_pattern_channel_run;

  copy_pattern_channel_run = (AgsCopyPatternChannelRun *) gobject;

  /* note */
  g_list_free_full(copy_pattern_channel_run->note,
		   g_object_unref);

  copy_pattern_channel_run->note = NULL;
  
  /* call parent */
  G_OBJECT_CLASS(ags_copy_pattern_channel_run_parent_class)->dispose(gobject);
}

void
ags_copy_pattern_channel_run_finalize(GObject *gobject)
{
  AgsCopyPatternChannelRun *copy_pattern_channel_run;

  copy_pattern_channel_run = (AgsCopyPatternChannelRun *) gobject;

  /* note */
  g_list_free_full(copy_pattern_channel_run->note,
		   g_object_unref);
  
  /* call parent */
  G_OBJECT_CLASS(ags_copy_pattern_channel_run_parent_class)->finalize(gobject);
}

void
ags_copy_pattern_channel_run_notify_recall_audio_run(GObject *gobject,
						     GParamSpec *pspec,
						     gpointer user_data)
{
  AgsCopyPatternAudioRun *copy_pattern_audio_run;
  AgsCopyPatternChannelRun *copy_pattern_channel_run;

  copy_pattern_channel_run = AGS_COPY_PATTERN_CHANNEL_RUN(gobject);

  g_object_get(copy_pattern_channel_run,
	       "recall-audio-run", &copy_pattern_audio_run,
	       NULL);

  if(copy_pattern_audio_run != NULL){
    AgsDelayAudioRun *delay_audio_run;
    
    //    g_object_disconnect(copy_pattern_audio_run,
    //			"any_signal::notify::delay-audio-run",
    //			G_CALLBACK(ags_copy_pattern_channel_run_notify_delay_audio_run),
    //			copy_pattern_channel_run,
    //			NULL);
  
    g_object_disconnect(copy_pattern_audio_run,
			"any_signal::notify::delay-audio-run",
			G_CALLBACK(ags_copy_pattern_channel_run_notify_delay_audio_run_after),
			copy_pattern_channel_run,
			NULL);

    /* connection */
    g_object_get(copy_pattern_audio_run,
		 "delay-audio-run", &delay_audio_run,
		 NULL);

    if(delay_audio_run != NULL &&
       ags_connectable_is_connected(AGS_CONNECTABLE(copy_pattern_channel_run))){
      ags_copy_pattern_channel_run_disconnect_connection(AGS_CONNECTABLE(copy_pattern_channel_run), (GObject *) delay_audio_run);
    }

    if(delay_audio_run != NULL){
      g_object_unref(delay_audio_run);
    }

    g_object_unref(copy_pattern_audio_run);
  }
}

void
ags_copy_pattern_channel_run_notify_recall_audio_run_after(GObject *gobject,
							   GParamSpec *pspec,
							   gpointer user_data)
{
  AgsCopyPatternAudioRun *copy_pattern_audio_run;
  AgsCopyPatternChannelRun *copy_pattern_channel_run;

  copy_pattern_channel_run = AGS_COPY_PATTERN_CHANNEL_RUN(gobject);
  
  g_object_get(copy_pattern_channel_run,
	       "recall-audio-run", &copy_pattern_audio_run,
	       NULL);

  if(copy_pattern_audio_run != NULL){
    AgsDelayAudioRun *delay_audio_run;
    
    //    g_signal_connect(copy_pattern_audio_run, "notify::delay-audio-run",
    //		     G_CALLBACK(ags_copy_pattern_channel_run_notify_delay_audio_run), copy_pattern_channel_run);
  
    g_signal_connect_after(copy_pattern_audio_run, "notify::delay-audio-run",
			   G_CALLBACK(ags_copy_pattern_channel_run_notify_delay_audio_run_after), copy_pattern_channel_run);

    /* connection */
    g_object_get(copy_pattern_audio_run,
		 "delay-audio-run", &delay_audio_run,
		 NULL);

    if(delay_audio_run != NULL &&
       ags_connectable_is_connected(AGS_CONNECTABLE(copy_pattern_channel_run))){
      ags_copy_pattern_channel_run_connect_connection(AGS_CONNECTABLE(copy_pattern_channel_run), (GObject *) delay_audio_run);
    }

    if(delay_audio_run != NULL){
      g_object_unref(delay_audio_run);
    }
    
    g_object_unref(copy_pattern_audio_run);
  }
}

void
ags_copy_pattern_channel_run_notify_delay_audio_run(GObject *gobject,
						    GParamSpec *pspec,
						    AgsCopyPatternChannelRun *copy_pattern_channel_run)
{
  AgsCopyPatternAudioRun *copy_pattern_audio_run;
  AgsDelayAudioRun *delay_audio_run;

  copy_pattern_audio_run = AGS_COPY_PATTERN_AUDIO_RUN(gobject);

  g_object_get(copy_pattern_audio_run,
	       "delay-audio-run", &delay_audio_run,
	       NULL);

  if(delay_audio_run != NULL &&
     ags_connectable_is_connected(AGS_CONNECTABLE(copy_pattern_channel_run))){
    ags_connectable_disconnect_connection(AGS_CONNECTABLE(copy_pattern_channel_run), (GObject *) delay_audio_run);
  }

  if(delay_audio_run != NULL){
    g_object_unref(delay_audio_run);
  }
}

void
ags_copy_pattern_channel_run_notify_delay_audio_run_after(GObject *gobject,
							  GParamSpec *pspec,
							  AgsCopyPatternChannelRun *copy_pattern_channel_run)
{
  AgsCopyPatternAudioRun *copy_pattern_audio_run;
  AgsDelayAudioRun *delay_audio_run;

  copy_pattern_audio_run = AGS_COPY_PATTERN_AUDIO_RUN(gobject);

  g_object_get(copy_pattern_audio_run,
	       "delay-audio-run", &delay_audio_run,
	       NULL);

  if(delay_audio_run != NULL &&
     ags_connectable_is_connected(AGS_CONNECTABLE(copy_pattern_channel_run))){
    ags_connectable_connect_connection(AGS_CONNECTABLE(copy_pattern_channel_run), (GObject *) delay_audio_run);
  }

  if(delay_audio_run != NULL){
    g_object_unref(delay_audio_run);
  }
}

void
ags_copy_pattern_channel_run_connect(AgsConnectable *connectable)
{
  AgsCopyPatternAudioRun *copy_pattern_audio_run;
  AgsCopyPatternChannelRun *copy_pattern_channel_run;
  
  GRecMutex *recall_mutex;

  if(ags_connectable_is_connected(connectable)){
    return;
  }

  copy_pattern_channel_run = AGS_COPY_PATTERN_CHANNEL_RUN(connectable);
  
  ags_copy_pattern_channel_run_parent_connectable_interface->connect(connectable);

  /* get recall mutex */
  recall_mutex = AGS_RECALL_GET_OBJ_MUTEX(copy_pattern_channel_run);

  /* connection */
  g_object_get(copy_pattern_channel_run,
	       "recall-audio-run", &copy_pattern_audio_run,
	       NULL);

  if(copy_pattern_audio_run != NULL){
    AgsDelayAudioRun *delay_audio_run;
    
    g_object_get(copy_pattern_audio_run,
		 "delay-audio-run", &delay_audio_run,
		 NULL);
    
    if(delay_audio_run != NULL){
      ags_connectable_connect_connection(connectable, (GObject *) delay_audio_run);

      g_object_unref(delay_audio_run);
    }

    g_object_unref(copy_pattern_audio_run);
  }
}

void
ags_copy_pattern_channel_run_disconnect(AgsConnectable *connectable)
{
  AgsCopyPatternAudioRun *copy_pattern_audio_run;
  AgsCopyPatternChannelRun *copy_pattern_channel_run;
  
  GRecMutex *recall_mutex;

  if(!ags_connectable_is_connected(connectable)){
    return;
  }

  copy_pattern_channel_run = AGS_COPY_PATTERN_CHANNEL_RUN(connectable);

  ags_copy_pattern_channel_run_parent_connectable_interface->disconnect(connectable);

  /* get recall mutex */
  recall_mutex = AGS_RECALL_GET_OBJ_MUTEX(copy_pattern_channel_run);

  /* connection */
  g_object_get(copy_pattern_channel_run,
	       "recall-audio-run", &copy_pattern_audio_run,
	       NULL);

  if(copy_pattern_audio_run != NULL){
    AgsDelayAudioRun *delay_audio_run;
    
    g_object_get(copy_pattern_audio_run,
		 "delay-audio-run", &delay_audio_run,
		 NULL);

    if(delay_audio_run != NULL){
      ags_connectable_disconnect_connection(connectable, (GObject *) delay_audio_run);

      g_object_unref(delay_audio_run);
    }

    g_object_unref(copy_pattern_audio_run);
  }
}

void
ags_copy_pattern_channel_run_connect_connection(AgsConnectable *connectable, GObject *connection)
{
  AgsCopyPatternAudioRun *copy_pattern_audio_run;
  AgsCopyPatternChannelRun *copy_pattern_channel_run;

  copy_pattern_channel_run = AGS_COPY_PATTERN_CHANNEL_RUN(connectable);
  
  g_object_get(copy_pattern_channel_run,
	       "recall-audio-run", &copy_pattern_audio_run,
	       NULL);

  if(copy_pattern_audio_run != NULL){
    AgsDelayAudioRun *delay_audio_run;
    
    g_object_get(copy_pattern_audio_run,
		 "delay-audio-run", &delay_audio_run,
		 NULL);

    if((GObject *) delay_audio_run == connection){
      g_signal_connect(G_OBJECT(delay_audio_run), "sequencer-alloc-input",
		       G_CALLBACK(ags_copy_pattern_channel_run_sequencer_alloc_callback), copy_pattern_channel_run);
    }

    if(delay_audio_run != NULL){
      g_object_unref(delay_audio_run);
    }
    
    g_object_unref(copy_pattern_audio_run);
  }
}

void
ags_copy_pattern_channel_run_disconnect_connection(AgsConnectable *connectable, GObject *connection)
{
  AgsCopyPatternAudioRun *copy_pattern_audio_run;
  AgsCopyPatternChannelRun *copy_pattern_channel_run;

  copy_pattern_channel_run = AGS_COPY_PATTERN_CHANNEL_RUN(connectable);
  
  g_object_get(copy_pattern_channel_run,
	       "recall-audio-run", &copy_pattern_audio_run,
	       NULL);

  if(copy_pattern_audio_run != NULL){
    AgsDelayAudioRun *delay_audio_run;
    
    g_object_get(copy_pattern_audio_run,
		 "delay-audio-run", &delay_audio_run,
		 NULL);

    if((GObject *) delay_audio_run == connection){
      g_object_disconnect(G_OBJECT(delay_audio_run),
			  "any_signal::sequencer-alloc-input",
			  G_CALLBACK(ags_copy_pattern_channel_run_sequencer_alloc_callback),
			  copy_pattern_channel_run,
			  NULL);
    }

    if(delay_audio_run != NULL){
      g_object_unref(delay_audio_run);
    }
    
    g_object_unref(copy_pattern_audio_run);
  }
}

void
ags_copy_pattern_channel_run_run_init_pre(AgsRecall *recall)
{
  AgsChannel *source;
  AgsPattern *pattern;
  AgsNote *note;
  AgsPort *port;
  
  AgsCopyPatternChannel *copy_pattern_channel;
  AgsCopyPatternChannelRun *copy_pattern_channel_run;

  guint pad;
  guint i, i_stop;
  
  GValue pattern_value = { 0, };  

  void (*parent_class_run_init_pre)(AgsRecall *recall);
  
  GRecMutex *recall_mutex;
  GRecMutex *pattern_mutex;

  copy_pattern_channel_run = AGS_COPY_PATTERN_CHANNEL_RUN(recall);

  /* get mutex */
  recall_mutex = AGS_RECALL_GET_OBJ_MUTEX(recall);

  /* get parent class */
  parent_class_run_init_pre = AGS_RECALL_CLASS(ags_copy_pattern_channel_run_parent_class)->run_init_pre;

  /* call parent */
  parent_class_run_init_pre(recall);

  /* get some fields */
  g_object_get(copy_pattern_channel_run,
	       "source", &source,
	       "recall-channel", &copy_pattern_channel,
	       NULL);

  /* get AgsPattern */
  g_object_get(copy_pattern_channel,
	       "pattern", &port,
	       NULL);
  
  g_value_init(&pattern_value, G_TYPE_POINTER);
  ags_port_safe_read(port,
		     &pattern_value);

  pattern = g_value_get_pointer(&pattern_value);

  g_value_unset(&pattern_value);

  g_object_unref(port);
  
  /* add note */
  g_object_get(source,
	       "pad", &pad,
	       NULL);

  /* get pattern mutex */
  pattern_mutex = AGS_PATTERN_GET_OBJ_MUTEX(pattern);

  /* i stop */  
  g_rec_mutex_lock(pattern_mutex);

  i_stop = pattern->dim[2];

  g_rec_mutex_unlock(pattern_mutex);
  
  for(i = 0; i < i_stop; i++){
    note = ags_note_new();

    note->x[0] = i;
    note->x[1] = i + 1;

    note->y = pad;

    copy_pattern_channel_run->note = g_list_prepend(copy_pattern_channel_run->note,
						    note);
    //    g_object_ref(note);
  }

  copy_pattern_channel_run->note = g_list_reverse(copy_pattern_channel_run->note);

  /* unref */
  g_object_unref(source);
  
  g_object_unref(copy_pattern_channel);
}

void
ags_copy_pattern_channel_run_done(AgsRecall *recall)
{
  AgsCopyPatternAudioRun *copy_pattern_audio_run;
  AgsCopyPatternChannelRun *copy_pattern_channel_run;
  AgsCountBeatsAudioRun *count_beats_audio_run;

  void (*parent_class_done)(AgsRecall *recall);
  
  GRecMutex *recall_mutex;

  copy_pattern_channel_run = AGS_COPY_PATTERN_CHANNEL_RUN(recall);

  /* get mutex */
  recall_mutex = AGS_RECALL_GET_OBJ_MUTEX(recall);

  /* get parent class */  
  parent_class_done = AGS_RECALL_CLASS(ags_copy_pattern_channel_run_parent_class)->done;
  
  /* get AgsCopyPatternAudioRun */
  g_object_get(recall,
	       "recall-audio-run", &copy_pattern_audio_run,
	       NULL);

  /* denotify dependency */
  g_object_get(copy_pattern_audio_run,
	       "count-beats-audio-run", &count_beats_audio_run,
	       NULL);

  ags_recall_notify_dependency((AgsRecall *) count_beats_audio_run,
 			       AGS_RECALL_NOTIFY_CHANNEL_RUN, FALSE);

  /* free notes */
  g_rec_mutex_lock(recall_mutex);

  g_list_free_full(copy_pattern_channel_run->note,
		   g_object_unref);

  copy_pattern_channel_run->note = NULL;
  
  g_rec_mutex_unlock(recall_mutex);

  /* call parent */
  parent_class_done(recall);

  /* unref */
  g_object_unref(copy_pattern_audio_run);

  g_object_unref(count_beats_audio_run);
}

void
ags_copy_pattern_channel_run_sequencer_alloc_callback(AgsDelayAudioRun *delay_audio_run,
						      guint run_order,
						      gdouble delay, guint attack,
						      AgsCopyPatternChannelRun *copy_pattern_channel_run)
{
  AgsAudio *audio;
  AgsChannel *source;
  AgsPattern *pattern;
  AgsPort *port;
  
  AgsCopyPatternAudio *copy_pattern_audio;
  AgsCopyPatternAudioRun *copy_pattern_audio_run;
  AgsCopyPatternChannel *copy_pattern_channel;
  AgsCountBeatsAudioRun *count_beats_audio_run;

  guint64 sequencer_counter;
  gboolean current_bit;

  GValue pattern_value = { 0, };  
  GValue i_value = { 0, };
  GValue j_value = { 0, };

  GRecMutex *pattern_mutex;

  if(delay != 0.0){
    return;
  }
    
  g_object_get(copy_pattern_channel_run,
	       "recall-audio", &copy_pattern_audio,
	       "recall-audio-run", &copy_pattern_audio_run,
	       "recall-channel", &copy_pattern_channel,
	       NULL);

  g_object_get(copy_pattern_audio_run,
	       "count-beats-audio-run", &count_beats_audio_run,
	       NULL);
  
  /* get bank index 0 */
  g_object_get(copy_pattern_audio,
	       "bank-index-0", &port,
	       NULL);
  
  g_value_init(&i_value, G_TYPE_FLOAT);
  ags_port_safe_read(port, &i_value);

  g_object_unref(port);
  
  /* get bank index 1 */
  g_object_get(copy_pattern_audio,
	       "bank-index-1", &port,
	       NULL);

  g_value_init(&j_value, G_TYPE_FLOAT);
  ags_port_safe_read(port, &j_value);

  g_object_unref(port);

  /* get AgsPattern */
  g_object_get(copy_pattern_channel,
	       "pattern", &port,
	       NULL);
  
  g_value_init(&pattern_value, G_TYPE_POINTER);
  ags_port_safe_read(port,
		     &pattern_value);

  pattern = g_value_get_pointer(&pattern_value);

  g_object_unref(port);

  /* get pattern mutex */
  pattern_mutex = AGS_PATTERN_GET_OBJ_MUTEX(pattern);
  
  /* write pattern port - current offset */
#if 0
  g_object_set(pattern,
	       "first-index", (guint) g_value_get_float(&i_value),
	       "second-index", (guint) g_value_get_float(&j_value),
	       NULL);
#endif
  
  /* get sequencer counter */
  g_object_get(count_beats_audio_run,
	       "sequencer-counter", &sequencer_counter,
	       NULL);
  
  /* read pattern port - current bit */  
  current_bit = ags_pattern_get_bit(pattern,
				    (guint) g_value_get_float(&i_value),
				    (guint) g_value_get_float(&j_value),
				    (guint) sequencer_counter);
  
  g_value_unset(&pattern_value);

  g_value_unset(&i_value);
  g_value_unset(&j_value);

  /*  */
  if(current_bit){
    AgsChannel *link;
    AgsRecycling *first_recycling, *last_recycling;
    AgsRecycling *recycling, *next_recycling;
    AgsRecycling *end_recycling;
    AgsAudioSignal *audio_signal;
    AgsNote *note;
    AgsPreset *current_preset;

    GObject *output_soundcard;
    
    GList *preset_start, *preset;

    guint pad;
    guint audio_channel;
    guint note_offset;
    
//    g_message("ags_copy_pattern_channel_run_sequencer_alloc_callback - playing channel: %u; playing pattern: %u",
//	      AGS_RECALL_CHANNEL(copy_pattern_channel)->source->line,
//	      copy_pattern_audio_run->count_beats_audio_run->sequencer_counter);

    /* get audio */
    g_object_get(copy_pattern_audio,
		 "audio", &audio,
		 NULL);
    
    /* get source */
    g_object_get(copy_pattern_channel,
		 "source", &source,
		 NULL);

    /* get output soundcard */
    g_object_get(copy_pattern_channel_run,
		 "output-soundcard", &output_soundcard,
		 NULL);

    note_offset = ags_soundcard_get_note_offset(AGS_SOUNDCARD(output_soundcard));
    
    /* source fields */
    g_object_get(source,
		 "link", &link,
		 "first-recycling", &first_recycling,
		 "last-recycling", &last_recycling,
		 "pad", &pad,
		 "audio-channel", &audio_channel,
		 NULL);

    recycling = first_recycling;
    g_object_ref(recycling);

    end_recycling = NULL;
    
    if(last_recycling != NULL){
      end_recycling = ags_recycling_next(last_recycling);
    }

    /* find preset scope envelope */
    current_preset = NULL;

    g_object_get(audio,
		 "preset", &preset_start,
		 NULL);

    preset = preset_start;

    while((preset = ags_preset_find_scope(preset,
					  "ags-envelope")) != NULL){
      guint audio_channel_start, audio_channel_end;
      guint pad_start, pad_end;
      guint x_start, x_end;

      g_object_get(preset->data,
		   "audio-channel-start", &audio_channel_start,
		   "audio-channel-end", &audio_channel_end,
		   "pad-start", &pad_start,
		   "pad-end", &pad_end,
		   "x-start", &x_start,
		   "x-end", &x_end,
		   NULL);
      
      if(audio_channel >= audio_channel_start &&
	 audio_channel < audio_channel_end &&
	 pad >= pad_start &&
	 pad < pad_end &&
	 sequencer_counter >= x_start &&
	 sequencer_counter < x_end){
	current_preset = preset->data;
	
	break;
      }

      preset = preset->next;
    }

    g_list_free_full(preset_start,
		     g_object_unref);

    note = g_list_nth_data(copy_pattern_channel_run->note,
			   sequencer_counter);

    g_object_set(note,
		 "rt-attack", attack,
		 NULL);
    
    /* create audio signals */
    next_recycling = NULL;

    if(recycling != NULL){
      AgsRecallID *child_recall_id;
      
      while(recycling != end_recycling){
	AgsNote *copy_note;

	child_recall_id = AGS_RECALL(copy_pattern_channel_run)->recall_id;

	/* apply preset */
	copy_note = ags_note_duplicate(note);
	g_object_set(copy_note,
		     "x0", note_offset,
		     "x1", note_offset + 1,
		     NULL);

	if(current_preset != NULL){
	  AgsComplex *val;
	  
	  GValue value = {0,};

	  GError *error;
	  
	  ags_note_set_flags(copy_note,
			     AGS_NOTE_ENVELOPE);
	  
	  /* get attack */
	  g_value_init(&value,
		       AGS_TYPE_COMPLEX);

	  error = NULL;
	  ags_preset_get_parameter((AgsPreset *) current_preset,
				   "attack", &value,
				   &error);

	  if(error == NULL){
	    g_object_set_property((GObject *) copy_note,
				  "attack",
				  &value);
	  }else{
	    g_error_free(error);
	  }

	  /* get decay */
	  g_value_reset(&value);

	  error = NULL;
	  ags_preset_get_parameter((AgsPreset *) current_preset,
				   "decay", &value,
				   &error);

	  if(error == NULL){
	    g_object_set_property((GObject *) copy_note,
				  "decay",
				  &value);
	  }else{
	    g_error_free(error);
	  }

	  /* get sustain */
	  g_value_reset(&value);

	  error = NULL;
	  ags_preset_get_parameter((AgsPreset *) current_preset,
				   "sustain", &value,
				   &error);

	  if(error == NULL){
	    g_object_set_property((GObject *) copy_note,
				  "sustain",
				  &value);
	  }else{
	    g_error_free(error);
	  }

	  /* get release */
	  g_value_reset(&value);

	  error = NULL;
	  ags_preset_get_parameter((AgsPreset *) current_preset,
				   "release", &value,
				   &error);

	  if(error == NULL){
	    g_object_set_property((GObject *) copy_note,
				  "release",
				  &value);
	  }else{
	    g_error_free(error);
	  }

	  /* get ratio */
	  g_value_reset(&value);

	  error = NULL;
	  ags_preset_get_parameter((AgsPreset *) current_preset,
				   "ratio", &value,
				   &error);

	  if(error == NULL){
	    g_object_set_property((GObject *) copy_note,
				  "ratio",
				  &value);
	  }else{
	    g_error_free(error);
	  }

	  g_value_unset(&value);
	}
		  
	if(!ags_recall_global_get_rt_safe()){
	  /* create audio signal */
	  audio_signal = ags_audio_signal_new(output_soundcard,
					      (GObject *) recycling,
					      (GObject *) child_recall_id);
	  ags_recycling_create_audio_signal_with_defaults(recycling,
							  audio_signal,
							  0.0, attack);

	  ags_audio_signal_unset_flags(audio_signal, AGS_AUDIO_SIGNAL_TEMPLATE);
	  
	  audio_signal->stream_current = audio_signal->stream;

	  ags_connectable_connect(AGS_CONNECTABLE(audio_signal));

	  ags_recycling_add_audio_signal(recycling,
					 audio_signal);
	  
	  g_object_set(audio_signal,
		       "note", copy_note,
		       NULL);

	  g_object_unref(copy_note);
	  //g_object_unref(audio_signal);
	}else{
	  GList *list_start, *list;

	  audio_signal = NULL;

	  g_object_get(recycling,
		       "audio-signal", &list_start,
		       NULL);
	  
	  list = ags_audio_signal_find_by_recall_id(list_start,
						    (GObject *) child_recall_id);
	    
	  if(list != NULL){
	    audio_signal = list->data;

	    g_object_set(audio_signal,
			 "note", copy_note,
			 NULL);

	    g_object_unref(copy_note);
	  }

	  g_list_free_full(list_start,
			   g_object_unref);
	  
	  g_object_set(note,
		       "rt-offset", 0,
		       NULL);
	}
		
	/*
	 * emit add_audio_signal on AgsRecycling
	 */
#ifdef AGS_DEBUG
	g_message("play %d %x", AGS_RECALL_CHANNEL_RUN(copy_pattern_channel_run)->source->line, AGS_RECALL(copy_pattern_channel_run)->recall_id);
#endif

	/*
	 * unref AgsAudioSignal because AgsCopyPatternChannelRun has no need for it
	 * if you need a valid reference to audio_signal you have to g_object_ref(audio_signal)
	 */
	//	g_object_unref(audio_signal);

	/* iterate */
	next_recycling = ags_recycling_next(recycling);

	g_object_unref(recycling);

	recycling = next_recycling;
      }
    }
    
    /* unref */
    g_object_unref(audio);
    g_object_unref(source);

    g_object_unref(output_soundcard);

    if(link != NULL){
      g_object_unref(link);
    }

    if(first_recycling != NULL){
      g_object_unref(first_recycling);
      g_object_unref(last_recycling);
    }
    
    if(end_recycling != NULL){
      g_object_unref(end_recycling);
    }
    
    if(next_recycling != NULL){
      g_object_unref(next_recycling);
    }
  }

  /* unref */
  g_object_unref(copy_pattern_audio);
  g_object_unref(copy_pattern_audio_run);
  g_object_unref(copy_pattern_channel);

  g_object_unref(count_beats_audio_run);
}

/**
 * ags_copy_pattern_channel_run_new:
 * @destination: the destination #AgsChannel
 * @source: the source #AgsChannel
 *
 * Create a new instance of #AgsCopyPatternChannelRun
 *
 * Returns: the new #AgsCopyPatternChannelRun
 *
 * Since: 3.0.0
 */
AgsCopyPatternChannelRun*
ags_copy_pattern_channel_run_new(AgsChannel *destination,
				 AgsChannel *source)
{
  AgsCopyPatternChannelRun *copy_pattern_channel_run;

  copy_pattern_channel_run = (AgsCopyPatternChannelRun *) g_object_new(AGS_TYPE_COPY_PATTERN_CHANNEL_RUN,
								       "destination", destination,
								       "channel", source,
								       NULL);

  return(copy_pattern_channel_run);
}
