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

#include <ags/object/ags_config.h>

#include <ags/object/ags_marshal.h>
#include <ags/object/ags_application_context.h>

#include <gio/gio.h>

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pwd.h>

#include <ags/config.h>
#include <ags/i18n.h>

void ags_config_class_init(AgsConfigClass *config_class);
void ags_config_init(AgsConfig *config);
void ags_config_set_property(GObject *gobject,
			     guint prop_id,
			     const GValue *value,
			     GParamSpec *param_spec);
void ags_config_get_property(GObject *gobject,
			     guint prop_id,
			     GValue *value,
			     GParamSpec *param_spec);
void ags_config_dispose(GObject *gobject);
void ags_config_finalize(GObject *gobject);

gchar* ags_config_get_version(AgsConfig *config);
void ags_config_set_version(AgsConfig *config, gchar *version);
gchar* ags_config_get_build_id(AgsConfig *config);
void ags_config_set_build_id(AgsConfig *config, gchar *build_id);

void ags_config_real_load_defaults(AgsConfig *config);
void ags_config_real_set_value(AgsConfig *config, gchar *group, gchar *key, gchar *value);
gchar* ags_config_real_get_value(AgsConfig *config, gchar *group, gchar *key);

/**
 * SECTION:ags_config
 * @short_description: Config Advanced Gtk+ Sequencer
 * @title: AgsConfig
 * @section_id:
 * @include: ags/object/ags_config.h
 *
 * #AgsConfig provides configuration to Advanced Gtk+ Sequencer.
 */

enum{
  LOAD_DEFAULTS,
  SET_VALUE,
  GET_VALUE,
  LAST_SIGNAL,
};

enum{
  PROP_0,
  PROP_APPLICATION_CONTEXT,
};

static gpointer ags_config_parent_class = NULL;
static guint config_signals[LAST_SIGNAL];

AgsConfig *ags_config = NULL;

static pthread_mutex_t ags_config_class_mutex = PTHREAD_MUTEX_INITIALIZER;

GType
ags_config_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;

  if(g_once_init_enter (&g_define_type_id__volatile)){
    GType ags_type_config = 0;

    static const GTypeInfo ags_config_info = {
      sizeof (AgsConfigClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_config_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsConfig),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_config_init,
    };

    ags_type_config = g_type_register_static(G_TYPE_OBJECT,
					     "AgsConfig",
					     &ags_config_info,
					     0);

    g_once_init_leave(&g_define_type_id__volatile, ags_type_config);
  }

  return g_define_type_id__volatile;
}

