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

#include <glib.h>
#include <glib-object.h>

#include <CUnit/CUnit.h>
#include <CUnit/Automated.h>
#include <CUnit/Basic.h>

#include <ags/libags.h>

int ags_thread_test_init_suite();
int ags_thread_test_clean_suite();

void ags_thread_test_sync();
void ags_thread_test_lock();
void ags_thread_test_trylock();
void ags_thread_test_get_toplevel();
void ags_thread_test_first();
void ags_thread_test_last();
void ags_thread_test_remove_child();
void ags_thread_test_add_child();
void ags_thread_test_parental_is_locked();
void ags_thread_test_sibling_is_locked();
void ags_thread_test_children_is_locked();
void ags_thread_test_is_current_ready();
void ags_thread_test_is_tree_ready();
void ags_thread_test_next_parent_locked();
void ags_thread_test_next_sibling_locked();
void ags_thread_test_next_children_locked();
void ags_thread_test_lock_parent();
void ags_thread_test_lock_sibling();
void ags_thread_test_lock_children();
void ags_thread_test_lock_all();
void ags_thread_test_wait_parent();
void ags_thread_test_wait_sibling();
void ags_thread_test_wait_children();
void ags_thread_test_suspend();
void ags_thread_test_timelock();
void ags_thread_test_stop();

#define AGS_THREAD_TEST_SYNC_N_THREADS (16)

#define AGS_THREAD_TEST_LOCK_N_THREADS (4)

#define AGS_THREAD_TEST_GET_TOPLEVEL_N_LEVELS (7)

#define AGS_THREAD_TEST_FIRST_N_THREADS (16)

#define AGS_THREAD_TEST_LAST_N_THREADS (16)

#define AGS_THREAD_TEST_REMOVE_CHILD_N_THREADS (16)

#define AGS_THREAD_TEST_ADD_CHILD_N_THREADS (16)

#define AGS_THREAD_TEST_LOCK_PARENT_PARENT_COUNT (8)

#define AGS_THREAD_TEST_LOCK_SIBLING_SIBLING_COUNT (64)

#define AGS_THREAD_TEST_LOCK_CHILDREN_CHILDREN_COUNT (8)

AgsApplicationContext *application_context;

AgsThread *main_loop;

/* The suite initialization function.
 * Opens the temporary file used by the tests.
 * Returns zero on success, non-zero otherwise.
 */
int
ags_thread_test_init_suite()
{
  application_context = ags_application_context_new(NULL,
						    NULL);
  
  main_loop = ags_generic_main_loop_new(application_context);
  g_object_ref(main_loop);
  
  ags_main_loop_set_async_queue(AGS_MAIN_LOOP(main_loop),
				ags_task_thread_new());
  ags_thread_start(main_loop);
  
  return(0);
}

/* The suite cleanup function.
 * Closes the temporary file used by the tests.
 * Returns zero on success, non-zero otherwise.
 */
int
ags_thread_test_clean_suite()
{
  ags_thread_stop(main_loop);
  g_object_unref(main_loop);
  
  g_object_unref(application_context);
  
  return(0);
}

void
ags_thread_test_sync()
{
  AgsThread *main_loop;
  AgsThread **thread;

  volatile guint n_waiting;
  guint i;

  auto void* ags_thread_test_sync_waiter_thread(void *ptr);

  void* ags_thread_test_sync_waiter_thread(void *ptr){
    AgsThread *thread;

    thread = (AgsThread *) ptr;

    g_rec_mutex_lock(&(thread->mutex));

    g_atomic_int_inc(&n_waiting);
    
    while(((AGS_THREAD_WAIT_0 & (g_atomic_int_get(&(thread->flags)))) != 0) ||
	  ((AGS_THREAD_WAIT_1 & (g_atomic_int_get(&(thread->flags)))) != 0) ||
	  ((AGS_THREAD_WAIT_2 & (g_atomic_int_get(&(thread->flags)))) != 0)){
      g_cond_wait(&(thread->cond),
		  &(thread->mutex));
    }
    
    g_rec_mutex_unlock(&(thread->mutex));

    g_thread_exit(NULL);

    return(NULL);
  }
  
  main_loop = ags_generic_main_loop_new(application_context);
  ags_main_loop_set_async_queue(AGS_MAIN_LOOP(main_loop),
				ags_task_thread_new());
  g_atomic_int_set(&n_waiting,
		   0);
  
  thread = (AgsThread **) malloc(AGS_THREAD_TEST_SYNC_N_THREADS * sizeof(AgsThread*));
  
  for(i = 0; i < AGS_THREAD_TEST_SYNC_N_THREADS; i++){
    thread[i] = ags_thread_new(NULL);
    ags_thread_add_child_extended(main_loop,
				  thread[i],
				  TRUE, TRUE);
  }
  
  /* check AGS_THREAD_WAIT_0 - setup */
  ags_thread_set_status_flags(main_loop, AGS_THREAD_STATUS_WAIT_0);
  main_loop->thread = g_thread_new("libags.so - unit test",
				   ags_thread_test_sync_waiter_thread,
				   main_loop);

  for(i = 0; i < AGS_THREAD_TEST_SYNC_N_THREADS; i++){
    ags_thread_set_status_flags(thread[i], AGS_THREAD_STATUS_WAIT_0);

    /* since signal expects a thread waiting we do one */
    thread[i]->thread = g_thread_new("libags.so - unit test",
				     ags_thread_test_sync_waiter_thread,
				     thread[i]);
  }

  /* wait until all waiting */
  while(g_atomic_int_get(&n_waiting) < AGS_THREAD_TEST_SYNC_N_THREADS + 1){
    usleep(4);
  }

  /* call sync all */
  ags_thread_set_sync_all(main_loop,
			  0);

  /* assert flag not set anymore */
  CU_ASSERT(!ags_thread_test_status_flags(main_loop, AGS_THREAD_STATUS_WAIT_0));

  for(i = 0; i < AGS_THREAD_TEST_SYNC_N_THREADS; i++){
    CU_ASSERT(!ags_thread_test_status_flags(thread[i], AGS_THREAD_STATUS_WAIT_0));
  }
  
  /* check AGS_THREAD_WAIT_1 - setup */
  ags_thread_set_status_flags(main_loop, AGS_THREAD_STATUS_WAIT_1);
  main_loop->thread = g_thread_new("libags.so - unit test",
				   ags_thread_test_sync_waiter_thread,
				   main_loop);

  for(i = 0; i < AGS_THREAD_TEST_SYNC_N_THREADS; i++){
    ags_thread_set_status_flags(thread[i], AGS_THREAD_STATUS_WAIT_1);

    /* since signal expects a thread waiting we do one */
    thread[i]->thread = g_thread_new("libags.so - unit test",
				     ags_thread_test_sync_waiter_thread,
				     thread[i]);
  }

  /* wait until all waiting */
  while(g_atomic_int_get(&n_waiting) < AGS_THREAD_TEST_SYNC_N_THREADS + 1){
    usleep(4);
  }

  /* call sync all */
  ags_thread_set_sync_all(main_loop,
			  1);

  /* assert flag not set anymore */
  CU_ASSERT(!ags_thread_test_status_flags(main_loop, AGS_THREAD_STATUS_WAIT_1));

  for(i = 0; i < AGS_THREAD_TEST_SYNC_N_THREADS; i++){
    CU_ASSERT(!ags_thread_test_status_flags(thread[i], AGS_THREAD_STATUS_WAIT_1));
  }

  /* check AGS_THREAD_WAIT_2 - setup */
  ags_thread_set_status_flags(main_loop, AGS_THREAD_STATUS_WAIT_2);
  main_loop->thread = g_thread_new("libags.so - unit test",
				   ags_thread_test_sync_waiter_thread,
				   main_loop);

  for(i = 0; i < AGS_THREAD_TEST_SYNC_N_THREADS; i++){
    ags_thread_set_status_flags(thread[i], AGS_THREAD_STATUS_WAIT_2);

    /* since signal expects a thread waiting we do one */
    thread[i]->thread = g_thread_new("libags.so - unit test",
				     ags_thread_test_sync_waiter_thread,
				     thread[i]);
  }

  /* wait until all waiting */
  while(g_atomic_int_get(&n_waiting) < AGS_THREAD_TEST_SYNC_N_THREADS + 1){
    usleep(4);
  }

  /* call sync all */
  ags_thread_set_sync_all(main_loop,
			  2);

  /* assert flag not set anymore */
  CU_ASSERT(!ags_thread_test_status_flags(main_loop, AGS_THREAD_STATUS_WAIT_2));

  for(i = 0; i < AGS_THREAD_TEST_SYNC_N_THREADS; i++){
    CU_ASSERT(!ags_thread_test_status_flags(thread[i], AGS_THREAD_STATUS_WAIT_2));
  }
}

