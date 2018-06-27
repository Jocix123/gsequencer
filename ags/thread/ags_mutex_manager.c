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

#include <ags/thread/ags_mutex_manager.h>

#include <ags/object/ags_connectable.h>

#include <pthread.h>

void ags_mutex_manager_class_init(AgsMutexManagerClass *mutex_manager);
void ags_mutex_manager_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_mutex_manager_init(AgsMutexManager *mutex_manager);
void ags_mutex_manager_connect(AgsConnectable *connectable);
void ags_mutex_manager_disconnect(AgsConnectable *connectable);
void ags_mutex_manager_finalize(GObject *gobject);

void ags_mutex_manager_destroy_data(gpointer data);

/**
 * SECTION:ags_mutex_manager
 * @short_description: mutex manager
 * @title: AgsMutexManager
 * @section_id:
 * @include: ags/thread/ags_mutex_manager.h
 *
 * The #AgsMutexManager keeps your mutex in a hash table where you can lookup your
 * mutices.
 */

static gpointer ags_mutex_manager_parent_class = NULL;

AgsMutexManager *ags_mutex_manager = NULL;
pthread_mutex_t *ags_application_mutex = NULL;

GType
ags_mutex_manager_get_type()
{
  static volatile gsize g_define_type_id__volatile = 0;

  if(g_once_init_enter (&g_define_type_id__volatile)){
    GType ags_type_mutex_manager;
    
    const GTypeInfo ags_mutex_manager_info = {
      sizeof(AgsMutexManagerClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_mutex_manager_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof(AgsMutexManager),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_mutex_manager_init,
    };

    const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_mutex_manager_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_mutex_manager = g_type_register_static(G_TYPE_OBJECT,
						    "AgsMutexManager",
						    &ags_mutex_manager_info,
						    0);

    g_type_add_interface_static(ags_type_mutex_manager,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);

    g_once_init_leave (&g_define_type_id__volatile, ags_type_mutex_manager);
  }

  return g_define_type_id__volatile;
}

void
ags_mutex_manager_class_init(AgsMutexManagerClass *mutex_manager)
{
  GObjectClass *gobject;
  GParamSpec *param_spec;

  ags_mutex_manager_parent_class = g_type_class_peek_parent(mutex_manager);

  /* GObject */
  gobject = (GObjectClass *) mutex_manager;

  gobject->finalize = ags_mutex_manager_finalize;
}

void
ags_mutex_manager_connectable_interface_init(AgsConnectableInterface *connectable)
{
  connectable->connect = ags_mutex_manager_connect;
  connectable->disconnect = ags_mutex_manager_disconnect;
}

void
ags_mutex_manager_init(AgsMutexManager *mutex_manager)
{
  mutex_manager->lock_object = g_hash_table_new_full(g_direct_hash, g_direct_equal,
						     NULL,
						     (GDestroyNotify) ags_mutex_manager_destroy_data);
}

void
ags_mutex_manager_connect(AgsConnectable *connectable)
{
  /* empty */
}

void
ags_mutex_manager_disconnect(AgsConnectable *connectable)
{
  /* empty */
}

void
ags_mutex_manager_finalize(GObject *gobject)
{
  AgsMutexManager *mutex_manager;

  mutex_manager = AGS_MUTEX_MANAGER(gobject);

  g_hash_table_destroy(mutex_manager->lock_object);

  if(mutex_manager == ags_mutex_manager){
    ags_mutex_manager = NULL;
  }
  
  /* call parent */
  G_OBJECT_CLASS(ags_mutex_manager_parent_class)->finalize(gobject);
}

void
ags_mutex_manager_destroy_data(gpointer data)
{
  pthread_mutex_destroy((pthread_mutex_t *) data);
}

/**
 * ags_mutex_manager_get_application_mutex:
 * @mutex_manager: the #AgsMutexManager
 *
 * Retrieve the application mutex as per process pointer.
 *
 * Returns: the application mutex
 *
 * Since: 1.0.0
 */
pthread_mutex_t*
ags_mutex_manager_get_application_mutex(AgsMutexManager *mutex_manager)
{
  if(ags_application_mutex == NULL){
    pthread_mutexattr_t *attr;

    attr = (pthread_mutexattr_t *) malloc(sizeof(pthread_mutexattr_t));

    pthread_mutexattr_init(attr);
    pthread_mutexattr_settype(attr,
			      PTHREAD_MUTEX_RECURSIVE);

#ifdef __linux__
    pthread_mutexattr_setprotocol(attr,
				  PTHREAD_PRIO_INHERIT);
#endif

    ags_application_mutex = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));

    pthread_mutex_init(ags_application_mutex,
		       attr);
  }
  
  return(ags_application_mutex);
}

