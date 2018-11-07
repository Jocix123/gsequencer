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

#include <ags/audio/osc/ags_osc_builder.h>

#include <ags/libags.h>

#include <ags/audio/osc/ags_osc_buffer_util.h>

void ags_osc_builder_class_init(AgsOscBuilderClass *osc_builder);
void ags_osc_builder_init(AgsOscBuilder *osc_builder);
void ags_osc_builder_set_property(GObject *gobject,
				  guint prop_id,
				  const GValue *value,
				  GParamSpec *param_spec);
void ags_osc_builder_get_property(GObject *gobject,
				  guint prop_id,
				  GValue *value,
				  GParamSpec *param_spec);
void ags_osc_builder_finalize(GObject *gobject);

/**
 * SECTION:ags_osc_builder
 * @short_description: OSC buidler
 * @title: AgsOscBuilder
 * @section_id:
 * @include: ags/audio/osc/ags_osc_builder.h
 *
 * The #AgsOscBuilder lets you create OSC files using its builder functions.
 */

enum{
  PROP_0,
};

enum{
  PUTC,
  ON_ERROR,
  APPEND_PACKET,
  APPEND_BUNDLE,
  APPEND_MESSAGE,
  APPEND_VALUE,
  LAST_SIGNAL,
};

static gpointer ags_osc_builder_parent_class = NULL;

static guint osc_builder_signals[LAST_SIGNAL];

static pthread_mutex_t ags_osc_builder_class_mutex = PTHREAD_MUTEX_INITIALIZER;

GType
ags_osc_builder_get_type(void)
{
  static volatile gsize g_define_type_id__volatile = 0;

  if(g_once_init_enter (&g_define_type_id__volatile)){
    GType ags_type_osc_builder = 0;

    static const GTypeInfo ags_osc_builder_info = {
      sizeof (AgsOscBuilderClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_osc_builder_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsOscBuilder),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_osc_builder_init,
    };

    ags_type_osc_builder = g_type_register_static(G_TYPE_OBJECT,
						  "AgsOscBuilder", &ags_osc_builder_info,
						  0);

    g_once_init_leave(&g_define_type_id__volatile, ags_type_osc_builder);
  }

  return g_define_type_id__volatile;
}

void
ags_osc_builder_class_init(AgsOscBuilderClass *osc_builder)
{
  GObjectClass *gobject;
  GParamSpec *param_spec;
  
  ags_osc_builder_parent_class = g_type_class_peek_parent(osc_builder);

  /* GObjectClass */
  gobject = (GObjectClass *) osc_builder;
  
  gobject->set_property = ags_osc_builder_set_property;
  gobject->get_property = ags_osc_builder_get_property;  

  gobject->finalize = ags_osc_builder_finalize;
}