void
ags_thread_test_lock()
{
  AgsThread **thread;

  guint i;
  
  GThread *assert_thread;
  
  auto void* ags_thread_test_lock_assert_locked(void *ptr);

  void* ags_thread_test_lock_assert_locked(void *ptr){
    AgsThread **thread;

    guint i;

    thread = (AgsThread **) ptr;

    for(i = 0; i < AGS_THREAD_TEST_LOCK_N_THREADS; i++){
      CU_ASSERT(ags_thread_trylock(thread[i]) == FALSE);
    }

    g_thread_exit(NULL);

    return(NULL);
  }

  thread = (AgsThread **) malloc(AGS_THREAD_TEST_LOCK_N_THREADS * sizeof(AgsThread*));
  
  for(i = 0; i < AGS_THREAD_TEST_LOCK_N_THREADS; i++){
    thread[i] = ags_thread_new(NULL);
    ags_thread_add_child_extended(main_loop,
				  thread[i],
				  TRUE, TRUE);
  }

  /* lock the threads */
  for(i = 0; i < AGS_THREAD_TEST_LOCK_N_THREADS; i++){
    ags_thread_lock(thread[i]);
  }

  /* try to lock from another thread */
  assert_thread = g_thread_new("libags.so - unit test",
			       ags_thread_test_lock_assert_locked,
			       main_loop);

  g_thread_join(assert_thread);
  
  /* unlock the threads */
  for(i = 0; i < AGS_THREAD_TEST_LOCK_N_THREADS; i++){
    ags_thread_unlock(thread[i]);
  }
}

void
ags_thread_test_trylock()
{
  //TODO:JK: implement me
}

void
ags_thread_test_get_toplevel()
{
  AgsThread *main_loop;
  AgsThread *thread, *current;

  guint i;

  main_loop = ags_generic_main_loop_new(application_context);
  thread = main_loop;
  
  for(i = 0; i < AGS_THREAD_TEST_GET_TOPLEVEL_N_LEVELS; i++){
    current = ags_thread_new(NULL);
    ags_thread_add_child_extended(thread,
				  current,
				  TRUE, TRUE);

    thread = current;
  }

  CU_ASSERT(ags_thread_get_toplevel(thread) == main_loop);
}

void
ags_thread_test_first()
{
  AgsThread *parent;
  AgsThread **thread;
  AgsThread *first_thread;

  guint i;

  parent = ags_thread_new(NULL);
  ags_thread_add_child_extended(main_loop,
				parent,
				TRUE, TRUE);

  thread = (AgsThread **) malloc(AGS_THREAD_TEST_FIRST_N_THREADS * sizeof(AgsThread*));

  for(i = 0; i < AGS_THREAD_TEST_FIRST_N_THREADS; i++){
    thread[i] = ags_thread_new(NULL);
    ags_thread_add_child_extended(parent,
				  thread[i],
				  TRUE, TRUE);
  }

  first_thread = thread[0];
  
  for(i = 0; i < AGS_THREAD_TEST_FIRST_N_THREADS; i++){
    CU_ASSERT(ags_thread_first(thread[i]) == first_thread);
  }
}

void
ags_thread_test_last()
{
  AgsThread *parent;
  AgsThread **thread;
  AgsThread *last_thread;

  guint i;

  parent = ags_thread_new(NULL);
  ags_thread_add_child_extended(main_loop,
				parent,
				TRUE, TRUE);

  thread = (AgsThread **) malloc(AGS_THREAD_TEST_LAST_N_THREADS * sizeof(AgsThread*));

  for(i = 0; i < AGS_THREAD_TEST_LAST_N_THREADS; i++){
    thread[i] = ags_thread_new(NULL);
    ags_thread_add_child_extended(parent,
				  thread[i],
				  TRUE, TRUE);
  }

  last_thread = thread[AGS_THREAD_TEST_LAST_N_THREADS - 1];

  for(i = 0; i < AGS_THREAD_TEST_LAST_N_THREADS; i++){
    CU_ASSERT(ags_thread_last(thread[i]) == last_thread);
  }
}

