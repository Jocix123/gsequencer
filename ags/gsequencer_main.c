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

#include <glib.h>
#include <glib-object.h>

#include "gsequencer_main.h"

#include <ags/object/ags_application_context.h>
#include <ags/object/ags_connectable.h>
#include <ags/object/ags_main_loop.h>
#include <ags/object/ags_soundcard.h>

#include <ags/thread/ags_concurrency_provider.h>
#ifdef AGS_USE_LINUX_THREADS
#include <ags/thread/ags_thread-kthreads.h>
#else
#include <ags/thread/ags_thread-posix.h>
#endif
#include <ags/thread/ags_single_thread.h>
#include <ags/thread/ags_autosave_thread.h>
#include <ags/thread/ags_task_thread.h>
#include <ags/thread/ags_thread_pool.h>

#include <ags/audio/thread/ags_audio_loop.h>
#include <ags/audio/thread/ags_soundcard_thread.h>
#include <ags/audio/thread/ags_export_thread.h>

#include <ags/server/ags_server.h>

#include <ags/plugin/ags_ladspa_manager.h>
#include <ags/plugin/ags_dssi_manager.h>
#include <ags/plugin/ags_lv2_manager.h>
#include <ags/plugin/ags_lv2_worker_manager.h>
#include <ags/plugin/ags_lv2ui_manager.h>

#include <ags/audio/ags_sound_provider.h>

#include <ags/X/ags_xorg_application_context.h>
#include <ags/X/ags_window.h>

#include <ags/X/file/ags_simple_file.h>

#include <ags/X/thread/ags_gui_thread.h>
#include <ags/X/thread/ags_simple_autosave_thread.h>

#include <ags/X/task/ags_simple_file_read.h>

#include "config.h"

void ags_signal_handler(int signr);
void ags_signal_handler_timer(int sig, siginfo_t *si, void *uc);
static void ags_signal_cleanup();

void ags_setup(int argc, char **argv);

void ags_launch(gboolean single_thread);
void ags_launch_filename(gchar *filename,
			 gboolean single_thread);

timer_t* ags_timer_setup();
void ags_timer_start(timer_t *timer_id);
void ags_timer_launch(timer_t *timer_id,
		      gboolean single_thread);
void ags_timer_launch_filename(timer_t *timer_id, gchar *filename,
			       gboolean single_thread);

void ags_show_file_error(gchar *filename,
			 GError *error);

static sigset_t ags_wait_mask;
static sigset_t ags_timer_mask;

struct sigaction ags_sigact;
struct sigaction ags_sigact_timer;

struct sigevent ags_sev_timer;
struct itimerspec its;

AgsApplicationContext *application_context;

void
ags_signal_handler(int signr)
{
  if(signr == SIGINT){
    //TODO:JK: do backup
    
    exit(-1);
  }else{
    sigemptyset(&(ags_sigact.sa_mask));

    //    if(signr == AGS_ASYNC_QUEUE_SIGNAL_HIGH){
      // pthread_yield();
    //    }
  }
}

void
ags_signal_handler_timer(int sig, siginfo_t *si, void *uc)
{
    pthread_mutex_lock(AGS_THREAD(application_context->main_loop)->timer_mutex);

    g_atomic_int_set(&(AGS_THREAD(application_context->main_loop)->timer_expired),
		     TRUE);
  
    if(AGS_THREAD(application_context->main_loop)->timer_wait){
      pthread_cond_signal(AGS_THREAD(application_context->main_loop)->timer_cond);
    }
    
    pthread_mutex_unlock(AGS_THREAD(application_context->main_loop)->timer_mutex);
  //  signal(sig, SIG_IGN);
}

static void
ags_signal_cleanup()
{
  sigemptyset(&(ags_sigact.sa_mask));
}

