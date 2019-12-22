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

#include <ags/audio/osc/ags_osc_xmlrpc_server.h>

#include <ags/audio/osc/ags_osc_xmlrpc_connection.h>
#include <ags/audio/osc/ags_osc_xmlrpc_message.h>
#include <ags/audio/osc/ags_osc_websocket_connection.h>

#include <ags/audio/osc/controller/ags_osc_controller.h>
#include <ags/audio/osc/controller/ags_osc_front_controller.h>
#include <ags/audio/osc/controller/ags_osc_action_controller.h>
#include <ags/audio/osc/controller/ags_osc_config_controller.h>
#include <ags/audio/osc/controller/ags_osc_info_controller.h>
#include <ags/audio/osc/controller/ags_osc_meter_controller.h>
#include <ags/audio/osc/controller/ags_osc_node_controller.h>
#include <ags/audio/osc/controller/ags_osc_renew_controller.h>
#include <ags/audio/osc/controller/ags_osc_status_controller.h>

#include <ags/audio/osc/xmlrpc/ags_osc_xmlrpc_controller.h>

#include <libxml/parser.h>
#include <libxml/xlink.h>
#include <libxml/xpath.h>
#include <libxml/valid.h>
#include <libxml/xmlIO.h>
#include <libxml/xmlmemory.h>
#include <libxml/xmlsave.h>

#include <ags/i18n.h>

void ags_osc_xmlrpc_server_class_init(AgsOscXmlrpcServerClass *osc_xmlrpc_server);
void ags_osc_xmlrpc_server_init(AgsOscXmlrpcServer *osc_xmlrpc_server);
void ags_osc_xmlrpc_server_set_property(GObject *gobject,
					guint prop_id,
					const GValue *value,
					GParamSpec *param_spec);
void ags_osc_xmlrpc_server_get_property(GObject *gobject,
					guint prop_id,
					GValue *value,
					GParamSpec *param_spec);
void ags_osc_xmlrpc_server_dispose(GObject *gobject);
void ags_osc_xmlrpc_server_finalize(GObject *gobject);

void ags_osc_xmlrpc_server_start(AgsOscServer *osc_server);
void ags_osc_xmlrpc_server_stop(AgsOscServer *osc_server);

void ags_osc_xmlrpc_server_websocket_callback(SoupServer *server,
					      SoupWebsocketConnection *connection,
					      const char *path,
					      SoupClientContext *client,
					      AgsOscXmlrpcServer *osc_xmlrpc_server);

/**
 * SECTION:ags_osc_xmlrpc_server
 * @short_description: the OSC XMLRPC server
 * @title: AgsOscXmlrpcServer
 * @section_id:
 * @include: ags/audio/osc/ags_osc_xmlrpc_server.h
 *
 * #AgsOscXmlrpcServer your OSC XMLRPC server.
 */

enum{
  PROP_0,
  PROP_OSC_XMLRPC_CONTROLLER,
  PROP_XMLRPC_SERVER,
};

static gpointer ags_osc_xmlrpc_server_parent_class = NULL;

GType
ags_osc_xmlrpc_server_get_type(void)
{
  static volatile gsize g_define_type_id__volatile = 0;

  if(g_once_init_enter (&g_define_type_id__volatile)){
    GType ags_type_osc_xmlrpc_server = 0;

    static const GTypeInfo ags_osc_xmlrpc_server_info = {
      sizeof (AgsOscXmlrpcServerClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_osc_xmlrpc_server_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsOscXmlrpcServer),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_osc_xmlrpc_server_init,
    };

    ags_type_osc_xmlrpc_server = g_type_register_static(AGS_TYPE_OSC_SERVER,
							"AgsOscXmlrpcServer", &ags_osc_xmlrpc_server_info,
							0);

    g_once_init_leave(&g_define_type_id__volatile, ags_type_osc_xmlrpc_server);
  }

  return g_define_type_id__volatile;
}

