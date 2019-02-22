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

#ifndef __AGS_LOOP_CHANNEL_RUN_H__
#define __AGS_LOOP_CHANNEL_RUN_H__

#include <glib.h>
#include <glib-object.h>

#include <ags/libags.h>

#include <ags/audio/ags_channel.h>
#include <ags/audio/ags_recall_channel_run.h>

#include <ags/audio/recall/ags_count_beats_audio_run.h>

#define AGS_TYPE_LOOP_CHANNEL_RUN                (ags_loop_channel_run_get_type())
#define AGS_LOOP_CHANNEL_RUN(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_LOOP_CHANNEL_RUN, AgsLoopChannelRun))
#define AGS_LOOP_CHANNEL_RUN_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST((class), AGS_TYPE_LOOP_CHANNEL_RUN, AgsLoopChannelRunClass))
#define AGS_IS_LOOP_CHANNEL_RUN(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AGS_TYPE_LOOP_CHANNEL_RUN))
#define AGS_IS_LOOP_CHANNEL_RUN_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE ((class), AGS_TYPE_LOOP_CHANNEL_RUN))
#define AGS_LOOP_CHANNEL_RUN_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), AGS_TYPE_LOOP_CHANNEL_RUN, AgsLoopChannelRunClass))

typedef struct _AgsLoopChannelRun AgsLoopChannelRun;
typedef struct _AgsLoopChannelRunClass AgsLoopChannelRunClass;

struct _AgsLoopChannelRun
{
  AgsRecallChannelRun recall_channel_run;

  AgsCountBeatsAudioRun *count_beats_audio_run;
};

struct _AgsLoopChannelRunClass
{
  AgsRecallChannelRunClass recall_channel_run;
};

GType ags_loop_channel_run_get_type();

AgsLoopChannelRun* ags_loop_channel_run_new(AgsChannel *source,
					    AgsCountBeatsAudioRun *count_beats_audio_run);

#endif /*__AGS_LOOP_CHANNEL_RUN_H__*/