void
ags_setup(int argc, char **argv)
{
  AgsLadspaManager *ladspa_manager;
  AgsDssiManager *dssi_manager;
  AgsLv2Manager *lv2_manager;
  AgsLv2WorkerManager *lv2_worker_manager;
  AgsLv2uiManager *lv2ui_manager;
  
  struct passwd *pw;

  gchar *rc_filename;
  
  uid_t uid;

  /* parse gtkrc */
  uid = getuid();
  pw = getpwuid(uid);

  /* load managers */
  ladspa_manager = ags_ladspa_manager_get_instance();

  dssi_manager = ags_dssi_manager_get_instance();

  lv2_manager = ags_lv2_manager_get_instance();
  lv2_worker_manager = ags_lv2_worker_manager_get_instance();
  
  lv2ui_manager = ags_lv2ui_manager_get_instance();  

  application_context = ags_xorg_application_context_new();
  application_context->argc = argc;
  application_context->argv = argv;

  /* parse rc file */
  rc_filename = g_strdup_printf("%s/%s/ags.rc\0",
				pw->pw_dir,
				AGS_DEFAULT_DIRECTORY);
  gtk_rc_parse(rc_filename);
  g_free(rc_filename);
}

void
ags_launch(gboolean single_thread)
{
  AgsMutexManager *mutex_manager;
  AgsThread *audio_loop, *gui_thread, *task_thread;
  AgsThreadPool *thread_pool;

  AgsConfig *config;

  GList *start_queue;  

  pthread_mutex_t *application_mutex;

  mutex_manager = ags_mutex_manager_get_instance();
  application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);
    
  /* get threads, thread pool and config */
  pthread_mutex_lock(application_mutex);
  
  audio_loop = application_context->main_loop;
  task_thread = application_context->task_thread;
  thread_pool = AGS_TASK_THREAD(task_thread)->thread_pool;

  config = application_context->config;
  
  pthread_mutex_unlock(application_mutex);
  
  gui_thread = ags_thread_find_type(audio_loop,
				    AGS_TYPE_GUI_THREAD);

  /* start engine */
  pthread_mutex_lock(audio_loop->start_mutex);
    
  start_queue = NULL;
  start_queue = g_list_prepend(start_queue,
			       task_thread);
  start_queue = g_list_prepend(start_queue,
			       gui_thread);
  g_atomic_pointer_set(&(audio_loop->start_queue),
		       start_queue);
  
  pthread_mutex_unlock(audio_loop->start_mutex);

  /* start audio loop and thread pool*/
  ags_thread_start(audio_loop);
  
  ags_thread_pool_start(thread_pool);

  if(!single_thread){
    /* wait for audio loop */
    pthread_mutex_lock(audio_loop->start_mutex);

    if(g_atomic_int_get(&(audio_loop->start_wait)) == TRUE){	
      g_atomic_int_set(&(audio_loop->start_done),
		       FALSE);
      
      while(g_atomic_int_get(&(audio_loop->start_wait)) == TRUE &&
	    g_atomic_int_get(&(audio_loop->start_done)) == FALSE){
	pthread_cond_wait(audio_loop->start_cond,
			  audio_loop->start_mutex);
      }
    }
    
    pthread_mutex_unlock(audio_loop->start_mutex);

    /* wait for gui thread */
    pthread_mutex_lock(gui_thread->start_mutex);

    if(g_atomic_int_get(&(gui_thread->start_done)) == FALSE){
      
      g_atomic_int_set(&(gui_thread->start_wait),
		       TRUE);

      while(g_atomic_int_get(&(gui_thread->start_done)) == FALSE){
	g_atomic_int_set(&(gui_thread->start_wait),
			 TRUE);
	
	pthread_cond_wait(gui_thread->start_cond,
			  gui_thread->start_mutex);
      }
    }
    
    pthread_mutex_unlock(gui_thread->start_mutex);

    /* autosave thread */
    if(!g_strcmp0(ags_config_get_value(config,
				       AGS_CONFIG_GENERIC,
				       "autosave-thread\0"),
		  "true\0")){
      pthread_mutex_lock(audio_loop->start_mutex);

      start_queue = g_atomic_pointer_get(&(audio_loop->start_queue));
      start_queue = g_list_prepend(start_queue,
				   task_thread);

      g_atomic_pointer_set(&(audio_loop->start_queue),
			   start_queue);
	
      pthread_mutex_unlock(audio_loop->start_mutex);
    }

    /* join gui thread */
    pthread_join(*(gui_thread->thread),
		 NULL);
  }else{
    AgsSingleThread *single_thread;

    /* single thread */
    single_thread = ags_single_thread_new((GObject *) ags_sound_provider_get_soundcard(application_context)->data);

    /* add known threads to single_thread */
    ags_thread_add_child(AGS_THREAD(single_thread),
			 audio_loop);
    
    /* autosave thread */
    if(!g_strcmp0(ags_config_get_value(config,
				       AGS_CONFIG_GENERIC,
				       "autosave-thread\0"),
		  "true\0")){
      pthread_mutex_lock(audio_loop->start_mutex);

      start_queue = g_atomic_pointer_get(&(audio_loop->start_queue));
      start_queue = g_list_prepend(start_queue,
				   task_thread);

      g_atomic_pointer_set(&(audio_loop->start_queue),
			   start_queue);
	
      pthread_mutex_unlock(audio_loop->start_mutex);
    }

    /* start thread tree */
    ags_thread_start((AgsThread *) single_thread);
  }
}

