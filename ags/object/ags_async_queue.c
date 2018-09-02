/* GSequencer - Advanced GTK Sequencer
 * Copyright (C) 2005-2015 Joël Krähemann
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

#include <ags/object/ags_async_queue.h>

#include <stdio.h>

void ags_async_queue_base_init(AgsAsyncQueueInterface *interface);

/**
 * SECTION:ags_async_queue
 * @short_description: asynchronous run
 * @title: AgsAsyncQueue
 * @section_id: AgsAsyncQueue
 * @include: ags/object/ags_async_queue.h
 *
 * The #AgsAsyncQueue interface determines a thread-safe/collision-free slot. The implementation shall
 * run tasks in a safe context.
 */

GType
ags_async_queue_get_type()
{
  static GType ags_type_async_queue = 0;

  if(!ags_type_async_queue){
    static const GTypeInfo ags_async_queue_info = {
      sizeof(AgsAsyncQueueInterface),
      (GBaseInitFunc) ags_async_queue_base_init,
      NULL, /* base_finalize */
    };
    
    ags_type_async_queue = g_type_register_static(G_TYPE_INTERFACE,
						  "AgsAsyncQueue", &ags_async_queue_info,
						  0);
  }

  return(ags_type_async_queue);
}

void
ags_async_queue_base_init(AgsAsyncQueueInterface *interface)
{
  /* empty */
}

/**
 * ags_async_queue_increment_wait_ref:
 * @async_queue: the #AgsAsyncQueue
 * 
 * Increments wait ref.
 * 
 * Since: 2.0.0
 */
void
ags_async_queue_increment_wait_ref(AgsAsyncQueue *async_queue)
{
  AgsAsyncQueueInterface *async_queue_interface;

  g_return_if_fail(AGS_IS_ASYNC_QUEUE(async_queue));
  async_queue_interface = AGS_ASYNC_QUEUE_GET_INTERFACE(async_queue);
  g_return_if_fail(async_queue_interface->increment_wait_ref);
  async_queue_interface->increment_wait_ref(async_queue);
}

/**
 * ags_async_queue_get_wait_ref:
 * @async_queue: the #AgsAsyncQueue
 *
 * Get wait ref.
 *
 * Returns: The number of threads waiting.
 *
 * Since: 2.0.0
 */
guint
ags_async_queue_get_wait_ref(AgsAsyncQueue *async_queue)
{
  AgsAsyncQueueInterface *async_queue_interface;

  g_return_val_if_fail(AGS_IS_ASYNC_QUEUE(async_queue), FALSE);
  async_queue_interface = AGS_ASYNC_QUEUE_GET_INTERFACE(async_queue);
  g_return_val_if_fail(async_queue_interface->get_wait_ref, FALSE);

  return(async_queue_interface->get_wait_ref(async_queue));
}

/**
 * ags_async_queue_set_run_mutex:
 * @async_queue: the #AgsAsyncQueue
 * @run_mutex: a pthread_mutex_t
 *
 * Sets the mutex to access the condition variable.
 *
 * Since: 2.0.0
 */
void
ags_async_queue_set_run_mutex(AgsAsyncQueue *async_queue, pthread_mutex_t *run_mutex)
{
  AgsAsyncQueueInterface *async_queue_interface;

  g_return_if_fail(AGS_IS_ASYNC_QUEUE(async_queue));
  async_queue_interface = AGS_ASYNC_QUEUE_GET_INTERFACE(async_queue);
  g_return_if_fail(async_queue_interface->set_run_mutex);
  async_queue_interface->set_run_mutex(async_queue, run_mutex);
}

/**
 * ags_async_queue_get_run_mutex:
 * @async_queue: the #AgsAsyncQueue
 *
 * Get the mutex to access the condition variable.
 *
 * Returns: a pthread_mutex_t
 *
 * Since: 2.0.0
 */
pthread_mutex_t*
ags_async_queue_get_run_mutex(AgsAsyncQueue *async_queue)
{
  AgsAsyncQueueInterface *async_queue_interface;

  g_return_val_if_fail(AGS_IS_ASYNC_QUEUE(async_queue), NULL);
  async_queue_interface = AGS_ASYNC_QUEUE_GET_INTERFACE(async_queue);
  g_return_val_if_fail(async_queue_interface->get_run_mutex, NULL);
  
  return(async_queue_interface->get_run_mutex(async_queue));
}

