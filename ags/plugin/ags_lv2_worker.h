/* GSequencer - Advanced GTK Sequencer
 * Copyright (C) 2005-2018 Joël Krähemann
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

#ifndef __AGS_LV2_WORKER_H__
#define __AGS_LV2_WORKER_H__

#include <glib.h>
#include <glib-object.h>

#include <ags/libags.h>

#include <lv2.h>
#include <lv2/lv2plug.in/ns/ext/worker/worker.h>

#define AGS_TYPE_LV2_WORKER                (ags_lv2_worker_get_type())
#define AGS_LV2_WORKER(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_LV2_WORKER, AgsLv2Worker))
#define AGS_LV2_WORKER_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST((class), AGS_TYPE_LV2_WORKER, AgsLv2WorkerClass))
#define AGS_IS_LV2_WORKER(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AGS_TYPE_LV2_WORKER))
#define AGS_IS_LV2_WORKER_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE ((class), AGS_TYPE_LV2_WORKER))
#define AGS_LV2_WORKER_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), AGS_TYPE_LV2_WORKER, AgsLv2WorkerClass))

#define AGS_LV2_WORKER_GET_OBJ_MUTEX(obj) (((AgsLv2Worker *) obj)->obj_mutex)

#define AGS_LV2_WORKER_RESPONSE_DATA(ptr) ((AgsLv2WorkerResponseData *)(ptr))

typedef struct _AgsLv2Worker AgsLv2Worker;
typedef struct _AgsLv2WorkerClass AgsLv2WorkerClass;
typedef struct _AgsLv2WorkerResponseData AgsLv2WorkerResponseData;

/**
 * AgsLv2WorkerFlags:
 * @AGS_LV2_WORKER_RUN: the worker is running
 * 
 * Enum values to control the behavior or indicate internal state of #AgsLv2Worker by
 * enable/disable as flags.
 */
typedef enum{
  AGS_LV2_WORKER_RUN    = 1,
}AgsLv2WorkerFlags;

struct _AgsLv2Worker
{
  GObject gobject;

  guint flags;

  pthread_mutex_t *obj_mutex;
  pthread_mutexattr_t *obj_mutexattr;

  LV2_Handle handle;
  LV2_Worker_Interface *worker_interface;

  guint work_size;
  void *work_data;
  
  GList *response_data;
  
  AgsThread *returnable_thread;  
  AgsThread *worker_thread;
};

struct _AgsLv2WorkerClass
{
  GObjectClass gobject;
};

struct _AgsLv2WorkerResponseData
{
  uint32_t data_size;
  void *data;
};

GType ags_lv2_worker_get_type(void);

pthread_mutex_t* ags_lv2_worker_get_class_mutex();

gboolean ags_lv2_worker_test_flags(AgsLv2Worker *lv2_worker, guint flags);
void ags_lv2_worker_set_flags(AgsLv2Worker *lv2_worker, guint flags);
void ags_lv2_worker_unset_flags(AgsLv2Worker *lv2_worker, guint flags);

AgsLv2WorkerResponseData* ags_lv2_worker_alloc_response_data();
void ags_lv2_worker_free_response_data(AgsLv2WorkerResponseData *response_data);

LV2_Worker_Status ags_lv2_worker_respond(LV2_Worker_Respond_Handle handle,
					 uint32_t size,
					 const void* data);

LV2_Worker_Status ags_lv2_worker_schedule_work(LV2_Worker_Schedule_Handle handle,
					       uint32_t size,
					       const void* data);

void ags_lv2_worker_do_poll(AgsWorkerThread *worker_thread, gpointer data);

void ags_lv2_worker_safe_run(AgsReturnableThread *returnable_thread, gpointer data);
void ags_lv2_worker_interrupted_callback(AgsThread *thread,
					 int sig,
					 guint time_cycle, guint *time_spent,
					 AgsLv2Worker *lv2_worker);

AgsLv2Worker* ags_lv2_worker_new(AgsThread *returnable_thread);

#endif /*__AGS_LV2_WORKER_H__*/
