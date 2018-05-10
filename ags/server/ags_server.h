/* GSequencer - Advanced GTK Sequencer
 * Copyright (C) 2005-2018 Joël Krähemann
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

#ifndef __AGS_SERVER_H__
#define __AGS_SERVER_H__

#include <glib.h>
#include <glib-object.h>

#include <netinet/in.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <ags/config.h>

#ifdef AGS_WITH_XMLRPC_C
#include <xmlrpc-c/util.h>

#include <xmlrpc-c/base.h>
#include <xmlrpc-c/abyss.h>
#include <xmlrpc-c/server.h>
#include <xmlrpc-c/server_abyss.h>
#endif

#include <pthread.h>

#define AGS_TYPE_SERVER                (ags_server_get_type())
#define AGS_SERVER(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_SERVER, AgsServer))
#define AGS_SERVER_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST(class, AGS_TYPE_SERVER, AgsServerClass))
#define AGS_IS_SERVER(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AGS_TYPE_SERVER))
#define AGS_IS_SERVER_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE ((class), AGS_TYPE_SERVER))
#define AGS_SERVER_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS(obj, AGS_TYPE_SERVER, AgsServerClass))

#define AGS_SERVER_DEFAULT_AUTH_MODULE "ags-xml-password-store"

typedef struct _AgsServer AgsServer;
typedef struct _AgsServerClass AgsServerClass;
typedef struct _AgsServerInfo AgsServerInfo;

/**
 * AgsServerFlags:
 * @AGS_SERVER_CONNECTED: the server was connected by #AgsConnectable::connect()
 * @AGS_SERVER_STARTED: the server was started
 * @AGS_SERVER_RUNNING: the server is up and running
 * 
 * Enum values to control the behavior or indicate internal state of #AgsServer by
 * enable/disable as flags.
 */
typedef enum{
  AGS_SERVER_ADDED_TO_REGISTRY  = 1,
  AGS_SERVER_CONNECTED          = 1 <<  1,
  AGS_SERVER_STARTED            = 1 <<  2,
  AGS_SERVER_RUNNING            = 1 <<  3,
}AgsServerFlags;

struct _AgsServer
{
  GObject object;

  guint flags;

  pthread_mutex_t *obj_mutex;
  pthread_mutexattr_t *obj_mutexattr;

  AgsUUID *uuid;

  AgsServerInfo *server_info;

#ifdef AGS_WITH_XMLRPC_C
  TServer *abyss_server;
  TSocket *socket;
#else
  void *abyss_server;
  void *socket;  
#endif

  int socket_fd;
  struct sockaddr_in *address;

  gchar *auth_module;
  
  GList *controller;
  
  GObject *application_context;
};

struct _AgsServerClass
{
  GObjectClass object;
  
  void (*start)(AgsServer *server);
};

/**
 * AgsServerInfo:
 * @uuid: the assigned uuid
 * @server_name: the server's name
 * 
 * #AgsServerInfo does identify the server and provides some basic information about it.
 */
struct _AgsServerInfo
{
  gchar *uuid;
  gchar *server_name;
};

GType ags_server_get_type();

pthread_mutex_t* ags_server_get_class_mutex();

gboolean ags_server_test_flags(AgsServer *server, guint flags);
void ags_server_set_flags(AgsServer *server, guint flags);
void ags_server_unset_flags(AgsServer *server, guint flags);

AgsServerInfo* ags_server_info_alloc(gchar *server_name);

void ags_server_start(AgsServer *server);

AgsServer* ags_server_lookup(AgsServerInfo *server_info);

AgsServer* ags_server_new(GObject *application_context);

#endif /*__AGS_SERVER_H__*/
