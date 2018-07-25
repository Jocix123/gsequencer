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

#include <ags/plugin/ags_dssi_manager.h>

#include <ags/libags.h>

#include <ags/plugin/ags_base_plugin.h>

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include <string.h>

#include <dssi.h>

#include <ags/config.h>

void ags_dssi_manager_class_init(AgsDssiManagerClass *dssi_manager);
void ags_dssi_manager_init (AgsDssiManager *dssi_manager);
void ags_dssi_manager_dispose(GObject *gobject);
void ags_dssi_manager_finalize(GObject *gobject);

/**
 * SECTION:ags_dssi_manager
 * @short_description: Singleton pattern to organize DSSI
 * @title: AgsDssiManager
 * @section_id:
 * @include: ags/plugin/ags_dssi_manager.h
 *
 * The #AgsDssiManager loads/unloads DSSI plugins.
 */

static gpointer ags_dssi_manager_parent_class = NULL;

static pthread_mutex_t ags_dssi_manager_class_mutex = PTHREAD_MUTEX_INITIALIZER;

AgsDssiManager *ags_dssi_manager = NULL;
gchar **ags_dssi_default_path = NULL;

GType
ags_dssi_manager_get_type (void)
{
  static GType ags_type_dssi_manager = 0;

  if(!ags_type_dssi_manager){
    static const GTypeInfo ags_dssi_manager_info = {
      sizeof (AgsDssiManagerClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_dssi_manager_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsDssiManager),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_dssi_manager_init,
    };

    ags_type_dssi_manager = g_type_register_static(G_TYPE_OBJECT,
						   "AgsDssiManager",
						   &ags_dssi_manager_info,
						   0);
  }

  return (ags_type_dssi_manager);
}

void
ags_dssi_manager_class_init(AgsDssiManagerClass *dssi_manager)
{
  GObjectClass *gobject;

  ags_dssi_manager_parent_class = g_type_class_peek_parent(dssi_manager);

  /* GObjectClass */
  gobject = (GObjectClass *) dssi_manager;

  gobject->dispose = ags_dssi_manager_dispose;
  gobject->finalize = ags_dssi_manager_finalize;
}

void
ags_dssi_manager_init(AgsDssiManager *dssi_manager)
{
  /* dssi manager mutex */
  dssi_manager->obj_mutexattr = (pthread_mutexattr_t *) malloc(sizeof(pthread_mutexattr_t));
  pthread_mutexattr_init(dssi_manager->obj_mutexattr);
  pthread_mutexattr_settype(dssi_manager->obj_mutexattr,
			    PTHREAD_MUTEX_RECURSIVE);

#ifdef __linux__
  pthread_mutexattr_setprotocol(dssi_manager->obj_mutexattr,
				PTHREAD_PRIO_INHERIT);
#endif

  dssi_manager->obj_mutex = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(dssi_manager->obj_mutex,
		     dssi_manager->obj_mutexattr);

  /* dssi plugin and path */
  dssi_manager->dssi_plugin = NULL;

  if(ags_dssi_default_path == NULL){
    gchar *dssi_env;

    if((dssi_env = getenv("DSSI_PATH")) != NULL){
      gchar *iter, *next;
      guint i;
      
      ags_dssi_default_path = (gchar **) malloc(sizeof(gchar *));

      iter = dssi_env;
      i = 0;
      
      while((next = index(iter, ':')) != NULL){
	ags_dssi_default_path = (gchar **) realloc(ags_dssi_default_path,
						   (i + 2) * sizeof(gchar *));
	ags_dssi_default_path[i] = g_strndup(iter,
					     next - iter);

	iter = next + 1;
	i++;
      }

      if(*iter != '\0'){
	ags_dssi_default_path = (gchar **) realloc(ags_dssi_default_path,
						   (i + 2) * sizeof(gchar *));
	ags_dssi_default_path[i] = g_strdup(iter);

	i++;	
      }

      ags_dssi_default_path[i] = NULL;
    }else{
      gchar *home_dir;
      guint i;

#ifdef AGS_MAC_BUNDLE
      if((home_dir = getenv("HOME")) != NULL){
	ags_dssi_default_path = (gchar **) malloc(7 * sizeof(gchar *));
      }else{
	ags_dssi_default_path = (gchar **) malloc(6 * sizeof(gchar *));
      }
#else
      if((home_dir = getenv("HOME")) != NULL){
	ags_dssi_default_path = (gchar **) malloc(6 * sizeof(gchar *));
      }else{
	ags_dssi_default_path = (gchar **) malloc(5 * sizeof(gchar *));
      }
#endif
      
      i = 0;

#ifdef AGS_MAC_BUNDLE
      ags_dssi_default_path[i++] = g_strdup_printf("%s/dssi",
						   getenv("GSEQUENCER_PLUGIN_DIR"));
#endif
      
      ags_dssi_default_path[i++] = g_strdup("/usr/lib64/dssi");
      ags_dssi_default_path[i++] = g_strdup("/usr/local/lib64/dssi");
      ags_dssi_default_path[i++] = g_strdup("/usr/lib/dssi");
      ags_dssi_default_path[i++] = g_strdup("/usr/local/lib/dssi");

      if(home_dir != NULL){
	ags_dssi_default_path[i++] = g_strdup_printf("%s/.dssi",
						     home_dir);
      }
    
      ags_dssi_default_path[i++] = NULL;
    }
  }
}