void
ags_thread_test_remove_child()
{
  AgsThread *parent;
  AgsThread **thread;
  AgsThread *current;

  guint i;

  parent = ags_thread_new(NULL);
  ags_thread_add_child_extended(main_loop,
				parent,
				TRUE, TRUE);

  thread = (AgsThread **) malloc(AGS_THREAD_TEST_REMOVE_CHILD_N_THREADS * sizeof(AgsThread*));

  for(i = 0; i < AGS_THREAD_TEST_REMOVE_CHILD_N_THREADS; i++){
    thread[i] = ags_thread_new(NULL);
    ags_thread_add_child_extended(parent,
				  thread[i],
				  TRUE, TRUE);
  }

  for(i = 0; i < AGS_THREAD_TEST_REMOVE_CHILD_N_THREADS; i++){
    g_object_ref(thread[i]);
    ags_thread_remove_child(parent,
			    thread[i]);

    current = g_atomic_pointer_get(&(parent->children));

    while(current != NULL){
      CU_ASSERT(current != thread[i]);

      current = g_atomic_pointer_get(&(current->next));
    }

    CU_ASSERT(g_atomic_pointer_get(&(thread[i]->parent)) == NULL);
    
    g_object_unref(thread[i]);
  }
}

void
ags_thread_test_add_child()
{
  AgsThread *parent;
  AgsThread **thread;
  AgsThread *current;

  guint i;
  gboolean success;

  parent = ags_thread_new(NULL);
  ags_thread_add_child_extended(main_loop,
				parent,
				TRUE, TRUE);

  thread = (AgsThread **) malloc(AGS_THREAD_TEST_ADD_CHILD_N_THREADS * sizeof(AgsThread*));

  for(i = 0; i < AGS_THREAD_TEST_ADD_CHILD_N_THREADS; i++){
    thread[i] = ags_thread_new(NULL);
    ags_thread_add_child_extended(parent,
				  thread[i],
				  TRUE, TRUE);

    current = g_atomic_pointer_get(&(parent->children));

    success = FALSE;
    
    while(current != NULL){
      if(current = thread[i]){
	success = TRUE;
	
	break;
      }
      
      current = g_atomic_pointer_get(&(current->next));
    }

    CU_ASSERT(success);
    CU_ASSERT(g_atomic_pointer_get(&(thread[i]->parent)) == parent);
  }
}

void
ags_thread_test_parental_is_locked()
{
  AgsThread *parent;
  AgsThread *thread;

  parent = ags_thread_new(NULL);
  
  thread = ags_thread_new(NULL);
  ags_thread_add_child_extended(parent,
				thread,
				TRUE, TRUE);

  CU_ASSERT(ags_thread_parental_is_locked(thread, NULL) == FALSE);

  ags_thread_lock(parent);
  
  CU_ASSERT(ags_thread_parental_is_locked(thread, NULL) == TRUE);
  
  ags_thread_unlock(parent);
}

void
ags_thread_test_sibling_is_locked()
{
  AgsThread *parent;
  AgsThread *thread_0, *thread_1;

  parent = ags_thread_new(NULL);
  
  thread_0 = ags_thread_new(NULL);
  ags_thread_add_child_extended(parent,
				thread_0,
				TRUE, TRUE);

  thread_1 = ags_thread_new(NULL);
  ags_thread_add_child_extended(parent,
				thread_1,
				TRUE, TRUE);

  /* thread 0 locked */
  CU_ASSERT(ags_thread_sibling_is_locked(thread_1) == FALSE);

  ags_thread_lock(thread_0);

  CU_ASSERT(ags_thread_sibling_is_locked(thread_1) == TRUE);

  ags_thread_unlock(thread_0);

  /* thread 1 locked */
  CU_ASSERT(ags_thread_sibling_is_locked(thread_0) == FALSE);

  ags_thread_lock(thread_1);

  CU_ASSERT(ags_thread_sibling_is_locked(thread_0) == TRUE);

  ags_thread_unlock(thread_1);
}

void
ags_thread_test_children_is_locked()
{
  AgsThread *parent;
  AgsThread *thread_0, *thread_1;

  parent = ags_thread_new(NULL);
  
  thread_0 = ags_thread_new(NULL);
  ags_thread_add_child_extended(parent,
				thread_0,
				TRUE, TRUE);

  thread_1 = ags_thread_new(NULL);
  ags_thread_add_child_extended(parent,
				thread_1,
				TRUE, TRUE);

  /* thread 0 locked */
  CU_ASSERT(ags_thread_children_is_locked(parent) == FALSE);

  ags_thread_lock(thread_0);

  CU_ASSERT(ags_thread_children_is_locked(parent) == TRUE);

  ags_thread_unlock(thread_0);

  /* thread 1 locked */
  CU_ASSERT(ags_thread_children_is_locked(parent) == FALSE);

  ags_thread_lock(thread_1);

  CU_ASSERT(ags_thread_children_is_locked(parent) == TRUE);

  ags_thread_unlock(thread_1);
}