void
ags_launch_filename(gchar *filename,
		    gboolean single_thread)
{
  AgsMutexManager *mutex_manager;
  AgsThread *audio_loop, *gui_thread, *task_thread;
  AgsThreadPool *thread_pool;

  AgsConfig *config;

  GList *start_queue;  

  pthread_mutex_t *application_mutex;

  mutex_manager = ags_mutex_manager_get_instance();
  application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);
    
  /* get threads, thread pool and config */
  pthread_mutex_lock(application_mutex);

  audio_loop = application_context->main_loop;
  task_thread = application_context->task_thread;
  thread_pool = AGS_TASK_THREAD(task_thread)->thread_pool;

  config = application_context->config;
  
  pthread_mutex_unlock(application_mutex);

  gui_thread = ags_thread_find_type(audio_loop,
				    AGS_TYPE_GUI_THREAD);

  /* open file */
  if(g_strcmp0(ags_config_get_value(config,
				    AGS_CONFIG_GENERIC,
				    "simple-file\0"),
		 "false\0")){
    AgsSimpleFile *simple_file;

    AgsSimpleFileRead *simple_file_read;
      
    GError *error;

    simple_file = (AgsSimpleFile *) g_object_new(AGS_TYPE_SIMPLE_FILE,
						 "application-context\0", application_context,
						 "filename\0", filename,
						 NULL);
    error = NULL;
    ags_simple_file_open(simple_file,
			 &error);

    if(error != NULL){
      ags_show_file_error(filename,
			  error);
      exit(-1);
    }
    
    /* start engine */
    pthread_mutex_lock(audio_loop->start_mutex);
    
    start_queue = NULL;
    start_queue = g_list_prepend(start_queue,
				 task_thread);
    start_queue = g_list_prepend(start_queue,
				 gui_thread);
    g_atomic_pointer_set(&(audio_loop->start_queue),
			 start_queue);
  
    pthread_mutex_unlock(audio_loop->start_mutex);

    /* start audio loop and thread pool */
    ags_thread_start(audio_loop);

    ags_thread_pool_start(thread_pool);

    if(!single_thread){
      /* wait for audio loop */
      pthread_mutex_lock(audio_loop->start_mutex);

      if(g_atomic_int_get(&(audio_loop->start_wait)) == TRUE){	
	g_atomic_int_set(&(audio_loop->start_done),
			 FALSE);
      
	while(g_atomic_int_get(&(audio_loop->start_wait)) == TRUE &&
	      g_atomic_int_get(&(audio_loop->start_done)) == FALSE){
	  pthread_cond_wait(audio_loop->start_cond,
			    audio_loop->start_mutex);
	}
      }
    
      pthread_mutex_unlock(audio_loop->start_mutex);

      /* wait for gui thread */
      pthread_mutex_lock(gui_thread->start_mutex);

      if(g_atomic_int_get(&(gui_thread->start_done)) == FALSE){
      
	g_atomic_int_set(&(gui_thread->start_wait),
			 TRUE);

	while(g_atomic_int_get(&(gui_thread->start_done)) == FALSE){
	  g_atomic_int_set(&(gui_thread->start_wait),
			   TRUE);
	
	  pthread_cond_wait(gui_thread->start_cond,
			    gui_thread->start_mutex);
	}
      }
    
      pthread_mutex_unlock(gui_thread->start_mutex);

      /* autosave thread */
      if(!g_strcmp0(ags_config_get_value(config,
					 AGS_CONFIG_GENERIC,
					 "autosave-thread\0"),
		    "true\0")){
	pthread_mutex_lock(audio_loop->start_mutex);

	start_queue = g_atomic_pointer_get(&(audio_loop->start_queue));
	start_queue = g_list_prepend(start_queue,
				     task_thread);

	g_atomic_pointer_set(&(audio_loop->start_queue),
			     start_queue);
	
	pthread_mutex_unlock(audio_loop->start_mutex);
      }
    
      /* now start read task */
      simple_file_read = ags_simple_file_read_new(simple_file);
      ags_task_thread_append_task(task_thread,
				  simple_file_read);
    }else{
      AgsFile *file;

      GError *error;
    
      file = g_object_new(AGS_TYPE_FILE,
			  "application-context\0", application_context,
			  "filename\0", filename,
			  NULL);
      error = NULL;
      ags_file_open(file,
		    &error);

      if(error != NULL){
	ags_show_file_error(filename,
			    error);
	exit(-1);
      }
    
      ags_file_read(file);
      ags_file_close(file);
    }
  }
  
  if(!single_thread){
    /* join gui thread */
    pthread_join(*(gui_thread->thread),
		 NULL);
  }
}