void
ags_osc_builder_init(AgsOscBuilder *osc_builder)
{
  osc_builder->flags = 0;

  /* osc builder mutex */
  osc_builder->obj_mutexattr = (pthread_mutexattr_t *) malloc(sizeof(pthread_mutexattr_t));
  pthread_mutexattr_init(osc_builder->obj_mutexattr);
  pthread_mutexattr_settype(osc_builder->obj_mutexattr,
			    PTHREAD_MUTEX_RECURSIVE);

#ifdef __linux__
  pthread_mutexattr_setprotocol(osc_builder->obj_mutexattr,
				PTHREAD_PRIO_INHERIT);
#endif

  osc_builder->obj_mutex =  (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(osc_builder->obj_mutex,
		     osc_builder->obj_mutexattr);

  osc_builder->data = NULL;
  osc_builder->length = 0;

  osc_builder->offset = 0;
  
  osc_builder->packet = NULL;
}

void
ags_osc_builder_set_property(GObject *gobject,
			     guint prop_id,
			     const GValue *value,
			     GParamSpec *param_spec)
{
  AgsOscBuilder *osc_builder;

  pthread_mutex_t *osc_builder_mutex;

  osc_builder = AGS_OSC_BUILDER(gobject);

  /* get osc builder mutex */
  pthread_mutex_lock(ags_osc_builder_get_class_mutex());
  
  osc_builder_mutex = osc_builder->obj_mutex;
  
  pthread_mutex_unlock(ags_osc_builder_get_class_mutex());
  
  switch(prop_id){
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_osc_builder_get_property(GObject *gobject,
			     guint prop_id,
			     GValue *value,
			     GParamSpec *param_spec)
{
  AgsOscBuilder *osc_builder;

  pthread_mutex_t *osc_builder_mutex;

  osc_builder = AGS_OSC_BUILDER(gobject);

  /* get osc builder mutex */
  pthread_mutex_lock(ags_osc_builder_get_class_mutex());
  
  osc_builder_mutex = osc_builder->obj_mutex;
  
  pthread_mutex_unlock(ags_osc_builder_get_class_mutex());
  
  switch(prop_id){
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_osc_builder_finalize(GObject *gobject)
{
  AgsOscBuilder *osc_builder;
    
  osc_builder = (AgsOscBuilder *) gobject;

  pthread_mutex_destroy(osc_builder->obj_mutex);
  free(osc_builder->obj_mutex);

  pthread_mutexattr_destroy(osc_builder->obj_mutexattr);
  free(osc_builder->obj_mutexattr);

  if(osc_builder->data != NULL){
    free(osc_builder->data);
  }
  
  g_list_free_full(osc_builder->packet,
		   ags_osc_builder_packet_free);
  
  /* call parent */
  G_OBJECT_CLASS(ags_osc_builder_parent_class)->finalize(gobject);
}

/**
 * ags_osc_builder_get_class_mutex:
 * 
 * Use this function's returned mutex to access mutex fields.
 *
 * Returns: the class mutex
 * 
 * Since: 2.1.0
 */
pthread_mutex_t*
ags_osc_builder_get_class_mutex()
{
  return(&ags_osc_builder_class_mutex);
}

/**
 * ags_osc_builder_packet_alloc:
 * @offset: the current offset
 * 
 * Allocate #AgsOscBuilderPacket-struct.
 * 
 * Returns: the newly allocated #AgsOscBuilderPacket-struct
 *
 * Since: 2.1.0
 */
AgsOscBuilderPacket*
ags_osc_builder_packet_alloc(guint64 offset)
{
  AgsOscBuilderPacket *packet;

  packet = (AgsOscBuilderPacket *) malloc(sizeof(AgsOscBuilderPacket));

  packet->offset = offset;

  packet->message = NULL;
  packet->bundle = NULL;
  
  return(packet);
}

/**
 * ags_osc_builder_packet_free:
 * @packet: the #AgsOscBuilderPacket-struct
 *
 * Free @packet.
 * 
 * Since: 2.1.0
 */
void
ags_osc_builder_packet_free(AgsOscBuilderPacket *packet)
{
  if(packet == NULL){
    return;
  }

  g_list_free_full(packet->message,
		   ags_osc_builder_message_free);

  g_list_free_full(packet->bundle,
		   ags_osc_builder_bundle_free);

  free(packet);
}

/**
 * ags_osc_builder_bundle_alloc:
 * @offset: the current offset
 * 
 * Allocate #AgsOscBuilderBundle-struct.
 * 
 * Returns: the newly allocated #AgsOscBuilderBundle-struct
 * 
 * Since: 2.1.0
 */
AgsOscBuilderBundle*
ags_osc_builder_bundle_alloc(guint64 offset)
{
  AgsOscBuilderBundle *bundle;

  bundle = (AgsOscBuilderBundle *) malloc(sizeof(AgsOscBuilderBundle));

  bundle->offset = offset;

  bundle->tv_secs = -1;
  bundle->tv_fraction = -1;
  bundle->immediately = FALSE;

  bundle->message = NULL;
  bundle->bundle = NULL;
  
  return(bundle);
}

/**
 * ags_osc_builder_bundle_free:
 * @bundle: the #AgsOscBuilderBundle-struct
 *
 * Free @bundle.
 * 
 * Since: 2.1.0
 */
void
ags_osc_builder_bundle_free(AgsOscBuilderBundle *bundle)
{
  if(bundle == NULL){
    return;
  }

  g_list_free_full(bundle->message,
		   ags_osc_builder_message_free);

  g_list_free_full(bundle->bundle,
		   ags_osc_builder_bundle_free);

  free(bundle);
}

/**
 * ags_osc_builder_message_alloc:
 * @offset: the current offset
 * 
 * Allocate #AgsOscBuilderMessage-struct.
 * 
 * Returns: the newly allocated #AgsOscBuilderMessage-struct
 * 
 * Since: 2.1.0
 */
AgsOscBuilderMessage*
ags_osc_builder_message_alloc(guint64 offset)
{
  AgsOscBuilderMessage *message;

  message = (AgsOscBuilderMessage *) malloc(sizeof(AgsOscBuilderMessage));

  message->offset = offset;

  return(message);
}

/**
 * ags_osc_builder_message_free:
 * @message: the #AgsOscBuilderMessage-struct
 *
 * Free @message.
 * 
 * Since: 2.1.0
 */
void
ags_osc_builder_message_free(AgsOscBuilderMessage *message)
{
  if(message == NULL){
    return;
  }

  if(message->address_pattern != NULL){
    free(message->address_pattern);
  }
  
  if(message->type_tag != NULL){
    free(message->type_tag);
  }

  if(message->data != NULL){
    free(message->data);
  }

  free(message);
}

/**
 * ags_osc_builder_osc_putc:
 * @osc_builder: the #AgsOscBuilder
 * @c: the character
 * 
 * Put character
 * 
 * Since: 2.1.0
 */
void
ags_osc_builder_osc_putc(AgsOscBuilder *osc_builder,
			 gint c)
{
  //TODO:JK: implement me
}

/**
 * ags_osc_builder_on_error:
 * @osc_builder: the #AgsOscBuilder
 * @error: the return location of #GError-struct
 * 
 * Report error.
 * 
 * Since: 2.1.0
 */
void
ags_osc_builder_on_error(AgsOscBuilder *osc_builder,
			 GError **error)
{
  //TODO:JK: implement me
}

/**
 * ags_osc_builder_append_packet:
 * @osc_builder: the #AgsOscBuilder
 * 
 * Since: 2.1.0
 */
void
ags_osc_builder_append_packet(AgsOscBuilder *osc_builder)
{
  //TODO:JK: implement me
}

/**
 * ags_osc_builder_append_bundle:
 * @osc_builder: the #AgsOscBuilder
 * @parent_bundle: the parent #AgsOscBuilderBundle-struct or %NULL
 * @tv_secs: number of seconds since midnight on January 1, 1900
 * @tv_fraction: fraction of seconds to a precision of about 200 picoseconds
 * @immediately: if %TRUE apply immediately, otherwise %FALSE not immediately
 * 
 * Append bundle.
 * 
 * Since: 2.1.0
 */
void
ags_osc_builder_append_bundle(AgsOscBuilder *osc_builder,
			      AgsOscBuilderBundle *parent_bundle,
			      gint32 tv_secs, gint32 tv_fraction, gboolean immediately)
{
  //TODO:JK: implement me
}

/**
 * ags_osc_builder_append_message:
 * @osc_builder: the #AgsOscBuilder
 * @parent_bundle: the parent #AgsOscBuilderBundle-struct or %NULL
 * @address_pattern: the address pattern
 * @type_tag: the type tag
 * 
 * Append message.
 * 
 * Since: 2.1.0
 */
void
ags_osc_builder_append_message(AgsOscBuilder *osc_builder,
			       AgsOscBuilderBundle *parent_bundle,
			       gchar *address_pattern,
			       gchar *type_tag)
{
  //TODO:JK: implement me
}

/**
 * ags_osc_builder_append_value:
 * @osc_builder: the #AgsOscBuilder
 * @type: the type as char
 * @value: the #GValue-struct containinig value
 * 
 * Append value.
 * 
 * Since: 2.1.0
 */
void
ags_osc_builder_append_value(AgsOscBuilder *osc_builder,
			     AgsOscBuilderMessage *message,
			     gchar type,
			     GValue *value)
{
  //TODO:JK: implement me
}

/**
 * ags_osc_builder_build:
 * @osc_builder: the #AgsOscBuilder
 *
 * Build the OSC data.
 * 
 * Since: 2.1.0
 */
void
ags_osc_builder_build(AgsOscBuilder *osc_builder)
{
  //TODO:JK: implement me
}

/**
 * ags_osc_builder_new:
 * 
 * Creates a new instance of #AgsOscBuilder
 *
 * Returns: the new #AgsOscBuilder
 * 
 * Since: 2.1.0
 */
AgsOscBuilder*
ags_osc_builder_new()
{
  AgsOscBuilder *osc_builder;
  
  osc_builder = (AgsOscBuilder *) g_object_new(AGS_TYPE_OSC_BUILDER,
					       NULL);

  
  return(osc_builder);
}