void
ags_osc_xmlrpc_server_class_init(AgsOscXmlrpcServerClass *osc_xmlrpc_server)
{
  AgsOscServerClass *osc_server;
  GObjectClass *gobject;

  GParamSpec *param_spec;
  
  ags_osc_xmlrpc_server_parent_class = g_type_class_peek_parent(osc_xmlrpc_server);

  /* GObjectClass */
  gobject = (GObjectClass *) osc_xmlrpc_server;

  gobject->set_property = ags_osc_xmlrpc_server_set_property;
  gobject->get_property = ags_osc_xmlrpc_server_get_property;

  gobject->dispose = ags_osc_xmlrpc_server_dispose;
  gobject->finalize = ags_osc_xmlrpc_server_finalize;

  /* properties */
  /**
   * AgsOscXmlrpcServer:xmlrpc-server:
   *
   * The assigned #AgsServer.
   * 
   * Since: 3.0.0
   */
  param_spec = g_param_spec_object("xmlrpc-server",
				   i18n_pspec("assigned server"),
				   i18n_pspec("The server it is assigned with"),
				   AGS_TYPE_SERVER,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_XMLRPC_SERVER,
				  param_spec);

  /**
   * AgsOscXmlrpcServer:osc-xmlrpc-controller:
   *
   * The assigned #AgsOscXmlrpcController.
   * 
   * Since: 3.0.0
   */
  param_spec = g_param_spec_object("osc-xmlrpc-controller",
				   i18n_pspec("OSC XMLRPC controller"),
				   i18n_pspec("The OSC XMLRPC controller"),
				   AGS_TYPE_CONTROLLER,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_OSC_XMLRPC_CONTROLLER,
				  param_spec);

  /* AgsOscServerClass */
  osc_server = (AgsOscServerClass *) osc_xmlrpc_server;

  osc_server->start = ags_osc_xmlrpc_server_start;
  osc_server->stop = ags_osc_xmlrpc_server_stop;

  osc_server->listen = NULL;
  osc_server->dispatch = NULL;
}

void
ags_osc_xmlrpc_server_init(AgsOscXmlrpcServer *osc_xmlrpc_server)
{
  osc_xmlrpc_server->xmlrpc_server = NULL;

  osc_xmlrpc_server->osc_xmlrpc_controller = NULL;
}

