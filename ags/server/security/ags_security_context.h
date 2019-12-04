/* GSequencer - Advanced GTK Sequencer
 * Copyright (C) 2005-2019 Joël Krähemann
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

#ifndef __AGS_SECURITY_CONTEXT_H__
#define __AGS_SECURITY_CONTEXT_H__

#include <glib.h>
#include <glib-object.h>

#include <libxml/tree.h>

G_BEGIN_DECLS

#define AGS_TYPE_SECURITY_CONTEXT                (ags_security_context_get_type())
#define AGS_SECURITY_CONTEXT(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_SECURITY_CONTEXT, AgsSecurityContext))
#define AGS_SECURITY_CONTEXT_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST(class, AGS_TYPE_SECURITY_CONTEXT, AgsSecurityContextClass))
#define AGS_IS_SECURITY_CONTEXT(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AGS_TYPE_SECURITY_CONTEXT))
#define AGS_IS_SECURITY_CONTEXT_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE ((class), AGS_TYPE_SECURITY_CONTEXT))
#define AGS_SECURITY_CONTEXT_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS(obj, AGS_TYPE_SECURITY_CONTEXT, AgsSecurityContextClass))

#define AGS_SECURITY_CONTEXT_GET_OBJ_MUTEX(obj) (&(((AgsSecurityContext *) obj)->obj_mutex))

typedef struct _AgsSecurityContext AgsSecurityContext;
typedef struct _AgsSecurityContextClass AgsSecurityContextClass;

/**
 * AgsSecurityContextMode:
 * @AGS_SECURITY_CONTEXT_RPC_READ: allowed to read using RPC
 * @AGS_SECURITY_CONTEXT_RPC_WRITE: allowed to write using RPC
 * @AGS_SECURITY_CONTEXT_RPC_EXECUTE: allowed to execute using RPC
 * @AGS_SECURITY_CONTEXT_ACCOUNT_READ: allowed to read account information
 * @AGS_SECURITY_CONTEXT_ACCOUNT_WRITE: allowed to write account information
 * 
 * The #AgsSecurityContextMode specifies basic account permissions.
 */
typedef enum{
  AGS_SECURITY_CONTEXT_RPC_READ       = 1,
  AGS_SECURITY_CONTEXT_RPC_WRITE      = 1 <<  1,
  AGS_SECURITY_CONTEXT_RPC_EXECUTE    = 1 <<  2,
  AGS_SECURITY_CONTEXT_ACCOUNT_READ   = 1 <<  3,
  AGS_SECURITY_CONTEXT_ACCOUNT_WRITE  = 1 <<  4,
}AgsSecurityContextMode;

struct _AgsSecurityContext
{
  GObject gobject;

  GRecMutex obj_mutex;
  
  gchar *certs;

  guint server_context_umask;
  
  gchar **business_group;

  gchar **server_context;
};

struct _AgsSecurityContextClass
{
  GObjectClass gobject;
};

GType ags_security_context_get_type();

void ags_security_context_parse_business_group(AgsSecurityContext *security_context,
					       xmlNode *business_group_list);

void ags_security_context_add_server_context(AgsSecurityContext *security_context,
					     gchar *server_context);
gboolean ags_security_context_remove_server_context(AgsSecurityContext *security_context,
						    gchar *server_context);

AgsSecurityContext* ags_security_context_new();

G_END_DECLS

#endif /*__AGS_SECURITY_CONTEXT_H__*/