/**
 * ags_async_queue_set_run_cond:
 * @async_queue: the #AgsAsyncQueue
 * @run_cond: a pthread_cond_t
 *
 * Sets the condition to determine if @async_queue was run.
 * 
 * Since: 2.0.0
 */
void
ags_async_queue_set_run_cond(AgsAsyncQueue *async_queue, pthread_cond_t *run_cond)
{
  AgsAsyncQueueInterface *async_queue_interface;

  g_return_if_fail(AGS_IS_ASYNC_QUEUE(async_queue));
  async_queue_interface = AGS_ASYNC_QUEUE_GET_INTERFACE(async_queue);
  g_return_if_fail(async_queue_interface->set_run_cond);
  async_queue_interface->set_run_cond(async_queue, run_cond);
}

/**
 * ags_async_queue_get_run_cond:
 * @async_queue: the #AgsAsyncQueue
 *
 * Get the condition to wait for @async_queue. 
 *
 * Returns: the pthread_cond_t
 *
 * Since: 2.0.0
 */
pthread_cond_t*
ags_async_queue_get_run_cond(AgsAsyncQueue *async_queue)
{
  AgsAsyncQueueInterface *async_queue_interface;

  g_return_val_if_fail(AGS_IS_ASYNC_QUEUE(async_queue), NULL);
  async_queue_interface = AGS_ASYNC_QUEUE_GET_INTERFACE(async_queue);
  g_return_val_if_fail(async_queue_interface->get_run_cond, NULL);

  return(async_queue_interface->get_run_cond(async_queue));
}

/**
 * ags_async_queue_set_run:
 * @async_queue: the #AgsAsyncQueue
 * @is_run: if %TRUE the queue was already running
 * 
 * Determine if you are allowed to pass the condition.
 *
 * Since: 2.0.0
 */
void
ags_async_queue_set_run(AgsAsyncQueue *async_queue, gboolean is_run)
{
  AgsAsyncQueueInterface *async_queue_interface;

  g_return_if_fail(AGS_IS_ASYNC_QUEUE(async_queue));
  async_queue_interface = AGS_ASYNC_QUEUE_GET_INTERFACE(async_queue);
  g_return_if_fail(async_queue_interface->set_run);
  async_queue_interface->set_run(async_queue, is_run);
}

/**
 * ags_async_queue_is_run:
 * @async_queue: the #AgsAsyncQueue
 * 
 * Decide if you are allowed to pass the condition.
 *
 * Returns: if %TRUE it is safe to continue, else you should wait for the condition.
 *
 * Since: 2.0.0
 */
gboolean
ags_async_queue_is_run(AgsAsyncQueue *async_queue)
{
  AgsAsyncQueueInterface *async_queue_interface;

  g_return_val_if_fail(AGS_IS_ASYNC_QUEUE(async_queue), FALSE);
  async_queue_interface = AGS_ASYNC_QUEUE_GET_INTERFACE(async_queue);
  g_return_val_if_fail(async_queue_interface->is_run, FALSE);

  return(async_queue_interface->is_run(async_queue));
}

/**
 * ags_async_queue_schedule_task:
 * @async_queue: the #AgsAsyncQueue
 * @task: the task object
 * 
 * Schedule @task to be launched in a safe context.
 *
 * Since: 2.0.0
 */
void
ags_async_queue_schedule_task(AgsAsyncQueue *async_queue, GObject *task)
{
  AgsAsyncQueueInterface *async_queue_interface;

  g_return_if_fail(AGS_IS_ASYNC_QUEUE(async_queue));
  async_queue_interface = AGS_ASYNC_QUEUE_GET_INTERFACE(async_queue);
  g_return_if_fail(async_queue_interface->schedule_task);
  async_queue_interface->schedule_task(async_queue, task);
}

/**
 * ags_async_queue_schedule_task_list:
 * @async_queue: the #AgsAsyncQueue
 * @task_list: a list of task objects
 * 
 * Schedule @task_list to be launched in a safe context.
 *
 * Since: 2.0.0
 */
void
ags_async_queue_schedule_task_list(AgsAsyncQueue *async_queue, GList *task_list)
{
  AgsAsyncQueueInterface *async_queue_interface;

  g_return_if_fail(AGS_IS_ASYNC_QUEUE(async_queue));
  async_queue_interface = AGS_ASYNC_QUEUE_GET_INTERFACE(async_queue);
  g_return_if_fail(async_queue_interface->schedule_task_list);
  async_queue_interface->schedule_task_list(async_queue, task_list);
}