void
ags_thread_test_is_current_ready()
{
  AgsThread *parent;
  AgsThread *thread;

  parent = ags_thread_new(NULL);
  
  thread = ags_thread_new(NULL);
  ags_thread_add_child_extended(parent,
				thread,
				TRUE, TRUE);

  /* not ready at all */
  CU_ASSERT(ags_thread_is_current_ready(parent, 0) == TRUE);
  CU_ASSERT(ags_thread_is_current_ready(thread, 0) == TRUE);

  CU_ASSERT(ags_thread_is_current_ready(parent, 1) == TRUE);
  CU_ASSERT(ags_thread_is_current_ready(thread, 1) == TRUE);

  CU_ASSERT(ags_thread_is_current_ready(parent, 2) == TRUE);
  CU_ASSERT(ags_thread_is_current_ready(thread, 2) == TRUE);

  /* wait 0 ready */
  g_atomic_int_or(&(parent->flags),
		  AGS_THREAD_WAIT_0);
  g_atomic_int_or(&(thread->flags),
		  AGS_THREAD_WAIT_0);

  CU_ASSERT(ags_thread_is_current_ready(parent, 0) == TRUE);
  CU_ASSERT(ags_thread_is_current_ready(thread, 0) == TRUE);

  g_atomic_int_and(&(parent->flags),
		   ~AGS_THREAD_WAIT_0);
  g_atomic_int_and(&(thread->flags),
		   ~AGS_THREAD_WAIT_0);

  /* wait 1 ready */
  g_atomic_int_or(&(parent->flags),
		  AGS_THREAD_WAIT_1);
  g_atomic_int_or(&(thread->flags),
		  AGS_THREAD_WAIT_1);

  CU_ASSERT(ags_thread_is_current_ready(parent, 1) == TRUE);
  CU_ASSERT(ags_thread_is_current_ready(thread, 1) == TRUE);

  g_atomic_int_and(&(parent->flags),
		   ~AGS_THREAD_WAIT_1);
  g_atomic_int_and(&(thread->flags),
		   ~AGS_THREAD_WAIT_1);

  /* wait 2 ready */
  g_atomic_int_or(&(parent->flags),
		  AGS_THREAD_WAIT_2);
  g_atomic_int_or(&(thread->flags),
		  AGS_THREAD_WAIT_2);

  CU_ASSERT(ags_thread_is_current_ready(parent, 2) == TRUE);
  CU_ASSERT(ags_thread_is_current_ready(thread, 2) == TRUE);

  g_atomic_int_and(&(parent->flags),
		   ~AGS_THREAD_WAIT_2);
  g_atomic_int_and(&(thread->flags),
		   ~AGS_THREAD_WAIT_2);
}

void
ags_thread_test_is_tree_ready()
{
  AgsThread *parent;
  AgsThread *thread;

  parent = ags_thread_new(NULL);
  
  thread = ags_thread_new(NULL);
  ags_thread_add_child_extended(parent,
				thread,
				TRUE, TRUE);

  /* not ready at all */
  CU_ASSERT(ags_thread_is_tree_ready(parent, 0) == TRUE);

  CU_ASSERT(ags_thread_is_tree_ready(parent, 1) == TRUE);

  CU_ASSERT(ags_thread_is_tree_ready(parent, 2) == TRUE);

  /* wait 0 ready */
  g_atomic_int_or(&(parent->flags),
		  AGS_THREAD_WAIT_0);
  g_atomic_int_or(&(thread->flags),
		  AGS_THREAD_WAIT_0);

  CU_ASSERT(ags_thread_is_tree_ready(parent, 0) == TRUE);

  g_atomic_int_and(&(parent->flags),
		   ~AGS_THREAD_WAIT_0);
  g_atomic_int_and(&(thread->flags),
		   ~AGS_THREAD_WAIT_0);

  /* wait 1 ready */
  g_atomic_int_or(&(parent->flags),
		  AGS_THREAD_WAIT_1);
  g_atomic_int_or(&(thread->flags),
		  AGS_THREAD_WAIT_1);

  CU_ASSERT(ags_thread_is_tree_ready(parent, 1) == TRUE);

  g_atomic_int_and(&(parent->flags),
		   ~AGS_THREAD_WAIT_1);
  g_atomic_int_and(&(thread->flags),
		   ~AGS_THREAD_WAIT_1);

  /* wait 2 ready */
  g_atomic_int_or(&(parent->flags),
		  AGS_THREAD_WAIT_2);
  g_atomic_int_or(&(thread->flags),
		  AGS_THREAD_WAIT_2);

  CU_ASSERT(ags_thread_is_tree_ready(parent, 2) == TRUE);

  g_atomic_int_and(&(parent->flags),
		   ~AGS_THREAD_WAIT_2);
  g_atomic_int_and(&(thread->flags),
		   ~AGS_THREAD_WAIT_2);

}