timer_t*
ags_timer_setup()
{
  timer_t *timer_id;

  timer_id = (timer_t *) malloc(sizeof(timer_t));
  
  /* create timer */
  ags_sigact_timer.sa_flags = SA_SIGINFO;
  ags_sigact_timer.sa_sigaction = ags_signal_handler_timer;
  sigemptyset(&ags_sigact_timer.sa_mask);
  
  if(sigaction(SIGRTMIN, &ags_sigact_timer, NULL) == -1){
    perror("sigaction\0");
    exit(EXIT_FAILURE);
  }
  
  /* Block timer signal temporarily */
  sigemptyset(&ags_timer_mask);
  sigaddset(&ags_timer_mask, SIGRTMIN);
  
  if(sigprocmask(SIG_SETMASK, &ags_timer_mask, NULL) == -1){
    perror("sigprocmask\0");
    exit(EXIT_FAILURE);
  }

  /* Create the timer */
  ags_sev_timer.sigev_notify = SIGEV_SIGNAL;
  ags_sev_timer.sigev_signo = SIGRTMIN;
  ags_sev_timer.sigev_value.sival_ptr = timer_id;
  
  if(timer_create(CLOCK_MONOTONIC, &ags_sev_timer, timer_id) == -1){
    perror("timer_create\0");
    exit(EXIT_FAILURE);
  }

  return(timer_id);
}

void
ags_timer_start(timer_t *timer_id)
{
  struct itimerspec value;

  /* Start the timer */
  its.it_value.tv_sec = 0;
  its.it_value.tv_nsec = NSEC_PER_SEC / AGS_THREAD_MAX_PRECISION;
  its.it_interval.tv_sec = its.it_value.tv_sec;
  its.it_interval.tv_nsec = its.it_value.tv_nsec;

  if(timer_settime(timer_id, 0, &its, NULL) == -1){
    perror("timer_settime\0");
    exit(EXIT_FAILURE);
    
  }

  if(sigprocmask(SIG_UNBLOCK, &ags_timer_mask, NULL) == -1){
    perror("sigprocmask\0");
    exit(EXIT_FAILURE);
  }
}

