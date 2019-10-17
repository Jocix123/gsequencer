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

#ifndef __AGS_AUDIO_UNIT_SERVER_H__
#define __AGS_AUDIO_UNIT_SERVER_H__

#include <glib.h>
#include <glib-object.h>

#include <ags/config.h>

#ifdef AGS_WITH_AUDIO_UNIT
#include <AudioToolbox/AudioToolbox.h>
#include <AudioUnit/AudioUnit.h>
#include <AudioUnit/AUComponent.h>
#include <AudioUnit/AudioComponent.h>
#endif

#include <pthread.h>

#include <ags/libags.h>

#define AGS_TYPE_AUDIO_UNIT_SERVER                (ags_audio_unit_server_get_type())
#define AGS_AUDIO_UNIT_SERVER(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_AUDIO_UNIT_SERVER, AgsAudioUnitServer))
#define AGS_AUDIO_UNIT_SERVER_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST(class, AGS_TYPE_AUDIO_UNIT_SERVER, AgsAudioUnitServer))
#define AGS_IS_AUDIO_UNIT_SERVER(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AGS_TYPE_AUDIO_UNIT_SERVER))
#define AGS_IS_AUDIO_UNIT_SERVER_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE ((class), AGS_TYPE_AUDIO_UNIT_SERVER))
#define AGS_AUDIO_UNIT_SERVER_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS(obj, AGS_TYPE_AUDIO_UNIT_SERVER, AgsAudioUnitServerClass))

#define AGS_AUDIO_UNIT_SERVER_GET_OBJ_MUTEX(obj) (((AgsAudioUnitServer *) obj)->obj_mutex)

typedef struct _AgsAudioUnitServer AgsAudioUnitServer;
typedef struct _AgsAudioUnitServerClass AgsAudioUnitServerClass;

/**
 * AgsAudioUnitServerFlags:
 * @AGS_AUDIO_UNIT_SERVER_ADDED_TO_REGISTRY: the AudioUnit server was added to registry, see #AgsConnectable::add_to_registry()
 * @AGS_AUDIO_UNIT_SERVER_CONNECTED: indicates the server was connected by calling #AgsConnectable::connect()
 * 
 * Enum values to control the behavior or indicate internal state of #AgsAudioUnitServer by
 * enable/disable as flags.
 */
typedef enum{
  AGS_AUDIO_UNIT_SERVER_ADDED_TO_REGISTRY  = 1,
  AGS_AUDIO_UNIT_SERVER_CONNECTED          = 1 <<  1,
}AgsAudioUnitServerFlags;

struct _AgsAudioUnitServer
{
  GObject gobject;

  guint flags;

  pthread_mutex_t *obj_mutex;
  pthread_mutexattr_t *obj_mutexattr;

  volatile gboolean running;
  pthread_t *thread;

  AgsApplicationContext *application_context;

  AgsUUID *uuid;
  
  gchar *url;

  guint *port;
  guint port_count;

  guint n_soundcards;
  guint n_sequencers;
  
  GObject *default_soundcard;  

  GObject *default_client;
  GObject *input_client;

  GList *client;
};

struct _AgsAudioUnitServerClass
{
  GObjectClass gobject;
};

GType ags_audio_unit_server_get_type();

pthread_mutex_t* ags_audio_unit_server_get_class_mutex();

gboolean ags_audio_unit_server_test_flags(AgsAudioUnitServer *audio_unit_server, guint flags);
void ags_audio_unit_server_set_flags(AgsAudioUnitServer *audio_unit_server, guint flags);
void ags_audio_unit_server_unset_flags(AgsAudioUnitServer *audio_unit_server, guint flags);

GList* ags_audio_unit_server_find_url(GList *audio_unit_server,
				      gchar *url);

GObject* ags_audio_unit_server_find_client(AgsAudioUnitServer *audio_unit_server,
					   gchar *client_uuid);

GObject* ags_audio_unit_server_find_port(AgsAudioUnitServer *audio_unit_server,
					 gchar *port_uuid);

void ags_audio_unit_server_add_client(AgsAudioUnitServer *audio_unit_server,
				      GObject *audio_unit_client);
void ags_audio_unit_server_remove_client(AgsAudioUnitServer *audio_unit_server,
					 GObject *audio_unit_client);

void ags_audio_unit_server_connect_client(AgsAudioUnitServer *audio_unit_server);

void ags_audio_unit_server_start_poll(AgsAudioUnitServer *audio_unit_server);

AgsAudioUnitServer* ags_audio_unit_server_new(AgsApplicationContext *application_context,
					      gchar *url);

#endif /*__AGS_AUDIO_UNIT_SERVER_H__*/
