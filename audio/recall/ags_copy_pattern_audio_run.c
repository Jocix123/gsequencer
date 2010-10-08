#include "ags_copy_pattern_audio_run.h"

#include "../../object/ags_run_connectable.h"

#include "ags_copy_pattern_audio.h"

GType ags_copy_pattern_audio_run_get_type();
void ags_copy_pattern_audio_run_class_init(AgsCopyPatternAudioRunClass *copy_pattern_audio_run);
void ags_copy_pattern_audio_run_run_connectable_interface_init(AgsRunConnectableInterface *run_connectable);
void ags_copy_pattern_audio_run_init(AgsCopyPatternAudioRun *copy_pattern_audio_run);
void ags_copy_pattern_audio_run_run_connect(AgsRunConnectable *run_connectable);
void ags_copy_pattern_audio_run_run_disconnect(AgsRunConnectable *run_connectable);
void ags_copy_pattern_audio_run_finalize(GObject *gobject);

AgsRecall* ags_copy_pattern_audio_run_duplicate(AgsRecall *recall, AgsRecallID *recall_id);

void ags_copy_pattern_audio_run_tic_callback(AgsDelayAudioRun *delay_audio_run, guint audio_channel,
					     AgsCopyPatternAudioRun *copy_pattern_audio_run);

static gpointer ags_copy_pattern_audio_run_parent_class = NULL;
static AgsRunConnectableInterface *ags_copy_pattern_audio_run_parent_run_connectable_interface;