void
ags_timer_launch(timer_t *timer_id,
		 gboolean single_thread)
{
  AgsMutexManager *mutex_manager;
  AgsThread *audio_loop, *gui_thread, *task_thread;
  AgsThreadPool *thread_pool;

  AgsConfig *config;

  GList *start_queue;  

  pthread_mutex_t *application_mutex;

  mutex_manager = ags_mutex_manager_get_instance();
  application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);

  /* get threads, thread pool and config */
  pthread_mutex_lock(application_mutex);
  
  audio_loop = application_context->main_loop;
  task_thread = application_context->task_thread;
  thread_pool = AGS_TASK_THREAD(task_thread)->thread_pool;

  config = application_context->config;
  
  pthread_mutex_unlock(application_mutex);
  
  gui_thread = ags_thread_find_type(audio_loop,
				    AGS_TYPE_GUI_THREAD);

  /* start queue */
  pthread_mutex_lock(audio_loop->start_mutex);
    
  start_queue = NULL;
  start_queue = g_list_prepend(start_queue,
			       task_thread);
  start_queue = g_list_prepend(start_queue,
			       gui_thread);
  g_atomic_pointer_set(&(audio_loop->start_queue),
		       start_queue);

  pthread_mutex_unlock(audio_loop->start_mutex);
    
  /* start audio loop and thread pool */
  ags_thread_start(audio_loop);

  ags_thread_pool_start(thread_pool);

  /* Start the timer */
  ags_timer_start(timer_id);

  if(!single_thread){
    /* wait for audio loop */
    pthread_mutex_lock(audio_loop->start_mutex);

    if(g_atomic_int_get(&(audio_loop->start_wait)) == TRUE){
	
      g_atomic_int_set(&(audio_loop->start_done),
		       FALSE);
      
      while(g_atomic_int_get(&(audio_loop->start_wait)) == TRUE &&
	    g_atomic_int_get(&(audio_loop->start_done)) == FALSE){
	pthread_cond_wait(audio_loop->start_cond,
			  audio_loop->start_mutex);
      }
    }
  
    pthread_mutex_unlock(audio_loop->start_mutex);
    
    /* wait for gui thread */
    pthread_mutex_lock(gui_thread->start_mutex);

    if(g_atomic_int_get(&(gui_thread->start_done)) == FALSE){
      
      g_atomic_int_set(&(gui_thread->start_wait),
		       TRUE);

      while(g_atomic_int_get(&(gui_thread->start_done)) == FALSE){
	g_atomic_int_set(&(gui_thread->start_wait),
			 TRUE);
	
	pthread_cond_wait(gui_thread->start_cond,
			  gui_thread->start_mutex);
      }
    }
    
    pthread_mutex_unlock(gui_thread->start_mutex);

    /* autosave thread */
    if(!g_strcmp0(ags_config_get_value(config,
				       AGS_CONFIG_GENERIC,
				       "autosave-thread\0"),
		  "true\0")){
      pthread_mutex_lock(audio_loop->start_mutex);

      start_queue = g_atomic_pointer_get(&(audio_loop->start_queue));
      start_queue = g_list_prepend(start_queue,
				   task_thread);

      g_atomic_pointer_set(&(audio_loop->start_queue),
			   start_queue);
	
      pthread_mutex_unlock(audio_loop->start_mutex);
    }

    /* join gui thread */
    pthread_join(*(gui_thread->thread),
		 NULL);
  }else{
    AgsSingleThread *single_thread;

    /* single thread */
    single_thread = ags_single_thread_new((GObject *) ags_sound_provider_get_soundcard(application_context)->data);

    /* add known threads to single_thread */
    ags_thread_add_child(AGS_THREAD(single_thread),
			 audio_loop);

    /* autosave thread */
    if(!g_strcmp0(ags_config_get_value(config,
				       AGS_CONFIG_GENERIC,
				       "autosave-thread\0"),
		  "true\0")){
      pthread_mutex_lock(audio_loop->start_mutex);

      start_queue = g_atomic_pointer_get(&(audio_loop->start_queue));
      start_queue = g_list_prepend(start_queue,
				   task_thread);

      g_atomic_pointer_set(&(audio_loop->start_queue),
			   start_queue);
	
      pthread_mutex_unlock(audio_loop->start_mutex);
    }

    /* start thread tree */
    ags_thread_start((AgsThread *) single_thread);
  }
}

