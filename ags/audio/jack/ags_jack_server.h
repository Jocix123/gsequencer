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

#ifndef __AGS_JACK_SERVER_H__
#define __AGS_JACK_SERVER_H__

#include <glib.h>
#include <glib-object.h>

#include <ags/config.h>

#ifdef AGS_WITH_JACK
#include <jack/jslist.h>
#include <jack/jack.h>
#include <jack/control.h>
#include <stdbool.h>
#endif

#define AGS_TYPE_JACK_SERVER                (ags_jack_server_get_type())
#define AGS_JACK_SERVER(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_JACK_SERVER, AgsJackServer))
#define AGS_JACK_SERVER_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST(class, AGS_TYPE_JACK_SERVER, AgsJackServer))
#define AGS_IS_JACK_SERVER(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AGS_TYPE_JACK_SERVER))
#define AGS_IS_JACK_SERVER_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE ((class), AGS_TYPE_JACK_SERVER))
#define AGS_JACK_SERVER_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS(obj, AGS_TYPE_JACK_SERVER, AgsJackServerClass))

#define AGS_JACK_SERVER_DEFAULT_PROTOCOL "udp"
#define AGS_JACK_SERVER_DEFAULT_HOST "localhost"
#define AGS_JACK_SERVER_DEFAULT_PORT (3000)

typedef struct _AgsJackServer AgsJackServer;
typedef struct _AgsJackServerClass AgsJackServerClass;

/**
 * AgsJackServerFlags:
 * @AGS_JACK_SERVER_CONNECTED: indicates the server was connected by calling #AgsConnectable::connect()
 * 
 * Enum values to control the behavior or indicate internal state of #AgsJackPort by
 * enable/disable as flags.
 */
typedef enum{
  AGS_JACK_SERVER_CONNECTED       = 1,
}AgsJackServerFlags;

struct _AgsJackServer
{
  GObject object;

  guint flags;

  pthread_mutex_t *mutex;
  pthread_mutexattr_t *mutexattr;

  GObject *application_context;

  gchar *url;
  //  jackctl_server_t *jackctl;
  
  guint *port;
  guint port_count;
  
  guint n_soundcards;
  guint n_sequencers;
  
  GObject *default_soundcard;  

  GObject *default_client;
  GList *client;
};

struct _AgsJackServerClass
{
  GObjectClass object;
};

GType ags_jack_server_get_type();

GList* ags_jack_server_find_url(GList *jack_server,
				gchar *url);

GObject* ags_jack_server_find_client(AgsJackServer *jack_server,
				     gchar *client_uuid);

GObject* ags_jack_server_find_port(AgsJackServer *jack_server,
				   gchar *port_uuid);

void ags_jack_server_add_client(AgsJackServer *jack_server,
				GObject *jack_client);
void ags_jack_server_remove_client(AgsJackServer *jack_server,
				   GObject *jack_client);

void ags_jack_server_connect_client(AgsJackServer *jack_server);

AgsJackServer* ags_jack_server_new(GObject *application_context,
				   gchar *url);

#endif /*__AGS_JACK_SERVER_H__*/