/**
 * ags_mutex_manager_insert:
 * @mutex_manager: the #AgsMutexManager
 * @lock_object: the object as key
 * @mutex: the mutex to insert
 *
 * Inserts a mutex into hash map associated with @lock_object.
 * 
 * Returns: %TRUE on success, otherwise %FALSE
 *
 * Since: 1.0.0
 */
gboolean
ags_mutex_manager_insert(AgsMutexManager *mutex_manager,
			 GObject *lock_object, pthread_mutex_t *mutex)
{
  if(mutex_manager == NULL ||
     lock_object == NULL ||
     mutex == NULL){
    return(FALSE);
  }

  g_hash_table_insert(mutex_manager->lock_object,
		      lock_object, mutex);

  return(TRUE);
}

/**
 * ags_mutex_manager_remove:
 * @mutex_manager: the #AgsMutexManager
 * @lock_object: the object to remove
 * 
 * Removes a lock associated with @lock_object.3
 *
 * Returns: %TRUE as successfully removed, otherwise %FALSE
 *
 * Since: 1.0.0
 */
gboolean
ags_mutex_manager_remove(AgsMutexManager *mutex_manager,
			 GObject *lock_object)
{
  pthread_mutex_t *mutex;

  mutex = g_hash_table_lookup(mutex_manager->lock_object,
			      lock_object);

  if(mutex == NULL ||
     pthread_mutex_trylock(mutex) != 0){
    return(FALSE);
  }

  g_hash_table_remove(mutex_manager->lock_object,
		      lock_object);

  pthread_mutex_unlock(mutex);

  pthread_mutex_destroy(mutex);
  free(mutex);
  
  return(TRUE);
}

/**
 * ags_mutex_manager_lookup:
 * @mutex_manager: the #AgsMutexManager
 * @lock_object: the object to lookup
 *
 * Lookup a mutex associated with @lock_object in @mutex_manager
 *
 * Returns: the mutex on success, else %NULL
 *
 * Since: 1.0.0
 */
pthread_mutex_t*
ags_mutex_manager_lookup(AgsMutexManager *mutex_manager,
			 GObject *lock_object)
{
  pthread_mutex_t *mutex;

  mutex = (pthread_mutex_t *) g_hash_table_lookup(mutex_manager->lock_object,
						  lock_object);

  return(mutex);
}

/**
 * ags_mutex_manager_get_instance:
 * 
 * Singleton function to optain the mutex manager instance.
 *
 * Returns: an instance of #AgsMutexManager
 *
 * Since: 1.0.0
 */
AgsMutexManager*
ags_mutex_manager_get_instance()
{
  if(ags_mutex_manager == NULL){
    ags_mutex_manager = ags_mutex_manager_new();
  }

  return(ags_mutex_manager);
}

/**
 * ags_mutex_manager_new:
 *
 * Instantiate a mutex manager. 
 *
 * Returns: a new #AgsMutexManager
 *
 * Since: 1.0.0
 */
AgsMutexManager*
ags_mutex_manager_new()
{
  AgsMutexManager *mutex_manager;

  mutex_manager = (AgsMutexManager *) g_object_new(AGS_TYPE_MUTEX_MANAGER,
						   NULL);

  return(mutex_manager);
}