void
ags_thread_test_next_parent_locked()
{
  AgsThread *toplevel;

  AgsThread *locked_thread_level_1_a;
  AgsThread *locked_thread_level_2_a;
  AgsThread *locked_thread_level_3_a;

  AgsThread *locked_thread_level_1_b;
  AgsThread *locked_thread_level_2_b;

  AgsThread *locked_thread_level_1_c;

  AgsThread *locked_thread_level_1_d;
  AgsThread *locked_thread_level_3_d;

  AgsThread *thread_level_4_a;
  AgsThread *thread_level_4_b;
  AgsThread *thread_level_4_c;
  AgsThread *thread_level_4_d;

  AgsThread *current, *parent;
  
  toplevel = ags_generic_main_loop_new(NULL);

  /* thread group a */
  locked_thread_level_1_a = ags_thread_new(NULL);
  ags_thread_add_child_extended(toplevel,
				locked_thread_level_1_a,
				TRUE, TRUE);

  g_atomic_int_or(&(locked_thread_level_1_a->sync_flags),
		  AGS_THREAD_WAITING_FOR_CHILDREN);

  locked_thread_level_2_a = ags_thread_new(NULL);
  ags_thread_add_child_extended(locked_thread_level_1_a,
				locked_thread_level_2_a,
				TRUE, TRUE);

  g_atomic_int_or(&(locked_thread_level_2_a->sync_flags),
		  AGS_THREAD_WAITING_FOR_CHILDREN);

  locked_thread_level_3_a = ags_thread_new(NULL);
  ags_thread_add_child_extended(locked_thread_level_2_a,
				locked_thread_level_3_a,
				TRUE, TRUE);

  g_atomic_int_or(&(locked_thread_level_3_a->sync_flags),
		  AGS_THREAD_WAITING_FOR_CHILDREN);

  thread_level_4_a = ags_thread_new(NULL);
  ags_thread_add_child_extended(locked_thread_level_3_a,
				thread_level_4_a,
				TRUE, TRUE);

  /* thread group b */
  locked_thread_level_1_b = ags_thread_new(NULL);
  ags_thread_add_child_extended(toplevel,
				locked_thread_level_1_b,
				TRUE, TRUE);

  g_atomic_int_or(&(locked_thread_level_1_b->sync_flags),
		  AGS_THREAD_WAITING_FOR_CHILDREN);

  locked_thread_level_2_b = ags_thread_new(NULL);
  ags_thread_add_child_extended(locked_thread_level_1_b,
				locked_thread_level_2_b,
				TRUE, TRUE);

  g_atomic_int_or(&(locked_thread_level_2_b->sync_flags),
		  AGS_THREAD_WAITING_FOR_CHILDREN);

  current = ags_thread_new(NULL);
  ags_thread_add_child_extended(locked_thread_level_2_b,
				current,
				TRUE, TRUE);

  parent = current;
  
  thread_level_4_b = ags_thread_new(NULL);
  ags_thread_add_child_extended(parent,
				thread_level_4_b,
				TRUE, TRUE);

  /* thread group c */
  locked_thread_level_1_c = ags_thread_new(NULL);
  ags_thread_add_child_extended(toplevel,
				locked_thread_level_1_c,
				TRUE, TRUE);

  g_atomic_int_or(&(locked_thread_level_1_c->sync_flags),
		  AGS_THREAD_WAITING_FOR_CHILDREN);

  current = ags_thread_new(NULL);
  ags_thread_add_child_extended(locked_thread_level_1_c,
				current,
				TRUE, TRUE);

  parent = current;
  
  current = ags_thread_new(NULL);
  ags_thread_add_child_extended(parent,
				current,
				TRUE, TRUE);

  parent = current;
  
  current = ags_thread_new(NULL);
  ags_thread_add_child_extended(parent,
				current,
				TRUE, TRUE);

  thread_level_4_c = ags_thread_new(NULL);
  ags_thread_add_child_extended(current,
				thread_level_4_c,
				TRUE, TRUE);

  /* thread group d */
  locked_thread_level_1_d = ags_thread_new(NULL);
  ags_thread_add_child_extended(toplevel,
				locked_thread_level_1_d,
				TRUE, TRUE);

  g_atomic_int_or(&(locked_thread_level_1_d->sync_flags),
		  AGS_THREAD_WAITING_FOR_CHILDREN);

  current = ags_thread_new(NULL);
  ags_thread_add_child_extended(locked_thread_level_1_d,
				current,
				TRUE, TRUE);

  parent = current;

  locked_thread_level_3_d = ags_thread_new(NULL);
  ags_thread_add_child_extended(parent,
				locked_thread_level_3_d,
				TRUE, TRUE);

  g_atomic_int_or(&(locked_thread_level_3_d->sync_flags),
		  AGS_THREAD_WAITING_FOR_CHILDREN);

  thread_level_4_d = ags_thread_new(NULL);
  ags_thread_add_child_extended(locked_thread_level_3_d,
				thread_level_4_d,
				TRUE, TRUE);

  /* assert thread group a */
  current = thread_level_4_a;
  
  CU_ASSERT((current = ags_thread_next_parent_locked(current, NULL)) == locked_thread_level_3_a);
  CU_ASSERT((current = ags_thread_next_parent_locked(current, NULL)) == locked_thread_level_2_a);
  CU_ASSERT((current = ags_thread_next_parent_locked(current, NULL)) == locked_thread_level_1_a);
  CU_ASSERT((current = ags_thread_next_parent_locked(current, NULL)) == NULL);

  /* assert thread group b */
  current = thread_level_4_b;
  
  CU_ASSERT((current = ags_thread_next_parent_locked(current, NULL)) == locked_thread_level_2_b);
  CU_ASSERT((current = ags_thread_next_parent_locked(current, NULL)) == locked_thread_level_1_b);
  CU_ASSERT((current = ags_thread_next_parent_locked(current, NULL)) == NULL);

  /* assert thread group c */
  current = thread_level_4_c;
  
  CU_ASSERT((current = ags_thread_next_parent_locked(current, NULL)) == locked_thread_level_1_c);
  CU_ASSERT((current = ags_thread_next_parent_locked(current, NULL)) == NULL);
  
  /* assert thread group d */
  current = thread_level_4_d;
  
  CU_ASSERT((current = ags_thread_next_parent_locked(current, NULL)) == locked_thread_level_3_d);
  CU_ASSERT((current = ags_thread_next_parent_locked(current, NULL)) == locked_thread_level_1_d);
  CU_ASSERT((current = ags_thread_next_parent_locked(current, NULL)) == NULL);
}

void
ags_thread_test_next_sibling_locked()
{
  AgsThread *toplevel;

  AgsThread *sibling0;
  AgsThread *sibling1;
  AgsThread *sibling3;
  AgsThread *sibling7;

  AgsThread *current, *prev;

  toplevel = ags_generic_main_loop_new(NULL);

  /* create sibling */
  sibling0 =
    current = ags_thread_new(NULL);
  ags_thread_add_child_extended(toplevel,
				current,
				TRUE, TRUE);

  sibling1 =
    current = ags_thread_new(NULL);
  ags_thread_add_child_extended(toplevel,
				current,
				TRUE, TRUE);
  g_atomic_int_or(&(current->sync_flags),
		  AGS_THREAD_WAITING_FOR_SIBLING);
  
  current = ags_thread_new(NULL);
  ags_thread_add_child_extended(toplevel,
				current,
				TRUE, TRUE);

  sibling3 = 
    current = ags_thread_new(NULL);
  ags_thread_add_child_extended(toplevel,
				current,
				TRUE, TRUE);

  g_atomic_int_or(&(current->sync_flags),
		  AGS_THREAD_WAITING_FOR_SIBLING);

  current = ags_thread_new(NULL);
  ags_thread_add_child_extended(toplevel,
				current,
				TRUE, TRUE);

  current = ags_thread_new(NULL);
  ags_thread_add_child_extended(toplevel,
				current,
				TRUE, TRUE);

  current = ags_thread_new(NULL);
  ags_thread_add_child_extended(toplevel,
				current,
				TRUE, TRUE);

  sibling7 =
    current = ags_thread_new(NULL);
  ags_thread_add_child_extended(toplevel,
				current,
				TRUE, TRUE);

  g_atomic_int_or(&(current->sync_flags),
		  AGS_THREAD_WAITING_FOR_SIBLING);

  current = ags_thread_new(NULL);
  ags_thread_add_child_extended(toplevel,
				current,
				TRUE, TRUE);

  current = ags_thread_new(NULL);
  ags_thread_add_child_extended(toplevel,
				current,
				TRUE, TRUE);

  current = ags_thread_new(NULL);
  ags_thread_add_child_extended(toplevel,
				current,
				TRUE, TRUE);

  /* assert sibling */
  current = sibling0;

  CU_ASSERT((current = ags_thread_next_sibling_locked(current)) == sibling1);
  
  g_atomic_int_and(&(sibling1->sync_flags),
		   (~AGS_THREAD_WAITING_FOR_SIBLING));
  
  CU_ASSERT((current = ags_thread_next_sibling_locked(current)) == sibling3);
  
  g_atomic_int_and(&(sibling3->sync_flags),
		   (~AGS_THREAD_WAITING_FOR_SIBLING));
  
  CU_ASSERT((current = ags_thread_next_sibling_locked(current)) == sibling7);
  
  g_atomic_int_and(&(sibling7->sync_flags),
		   (~AGS_THREAD_WAITING_FOR_SIBLING));
  
  CU_ASSERT((current = ags_thread_next_sibling_locked(current)) == NULL);
}