void
ags_timer_launch_filename(timer_t *timer_id, gchar *filename,
			  gboolean single_thread)
{
  AgsMutexManager *mutex_manager;
  AgsThread *audio_loop, *gui_thread, *task_thread;
  AgsThreadPool *thread_pool;

  AgsConfig *config;
      
  GList *start_queue;

  pthread_mutex_t *application_mutex;

  mutex_manager = ags_mutex_manager_get_instance();
  application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);

  /* get thread, thread pool and config */
  pthread_mutex_lock(application_mutex);

  audio_loop = application_context->main_loop;
  task_thread = application_context->task_thread;
  thread_pool = AGS_TASK_THREAD(task_thread)->thread_pool;

  config = application_context->config;
  
  pthread_mutex_unlock(application_mutex);
  
  gui_thread = ags_thread_find_type(audio_loop,
				    AGS_TYPE_GUI_THREAD);

  /* open file */
  if(g_strcmp0(ags_config_get_value(config,
				    AGS_CONFIG_GENERIC,
				    "simple-file\0"),
	       "false\0")){
    AgsSimpleFile *simple_file;

    AgsSimpleFileRead *simple_file_read;

    GError *error;    

    simple_file = (AgsSimpleFile *) g_object_new(AGS_TYPE_SIMPLE_FILE,
						 "application-context\0", application_context,
						 "filename\0", filename,
						 NULL);
    error = NULL;
    ags_simple_file_open(simple_file,
			 &error);

    if(error != NULL){
      ags_show_file_error(filename,
			  error);
      exit(-1);
    }

    /* start queue */
    pthread_mutex_lock(audio_loop->start_mutex);
    
    start_queue = NULL;
    start_queue = g_list_prepend(start_queue,
				 task_thread);
    start_queue = g_list_prepend(start_queue,
				 gui_thread);
    g_atomic_pointer_set(&(audio_loop->start_queue),
			 start_queue);

    pthread_mutex_unlock(audio_loop->start_mutex);
  
    /* start audio loop and thread pool*/
    ags_thread_start(audio_loop);

    ags_thread_pool_start(thread_pool);

    /* Start the timer */
    ags_timer_start(timer_id);

    if(!single_thread){
      /* wait for audio loop */
      pthread_mutex_lock(audio_loop->start_mutex);

      if(g_atomic_int_get(&(audio_loop->start_wait)) == TRUE){
	
	g_atomic_int_set(&(audio_loop->start_done),
			 FALSE);
      
	while(g_atomic_int_get(&(audio_loop->start_wait)) == TRUE &&
	      g_atomic_int_get(&(audio_loop->start_done)) == FALSE){
	  pthread_cond_wait(audio_loop->start_cond,
			    audio_loop->start_mutex);
	}
      }
  
      pthread_mutex_unlock(audio_loop->start_mutex);
    
      /* wait for gui thread */
      pthread_mutex_lock(gui_thread->start_mutex);

      if(g_atomic_int_get(&(gui_thread->start_done)) == FALSE){
      
	g_atomic_int_set(&(gui_thread->start_wait),
			 TRUE);

	while(g_atomic_int_get(&(gui_thread->start_done)) == FALSE){
	  g_atomic_int_set(&(gui_thread->start_wait),
			   TRUE);
	
	  pthread_cond_wait(gui_thread->start_cond,
			    gui_thread->start_mutex);
	}
      }
    
      pthread_mutex_unlock(gui_thread->start_mutex);

      /* autosave thread */
      if(!g_strcmp0(ags_config_get_value(config,
					 AGS_CONFIG_GENERIC,
					 "autosave-thread\0"),
		    "true\0")){
	pthread_mutex_lock(audio_loop->start_mutex);

	start_queue = g_atomic_pointer_get(&(audio_loop->start_queue));
	start_queue = g_list_prepend(start_queue,
				     task_thread);

	g_atomic_pointer_set(&(audio_loop->start_queue),
			     start_queue);
	
	pthread_mutex_unlock(audio_loop->start_mutex);
      }
    }
    
    /* now start read task */
    simple_file_read = ags_simple_file_read_new(simple_file);
    ags_task_thread_append_task(task_thread,
				simple_file_read);
  }else{
    AgsFile *file;

    GError *error;
    
    file = g_object_new(AGS_TYPE_FILE,
			"application-context\0", application_context,
			"filename\0", filename,
			NULL);
    error = NULL;
    ags_file_open(file,
		  &error);

    if(error != NULL){
      ags_show_file_error(filename,
			  error);
      exit(-1);
    }
    
    ags_file_read(file);
      
    /* Start the timer */
    ags_timer_start(timer_id);
    
    ags_file_close(file);
  }
  
  if(!single_thread){
    /* join gui thread */
    pthread_join(*(gui_thread->thread),
		 NULL);
  }
}

