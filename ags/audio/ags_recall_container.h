/* GSequencer - Advanced GTK Sequencer
 * Copyright (C) 2005-2017 Joël Krähemann
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

#ifndef __AGS_RECALL_CONTAINER_H__
#define __AGS_RECALL_CONTAINER_H__

#include <glib.h>
#include <glib-object.h>

#include <ags/libags.h>

#include <ags/audio/ags_recall.h>

#define AGS_TYPE_RECALL_CONTAINER                (ags_recall_container_get_type())
#define AGS_RECALL_CONTAINER(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_RECALL_CONTAINER, AgsRecallContainer))
#define AGS_RECALL_CONTAINER_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST((class), AGS_TYPE_RECALL_CONTAINER, AgsRecallContainerClass))
#define AGS_IS_RECALL_CONTAINER(obj)             (G_TYPE_CHECK_INSTANCE_TYPE((obj), AGS_TYPE_RECALL_CONTAINER))
#define AGS_IS_RECALL_CONTAINER_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE((class), AGS_TYPE_RECALL_CONTAINER))
#define AGS_RECALL_CONTAINER_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS((obj), AGS_TYPE_RECALL_CONTAINER, AgsRecallContainerClass))

typedef struct _AgsRecallContainer AgsRecallContainer;
typedef struct _AgsRecallContainerClass AgsRecallContainerClass;

/**
 * AgsRecallContainerFlags:
 * @AGS_RECALL_CONTAINER_PLAY: bound to play context
 * 
 * Enum values to control the behavior or indicate internal state of #AgsRecallContainer by
 * enable/disable as flags.
 */
typedef enum{
  AGS_RECALL_CONTAINER_PLAY    =  1,
}AgsRecallContainerFlags;

/**
 * AgsRecallContainerFindFlags:
 * @AGS_RECALL_CONTAINER_FIND_TYPE: find type
 * @AGS_RECALL_CONTAINER_FIND_TEMPLATE: find template
 * @AGS_RECALL_CONTAINER_FIND_RECALL_ID: find recall id
 * 
 * Enum values to specify find criteria.
 */
typedef enum{
  AGS_RECALL_CONTAINER_FIND_TYPE          = 1,
  AGS_RECALL_CONTAINER_FIND_TEMPLATE      = 1 <<  1,
  AGS_RECALL_CONTAINER_FIND_RECALL_ID     = 1 <<  2,
}AgsRecallContainerFindFlags;

struct _AgsRecallContainer
{
  GObject object;
  
  guint flags;

  pthread_mutex_t *obj_mutex;
  pthread_mutexattr_t *obj_mutexattr;

  GType recall_audio_type;
  AgsRecall *recall_audio;

  GType recall_audio_run_type;
  GList *recall_audio_run;

  GType recall_channel_type;
  GList *recall_channel;

  GType recall_channel_run_type;
  GList *recall_channel_run;
};

struct _AgsRecallContainerClass
{
  GObjectClass object;
};

GType ags_recall_container_get_type();

pthread_mutex_t* ags_recall_container_get_class_mutex();

AgsRecall* ags_recall_container_get_recall_audio(AgsRecallContainer *container);
GList* ags_recall_container_get_recall_audio_run(AgsRecallContainer *container);
GList* ags_recall_container_get_recall_channel(AgsRecallContainer *container);
GList* ags_recall_container_get_recall_channel_run(AgsRecallContainer *container);

GList* ags_recall_container_find(GList *recall_container,
				 GType type,
				 guint find_flags,
				 AgsRecallID *recall_id);

AgsRecallContainer* ags_recall_container_new();

#endif /*__AGS_RECALL_CONTAINER_H__*/