void
ags_thread_test_next_children_locked()
{
  AgsThread *toplevel;

  AgsThread *locked_thread_level_1_a;
  AgsThread *locked_thread_level_2_a;
  AgsThread *locked_thread_level_3_a;

  AgsThread *locked_thread_level_1_b;
  AgsThread *locked_thread_level_2_b;

  AgsThread *locked_thread_level_1_c;

  AgsThread *locked_thread_level_1_d;
  AgsThread *locked_thread_level_3_d;

  AgsThread *thread_level_4_a;
  AgsThread *thread_level_4_b;
  AgsThread *thread_level_4_c;
  AgsThread *thread_level_4_d;

  AgsThread *current, *parent;
  
  toplevel = ags_generic_main_loop_new(NULL);

  /* thread group a */
  locked_thread_level_1_a = ags_thread_new(NULL);
  ags_thread_add_child_extended(toplevel,
				locked_thread_level_1_a,
				TRUE, TRUE);

  g_atomic_int_or(&(locked_thread_level_1_a->sync_flags),
		  AGS_THREAD_WAITING_FOR_PARENT);

  locked_thread_level_2_a = ags_thread_new(NULL);
  ags_thread_add_child_extended(locked_thread_level_1_a,
				locked_thread_level_2_a,
				TRUE, TRUE);

  g_atomic_int_or(&(locked_thread_level_2_a->sync_flags),
		  AGS_THREAD_WAITING_FOR_PARENT);

  locked_thread_level_3_a = ags_thread_new(NULL);
  ags_thread_add_child_extended(locked_thread_level_2_a,
				locked_thread_level_3_a,
				TRUE, TRUE);

  g_atomic_int_or(&(locked_thread_level_3_a->sync_flags),
		  AGS_THREAD_WAITING_FOR_PARENT);

  thread_level_4_a = ags_thread_new(NULL);
  ags_thread_add_child_extended(locked_thread_level_3_a,
				thread_level_4_a,
				TRUE, TRUE);

  /* thread group b */
  locked_thread_level_1_b = ags_thread_new(NULL);
  ags_thread_add_child_extended(toplevel,
				locked_thread_level_1_b,
				TRUE, TRUE);

  g_atomic_int_or(&(locked_thread_level_1_b->sync_flags),
		  AGS_THREAD_WAITING_FOR_PARENT);

  locked_thread_level_2_b = ags_thread_new(NULL);
  ags_thread_add_child_extended(locked_thread_level_1_b,
				locked_thread_level_2_b,
				TRUE, TRUE);

  g_atomic_int_or(&(locked_thread_level_2_b->sync_flags),
		  AGS_THREAD_WAITING_FOR_PARENT);

  current = ags_thread_new(NULL);
  ags_thread_add_child_extended(locked_thread_level_2_b,
				current,
				TRUE, TRUE);

  parent = current;
  
  thread_level_4_b = ags_thread_new(NULL);
  ags_thread_add_child_extended(parent,
				thread_level_4_b,
				TRUE, TRUE);

  /* thread group c */
  locked_thread_level_1_c = ags_thread_new(NULL);
  ags_thread_add_child_extended(toplevel,
				locked_thread_level_1_c,
				TRUE, TRUE);

  g_atomic_int_or(&(locked_thread_level_1_c->sync_flags),
		  AGS_THREAD_WAITING_FOR_PARENT);

  current = ags_thread_new(NULL);
  ags_thread_add_child_extended(locked_thread_level_1_c,
				current,
				TRUE, TRUE);

  parent = current;
  
  current = ags_thread_new(NULL);
  ags_thread_add_child_extended(parent,
				current,
				TRUE, TRUE);

  parent = current;
  
  current = ags_thread_new(NULL);
  ags_thread_add_child_extended(parent,
				current,
				TRUE, TRUE);

  thread_level_4_c = ags_thread_new(NULL);
  ags_thread_add_child_extended(current,
				thread_level_4_c,
				TRUE, TRUE);

  /* thread group d */
  locked_thread_level_1_d = ags_thread_new(NULL);
  ags_thread_add_child_extended(toplevel,
				locked_thread_level_1_d,
				TRUE, TRUE);

  g_atomic_int_or(&(locked_thread_level_1_d->sync_flags),
		  AGS_THREAD_WAITING_FOR_PARENT);

  current = ags_thread_new(NULL);
  ags_thread_add_child_extended(locked_thread_level_1_d,
				current,
				TRUE, TRUE);

  parent = current;

  locked_thread_level_3_d = ags_thread_new(NULL);
  ags_thread_add_child_extended(parent,
				locked_thread_level_3_d,
				TRUE, TRUE);

  g_atomic_int_or(&(locked_thread_level_3_d->sync_flags),
		  AGS_THREAD_WAITING_FOR_PARENT);

  thread_level_4_d = ags_thread_new(NULL);
  ags_thread_add_child_extended(locked_thread_level_3_d,
				thread_level_4_d,
				TRUE, TRUE);

  /* assert children */
  //NOTE:JK: don't touch because you need the order
  current = toplevel;
  
  /* assert group d */
  CU_ASSERT((current = ags_thread_next_children_locked(current)) == locked_thread_level_3_d);
  g_atomic_int_and(&(locked_thread_level_3_d->sync_flags),
		   (~AGS_THREAD_WAITING_FOR_PARENT));
  
  current = toplevel;
  CU_ASSERT((current = ags_thread_next_children_locked(current)) == locked_thread_level_1_d);
  g_atomic_int_and(&(locked_thread_level_1_d->sync_flags),
		   (~AGS_THREAD_WAITING_FOR_PARENT));

  /* assert group c */
  current = toplevel;
  CU_ASSERT((current = ags_thread_next_children_locked(current)) == locked_thread_level_1_c);
  g_atomic_int_and(&(locked_thread_level_1_c->sync_flags),
		   (~AGS_THREAD_WAITING_FOR_PARENT)); 
  
  /* assert group b */
  current = toplevel;
  CU_ASSERT((current = ags_thread_next_children_locked(current)) == locked_thread_level_2_b);
  g_atomic_int_and(&(locked_thread_level_2_b->sync_flags),
		   (~AGS_THREAD_WAITING_FOR_PARENT));

  current = toplevel;
  CU_ASSERT((current = ags_thread_next_children_locked(current)) == locked_thread_level_1_b);
  g_atomic_int_and(&(locked_thread_level_1_b->sync_flags),
		   (~AGS_THREAD_WAITING_FOR_PARENT));

  /* assert group a */
  current = toplevel;
  CU_ASSERT((current = ags_thread_next_children_locked(current)) == locked_thread_level_3_a);
  g_atomic_int_and(&(locked_thread_level_3_a->sync_flags),
		   (~AGS_THREAD_WAITING_FOR_PARENT));

  current = toplevel;
  CU_ASSERT((current = ags_thread_next_children_locked(current)) == locked_thread_level_2_a);
  g_atomic_int_and(&(locked_thread_level_2_a->sync_flags),
		   (~AGS_THREAD_WAITING_FOR_PARENT));

  current = toplevel;
  CU_ASSERT((current = ags_thread_next_children_locked(current)) == locked_thread_level_1_a);
  g_atomic_int_and(&(locked_thread_level_1_a->sync_flags),
		   (~AGS_THREAD_WAITING_FOR_PARENT));

  /* assert group NULL */
  current = toplevel;
  CU_ASSERT((current = ags_thread_next_children_locked(current)) == NULL);
}

