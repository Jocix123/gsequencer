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

#ifndef __AGS_CONNECTABLE_H__
#define __AGS_CONNECTABLE_H__

#include <glib.h>
#include <glib-object.h>

#include <libxml/tree.h>

#define AGS_TYPE_CONNECTABLE                    (ags_connectable_get_type())
#define AGS_CONNECTABLE(obj)                    (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_CONNECTABLE, AgsConnectable))
#define AGS_CONNECTABLE_INTERFACE(vtable)       (G_TYPE_CHECK_CLASS_CAST((vtable), AGS_TYPE_CONNECTABLE, AgsConnectableInterface))
#define AGS_IS_CONNECTABLE(obj)                 (G_TYPE_CHECK_INSTANCE_TYPE((obj), AGS_TYPE_CONNECTABLE))
#define AGS_IS_CONNECTABLE_INTERFACE(vtable)    (G_TYPE_CHECK_CLASS_TYPE((vtable), AGS_TYPE_CONNECTABLE))
#define AGS_CONNECTABLE_GET_INTERFACE(obj)      (G_TYPE_INSTANCE_GET_INTERFACE((obj), AGS_TYPE_CONNECTABLE, AgsConnectableInterface))

typedef struct _AgsConnectable AgsConnectable;
typedef struct _AgsConnectableInterface AgsConnectableInterface;

struct _AgsConnectableInterface
{
  GTypeInterface ginterface;

  gboolean (*has_resource)(AgsConnectable *connectable);
  gboolean (*is_ready)(AgsConnectable *connectable);
  
  void (*add_to_registry)(AgsConnectable *connectable);
  void (*remove_from_registry)(AgsConnectable *connectable);

  xmlNode* (*update)(AgsConnectable *connectable);
  
  gboolean (*is_connected)(AgsConnectable *connectable);

  void (*connect)(AgsConnectable *connectable);
  void (*disconnect)(AgsConnectable *connectable);

  void (*connect_connection)(AgsConnectable *connectable,
			     GObject *connection);
  void (*disconnect_connection)(AgsConnectable *connectable,
				GObject *connection);
};

GType ags_connectable_get_type();

gboolean ags_connectable_has_resource(AgsConnectable *connectable);
gboolean ags_connectable_is_ready(AgsConnectable *connectable);

void ags_connectable_add_to_registry(AgsConnectable *connectable);
void ags_connectable_remove_from_registry(AgsConnectable *connectable);

xmlNode* ags_connectable_update(AgsConnectable *connectable);

gboolean ags_connectable_is_connected(AgsConnectable *connectable);

void ags_connectable_connect(AgsConnectable *connectable);
void ags_connectable_disconnect(AgsConnectable *connectable);

void ags_connectable_connect_connection(AgsConnectable *connectable,
					GObject *connection);
void ags_connectable_disconnect_connection(AgsConnectable *connectable,
					   GObject *connection);

#endif /*__AGS_CONNECTABLE_H__*/
