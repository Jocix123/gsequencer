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

#ifndef __AGS_PULSE_CLIENT_H__
#define __AGS_PULSE_CLIENT_H__

#include <glib.h>
#include <glib-object.h>

#include <ags/config.h>

#ifdef AGS_WITH_PULSE
#include <pulse/pulseaudio.h>
#include <pulse/stream.h>
#include <pulse/error.h>
#endif

#include <ags/libags.h>

#define AGS_TYPE_PULSE_CLIENT                (ags_pulse_client_get_type())
#define AGS_PULSE_CLIENT(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_PULSE_CLIENT, AgsPulseClient))
#define AGS_PULSE_CLIENT_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST(class, AGS_TYPE_PULSE_CLIENT, AgsPulseClient))
#define AGS_IS_PULSE_CLIENT(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AGS_TYPE_PULSE_CLIENT))
#define AGS_IS_PULSE_CLIENT_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE ((class), AGS_TYPE_PULSE_CLIENT))
#define AGS_PULSE_CLIENT_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS(obj, AGS_TYPE_PULSE_CLIENT, AgsPulseClientClass))

typedef struct _AgsPulseClient AgsPulseClient;
typedef struct _AgsPulseClientClass AgsPulseClientClass;

/**
 * AgsPulseClientFlags:
 * @AGS_PULSE_CLIENT_ADDED_TO_REGISTRY: the PULSE client was added to registry, see #AgsConnectable::add_to_registry()
 * @AGS_PULSE_CLIENT_CONNECTED: indicates the client was connected by calling #AgsConnectable::connect()
 * @AGS_PULSE_CLIENT_ACTIVATED: the client was activated
 * @AGS_PULSE_CLIENT_READY: the client is ready
 * 
 * Enum values to control the behavior or indicate internal state of #AgsPulseClient by
 * enable/disable as flags.
 */
typedef enum{
  AGS_PULSE_CLIENT_ADDED_TO_REGISTRY  = 1,
  AGS_PULSE_CLIENT_CONNECTED          = 1 <<  1,
  AGS_PULSE_CLIENT_ACTIVATED          = 1 <<  2,
  AGS_PULSE_CLIENT_READY              = 1 <<  3,
}AgsPulseClientFlags;

struct _AgsPulseClient
{
  GObject gobject;

  guint flags;

  pthread_mutex_t *obj_mutex;
  pthread_mutexattr_t *obj_mutexattr;
  
  GObject *pulse_server;

  AgsUUID *uuid;

  gchar *client_uuid;
  gchar *client_name;

#ifdef AGS_WITH_PULSE
  pa_context *context;
#else
  gpointer context;
#endif

  GList *device;
  GList *port;
};

struct _AgsPulseClientClass
{
  GObjectClass gobject;
};

GType ags_pulse_client_get_type();

pthread_mutex_t* ags_pulse_client_get_class_mutex();

gboolean ags_pulse_client_test_flags(AgsPulseClient *pulse_client, guint flags);
void ags_pulse_client_set_flags(AgsPulseClient *pulse_client, guint flags);
void ags_pulse_client_unset_flags(AgsPulseClient *pulse_client, guint flags);

GList* ags_pulse_client_find_uuid(GList *pulse_client,
				  gchar *client_uuid);
GList* ags_pulse_client_find(GList *pulse_client,
			     gchar *client_name);

void ags_pulse_client_open(AgsPulseClient *pulse_client,
			   gchar *client_name);
void ags_pulse_client_close(AgsPulseClient *pulse_client);

void ags_pulse_client_add_device(AgsPulseClient *pulse_client,
				 GObject *pulse_device);
void ags_pulse_client_remove_device(AgsPulseClient *pulse_client,
				    GObject *pulse_device);

void ags_pulse_client_add_port(AgsPulseClient *pulse_client,
			       GObject *pulse_port);
void ags_pulse_client_remove_port(AgsPulseClient *pulse_client,
				  GObject *pulse_port);

void ags_pulse_client_activate(AgsPulseClient *pulse_client);
void ags_pulse_client_deactivate(AgsPulseClient *pulse_client);

AgsPulseClient* ags_pulse_client_new(GObject *pulse_server);

#endif /*__AGS_PULSE_CLIENT_H__*/