void
ags_thread_test_lock_parent()
{
  AgsThread *toplevel;
  AgsThread *parent, *current;
  
  GList *thread;

  guint i;
  gboolean success;

  //TODO:JK: improve this test
  toplevel = ags_generic_main_loop_new(NULL);

  /* create tree */
  parent = toplevel;
  
  for(i = 0; i < AGS_THREAD_TEST_LOCK_PARENT_PARENT_COUNT; i++){
    current = ags_thread_new(NULL);
    ags_thread_add_child_extended(parent,
				  current,
				  TRUE, TRUE);

    parent = current;
  }

  /* lock parent */
  ags_thread_lock_parent(current,
			 NULL);

  /* assert current not locked */
  CU_ASSERT((AGS_THREAD_LOCKED & (g_atomic_int_get(&(current->flags)))) != 0);
  CU_ASSERT((AGS_THREAD_WAITING_FOR_CHILDREN & (g_atomic_int_get(&(current->sync_flags)))) == 0);
  
  /* assert all parent locked */
  current = g_atomic_pointer_get(&(current->parent));
  success = TRUE;
  
  while(current != NULL){
    if((AGS_THREAD_LOCKED & (g_atomic_int_get(&(current->flags)))) == 0 ||
       (AGS_THREAD_WAITING_FOR_CHILDREN & (g_atomic_int_get(&(current->sync_flags)))) == 0){
      success = FALSE;

      break;
    }
    
    current = g_atomic_pointer_get(&(current->parent));
  }

  CU_ASSERT(success == TRUE);
}

void
ags_thread_test_lock_sibling()
{
  AgsThread *toplevel;
  AgsThread *parent, *current, *iter;

  guint nth;
  guint i;

  auto gboolean ags_thread_test_lock_sibling_assert(AgsThread *parent, AgsThread *current){
    AgsThread *iter;
    
    gboolean success;
    
    /* assert sibling */
    iter = g_atomic_pointer_get(&(parent->children));
    success = TRUE;

    while(iter != NULL){
      if(iter == current){
	if((AGS_THREAD_LOCKED & (g_atomic_int_get(&(iter->flags)))) == 0 ||
	   (AGS_THREAD_WAITING_FOR_SIBLING & (g_atomic_int_get(&(iter->sync_flags)))) != 0){
	  success = FALSE;
	  
	  break;
	}
	
	iter = g_atomic_pointer_get(&(iter->next));

	continue;
      }
    
      if((AGS_THREAD_LOCKED & (g_atomic_int_get(&(iter->flags)))) == 0 ||
	 (AGS_THREAD_WAITING_FOR_SIBLING & (g_atomic_int_get(&(iter->sync_flags)))) == 0){
	success = FALSE;

	break;
      }
    
      iter = g_atomic_pointer_get(&(iter->next));
    }


    return(success);
  }
  
  toplevel = ags_thread_new(NULL);

  /* create tree */
  parent = toplevel;
  
  for(i = 0; i < AGS_THREAD_TEST_LOCK_SIBLING_SIBLING_COUNT; i++){
    current = ags_thread_new(NULL);
    ags_thread_add_child_extended(toplevel,
				  current,
				  TRUE, TRUE);
  }

  /* lock sibling first */
  current = g_atomic_pointer_get(&(parent->children));
  ags_thread_lock_sibling(current);

  /* assert sibling */
  CU_ASSERT(ags_thread_test_lock_sibling_assert(parent, current) == TRUE);  
  
  ags_thread_unlock_sibling(current);
  
  /* lock sibling last */
  current = ags_thread_last(g_atomic_pointer_get(&(parent->children)));
  ags_thread_lock_sibling(current);
  
  /* assert sibling */
  CU_ASSERT(ags_thread_test_lock_sibling_assert(parent, current) == TRUE);
  
  ags_thread_unlock_sibling(current);

  /* lock sibling random 0 */
  current = g_atomic_pointer_get(&(parent->children));
  nth = rand() % AGS_THREAD_TEST_LOCK_SIBLING_SIBLING_COUNT;

  for(i = 0; i < nth; i++){
    current = g_atomic_pointer_get(&(current->next));
  }  
  
  ags_thread_lock_sibling(current);

  /* assert sibling */
  CU_ASSERT(ags_thread_test_lock_sibling_assert(parent, current) == TRUE);
  
  ags_thread_unlock_sibling(current);

  /* lock sibling random 1 */
  current = g_atomic_pointer_get(&(parent->children));
  nth = rand() % AGS_THREAD_TEST_LOCK_SIBLING_SIBLING_COUNT;

  for(i = 0; i < nth; i++){
    current = g_atomic_pointer_get(&(current->next));
  }  
  
  ags_thread_lock_sibling(current);

  /* assert sibling */
  CU_ASSERT(ags_thread_test_lock_sibling_assert(parent, current) == TRUE);
  
  ags_thread_unlock_sibling(current);
}