void
ags_osc_xmlrpc_server_set_property(GObject *gobject,
				   guint prop_id,
				   const GValue *value,
				   GParamSpec *param_spec)
{
  AgsOscXmlrpcServer *osc_xmlrpc_server;

  GRecMutex *osc_server_mutex;

  osc_xmlrpc_server = AGS_OSC_XMLRPC_SERVER(gobject);

  /* get osc server mutex */
  osc_server_mutex = AGS_OSC_SERVER_GET_OBJ_MUTEX(osc_xmlrpc_server);
  
  switch(prop_id){
  case PROP_XMLRPC_SERVER:
  {
    GObject *xmlrpc_server;

    xmlrpc_server = g_value_get_object(value);

    g_rec_mutex_lock(osc_server_mutex);

    if(osc_xmlrpc_server->xmlrpc_server == xmlrpc_server){
      g_rec_mutex_unlock(osc_server_mutex);

      return;
    }

    if(osc_xmlrpc_server->xmlrpc_server != NULL){
      g_object_unref(osc_xmlrpc_server->xmlrpc_server);
    }
      
    if(xmlrpc_server != NULL){
      g_object_ref(xmlrpc_server);
    }
      
    osc_xmlrpc_server->xmlrpc_server = xmlrpc_server;

    g_rec_mutex_unlock(osc_server_mutex);
  }
  break;
  case PROP_OSC_XMLRPC_CONTROLLER:
  {
    GObject *osc_xmlrpc_controller;

    osc_xmlrpc_controller = g_value_get_object(value);

    g_rec_mutex_lock(osc_server_mutex);

    if(osc_xmlrpc_server->osc_xmlrpc_controller == osc_xmlrpc_controller){
      g_rec_mutex_unlock(osc_server_mutex);

      return;
    }

    if(osc_xmlrpc_server->osc_xmlrpc_controller != NULL){
      g_object_unref(osc_xmlrpc_server->osc_xmlrpc_controller);
    }
      
    if(osc_xmlrpc_controller != NULL){
      g_object_ref(osc_xmlrpc_controller);
    }
      
    osc_xmlrpc_server->osc_xmlrpc_controller = osc_xmlrpc_controller;

    g_rec_mutex_unlock(osc_server_mutex);
  }
  break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_osc_xmlrpc_server_get_property(GObject *gobject,
				   guint prop_id,
				   GValue *value,
				   GParamSpec *param_spec)
{
  AgsOscXmlrpcServer *osc_xmlrpc_server;

  GRecMutex *osc_server_mutex;

  osc_xmlrpc_server = AGS_OSC_XMLRPC_SERVER(gobject);

  /* get osc server mutex */
  osc_server_mutex = AGS_OSC_SERVER_GET_OBJ_MUTEX(osc_xmlrpc_server);
  
  switch(prop_id){
  case PROP_XMLRPC_SERVER:
  {
    g_rec_mutex_lock(osc_server_mutex);

    g_value_set_object(value, osc_xmlrpc_server->xmlrpc_server);

    g_rec_mutex_unlock(osc_server_mutex);
  }
  break;
  case PROP_OSC_XMLRPC_CONTROLLER:
  {
    g_rec_mutex_lock(osc_server_mutex);

    g_value_set_object(value, osc_xmlrpc_server->osc_xmlrpc_controller);

    g_rec_mutex_unlock(osc_server_mutex);
  }
  break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_osc_xmlrpc_server_dispose(GObject *gobject)
{
  AgsOscXmlrpcServer *osc_xmlrpc_server;
  
  osc_xmlrpc_server = (AgsOscXmlrpcServer *) gobject;

  if(osc_xmlrpc_server->xmlrpc_server != NULL){
    g_object_unref(osc_xmlrpc_server->xmlrpc_server);

    osc_xmlrpc_server->xmlrpc_server = NULL;
  }

  if(osc_xmlrpc_server->osc_xmlrpc_controller != NULL){
    g_object_unref(osc_xmlrpc_server->osc_xmlrpc_controller);

    osc_xmlrpc_server->osc_xmlrpc_controller = NULL;
  }
  
  /* call parent */
  G_OBJECT_CLASS(ags_osc_xmlrpc_server_parent_class)->dispose(gobject);
}

void
ags_osc_xmlrpc_server_finalize(GObject *gobject)
{
  AgsOscXmlrpcServer *osc_xmlrpc_server;
    
  osc_xmlrpc_server = (AgsOscXmlrpcServer *) gobject;    

  if(osc_xmlrpc_server->xmlrpc_server != NULL){
    g_object_unref(osc_xmlrpc_server->xmlrpc_server);
  }

  if(osc_xmlrpc_server->osc_xmlrpc_controller != NULL){
    g_object_unref(osc_xmlrpc_server->osc_xmlrpc_controller);
  }

  /* call parent */
  G_OBJECT_CLASS(ags_osc_xmlrpc_server_parent_class)->finalize(gobject);
}

void
ags_osc_xmlrpc_server_start(AgsOscServer *osc_server)
{
  AgsOscXmlrpcController *osc_xmlrpc_controller;
  
  GList *start_controller, *controller;

  if(ags_osc_server_test_flags(osc_server, AGS_OSC_SERVER_STARTED)){
    return;
  }
  
  ags_osc_server_set_flags(osc_server, AGS_OSC_SERVER_STARTED);

  ags_osc_server_set_flags(osc_server, AGS_OSC_SERVER_RUNNING);

  g_message("starting OSC XMLRPC threads");

  /* controller */
  g_object_get(osc_server,
	       "osc-xmlrpc-controller", &osc_xmlrpc_controller,
	       "controller", &start_controller,
	       NULL);

  ags_osc_xmlrpc_controller_start_delegate(osc_xmlrpc_controller);
  
  controller = start_controller;

  while(controller != NULL){
    if(AGS_IS_OSC_METER_CONTROLLER(controller->data)){
      ags_osc_meter_controller_start_monitor(controller->data);
    }
    
    controller = controller->next;
  }

  g_object_unref(osc_xmlrpc_controller);
  
  g_list_free_full(start_controller,
		   g_object_unref);
}

void
ags_osc_xmlrpc_server_stop(AgsOscServer *osc_server)
{
  AgsOscXmlrpcController *osc_xmlrpc_controller;
  
  GList *start_controller, *controller;

  if(!ags_osc_server_test_flags(osc_server, AGS_OSC_SERVER_RUNNING)){
    return;
  }
  
  /* stop */
  ags_osc_server_set_flags(osc_server, AGS_OSC_SERVER_TERMINATING);
  ags_osc_server_unset_flags(osc_server, AGS_OSC_SERVER_RUNNING);

  /* controller */
  g_object_get(osc_server,
	       "osc-xmlrpc-controller", &osc_xmlrpc_controller,
	       "controller", &start_controller,
	       NULL);

  ags_osc_xmlrpc_controller_stop_delegate(osc_xmlrpc_controller);
  
  controller = start_controller;

  while(controller != NULL){
    if(AGS_IS_OSC_METER_CONTROLLER(controller->data)){
      ags_osc_meter_controller_stop_monitor(controller->data);
    }
    
    controller = controller->next;
  }

  g_object_unref(osc_xmlrpc_controller);
  
  g_list_free_full(start_controller,
		   g_object_unref);

  ags_osc_server_unset_flags(osc_server, (AGS_OSC_SERVER_STARTED |
					  AGS_OSC_SERVER_TERMINATING));
}

/**
 * ags_osc_xmlrpc_server_find_xmlrpc_connection:
 * @osc_xmlrpc_server: the #AgsOscXmlrpcServer
 * @client: the #SoupeClientContext-struct
 * 
 * Find @client in @osc_xmlrpc_server's connection property.
 * 
 * Returns: the matching #GObject or %NULL
 * 
 * Since: 3.0.0
 */
GObject*
ags_osc_xmlrpc_server_find_xmlrpc_connection(AgsOscXmlrpcServer *osc_xmlrpc_server,
					     SoupClientContext *client)
{
  AgsOscXmlrpcConnection *osc_xmlrpc_connection;

  GList *start_list, *list;

  if(!AGS_IS_OSC_XMLRPC_SERVER(osc_xmlrpc_server)){
    return(NULL);
  }
  
  osc_xmlrpc_connection = NULL;

  g_object_get(osc_xmlrpc_server,
	       "connection", &start_list,
	       NULL);

  list = start_list;

  while(list != NULL){
    SoupClientContext *current;

    g_object_get(list->data,
		 "client", &current,
		 NULL);

    if(current == client){
      osc_xmlrpc_connection = list->data;
    }

    if(osc_xmlrpc_connection != NULL){
      break;
    }
    
    list = list->next;
  }

  g_list_free_full(start_list,
		   g_object_unref);
  
  return(osc_xmlrpc_connection);
}

/**
 * ags_osc_xmlrpc_server_find_websocket_connection:
 * @osc_xmlrpc_server: the #AgsOscXmlrpcServer
 * @websocket_connection: the #SoupWebsocketConnection
 * 
 * Find @websocket_connection in @osc_xmlrpc_server's connection property.
 * 
 * Returns: the matching #GObject or %NULL
 * 
 * Since: 3.0.0
 */
GObject*
ags_osc_xmlrpc_server_find_websocket_connection(AgsOscXmlrpcServer *osc_xmlrpc_server,
						SoupWebsocketConnection *websocket_connection)
{
  AgsOscWebsocketConnection *osc_websocket_connection;

  GList *start_list, *list;

  if(!AGS_IS_OSC_XMLRPC_SERVER(osc_xmlrpc_server)){
    return(NULL);
  }
  
  osc_websocket_connection = NULL;

  g_object_get(osc_xmlrpc_server,
	       "connection", &start_list,
	       NULL);

  list = start_list;

  while(list != NULL){
    SoupWebsocketConnection *current;

    g_object_get(list->data,
		 "websocket-connection", &current,
		 NULL);

    if(current == websocket_connection){
      osc_websocket_connection = list->data;
    }

    g_object_unref(current);

    if(osc_websocket_connection != NULL){
      break;
    }
    
    list = list->next;
  }

  g_list_free_full(start_list,
		   g_object_unref);
  
  return(osc_websocket_connection);
}

/**
 * ags_osc_xmlrpc_server_add_websocket_handler:
 * @osc_xmlrpc_server: the #AgsOscXmlrpcServer
 * @path: the path
 * @origin: the origin
 * @protocols: the protocls as string vector
 * @callback: the callback
 * @user_data: user data
 * @destroy: destroy notify function
 * 
 * Add websocket handler to soup server. 
 *
 * Since: 3.0.0
 */
void
ags_osc_xmlrpc_server_add_websocket_handler(AgsOscXmlrpcServer *osc_xmlrpc_server,
					    char *path,
					    char *origin,
					    char **protocols,
					    SoupServerWebsocketCallback callback,
					    gpointer user_data,
					    GDestroyNotify destroy)
{
  AgsServer *xmlrpc_server;
    
  GRecMutex *server_mutex;

  if(!AGS_IS_OSC_XMLRPC_SERVER(osc_xmlrpc_server)){
    return;
  }

  g_object_get(osc_xmlrpc_server,
	       "xmlrpc-server", &xmlrpc_server,
	       NULL);

  if(!AGS_IS_SERVER(xmlrpc_server)){
    return;
  }

  /* get server mutex */
  server_mutex = AGS_SERVER_GET_OBJ_MUTEX(xmlrpc_server);

  g_rec_mutex_lock(server_mutex);

  soup_server_add_websocket_handler(xmlrpc_server->soup_server,
				    path,
				    origin,
				    protocols,
				    callback,
				    user_data,
				    destroy);
  
  g_rec_mutex_unlock(server_mutex);

  g_object_unref(xmlrpc_server);
}

void
ags_osc_xmlrpc_server_add_default_controller(AgsOscXmlrpcServer *osc_xmlrpc_server)
{  
  AgsOscActionController *action_controller;
  AgsOscConfigController *config_controller;
  AgsOscInfoController *info_controller;
  AgsOscMeterController *meter_controller;
  AgsOscNodeController *node_controller;
  AgsOscRenewController *renew_controller;
  AgsOscStatusController *status_controller;
  
  AgsServer *xmlrpc_server;  
  AgsOscXmlrpcController *osc_xmlrpc_controller;

  gchar *osc_monitor_path;
  
  if(!AGS_IS_OSC_XMLRPC_SERVER(osc_xmlrpc_server)){
    return;
  }
  
  g_object_get(osc_xmlrpc_server,
	       "xmlrpc-server", &xmlrpc_server,
	       NULL);

  if(!AGS_IS_SERVER(xmlrpc_server)){
    return;
  }

  /* XMLRPC controller */
  osc_xmlrpc_controller = ags_osc_xmlrpc_controller_new();
  g_object_set(osc_xmlrpc_controller,
	       "server", xmlrpc_server,
	       "osc-xmlrpc-server", osc_xmlrpc_server,
	       NULL);

  g_object_set(osc_xmlrpc_server,
	       "osc-xmlrpc-controller", osc_xmlrpc_controller,
	       NULL);
  
  ags_server_add_controller(xmlrpc_server,
			    (GObject *) osc_xmlrpc_controller);
  
  /* action controller */
  action_controller = ags_osc_action_controller_new();
  g_object_set(action_controller,
	       "osc-server", osc_xmlrpc_server,
	       NULL);

  ags_osc_server_add_controller(osc_xmlrpc_server,
				(GObject *) action_controller);

  /* config controller */
  config_controller = ags_osc_config_controller_new();
  g_object_set(config_controller,
	       "osc-server", osc_xmlrpc_server,
	       NULL);

  ags_osc_server_add_controller(osc_xmlrpc_server,
				(GObject *) config_controller);

  /* info controller */
  info_controller = ags_osc_info_controller_new();
  g_object_set(info_controller,
	       "osc-server", osc_xmlrpc_server,
	       NULL);

  ags_osc_server_add_controller(osc_xmlrpc_server,
				(GObject *) info_controller);

  /* meter controller */
  meter_controller = ags_osc_meter_controller_new();
  g_object_set(meter_controller,
	       "osc-server", osc_xmlrpc_server,
	       NULL);

  ags_osc_server_add_controller(osc_xmlrpc_server,
				(GObject *) meter_controller);

  /* node controller */
  node_controller = ags_osc_node_controller_new();
  g_object_set(node_controller,
	       "osc-server", osc_xmlrpc_server,
	       NULL);

  ags_osc_server_add_controller(osc_xmlrpc_server,
				(GObject *) node_controller);

  /* renew controller */
  renew_controller = ags_osc_renew_controller_new();
  g_object_set(renew_controller,
	       "osc-server", osc_xmlrpc_server,
	       NULL);

  ags_osc_server_add_controller(osc_xmlrpc_server,
				(GObject *) renew_controller);

  /* status controller */
  status_controller = ags_osc_status_controller_new();
  g_object_set(status_controller,
	       "osc-server", osc_xmlrpc_server,
	       NULL);

  ags_osc_server_add_controller(osc_xmlrpc_server,
				(GObject *) status_controller);

  /* OSC monitor websocket handler */
  osc_monitor_path = g_strdup_printf("%s/ags-osc-over-xmlrpc/monitor",
				     AGS_CONTROLLER_BASE_PATH);
  
  ags_osc_xmlrpc_server_add_websocket_handler(osc_xmlrpc_server,
					      osc_monitor_path,
					      NULL,
					      NULL,
					      ags_osc_xmlrpc_server_websocket_callback,
					      osc_xmlrpc_server,
					      NULL);
}

void
ags_osc_xmlrpc_server_websocket_callback(SoupServer *server,
					 SoupWebsocketConnection *websocket_connection,
					 const char *path,
					 SoupClientContext *client,
					 AgsOscXmlrpcServer *osc_xmlrpc_server)
{
  AgsOscXmlrpcController *osc_xmlrpc_controller;
  AgsOscXmlrpcMessage *osc_xmlrpc_message;

  AgsAuthenticationManager *authentication_manager;
  AgsSecurityContext *security_context;

  AgsLoginInfo *login_info;

  xmlDoc *doc;

  GIOStream *stream;
  GInputStream *input_stream;

  xmlXPathContext *xpath_context; 
  xmlXPathObject *xpath_object;
  xmlNode **node;
  xmlNode *root_node;
  
  guchar buffer[AGS_OSC_XMLRPC_SERVER_DEFAULT_CHUNK_SIZE];
  gchar *xpath;
  gchar *login;
  gchar *user_uuid;
  gchar *security_token;
  gchar *resource_id;
  
  gsize num_read;
  guint i;
  
  GError *error;
  
  GRecMutex *controller_mutex;
  GRecMutex *authentication_manager_mutex;

  g_object_get(osc_xmlrpc_server,
	       "osc-xmlrpc-controller", &osc_xmlrpc_controller,
	       NULL);

  if(osc_xmlrpc_controller == NULL){
    return;
  }
  
  controller_mutex = AGS_CONTROLLER_GET_OBJ_MUTEX(osc_xmlrpc_controller);

  authentication_manager = ags_authentication_manager_get_instance();
  
  authentication_manager_mutex = AGS_AUTHENTICATION_MANAGER_GET_OBJ_MUTEX(authentication_manager);
  
  stream = soup_websocket_connection_get_io_stream(websocket_connection);
  input_stream = g_io_stream_get_input_stream(stream);

  error = NULL;
  num_read = g_input_stream_read(input_stream,
				 buffer,
				 AGS_OSC_XMLRPC_SERVER_DEFAULT_CHUNK_SIZE,
				 NULL,
				 &error);

  if(error != NULL){
    g_warning("%s", error->message);

    g_error_free(error);
  }

  if(num_read <= 0){
    return;
  }
  
  buffer[num_read] = '\0';

  doc = xmlParseDoc(buffer);

  if(doc == NULL){
    return;
  }

  root_node = xmlDocGetRootElement(doc);

  if(root_node == NULL){
    return;
  }
  
  login = NULL;
  security_token = NULL;

  resource_id = NULL;

  /* login */
  xpath = "/ags-osc-over-xmlrpc/ags-srv-login";

  xpath_context = xmlXPathNewContext(doc);
  xpath_object = xmlXPathNodeEval(root_node,
				  xpath,
				  xpath_context);
  
  if(xpath_object->nodesetval != NULL){
    node = xpath_object->nodesetval->nodeTab;

    for(i = 0; i < xpath_object->nodesetval->nodeNr; i++){
      if(node[i]->type == XML_ELEMENT_NODE){
	xmlChar *tmp_login;
	
	tmp_login = xmlNodeGetContent(node[i]);

	login = g_strdup(tmp_login);

	xmlFree(tmp_login);
	
	break;
      }
    }
  }

  xmlXPathFreeObject(xpath_object);
  xmlXPathFreeContext(xpath_context);

  /* security token */
  xpath = "/ags-osc-over-xmlrpc/ags-srv-security-token";

  xpath_context = xmlXPathNewContext(doc);
  xpath_object = xmlXPathNodeEval(root_node,
				  xpath,
				  xpath_context);
  
  if(xpath_object->nodesetval != NULL){
    node = xpath_object->nodesetval->nodeTab;

    for(i = 0; i < xpath_object->nodesetval->nodeNr; i++){
      if(node[i]->type == XML_ELEMENT_NODE){
	xmlChar *tmp_security_token;
	
	tmp_security_token = xmlNodeGetContent(node[i]);

	security_token = g_strdup(tmp_security_token);

	xmlFree(tmp_security_token);
	
	break;
      }
    }
  }

  xmlXPathFreeObject(xpath_object);
  xmlXPathFreeContext(xpath_context);

  /* security token */
  xpath = "/ags-osc-over-xmlrpc/ags-srv-redirect";

  xpath_context = xmlXPathNewContext(doc);
  xpath_object = xmlXPathNodeEval(root_node,
				  xpath,
				  xpath_context);
  
  if(xpath_object->nodesetval != NULL){
    node = xpath_object->nodesetval->nodeTab;

    for(i = 0; i < xpath_object->nodesetval->nodeNr; i++){
      if(node[i]->type == XML_ELEMENT_NODE){
	xmlChar *tmp_resource_id;
	
	tmp_resource_id = xmlGetProp(node[i],
				     "resource-id");

	resource_id = g_strdup(tmp_resource_id);

	xmlFree(tmp_resource_id);
	
	break;
      }
    }
  }

  xmlXPathFreeObject(xpath_object);
  xmlXPathFreeContext(xpath_context);

  if(login == NULL ||
     security_token == NULL ||
     resource_id == NULL){
    return;
  }
  
  login_info = ags_authentication_manager_lookup_login(authentication_manager,
						       login);

  if(login_info != NULL){
    g_rec_mutex_lock(authentication_manager_mutex);

    security_context = login_info->security_context;
    g_object_ref(security_context);

    user_uuid = g_strdup(login_info->user_uuid);
    
    g_rec_mutex_unlock(authentication_manager_mutex);
  
    if(ags_authentication_manager_is_session_active(authentication_manager,
						    security_context,
						    user_uuid,
						    security_token)){
      AgsOscWebsocketConnection *osc_websocket_connection;

      GList *start_controller, *controller;
      GList *start_message, *message;

      gpointer start_osc_response;

      g_rec_mutex_lock(controller_mutex);

      start_message = g_list_copy_deep(osc_xmlrpc_controller->message,
				       (GCopyFunc) g_object_ref,
				       NULL);
      
      g_rec_mutex_unlock(controller_mutex);

      message = ags_osc_xmlrpc_message_find_resource_id(start_message,
							resource_id);

      if(message != NULL){
	AgsOscMessage *current;

	current = message->data;
	
	osc_websocket_connection = ags_osc_xmlrpc_server_find_websocket_connection(osc_xmlrpc_server,
										   websocket_connection);

	if(osc_websocket_connection == NULL){
	  osc_websocket_connection = ags_osc_websocket_connection_new(osc_xmlrpc_server);
	  g_object_set(osc_websocket_connection,
		       "websocket-connection", websocket_connection,
		       "client", client,
		       "security-context", security_context,
		       "path", path,
		       "login", login,
		       "security-token", security_token,
		       NULL);
	
	  ags_osc_server_add_connection(osc_xmlrpc_server,
					osc_websocket_connection);

	  g_object_set(current,
		       "osc-connection", osc_websocket_connection,
		       NULL);	  
	}

	g_object_get(osc_xmlrpc_server,
		     "controller", &start_controller,
		     NULL);

	controller = ags_list_util_find_type(start_controller,
					     AGS_TYPE_OSC_METER_CONTROLLER);

	if(controller != NULL){
	  start_osc_response = ags_osc_meter_controller_monitor_meter(controller->data,
								      current->osc_connection,
								      current->message, current->message_size);

	  g_list_free_full(start_osc_response,
			   g_object_unref);
	}

	g_list_free_full(start_controller,
			 g_object_unref);
      }
      
      g_list_free_full(start_message,
		       g_object_unref);
    }
    
    g_object_unref(security_context);

    ags_login_info_unref(login_info);

    g_free(user_uuid);
  }

  g_free(login);
  g_free(security_token);
}

/**
 * ags_osc_xmlrpc_server_new:
 * 
 * Creates a new instance of #AgsOscXmlrpcServer
 *
 * Returns: the new #AgsOscXmlrpcServer
 * 
 * Since: 3.0.0
 */
AgsOscXmlrpcServer*
ags_osc_xmlrpc_server_new()
{
  AgsOscXmlrpcServer *osc_xmlrpc_server;
   
  osc_xmlrpc_server = (AgsOscXmlrpcServer *) g_object_new(AGS_TYPE_OSC_XMLRPC_SERVER,
							  NULL);
  
  return(osc_xmlrpc_server);
}