void
ags_show_file_error(gchar *filename,
		    GError *error)
{
  GtkDialog *dialog;
      
  g_warning("could not parse file %s\0", filename);
      
  dialog = gtk_message_dialog_new(NULL,
				  0,
				  GTK_MESSAGE_WARNING,
				  GTK_BUTTONS_OK,
				  "Failed to open '%s'\0",
				  filename);
  gtk_widget_show_all(dialog);
  g_signal_connect(dialog, "response\0",
		   G_CALLBACK(gtk_main_quit), NULL);
  gtk_main();
}

int
main(int argc, char **argv)
{  
  AgsLadspaManager *ladspa_manager;
  AgsDssiManager *dssi_manager;
  AgsLv2Manager *lv2_manager;;

  AgsConfig *config;

  gchar *filename;
  gchar *str;

  gboolean autosave_thread_enabled;
  gboolean single_thread_enabled;
  gboolean jack_enabled;
  guint i;

  struct sched_param param;
  struct rlimit rl;
  struct sigaction sa;

  int result;

  const rlim_t kStackSize = 64L * 1024L * 1024L;   // min stack size = 64 Mb

#ifdef AGS_USE_TIMER
  timer_t *timer_id
#endif
  
  putenv("LC_ALL=C\0");
  putenv("LANG=C\0");

  autosave_thread_enabled = FALSE;
  single_thread_enabled = FALSE;
  
  //  mtrace();
  atexit(ags_signal_cleanup);

  result = getrlimit(RLIMIT_STACK, &rl);

  /* set stack size 64M */
  if(result == 0){
    if(rl.rlim_cur < kStackSize){
      rl.rlim_cur = kStackSize;
      result = setrlimit(RLIMIT_STACK, &rl);

      if(result != 0){
	//TODO:JK
      }
    }
  }

  /* Ignore interactive and job-control signals.  */
  signal(SIGINT, SIG_IGN);
  signal(SIGQUIT, SIG_IGN);
  signal(SIGTSTP, SIG_IGN);
  signal(SIGTTIN, SIG_IGN);
  signal(SIGTTOU, SIG_IGN);
  signal(SIGCHLD, SIG_IGN);

  ags_sigact.sa_handler = ags_signal_handler;
  sigemptyset(&ags_sigact.sa_mask);
  ags_sigact.sa_flags = 0;
  sigaction(SIGINT, &ags_sigact, (struct sigaction *) NULL);
  sigaction(SA_RESTART, &ags_sigact, (struct sigaction *) NULL);

#ifdef AGS_USE_TIMER
  timer_id = ags_timer_setup();
#endif
  
  /* parse command line parameter */
  filename = NULL;

  for(i = 0; i < argc; i++){
    if(!strncmp(argv[i], "--help\0", 7)){
      printf("GSequencer is an audio sequencer and notation editor\n\n\0");

      printf("Usage:\n\t%s\n\t%s\n\t%s\n\t%s\n\n",
	     "Report bugs to <jkraehemann@gmail.com>\n\0",
	     "--filename file     open file\0",
	     "--single-thread     run in single thread mode\0",     
	     "--help              display this help and exit\0",
	     "--version           output version information and exit\0");
      
      exit(0);
    }else if(!strncmp(argv[i], "--version\0", 10)){
      printf("GSequencer %s\n\n\0", AGS_VERSION);
      
      printf("%s\n%s\n%s\n\n\0",
	     "Copyright (C) 2005-2015 Joël Krähemann\0",
	     "This is free software; see the source for copying conditions.  There is NO\0",
	     "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\0");
      
      printf("Written by Joël Krähemann\n\0");

      exit(0);
    }else if(!strncmp(argv[i], "--single-thread\0", 16)){
      single_thread_enabled = TRUE;
    }else if(!strncmp(argv[i], "--filename\0", 11)){
      filename = argv[i + 1];
      i++;
    }
  }

  /**/
  LIBXML_TEST_VERSION;

  //ao_initialize();
  
  gdk_threads_enter();
  //  g_thread_init(NULL);
  gtk_init(&argc, &argv);
  ipatch_init();
  
  /* setup */
  ags_setup(argc, argv);
  config = application_context->config;

  /* JACK */
  str = ags_config_get_value(config,
			     AGS_CONFIG_SOUNDCARD,
			     "jack\0");
  jack_enabled = (str != NULL && !g_ascii_strncasecmp(str, "enabled\0", 8)) ? TRUE: FALSE;

  if(str != NULL){
    free(str);
  }
  
  if(jack_enabled){
    //    jackctl_setup_signals(0);
  }

  /* autosave thread */
  str = ags_config_get_value(config,
			     AGS_CONFIG_GENERIC,
			     "autosave-thread\0");
  autosave_thread_enabled = (str != NULL && !g_ascii_strncasecmp(str, "true\0", 8)) ? TRUE: FALSE;

  /* launch GUI */
  if(filename != NULL){
#ifdef AGS_USE_TIMER
    ags_timer_launch_filename(timer_id,
			      filename,
			      single_thread_enabled);
#else
    ags_launch_filename(filename,
			single_thread_enabled);
#endif
  }else{
#ifdef AGS_USE_TIMER
    ags_timer_launch(timer_id,
		     single_thread_enabled);
#else
    ags_launch(single_thread_enabled);
#endif
  }
  
  /* free managers */
  ladspa_manager = ags_ladspa_manager_get_instance();
  g_object_unref(ladspa_manager);

  dssi_manager = ags_dssi_manager_get_instance();
  g_object_unref(dssi_manager);

  lv2_manager = ags_lv2_manager_get_instance();
  g_object_unref(lv2_manager);
  
  /* delete autosave file */
  if(autosave_thread_enabled){
    GFile *autosave_file;

    struct passwd *pw;

    gchar *autosave_filename;

    uid_t uid;
    
    uid = getuid();
    pw = getpwuid(uid);

    if(g_strcmp0(ags_config_get_value(application_context->config,
				      AGS_CONFIG_GENERIC,
				      "simple-file\0"),
		 "false\0")){

      gchar *filename, *offset;
    
      filename = g_strdup_printf("%s/%s/%s\0",
				 pw->pw_dir,
				 AGS_DEFAULT_DIRECTORY,
				 AGS_SIMPLE_AUTOSAVE_THREAD_DEFAULT_FILENAME);

      if((offset = strstr(filename,
			  "{PID}")) != NULL){
	gchar *tmp;

	tmp = g_strndup(filename,
			offset - filename);
	autosave_filename = g_strdup_printf("%s%d%s",
					    tmp,
					    getpid(),
					    &(offset[5]));

	g_free(tmp);
	g_free(filename);
      }
    }else{
      autosave_filename = g_strdup_printf("%s/%s/%d-%s\0",
					  pw->pw_dir,
					  AGS_DEFAULT_DIRECTORY,
					  getpid(),
					  AGS_AUTOSAVE_THREAD_DEFAULT_FILENAME);
    }

    autosave_file = g_file_new_for_path(autosave_filename);
  
    if(g_file_query_exists(autosave_file,
			   NULL)){
      g_file_delete(autosave_file,
		    NULL,
		    NULL);
    }
    
    g_free(autosave_filename);
    g_object_unref(autosave_file);
  }
  
  //  muntrace();

  return(0);
}