void
ags_dssi_manager_dispose(GObject *gobject)
{
  AgsDssiManager *dssi_manager;

  dssi_manager = AGS_DSSI_MANAGER(gobject);

  if(dssi_manager->dssi_plugin != NULL){
    g_list_free_full(dssi_manager->dssi_plugin,
		     (GDestroyNotify) g_object_unref);

    dssi_manager->dssi_plugin = NULL;
  }
  
  /* call parent */
  G_OBJECT_CLASS(ags_dssi_manager_parent_class)->dispose(gobject);
}

void
ags_dssi_manager_finalize(GObject *gobject)
{
  AgsDssiManager *dssi_manager;

  GList *dssi_plugin;

  dssi_manager = AGS_DSSI_MANAGER(gobject);

  pthread_mutex_destroy(dssi_manager->obj_mutex);
  free(dssi_manager->obj_mutex);

  pthread_mutexattr_destroy(dssi_manager->obj_mutexattr);
  free(dssi_manager->obj_mutexattr);
  
  dssi_plugin = dssi_manager->dssi_plugin;

  g_list_free_full(dssi_plugin,
		   (GDestroyNotify) g_object_unref);

  if(dssi_manager == ags_dssi_manager){
    ags_dssi_manager = NULL;
  }
  
  /* call parent */
  G_OBJECT_CLASS(ags_dssi_manager_parent_class)->finalize(gobject);
}

/**
 * ags_dssi_manager_get_class_mutex:
 * 
 * Get class mutex.
 * 
 * Returns: the class mutex of #AgsDssiManager
 * 
 * Since: 2.0.0
 */
pthread_mutex_t*
ags_dssi_manager_get_class_mutex()
{
  return(&ags_dssi_manager_class_mutex);
}

/**
 * ags_dssi_manager_get_default_path:
 * 
 * Get dssi manager default plugin path.
 *
 * Returns: the plugin default search path as a string vector
 * 
 * Since: 2.0.0
 */
gchar**
ags_dssi_manager_get_default_path()
{
  return(ags_dssi_default_path);
}

/**
 * ags_dssi_manager_set_default_path:
 * @default_path: the string vector array to use as default path
 * 
 * Set dssi manager default plugin path.
 * 
 * Since: 2.0.0
 */
void
ags_dssi_manager_set_default_path(gchar** default_path)
{
  ags_dssi_default_path = default_path;
}