void
ags_thread_test_lock_children()
{
  AgsThread *toplevel;
  AgsThread *parent, *current;
  
  GList *thread;

  guint i;
  gboolean success;

  //TODO:JK: improve this test
  toplevel = ags_generic_main_loop_new(NULL);

  /* create tree */
  parent = toplevel;
  
  for(i = 0; i < AGS_THREAD_TEST_LOCK_PARENT_PARENT_COUNT; i++){
    current = ags_thread_new(NULL);
    ags_thread_add_child_extended(parent,
				  current,
				  TRUE, TRUE);

    parent = current;
  }

  /* lock children */
  ags_thread_lock_children(toplevel);

  /* assert toplevel not locked */
  CU_ASSERT((AGS_THREAD_LOCKED & (g_atomic_int_get(&(toplevel->flags)))) != 0);
  CU_ASSERT((AGS_THREAD_WAITING_FOR_PARENT & (g_atomic_int_get(&(toplevel->sync_flags)))) == 0);

  /* assert all children locked */
  current = g_atomic_pointer_get(&(current->children));
  success = TRUE;
  
  while(current != NULL){
    if((AGS_THREAD_LOCKED & (g_atomic_int_get(&(current->flags)))) == 0 ||
       (AGS_THREAD_WAITING_FOR_PARENT & (g_atomic_int_get(&(current->sync_flags)))) == 0){
      success = FALSE;

      break;
    }
    
    current = g_atomic_pointer_get(&(current->children));
  }

  CU_ASSERT(success == TRUE);
}

void
ags_thread_test_lock_all()
{
  //TODO:JK: implement me
}

void
ags_thread_test_wait_parent()
{
  //TODO:JK: implement me
}

void
ags_thread_test_wait_sibling()
{
  //TODO:JK: implement me
}

void
ags_thread_test_wait_children()
{
  //TODO:JK: implement me
}

void
ags_thread_test_suspend()
{
  //TODO:JK: implement me
}

void
ags_thread_test_timelock()
{
  //TODO:JK: implement me
}

void
ags_thread_test_stop()
{
  //TODO:JK: implement me
}

int
main(int argc, char **argv)
{
  CU_pSuite pSuite = NULL;

  putenv("LC_ALL=C\0");
  putenv("LANG=C\0");
  
  /* initialize the CUnit test registry */
  if(CUE_SUCCESS != CU_initialize_registry()){
    return CU_get_error();
  }

  /* add a suite to the registry */
  pSuite = CU_add_suite("AgsThreadTest\0", ags_thread_test_init_suite, ags_thread_test_clean_suite);
  
  if(pSuite == NULL){
    CU_cleanup_registry();
    
    return CU_get_error();
  }

  /* add the tests to the suite */
  if(/* (CU_add_test(pSuite, "test of AgsThread sync\0", ags_thread_test_sync) == NULL) || - needs fix because of a race-condition */
     (CU_add_test(pSuite, "test of AgsThread lock\0", ags_thread_test_lock) == NULL) ||
     (CU_add_test(pSuite, "test of AgsThread trylock\0", ags_thread_test_trylock) == NULL) ||
     (CU_add_test(pSuite, "test of AgsThread get toplevel\0", ags_thread_test_get_toplevel) == NULL) ||
     (CU_add_test(pSuite, "test of AgsThread first\0", ags_thread_test_first) == NULL) ||
     (CU_add_test(pSuite, "test of AgsThread last\0", ags_thread_test_last) == NULL) ||
     (CU_add_test(pSuite, "test of AgsThread remove child\0", ags_thread_test_remove_child) == NULL) ||
     (CU_add_test(pSuite, "test of AgsThread add child\0", ags_thread_test_add_child) == NULL) ||
     (CU_add_test(pSuite, "test of AgsThread parental is locked\0", ags_thread_test_parental_is_locked) == NULL) ||
     (CU_add_test(pSuite, "test of AgsThread sibling is locked\0", ags_thread_test_sibling_is_locked) == NULL) ||
     (CU_add_test(pSuite, "test of AgsThread children is locked\0", ags_thread_test_children_is_locked) == NULL) ||
     (CU_add_test(pSuite, "test of AgsThread is current ready\0", ags_thread_test_is_current_ready) == NULL) ||
     (CU_add_test(pSuite, "test of AgsThread is tree ready\0", ags_thread_test_is_tree_ready) == NULL) ||
     (CU_add_test(pSuite, "test of AgsThread next parent locked\0", ags_thread_test_next_parent_locked) == NULL) ||
     (CU_add_test(pSuite, "test of AgsThread next sibling locked\0", ags_thread_test_next_sibling_locked) == NULL) ||
     (CU_add_test(pSuite, "test of AgsThread next children locked\0", ags_thread_test_next_children_locked) == NULL) ||
     (CU_add_test(pSuite, "test of AgsThread lock parent\0", ags_thread_test_lock_parent) == NULL) ||
     (CU_add_test(pSuite, "test of AgsThread lock sibling\0", ags_thread_test_lock_sibling) == NULL) ||
     (CU_add_test(pSuite, "test of AgsThread lock children\0", ags_thread_test_lock_children) == NULL) ||
     (CU_add_test(pSuite, "test of AgsThread lock all\0", ags_thread_test_lock_all) == NULL) ||
     (CU_add_test(pSuite, "test of AgsThread wait parent\0", ags_thread_test_wait_parent) == NULL) ||
     (CU_add_test(pSuite, "test of AgsThread wait sibling\0", ags_thread_test_wait_sibling) == NULL) ||
     (CU_add_test(pSuite, "test of AgsThread wait children\0", ags_thread_test_wait_children) == NULL) ||
     (CU_add_test(pSuite, "test of AgsThread suspend\0", ags_thread_test_suspend) == NULL) ||
     (CU_add_test(pSuite, "test of AgsThread timelock\0", ags_thread_test_timelock) == NULL) ||
     (CU_add_test(pSuite, "test of AgsThread stop\0", ags_thread_test_stop) == NULL)){
    CU_cleanup_registry();
      
    return CU_get_error();
  }
  
  /* Run all tests using the CUnit Basic interface */
  CU_basic_set_mode(CU_BRM_VERBOSE);
  CU_basic_run_tests();
  
  CU_cleanup_registry();
  
  return(CU_get_error());
}