void
ags_config_class_init(AgsConfigClass *config)
{
  GObjectClass *gobject;
  GParamSpec *param_spec;

  ags_config_parent_class = g_type_class_peek_parent(config);

  /* GObjectClass */
  gobject = (GObjectClass *) config;

  gobject->set_property = ags_config_set_property;
  gobject->get_property = ags_config_get_property;

  gobject->dispose = ags_config_dispose;
  gobject->finalize = ags_config_finalize;

  /* properties */
  /**
   * AgsConfig:application-context:
   *
   * The assigned application context.
   * 
   * Since: 2.0.0
   */
  param_spec = g_param_spec_object("application-context",
				   i18n_pspec("application context of config"),
				   i18n_pspec("The application context which this config is packed into"),
				   AGS_TYPE_APPLICATION_CONTEXT,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_APPLICATION_CONTEXT,
				  param_spec);

  /* AgsConfigClass */
  config->load_defaults = ags_config_real_load_defaults;
  config->set_value = ags_config_real_set_value;
  config->get_value = ags_config_real_get_value;

  /* signals */
  /**
   * AgsConfig::load-defaults:
   * @config: the object to resolve
   *
   * The ::load-defaults signal notifies about loading defaults
   *
   * Since: 2.0.0
   */
  config_signals[LOAD_DEFAULTS] =
    g_signal_new("load-defaults",
		 G_TYPE_FROM_CLASS (config),
		 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET (AgsConfigClass, load_defaults),
		 NULL, NULL,
		 g_cclosure_marshal_VOID__VOID,
		 G_TYPE_NONE, 0);

  /**
   * AgsConfig::set-value:
   * @config: the object to resolve
   * @group: the group to apply to
   * @key: the key to set
   * @value: the value to apply
   *
   * The ::set-value signal notifies about value been setting.
   *
   * Since: 2.0.0
   */
  config_signals[SET_VALUE] =
    g_signal_new("set-value",
		 G_TYPE_FROM_CLASS (config),
		 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET (AgsConfigClass, set_value),
		 NULL, NULL,
		 ags_cclosure_marshal_VOID__STRING_STRING_STRING,
		 G_TYPE_NONE, 3,
		 G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

  /**
   * AgsConfig::get-value:
   * @config: the object to resolve
   * @group: the group to retrieve from
   * @key: the key to get
   *
   * The ::get-value signal notifies about value been getting.
   *
   * Returns: the value
   *
   * Since: 2.0.0
   */
  config_signals[GET_VALUE] =
    g_signal_new("get-value",
		 G_TYPE_FROM_CLASS (config),
		 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET (AgsConfigClass, get_value),
		 NULL, NULL,
		 ags_cclosure_marshal_STRING__STRING_STRING,
		 G_TYPE_STRING, 2,
		 G_TYPE_STRING, G_TYPE_STRING);
}

void
ags_config_init(AgsConfig *config)
{
  config->flags = 0;
  
  config->obj_mutexattr = (pthread_mutexattr_t *) malloc(sizeof(pthread_mutexattr_t));

  pthread_mutexattr_init(config->obj_mutexattr);
  pthread_mutexattr_settype(config->obj_mutexattr,
			    PTHREAD_MUTEX_RECURSIVE);

#ifdef __linux__
  pthread_mutexattr_setprotocol(config->obj_mutexattr,
				PTHREAD_PRIO_INHERIT);
#endif

  
  config->obj_mutex = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(config->obj_mutex, config->obj_mutexattr);

  /* version and build id */
  config->version = g_strdup(AGS_CONFIG_DEFAULT_VERSION);
  config->build_id = g_strdup(AGS_CONFIG_DEFAULT_BUILD_ID);

  config->application_context == NULL;

  config->key_file = g_key_file_new();
  g_key_file_ref(config->key_file);
}

void
ags_config_set_property(GObject *gobject,
			guint prop_id,
			const GValue *value,
			GParamSpec *param_spec)
{
  AgsConfig *config;

  pthread_mutex_t *config_mutex;

  config = AGS_CONFIG(gobject);

  /* get config mutex */
  pthread_mutex_lock(ags_config_get_class_mutex());
  
  config_mutex = config->obj_mutex;

  pthread_mutex_unlock(ags_config_get_class_mutex());

  switch(prop_id){
  case PROP_APPLICATION_CONTEXT:
    {
      AgsApplicationContext *application_context;
      
      application_context = (AgsApplicationContext *) g_value_get_object(value);

      pthread_mutex_lock(config_mutex);
      
      if(application_context == ((AgsApplicationContext *) config->application_context)){
	pthread_mutex_unlock(config_mutex);
	
	return;
      }

      if(config->application_context != NULL){
	g_object_unref(config->application_context);
      }
      
      if(application_context != NULL){
	g_object_ref(G_OBJECT(application_context));
      }
      
      config->application_context = (GObject *) application_context;

      pthread_mutex_unlock(config_mutex);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_config_get_property(GObject *gobject,
			guint prop_id,
			GValue *value,
			GParamSpec *param_spec)
{
  AgsConfig *config;

  pthread_mutex_t *config_mutex;

  config = AGS_CONFIG(gobject);

  /* get config mutex */
  pthread_mutex_lock(ags_config_get_class_mutex());
  
  config_mutex = config->obj_mutex;

  pthread_mutex_unlock(ags_config_get_class_mutex());
  
  switch(prop_id){
  case PROP_APPLICATION_CONTEXT:
    {
      pthread_mutex_lock(config_mutex);

      g_value_set_object(value, config->application_context);

      pthread_mutex_unlock(config_mutex);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}
void
ags_config_dispose(GObject *gobject)
{
  AgsConfig *config;

  config = (AgsConfig *) gobject;

  if(config->application_context != NULL){
    g_object_unref(config->application_context);

    config->application_context = NULL;
  }

  /* call parent */
  G_OBJECT_CLASS(ags_config_parent_class)->dispose(gobject);
}

void
ags_config_finalize(GObject *gobject)
{
  AgsConfig *config;

  config = (AgsConfig *) gobject;

  /* config mutex */
  pthread_mutexattr_destroy(config->obj_mutexattr);
  free(config->obj_mutexattr);

  pthread_mutex_destroy(config->obj_mutex);
  free(config->obj_mutex);

  /* application context */
  if(config->application_context != NULL){
    g_object_unref(config->application_context);
  }

  /* key file */
  if(config->key_file != NULL){
    g_key_file_unref(config->key_file);
  }

  /* global variable */
  if(ags_config == config){
    ags_config = NULL;
  }

  /* call parent */
  G_OBJECT_CLASS(ags_config_parent_class)->finalize(gobject);
}

gchar*
ags_config_get_version(AgsConfig *config)
{
  gchar *version;
  
  pthread_mutex_t *config_mutex;

  if(!AGS_IS_CONFIG(config)){
    return(NULL);
  }
  
  pthread_mutex_lock(ags_config_get_class_mutex());
  
  config_mutex = config->obj_mutex;

  pthread_mutex_unlock(ags_config_get_class_mutex());

  /* get version */
  pthread_mutex_lock(config_mutex);

  version = config->version;
  
  pthread_mutex_unlock(config_mutex);

  return(version);
}

void
ags_config_set_version(AgsConfig *config, gchar *version)
{
  pthread_mutex_t *config_mutex;

  if(!AGS_IS_CONFIG(config)){
    return;
  }
  
  pthread_mutex_lock(ags_config_get_class_mutex());
  
  config_mutex = config->obj_mutex;

  pthread_mutex_unlock(ags_config_get_class_mutex());

  /* set version */
  pthread_mutex_lock(config_mutex);

  config->version = g_strdup(version);
  
  pthread_mutex_unlock(config_mutex);
}

gchar*
ags_config_get_build_id(AgsConfig *config)
{
  gchar *build_id;
  
  pthread_mutex_t *config_mutex;

  if(!AGS_IS_CONFIG(config)){
    return(NULL);
  }
  
  pthread_mutex_lock(ags_config_get_class_mutex());
  
  config_mutex = config->obj_mutex;

  pthread_mutex_unlock(ags_config_get_class_mutex());

  /* get build id */
  pthread_mutex_lock(config_mutex);

  build_id = config->build_id;
  
  pthread_mutex_unlock(config_mutex);

  return(build_id);
}

void
ags_config_set_build_id(AgsConfig *config, gchar *build_id)
{
  pthread_mutex_t *config_mutex;

  if(!AGS_IS_CONFIG(config)){
    return;
  }
  
  pthread_mutex_lock(ags_config_get_class_mutex());
  
  config_mutex = config->obj_mutex;

  pthread_mutex_unlock(ags_config_get_class_mutex());

  /* set version */
  pthread_mutex_lock(config_mutex);

  config->build_id = g_strdup(build_id);
  
  pthread_mutex_unlock(config_mutex);
}

/**
 * ags_config_get_class_mutex:
 * 
 * Use this function's returned mutex to access mutex fields.
 *
 * Returns: the class mutex
 * 
 * Since: 2.0.0
 */
pthread_mutex_t*
ags_config_get_class_mutex()
{
  return(&ags_config_class_mutex);
}

void
ags_config_real_load_defaults(AgsConfig *config)
{

  pthread_mutex_t *config_mutex;
  
  pthread_mutex_lock(ags_config_get_class_mutex());
  
  config_mutex = config->obj_mutex;

  pthread_mutex_unlock(ags_config_get_class_mutex());

  /* load defaults */
  pthread_mutex_lock(config_mutex);

  ags_config_set_value(config, AGS_CONFIG_GENERIC, "autosave-thread", "false");
  ags_config_set_value(config, AGS_CONFIG_GENERIC, "simple-file", "true");
  ags_config_set_value(config, AGS_CONFIG_GENERIC, "disable-feature", "experimental");
  ags_config_set_value(config, AGS_CONFIG_GENERIC, "engine-mode", "performance");

  ags_config_set_value(config, AGS_CONFIG_THREAD, "model", "super-threaded");
  ags_config_set_value(config, AGS_CONFIG_THREAD, "super-threaded-scope", "audio");
  ags_config_set_value(config, AGS_CONFIG_THREAD, "lock-global", "ags-thread");
  ags_config_set_value(config, AGS_CONFIG_THREAD, "lock-parent", "ags-recycling-thread");
  ags_config_set_value(config, AGS_CONFIG_THREAD, "max-precision", "250");

#ifdef AGS_WITH_CORE_AUDIO
  ags_config_set_value(config, AGS_CONFIG_SOUNDCARD_0, "backend", "core-audio");
  ags_config_set_value(config, AGS_CONFIG_SOUNDCARD_0, "device", "ags-core-audio-devout-0");
#else
  ags_config_set_value(config, AGS_CONFIG_SOUNDCARD_0, "backend", "pulse");
  ags_config_set_value(config, AGS_CONFIG_SOUNDCARD_0, "device", "ags-pulse-devout-0");
#endif
  
  ags_config_set_value(config, AGS_CONFIG_SOUNDCARD_0, "pcm-channels", "2");
  ags_config_set_value(config, AGS_CONFIG_SOUNDCARD_0, "samplerate", "48000");
  ags_config_set_value(config, AGS_CONFIG_SOUNDCARD_0, "buffer-size", "512");
  ags_config_set_value(config, AGS_CONFIG_SOUNDCARD_0, "use-cache", "true");
  ags_config_set_value(config, AGS_CONFIG_SOUNDCARD_0, "cache-buffer-size", "4096");
  ags_config_set_value(config, AGS_CONFIG_SOUNDCARD_0, "format", "16");

  //ags_config_set_value(config, AGS_CONFIG_SEQUENCER_0, "backend", "jack");
  //ags_config_set_value(config, AGS_CONFIG_SEQUENCER_0, "device", "ags-jack-midiin-0");

  ags_config_set_value(config, AGS_CONFIG_RECALL, "auto-sense", "true");

  pthread_mutex_unlock(config_mutex);
}

/**
 * ags_config_load_defaults:
 * @config: the #AgsConfig
 *
 * Load configuration from default values.
 *
 * Since: 2.0.0
 */
void
ags_config_load_defaults(AgsConfig *config)
{
  g_return_if_fail(AGS_IS_CONFIG(config));

  g_object_ref(G_OBJECT(config));
  g_signal_emit(G_OBJECT(config),
		config_signals[LOAD_DEFAULTS], 0);
  g_object_unref(G_OBJECT(config));
}

/**
 * ags_config_load_from_file:
 * @config: the #AgsConfig
 * @filename: the configuration file
 *
 * Load configuration from @filename.
 *
 * Since: 2.0.0
 */
void
ags_config_load_from_file(AgsConfig *config, gchar *filename)
{
  GFile *file;

  pthread_mutex_t *config_mutex;

  if(!AGS_IS_CONFIG(config)){
    return;
  }
  
  pthread_mutex_lock(ags_config_get_class_mutex());
  
  config_mutex = config->obj_mutex;

  pthread_mutex_unlock(ags_config_get_class_mutex());

  file = g_file_new_for_path(filename);

  g_message("loading preferences for: %s", filename);

  if(!g_file_query_exists(file,
			  NULL)){
    ags_config_load_defaults(config);
  }else{
    GKeyFile *key_file;

    gchar **groups, **groups_start;
    gchar **keys, **keys_start;
    gchar *value;

    GError *error;

    pthread_mutex_lock(config_mutex);

    error = NULL;
    
    key_file = g_key_file_new();
    g_key_file_load_from_file(key_file,
			      filename,
			      G_KEY_FILE_NONE,
			      &error);

    if(error != NULL){
      g_warning("%s", error->message);

      g_error_free(error);
    }

    groups =
      groups_start = g_key_file_get_groups(key_file,
					   NULL);

    while(*groups != NULL){
      keys =
	keys_start = g_key_file_get_keys(key_file,
					 *groups,
					 NULL,
					 NULL);

      while(*keys != NULL){
	value = g_key_file_get_value(key_file,
				     *groups,
				     *keys,
				     NULL);
	ags_config_set_value(config,
			     *groups,
			     *keys,
			     value);
	
	keys++;
      }

      g_strfreev(keys_start);

      groups++;
    }

    g_strfreev(groups_start);
    g_key_file_unref(key_file);

    pthread_mutex_unlock(config_mutex);
  }

  g_object_unref(file);
}

/**
 * ags_config_load_from_data:
 * @config: the #AgsConfig
 * @buffer: the data buffer
 * @buffer_length: the size of the buffer
 *
 * Read configuration in memory.
 *
 * Since: 2.0.0
 */
void
ags_config_load_from_data(AgsConfig *config,
			  char *buffer, gsize buffer_length)
{

  pthread_mutex_t *config_mutex;

  if(!AGS_IS_CONFIG(config)){
    return;
  }
  
  pthread_mutex_lock(ags_config_get_class_mutex());
  
  config_mutex = config->obj_mutex;

  pthread_mutex_unlock(ags_config_get_class_mutex());

  /* load from data */
  //#ifdef AGS_DEBUG
  g_message("loading preferences from data[0x%x]", (unsigned int) buffer);
  //#endif
  
  if(buffer == NULL){
    ags_config_load_defaults(config);
  }else{
    GKeyFile *key_file;

    gchar **groups, **groups_start;
    gchar **keys, **keys_start;
    gchar *value;

    GError *error;

    pthread_mutex_lock(config_mutex);

    error = NULL;

    key_file = g_key_file_new();
    g_key_file_load_from_data(key_file,
			      buffer,
			      buffer_length,
			      G_KEY_FILE_NONE,
			      &error);

    if(error != NULL){
      g_warning("%s", error->message);

      g_error_free(error);
    }
    
    groups =
      groups_start = g_key_file_get_groups(key_file,
					   NULL);

    while(*groups != NULL){
      keys =
	keys_start = g_key_file_get_keys(key_file,
					 *groups,
					 NULL,
					 NULL);

      while(*keys != NULL){
	value = g_key_file_get_value(key_file,
				     *groups,
				     *keys,
				     NULL);
	ags_config_set_value(config,
			     *groups,
			     *keys,
			     value);
	
	keys++;
      }

      g_strfreev(keys_start);

      groups++;
    }

    g_strfreev(groups_start);
    g_key_file_unref(key_file);

    pthread_mutex_unlock(config_mutex);
  }
}

/**
 * ags_config_to_data:
 * @config: the #AgsConfig
 * @buffer: the data buffer
 * @buffer_length: the size of the buffer
 *
 * Save configuration.
 *
 * Since: 2.0.0
 */
void
ags_config_to_data(AgsConfig *config,
		   char **buffer, gsize *buffer_length)
{
  gchar *data;
  gsize length;

  GError *error;

  pthread_mutex_t *config_mutex;

  if(!AGS_IS_CONFIG(config)){
    return;
  }
  
  pthread_mutex_lock(ags_config_get_class_mutex());
  
  config_mutex = config->obj_mutex;

  pthread_mutex_unlock(ags_config_get_class_mutex());

  /* to data */
  pthread_mutex_lock(config_mutex);

  error = NULL;
  data = g_key_file_to_data(config->key_file,
			    &length,
			    &error);

  if(error != NULL){
    g_warning("%s", error->message);
  }

  if(buffer != NULL){
    *buffer = data;
  }

  if(buffer_length != NULL){
    *buffer_length = length;
  }

  pthread_mutex_unlock(config_mutex);
}

/**
 * ags_config_save:
 * @config: the #AgsConfig
 *
 * Save configuration.
 *
 * Since: 2.0.0
 */
void
ags_config_save(AgsConfig *config)
{
  struct passwd *pw;

  uid_t uid;
  gchar *path, *filename;
  gchar *content;
  gsize length;

  GError *error;

  pthread_mutex_t *config_mutex;

  if(!AGS_IS_CONFIG(config)){
    return;
  }
  
  pthread_mutex_lock(ags_config_get_class_mutex());
  
  config_mutex = config->obj_mutex;

  pthread_mutex_unlock(ags_config_get_class_mutex());

  /* save */
  pthread_mutex_lock(config_mutex);

  uid = getuid();
  pw = getpwuid(uid);

  /* open conf dir */
  path = g_strdup_printf("%s/%s",
			 pw->pw_dir,
			 AGS_DEFAULT_DIRECTORY);

  if(!g_mkdir_with_parents(path,
			   0755)){
    filename = g_strdup_printf("%s/%s",
			       path,
			       AGS_DEFAULT_CONFIG);

    /* get content */
    error = NULL;

    content = g_key_file_to_data(config->key_file,
				 &length,
				 &error);
    
    if(error != NULL){
      //TODO:JK: do recovery
      goto ags_config_save_END;
    }

    /* write content */
    error = NULL;

    g_file_set_contents(filename,
			content,
			length,
			&error);

  ags_config_save_END:
    g_free(filename);
  }

  g_free(path);

  pthread_mutex_unlock(config_mutex);
}

void
ags_config_real_set_value(AgsConfig *config, gchar *group, gchar *key, gchar *value)
{
  pthread_mutex_t *config_mutex;
  
  pthread_mutex_lock(ags_config_get_class_mutex());
  
  config_mutex = config->obj_mutex;

  pthread_mutex_unlock(ags_config_get_class_mutex());

  /* set value */
  pthread_mutex_lock(config_mutex);
  
  g_key_file_set_value(config->key_file, group, key, value);

  pthread_mutex_unlock(config_mutex);
}

/**
 * ags_config_set_value:
 * @config: the #AgsConfig
 * @group: the config group identifier
 * @key: the key of the property
 * @value: the value to set
 *
 * Set config by @group and @key, applying @value.
 *
 * Since: 2.0.0
 */
void
ags_config_set_value(AgsConfig *config, gchar *group, gchar *key, gchar *value)
{
  g_return_if_fail(AGS_IS_CONFIG(config));

  g_object_ref(G_OBJECT(config));
  g_signal_emit(G_OBJECT(config),
		config_signals[SET_VALUE], 0,
		group, key, value);
  g_object_unref(G_OBJECT(config));
}

gchar*
ags_config_real_get_value(AgsConfig *config, gchar *group, gchar *key)
{
  gchar *str;
  GError *error;
  
  pthread_mutex_t *config_mutex;
  
  pthread_mutex_lock(ags_config_get_class_mutex());
  
  config_mutex = config->obj_mutex;

  pthread_mutex_unlock(ags_config_get_class_mutex());

  /* get value */
  pthread_mutex_lock(config_mutex);
  
  error = NULL;

  str = g_key_file_get_value(config->key_file, group, key, &error);

  pthread_mutex_unlock(config_mutex);

  return(str);
}

/**
 * ags_config_get_value:
 * @config: the #AgsConfig
 * @group: the config group identifier
 * @key: the key of the property
 *
 * Retrieve config by @group and @key.
 *
 * Returns: the property's value
 *
 * Since: 2.0.0
 */
gchar*
ags_config_get_value(AgsConfig *config, gchar *group, gchar *key)
{
  gchar *value;

  g_return_val_if_fail(AGS_IS_CONFIG(config), NULL);

  g_object_ref(G_OBJECT(config));
  g_signal_emit(G_OBJECT(config),
		config_signals[GET_VALUE], 0,
		group, key,
		&value);
  g_object_unref(G_OBJECT(config));

  return(value);
}

/**
 * ags_config_clear:
 * @config: the #AgsConfig
 *
 * Clears configuration.
 *
 * Since: 2.0.0
 */
void
ags_config_clear(AgsConfig *config)
{
  gchar **group;

  gsize n_group;
  guint i;

  pthread_mutex_t *config_mutex;

  if(!AGS_IS_CONFIG(config)){
    return;
  }
    
  pthread_mutex_lock(ags_config_get_class_mutex());
  
  config_mutex = config->obj_mutex;

  pthread_mutex_unlock(ags_config_get_class_mutex());
  
  /* clear */
  pthread_mutex_lock(config_mutex);

  group = g_key_file_get_groups(config->key_file,
				&n_group);

  for(i = 0; i < n_group; i++){
    g_key_file_remove_group(config->key_file,
    			    group[i],
    			    NULL);
  }

  pthread_mutex_unlock(config_mutex);
}

/**
 * ags_config_get_instance:
 *
 * Get config instance.
 *
 * Returns: the config instance
 *
 * Since: 2.0.0
 */
AgsConfig*
ags_config_get_instance()
{
  static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

  pthread_mutex_lock(&mutex);

  if(ags_config == NULL){
    ags_config = ags_config_new(NULL);
  }

  pthread_mutex_unlock(&mutex);

  return(ags_config);
}

/**
 * ags_config_new:
 * @application_context: the #AgsApplicationContext
 *
 * Create a new instance of #AgsConfig.
 *
 * Returns: the new #AgsConfig.
 *
 * Since: 2.0.0
 */
AgsConfig*
ags_config_new(GObject *application_context)
{
  AgsConfig *config;

  config = (AgsConfig *) g_object_new(AGS_TYPE_CONFIG,
				      "application-context", application_context,
				      NULL);

  return(config);
}