/**
 * ags_dssi_manager_get_filenames:
 * @dssi_manager: the #AgsDssiManager
 * 
 * Retrieve all filenames
 *
 * Returns: a %NULL-terminated array of filenames
 *
 * Since: 2.0.0
 */
gchar**
ags_dssi_manager_get_filenames(AgsDssiManager *dssi_manager)
{
  GList *start_dssi_plugin, *dssi_plugin;

  gchar **filenames;

  guint i;
  gboolean contains_filename;

  pthread_mutex_t *dssi_manager_mutex;
  pthread_mutex_t *base_plugin_mutex;

  if(!AGS_DSSI_MANAGER(dssi_manager)){
    return(NULL);
  }
  
  /* get dssi manager mutex */
  pthread_mutex_lock(ags_dssi_manager_get_class_mutex());
  
  dssi_manager_mutex = dssi_manager->obj_mutex;
  
  pthread_mutex_unlock(ags_dssi_manager_get_class_mutex());

  /* collect */
  pthread_mutex_lock(dssi_manager_mutex);

  dssi_plugin = 
    start_dssi_plugin = g_list_copy(dssi_manager->dssi_plugin);

  pthread_mutex_unlock(dssi_manager_mutex);

  filenames = NULL;
  
  for(i = 0; dssi_plugin != NULL;){
    gchar *filename;
    
    /* get base plugin mutex */
    pthread_mutex_lock(ags_base_plugin_get_class_mutex());
  
    base_plugin_mutex = AGS_BASE_PLUGIN(dssi_plugin->data)->obj_mutex;
    
    pthread_mutex_unlock(ags_base_plugin_get_class_mutex());

    /* duplicate filename */
    pthread_mutex_lock(base_plugin_mutex);

    filename = g_strdup(AGS_BASE_PLUGIN(dssi_plugin->data)->filename);

    pthread_mutex_unlock(base_plugin_mutex);
    
    if(filenames == NULL){
      filenames = (gchar **) malloc(2 * sizeof(gchar *));

      /* set filename */
      filenames[i] = filename;

      /* terminate */
      filenames[i + 1] = NULL;

      i++;
    }else{
#ifdef HAVE_GLIB_2_44
      contains_filename = g_strv_contains(filenames,
					  filename);
#else
      contains_filename = ags_strv_contains(filenames,
					    filename);
#endif
      
      if(!contains_filename){
	filenames = (gchar **) realloc(filenames,
				       (i + 2) * sizeof(gchar *));
	filenames[i] = filename;
	filenames[i + 1] = NULL;

	i++;
      }else{
	g_free(filename);
      }
      
    }
    
    dssi_plugin = dssi_plugin->next;
  }

  g_list_free(start_dssi_plugin);
  
  return(filenames);
}

/**
 * ags_dssi_manager_find_dssi_plugin:
 * @dssi_manager: the #AgsDssiManager
 * @filename: the filename of the plugin
 * @effect: the effect's name
 *
 * Lookup filename in loaded plugins.
 *
 * Returns: the #AgsDssiPlugin-struct
 *
 * Since: 2.0.0
 */
