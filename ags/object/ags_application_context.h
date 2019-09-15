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

#ifndef __AGS_APPLICATION_CONTEXT_H__
#define __AGS_APPLICATION_CONTEXT_H__

#include <glib.h>
#include <glib-object.h>

#include <ags/lib/ags_uuid.h>

#include <ags/object/ags_config.h>

#include <ags/file/ags_file.h>

#define AGS_TYPE_APPLICATION_CONTEXT                (ags_application_context_get_type())
#define AGS_APPLICATION_CONTEXT(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_APPLICATION_CONTEXT, AgsApplicationContext))
#define AGS_APPLICATION_CONTEXT_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST(class, AGS_TYPE_APPLICATION_CONTEXT, AgsApplicationContextClass))
#define AGS_IS_APPLICATION_CONTEXT(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AGS_TYPE_APPLICATION_CONTEXT))
#define AGS_IS_APPLICATION_CONTEXT_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE ((class), AGS_TYPE_APPLICATION_CONTEXT))
#define AGS_APPLICATION_CONTEXT_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS(obj, AGS_TYPE_APPLICATION_CONTEXT, AgsApplicationContextClass))

#define AGS_APPLICATION_CONTEXT_GET_OBJ_MUTEX(obj) (((AgsApplicationContext *) obj)->obj_mutex)

#ifndef PACKAGE_VERSION
#define AGS_VERSION "1.2.0"
#else
#define AGS_VERSION PACKAGE_VERSION
#endif

#define AGS_BUILD_ID "CEST 14-09-2017 04:33"

#define AGS_DEFAULT_DIRECTORY ".gsequencer"
#define AGS_DEFAULT_CONFIG "ags.conf"

#define AGS_INIT_CONTEXT_TSD_APPLICATION_CONTEXT "ags-application-context"

typedef struct _AgsApplicationContext AgsApplicationContext;
typedef struct _AgsApplicationContextClass AgsApplicationContextClass;

/**
 * AgsApplicationContextFlags:
 * @AGS_APPLICATION_CONTEXT_DEFAULT: indicates the default context, used if siblings available
 * @AGS_APPLICATION_CONTEXT_REGISTER_TYPES: enable to register types
 * @AGS_APPLICATION_CONTEXT_ADD_TO_REGISTRY: add to #AgsRegistry
 * @AGS_APPLICATION_CONTEXT_CONNECT: call connect of #AgsConnectable descending objects
 * @AGS_APPLICATION_CONTEXT_TYPES_REGISTERED: indicates the types have been registered
 * @AGS_APPLICATION_CONTEXT_ADDED_TO_REGISTRY: indicates the application context was added to #AgsRegistry
 * @AGS_APPLICATION_CONTEXT_CONNECTED: indicates the application context was connected by calling #AgsConnectable::connect()
 * 
 * Enum values to control the behavior or indicate internal state of #AgsApplicationContext by
 * enable/disable as flags.
 */
typedef enum{
  AGS_APPLICATION_CONTEXT_DEFAULT            = 1,
  AGS_APPLICATION_CONTEXT_REGISTER_TYPES     = 1 <<  1,
  AGS_APPLICATION_CONTEXT_ADD_TO_REGISTRY    = 1 <<  2,
  AGS_APPLICATION_CONTEXT_CONNECT            = 1 <<  3,
  AGS_APPLICATION_CONTEXT_TYPES_REGISTERED   = 1 <<  4,
  AGS_APPLICATION_CONTEXT_ADDED_TO_REGISTRY  = 1 <<  5,
  AGS_APPLICATION_CONTEXT_CONNECTED          = 1 <<  6,
}AgsApplicationContextFlags;

struct _AgsApplicationContext
{
  GObject gobject;

  guint flags;

  pthread_mutex_t *obj_mutex;
  pthread_mutexattr_t *obj_mutexattr;

  AgsUUID *uuid;
  
  gchar *version;
  gchar *build_id;

  int argc;
  char **argv;
  
  GObject *log;

  gchar *domain;
  
  AgsConfig *config;
  
  GObject *main_loop;
  GObject *autosave_thread;
  GObject *task_thread;
  
  AgsFile *file;
  GObject *history;
};

struct _AgsApplicationContextClass
{
  GObjectClass gobject;

  void (*load_config)(AgsApplicationContext *application_context);
  
  void (*prepare)(AgsApplicationContext *application_context);
  void (*setup)(AgsApplicationContext *application_context);

  void (*register_types)(AgsApplicationContext *application_context);

  void (*read)(AgsFile *file, xmlNode *node, GObject **gobject);
  xmlNode* (*write)(AgsFile *file, xmlNode *parent, GObject *gobject);

  void (*quit)(AgsApplicationContext *application_context);
};

GType ags_application_context_get_type();

pthread_mutex_t* ags_application_context_get_class_mutex();

gboolean ags_application_context_test_flags(AgsApplicationContext *application_context, guint flags);
void ags_application_context_set_flags(AgsApplicationContext *application_context, guint flags);
void ags_application_context_unset_flags(AgsApplicationContext *application_context, guint flags);

void ags_application_context_load_config(AgsApplicationContext *application_context);

void ags_application_context_prepare(AgsApplicationContext *application_context);
void ags_application_context_setup(AgsApplicationContext *application_context);

void ags_application_context_register_types(AgsApplicationContext *application_context);

void ags_application_context_quit(AgsApplicationContext *application_context);

AgsApplicationContext* ags_application_context_get_instance();
AgsApplicationContext* ags_application_context_new(GObject *main_loop,
						   AgsConfig *config);

#endif /*__AGS_APPLICATION_CONTEXT_H__*/
