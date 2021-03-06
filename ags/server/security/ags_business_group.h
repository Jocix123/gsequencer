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

#ifndef __AGS_BUSINESS_GROUP_H__
#define __AGS_BUSINESS_GROUP_H__

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define AGS_TYPE_BUSINESS_GROUP                    (ags_business_group_get_type())
#define AGS_BUSINESS_GROUP(obj)                    (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_BUSINESS_GROUP, AgsBusinessGroup))
#define AGS_BUSINESS_GROUP_INTERFACE(vtable)       (G_TYPE_CHECK_CLASS_CAST((vtable), AGS_TYPE_BUSINESS_GROUP, AgsBusinessGroupInterface))
#define AGS_IS_BUSINESS_GROUP(obj)                 (G_TYPE_CHECK_INSTANCE_TYPE((obj), AGS_TYPE_BUSINESS_GROUP))
#define AGS_IS_BUSINESS_GROUP_INTERFACE(vtable)    (G_TYPE_CHECK_CLASS_TYPE((vtable), AGS_TYPE_BUSINESS_GROUP))
#define AGS_BUSINESS_GROUP_GET_INTERFACE(obj)      (G_TYPE_INSTANCE_GET_INTERFACE((obj), AGS_TYPE_BUSINESS_GROUP, AgsBusinessGroupInterface))

typedef struct _AgsBusinessGroup AgsBusinessGroup;
typedef struct _AgsBusinessGroupInterface AgsBusinessGroupInterface;

struct _AgsBusinessGroupInterface
{
  GTypeInterface ginterface;
  
  gchar** (*get_group_uuid)(AgsBusinessGroup *business_group,
			    GObject *security_context,
			    gchar *user_uuid,
			    gchar *security_token,
			    GError **error);
  
  void (*set_group_name)(AgsBusinessGroup *business_group,
			 GObject *security_context,
			 gchar *user_uuid,
			 gchar *security_token,
			 gchar *group_uuid,
			 gchar *group_name,
			 GError **error);
  gchar* (*get_group_name)(AgsBusinessGroup *business_group,
			   GObject *security_context,
			   gchar *user_uuid,
			   gchar *security_token,
			   gchar *group_uuid,
			   GError **error);
  
  void (*set_user)(AgsBusinessGroup *business_group,
		   GObject *security_context,
		   gchar *user_uuid,
		   gchar *security_token,
		   gchar *group_uuid,
		   gchar **user,
		   GError **error);
  gchar** (*get_user)(AgsBusinessGroup *business_group,
		      GObject *security_context,
		      gchar *user_uuid,
		      gchar *security_token,
		      gchar *group_uuid,
		      GError **error);
};

GType ags_business_group_get_type();

gchar** ags_business_group_get_group_uuid(AgsBusinessGroup *business_group,
					  GObject *security_context,
					  gchar *user_uuid,
					  gchar *security_token,
					  GError **error);

void ags_business_group_set_group_name(AgsBusinessGroup *business_group,
				       GObject *security_context,
				       gchar *user_uuid,
				       gchar *security_token,
				       gchar *group_uuid,
				       gchar *group_name,
				       GError **error);
gchar* ags_business_group_get_group_name(AgsBusinessGroup *business_group,
					 GObject *security_context,
					 gchar *user_uuid,
					 gchar *security_token,
					 gchar *group_uuid,
					 GError **error);
  
void ags_business_group_set_user(AgsBusinessGroup *business_group,
				 GObject *security_context,
				 gchar *user_uuid,
				 gchar *security_token,
				 gchar *group_uuid,
				 gchar **user,
				 GError **error);
gchar** ags_business_group_get_user(AgsBusinessGroup *business_group,
				    GObject *security_context,
				    gchar *user_uuid,
				    gchar *security_token,
				    gchar *group_uuid,
				    GError **error);

G_END_DECLS

#endif /*__AGS_BUSINESS_GROUP_H__*/