GType
ags_copy_pattern_audio_run_get_type()
{
  static GType ags_type_copy_pattern_audio_run = 0;

  if(!ags_type_copy_pattern_audio_run){
    static const GTypeInfo ags_copy_pattern_audio_run_info = {
      sizeof (AgsCopyPatternAudioRunClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_copy_pattern_audio_run_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsCopyPatternAudioRun),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_copy_pattern_audio_run_init,
    };

    static const GInterfaceInfo ags_run_connectable_interface_info = {
      (GInterfaceInitFunc) ags_copy_pattern_audio_run_run_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_copy_pattern_audio_run = g_type_register_static(AGS_TYPE_RECALL_AUDIO_RUN,
							     "AgsCopyPatternAudioRun\0",
							     &ags_copy_pattern_audio_run_info,
							     0);

    g_type_add_interface_static(ags_type_copy_pattern_audio_run,
				AGS_TYPE_RUN_CONNECTABLE,
				&ags_run_connectable_interface_info);
  }

  return(ags_type_copy_pattern_audio_run);
}

void
ags_copy_pattern_audio_run_class_init(AgsCopyPatternAudioRunClass *copy_pattern_audio_run)
{
  GObjectClass *gobject;

  ags_copy_pattern_audio_run_parent_class = g_type_class_peek_parent(copy_pattern_audio_run);

  gobject = (GObjectClass *) copy_pattern_audio_run;

  gobject->finalize = ags_copy_pattern_audio_run_finalize;
}

void
ags_copy_pattern_audio_run_run_connectable_interface_init(AgsRunConnectableInterface *run_connectable)
{
  ags_copy_pattern_audio_run_parent_run_connectable_interface = g_type_interface_peek_parent(run_connectable);

  run_connectable->connect = ags_copy_pattern_audio_run_run_connect;
  run_connectable->disconnect = ags_copy_pattern_audio_run_run_disconnect;
}

void
ags_copy_pattern_audio_run_init(AgsCopyPatternAudioRun *copy_pattern_audio_run)
{
  copy_pattern_audio_run->flags = 0;

  copy_pattern_audio_run->recall_ref = 0;

  copy_pattern_audio_run->hide_ref = 0;
  copy_pattern_audio_run->hide_ref_counter = 0;

  copy_pattern_audio_run->delay_audio_run = NULL;
  copy_pattern_audio_run->bit = 0;
}

void
ags_copy_pattern_audio_run_run_connect(AgsRunConnectable *run_connectable)
{
  AgsCopyPatternAudioRun *copy_pattern_audio_run;

  //  ags_copy_pattern_audio_run_parent_run_connectable_interface->connect(run_connectable);

  /* AgsCopyPattern */
  copy_pattern_audio_run = AGS_COPY_PATTERN_AUDIO_RUN(run_connectable);

  copy_pattern_audio_run->flags |= AGS_COPY_PATTERN_AUDIO_RUN_RUN_CONNECTED;

  copy_pattern_audio_run->tic_handler =
    g_signal_connect_after(G_OBJECT(copy_pattern_audio_run->delay_audio_run), "tic\0",
			   G_CALLBACK(ags_copy_pattern_audio_run_tic_callback), copy_pattern_audio_run);
}

void
ags_copy_pattern_audio_run_run_disconnect(AgsRunConnectable *run_connectable)
{
  AgsCopyPatternAudioRun *copy_pattern_audio_run;

  ags_copy_pattern_audio_run_parent_run_connectable_interface->connect(run_connectable);

  /* AgsCopyPattern */
  copy_pattern_audio_run = AGS_COPY_PATTERN_AUDIO_RUN(run_connectable);

  copy_pattern_audio_run->flags &= (~AGS_COPY_PATTERN_AUDIO_RUN_RUN_CONNECTED);
  g_signal_handler_disconnect(G_OBJECT(copy_pattern_audio_run), copy_pattern_audio_run->tic_handler);
}

void
ags_copy_pattern_audio_run_finalize(GObject *gobject)
{
  G_OBJECT_CLASS(ags_copy_pattern_audio_run_parent_class)->finalize(gobject);
}

AgsRecall*
ags_copy_pattern_audio_run_duplicate(AgsRecall *recall, AgsRecallID *recall_id)
{
  AgsCopyPatternAudio *copy_pattern_audio;
  AgsCopyPatternAudioRun *copy_pattern_audio_run;
  GList *list;
  
  copy_pattern_audio = AGS_COPY_PATTERN_AUDIO(recall->recall_audio);
  copy_pattern_audio_run = AGS_COPY_PATTERN_AUDIO_RUN(recall);

  //  list = AGS_RECALL_(copy_pattern_audio_run;

  //  copy_pattern_audio_run->delay_audio_run = ;
}

void
ags_copy_pattern_audio_run_tic_callback(AgsDelayAudioRun *delay_audio_run, guint audio_channel,
					AgsCopyPatternAudioRun *copy_pattern_audio_run)
{
  AgsCopyPatternAudio *copy_pattern_audio;

  copy_pattern_audio = AGS_COPY_PATTERN_AUDIO(AGS_RECALL(copy_pattern_audio_run)->recall_audio);

  if(copy_pattern_audio_run->bit == copy_pattern_audio->length - 1){
    if(copy_pattern_audio->loop ||
       copy_pattern_audio_run->recall_ref == 0)
      copy_pattern_audio_run->bit = 0;
  }else{
    if(copy_pattern_audio_run->hide_ref != 0)
      copy_pattern_audio_run->hide_ref_counter++;
    
    if(copy_pattern_audio_run->hide_ref_counter != copy_pattern_audio_run->hide_ref)
      return;
    
    copy_pattern_audio_run->hide_ref_counter = 0;
    
    copy_pattern_audio_run->bit++;
  }
}

AgsCopyPatternAudioRun*
ags_copy_pattern_audio_run_new(AgsRecallAudio *recall_audio,
			       AgsDelayAudioRun *delay_audio_run, guint bit)
{
  AgsCopyPatternAudioRun *copy_pattern_audio_run;

  copy_pattern_audio_run = (AgsCopyPatternAudioRun *) g_object_new(AGS_TYPE_COPY_PATTERN_AUDIO_RUN,
								   "recall_audio\0", recall_audio);

  copy_pattern_audio_run->delay_audio_run = delay_audio_run;
  copy_pattern_audio_run->bit = bit;

  return(copy_pattern_audio_run);
}
