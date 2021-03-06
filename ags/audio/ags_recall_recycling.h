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

#ifndef __AGS_RECALL_RECYCLING_H__
#define __AGS_RECALL_RECYCLING_H__

#include <glib.h>
#include <glib-object.h>

#include <ags/libags.h>

#include <ags/audio/ags_recycling.h>
#include <ags/audio/ags_audio_signal.h>
#include <ags/audio/ags_recall.h>

G_BEGIN_DECLS

#define AGS_TYPE_RECALL_RECYCLING                (ags_recall_recycling_get_type())
#define AGS_RECALL_RECYCLING(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_RECALL_RECYCLING, AgsRecallRecycling))
#define AGS_RECALL_RECYCLING_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST((class), AGS_TYPE_RECALL_RECYCLING, AgsRecallRecyclingClass))
#define AGS_IS_RECALL_RECYCLING(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AGS_TYPE_RECALL_RECYCLING))
#define AGS_IS_RECALL_RECYCLING_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE ((class), AGS_TYPE_RECALL_RECYCLING))
#define AGS_RECALL_RECYCLING_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), AGS_TYPE_RECALL_RECYCLING, AgsRecallRecyclingClass))

typedef struct _AgsRecallRecycling AgsRecallRecycling;
typedef struct _AgsRecallRecyclingClass AgsRecallRecyclingClass;

/**
 * AgsRecallRecyclingFlags:
 * @AGS_RECALL_RECYCLING_GARBAGE_COLLECTOR: not used
 * @AGS_RECALL_RECYCLING_MAP_CHILD_DESTINATION: not used
 * @AGS_RECALL_RECYCLING_MAP_CHILD_SOURCE: not used
 * @AGS_RECALL_RECYCLING_CREATE_DESTINATION_ON_MAP_SOURCE: not used
 * 
 * Enum values to control the behavior or indicate internal state of #AgsRecallRecycling by
 * enable/disable as flags.
 */
typedef enum{
  AGS_RECALL_RECYCLING_GARBAGE_COLLECTOR                = 1,
  AGS_RECALL_RECYCLING_MAP_CHILD_DESTINATION            = 1 <<  1,
  AGS_RECALL_RECYCLING_MAP_CHILD_SOURCE                 = 1 <<  2,
  AGS_RECALL_RECYCLING_CREATE_DESTINATION_ON_MAP_SOURCE = 1 <<  3,
}AgsRecallRecyclingFlags;

struct _AgsRecallRecycling
{
  AgsRecall recall;

  guint flags;

  guint audio_channel;

  AgsRecycling *destination;
  AgsRecycling *source;

  AgsAudioSignal *child_destination;
  GList *child_source;
};

struct _AgsRecallRecyclingClass
{
  AgsRecallClass recall;
};

GType ags_recall_recycling_get_type();

AgsRecallRecycling* ags_recall_recycling_new();

G_END_DECLS

#endif /*__AGS_RECALL_RECYCLING_H__*/