AgsDssiPlugin*
ags_dssi_manager_find_dssi_plugin(AgsDssiManager *dssi_manager,
				  gchar *filename, gchar *effect)
{
  AgsDssiPlugin *dssi_plugin;
  
  GList *start_list, *list;

  gboolean success;  

  pthread_mutex_t *dssi_manager_mutex;
  pthread_mutex_t *base_plugin_mutex;

  if(!AGS_DSSI_MANAGER(dssi_manager)){
    return(NULL);
  }
  
  /* get dssi manager mutex */
  pthread_mutex_lock(ags_dssi_manager_get_class_mutex());
  
  dssi_manager_mutex = dssi_manager->obj_mutex;
  
  pthread_mutex_unlock(ags_dssi_manager_get_class_mutex());

  /* collect */
  pthread_mutex_lock(dssi_manager_mutex);

  list = 
    start_list = g_list_copy(dssi_manager->dssi_plugin);

  pthread_mutex_unlock(dssi_manager_mutex);

  success = FALSE;
  
  while(list != NULL){
    dssi_plugin = AGS_DSSI_PLUGIN(list->data);

    /* get base plugin mutex */
    pthread_mutex_lock(ags_base_plugin_get_class_mutex());
  
    base_plugin_mutex = AGS_BASE_PLUGIN(dssi_plugin)->obj_mutex;
    
    pthread_mutex_unlock(ags_base_plugin_get_class_mutex());

    /* check filename and effect */
    pthread_mutex_lock(base_plugin_mutex);

    success = (!g_strcmp0(AGS_BASE_PLUGIN(dssi_plugin)->filename,
			  filename) &&
	       !g_strcmp0(AGS_BASE_PLUGIN(dssi_plugin)->effect,
			  effect)) ? TRUE: FALSE;
    
    pthread_mutex_unlock(base_plugin_mutex);
    
    if(success){
      break;
    }

    list = list->next;
  }

  g_list_free(start_list);

  if(!success){
    dssi_plugin = NULL;
  }

  return(dssi_plugin);
}

/**
 * ags_dssi_manager_load_blacklist:
 * @dssi_manager: the #AgsDssiManager
 * @blacklist_filename: the filename as string
 * 
 * Load blacklisted plugin filenames.
 * 
 * Since: 2.0.0
 */
void
ags_dssi_manager_load_blacklist(AgsDssiManager *dssi_manager,
				gchar *blacklist_filename)
{
  pthread_mutex_t *dssi_manager_mutex;

  if(!AGS_DSSI_MANAGER(dssi_manager) ||
     blacklist_filename == NULL){
    return;
  } 
  
  /* get dssi manager mutex */
  pthread_mutex_lock(ags_dssi_manager_get_class_mutex());
  
  dssi_manager_mutex = dssi_manager->obj_mutex;
  
  pthread_mutex_unlock(ags_dssi_manager_get_class_mutex());

  /* fill in */
  pthread_mutex_lock(dssi_manager_mutex);

  if(g_file_test(blacklist_filename,
		 (G_FILE_TEST_EXISTS |
		  G_FILE_TEST_IS_REGULAR))){
    FILE *file;

    gchar *str;
    
    file = fopen(blacklist_filename,
		 "r");

    while(getline(&str, NULL, file) != -1){
      dssi_manager->dssi_plugin_blacklist = g_list_prepend(dssi_manager->dssi_plugin_blacklist,
							   str);
    }
  }

  pthread_mutex_unlock(dssi_manager_mutex);
} 

/**
 * ags_dssi_manager_load_file:
 * @dssi_manager: the #AgsDssiManager
 * @dssi_path: the dssi path
 * @filename: the filename of the plugin
 *
 * Load @filename specified plugin.
 *
 * Since: 2.0.0
 */
