/* GSequencer - Advanced GTK Sequencer
 * Copyright (C) 2005-2019 Joël Krähemann
 *
 * This file is part of GSequencer.
 *
 * GSequencer is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * GSequencer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with GSequencer.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __AGS_REMOTE_TASK_THREAD_H__
#define __AGS_REMOTE_TASK_THREAD_H__

#include <glib.h>
#include <glib-object.h>

#include <ags/thread/ags_task_thread.h>

G_BEGIN_DECLS

#define AGS_TYPE_REMOTE_TASK_THREAD                (ags_remote_task_thread_get_type())
#define AGS_REMOTE_TASK_THREAD(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_REMOTE_TASK_THREAD, AgsRemoteTaskThread))
#define AGS_REMOTE_TASK_THREAD_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST(class, AGS_TYPE_REMOTE_TASK_THREAD, AgsRemoteTaskThread))
#define AGS_IS_REMOTE_TASK_THREAD(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AGS_TYPE_REMOTE_TASK_THREAD))
#define AGS_IS_REMOTE_TASK_THREAD_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE ((class), AGS_TYPE_REMOTE_TASK_THREAD))
#define AGS_REMOTE_TASK_THREAD_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS(obj, AGS_TYPE_REMOTE_TASK_THREAD, AgsRemoteTaskThreadClass))

#define AGS_REMOTE_TASK_GET_OBJ_MUTEX(obj) (&(((AgsRemoteTask *) obj)->obj_mutex))

#define AGS_REMOTE_TASK_THREAD_RT_PRIORITY (95)

#define AGS_REMOTE_TASK_THREAD_DEFAULT_JIFFIE (AGS_THREAD_MAX_PRECISION)

typedef struct _AgsRemoteTaskThread AgsRemoteTaskThread;
typedef struct _AgsRemoteTaskThreadClass AgsRemoteTaskThreadClass;

struct _AgsRemoteTaskThread
{
  AgsTaskThread task_thread;
};

struct _AgsRemoteTaskThreadClass
{
  AgsTaskThreadClass task_thread;
};

GType ags_remote_task_thread_get_type();

AgsRemoteTaskThread* ags_remote_task_thread_new();

G_END_DECLS

#endif /*__AGS_REMOTE_TASK_THREAD_H__*/