void
ags_dssi_manager_load_file(AgsDssiManager *dssi_manager,
			   gchar *dssi_path,
			   gchar *filename)
{
  AgsDssiPlugin *dssi_plugin;

  gchar *path;
  gchar *effect;

  void *plugin_so;
  DSSI_Descriptor_Function dssi_descriptor;
  DSSI_Descriptor *plugin_descriptor;
  unsigned long i;

  pthread_mutex_t *dssi_manager_mutex;

  if(!AGS_IS_DSSI_MANAGER(dssi_manager) ||
     dssi_path == NULL ||
     filename == NULL){
    return;
  }
  
  /* get dssi manager mutex */
  pthread_mutex_lock(ags_dssi_manager_get_class_mutex());
  
  dssi_manager_mutex = dssi_manager->obj_mutex;
  
  pthread_mutex_unlock(ags_dssi_manager_get_class_mutex());

  /* load */
  pthread_mutex_lock(dssi_manager_mutex);

  path = g_strdup_printf("%s/%s",
			 dssi_path,
			 filename);
  
  g_message("ags_dssi_manager.c loading - %s", path);

  plugin_so = dlopen(path,
		     RTLD_NOW);
	
  if(plugin_so == NULL){
    g_warning("ags_dssi_manager.c - failed to load static object file");
      
    dlerror();

    pthread_mutex_unlock(dssi_manager_mutex);
    
    return;
  }

  dssi_descriptor = (DSSI_Descriptor_Function) dlsym(plugin_so,
						     "dssi_descriptor");
    
  if(dlerror() == NULL && dssi_descriptor){
    for(i = 0; (plugin_descriptor = dssi_descriptor(i)) != NULL; i++){
      if(ags_base_plugin_find_effect(dssi_manager->dssi_plugin,
				     path,
				     plugin_descriptor->LADSPA_Plugin->Name) == NULL){
	dssi_plugin = ags_dssi_plugin_new(path,
					  plugin_descriptor->LADSPA_Plugin->Name,
					  i);
	ags_base_plugin_load_plugin((AgsBasePlugin *) dssi_plugin);
	dssi_manager->dssi_plugin = g_list_prepend(dssi_manager->dssi_plugin,
						   dssi_plugin);
      }
    }
  }

  pthread_mutex_unlock(dssi_manager_mutex);

  g_free(path);
}

/**
 * ags_dssi_manager_load_default_directory:
 * @dssi_manager: the #AgsDssiManager
 * 
 * Loads all available plugins.
 *
 * Since: 2.0.0
 */
void
ags_dssi_manager_load_default_directory(AgsDssiManager *dssi_manager)
{
  AgsDssiPlugin *dssi_plugin;

  GDir *dir;

  gchar **dssi_path;
  gchar *filename;

  GError *error;

  if(!AGS_DSSI_MANAGER(dssi_manager)){
    return;
  }

  dssi_path = ags_dssi_default_path;
  
  while(*dssi_path != NULL){
    if(!g_file_test(*dssi_path,
		    G_FILE_TEST_EXISTS)){
      dssi_path++;
      
      continue;
    }

    error = NULL;
    dir = g_dir_open(*dssi_path,
		     0,
		     &error);

    if(error != NULL){
      g_warning("%s", error->message);

      dssi_path++;

      continue;
    }

    while((filename = g_dir_read_name(dir)) != NULL){
      if(g_str_has_suffix(filename,
			  AGS_LIBRARY_SUFFIX) &&
	 !g_list_find_custom(dssi_manager->dssi_plugin_blacklist,
			     filename,
			     strcmp)){
	ags_dssi_manager_load_file(dssi_manager,
				   *dssi_path,
				   filename);
      }
    }
    
    dssi_path++;
  }
}

/**
 * ags_dssi_manager_get_instance:
 *
 * Get instance.
 *
 * Returns: the #AgsDssiManager
 *
 * Since: 2.0.0
 */
AgsDssiManager*
ags_dssi_manager_get_instance()
{
  static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

  pthread_mutex_lock(&(mutex));

  if(ags_dssi_manager == NULL){
    ags_dssi_manager = ags_dssi_manager_new();
  }

  pthread_mutex_unlock(&(mutex));

  return(ags_dssi_manager);
}

/**
 * ags_dssi_manager_new:
 *
 * Create a new instance of #AgsDssiManager
 *
 * Returns: the new #AgsDssiManager
 *
 * Since: 2.0.0
 */
AgsDssiManager*
ags_dssi_manager_new()
{
  AgsDssiManager *dssi_manager;

  dssi_manager = (AgsDssiManager *) g_object_new(AGS_TYPE_DSSI_MANAGER,
						 NULL);

  return(dssi_manager);
}
