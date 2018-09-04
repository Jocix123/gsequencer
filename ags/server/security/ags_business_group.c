/* GSequencer - Advanced GTK Sequencer
 * Copyright (C) 2005-2017 Joël Krähemann
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

#include <ags/server/security/ags_business_group.h>

void ags_business_group_base_init(AgsBusinessGroupInterface *interface);

/**
 * SECTION:ags_business_group
 * @short_description: base group permissions
 * @title: AgsBusinessGroup
 * @section_id: AgsBusinessGroup
 * @include: ags/server/security/ags_business_group.h
 *
 * The #AgsBusinessGroup interface gives you base group permissions.
 */

GType
ags_business_group_get_type()
{
  static volatile gsize g_define_type_id__volatile = 0;

  if(g_once_init_enter (&g_define_type_id__volatile)){
    GType ags_type_business_group = 0;

    static const GTypeInfo ags_business_group_info = {
      sizeof(AgsBusinessGroupInterface),
      (GBaseInitFunc) ags_business_group_base_init,
      NULL, /* base_finalize */
    };

    ags_type_business_group = g_type_register_static(G_TYPE_INTERFACE,
						     "AgsBusinessGroup", &ags_business_group_info,
						     0);

    g_once_init_leave(&g_define_type_id__volatile, ags_type_business_group);
  }

  return g_define_type_id__volatile;
}

void
ags_business_group_base_init(AgsBusinessGroupInterface *interface)
{
  /* empty */
}

/**
 * ags_business_group_set_business_group_name:
 * @business_group: the #AgsBusinessGroup
 * @security_context: the #AgsSecurityContext
 * @login: the login
 * @security_token: the security token
 * @group_id: the group's id
 * @business_group_name: the business group name
 * @error: the #GError-struct
 *
 * Set business group name.
 *
 * Since: 2.0.0
 */
void
ags_business_group_set_business_group_name(AgsBusinessGroup *business_group,
					   GObject *security_context,
					   gchar *login,
					   gchar *security_token,
					   gchar *group_id,
					   gchar *business_group_name,
					   GError **error)
{
  AgsBusinessGroupInterface *business_group_interface;

  g_return_if_fail(AGS_IS_BUSINESS_GROUP(business_group));
  business_group_interface = AGS_BUSINESS_GROUP_GET_INTERFACE(business_group);
  g_return_if_fail(business_group_interface->set_business_group_name);
  business_group_interface->set_business_group_name(business_group,
						    security_context,
						    login,
						    security_token,
						    group_id,
						    business_group_name,
						    error);
}

/**
 * ags_business_group_get_business_group_name:
 * @business_group: the #AgsBusinessGroup
 * @security_context: the #AgsSecurityContext
 * @login: the login
 * @security_token: the security token
 * @group_id: the group's id
 * @error: the #GError-struct
 *
 * Get business group name.
 *
 * Returns: the business group name as string
 * 
 * Since: 2.0.0
 */
gchar*
ags_business_group_get_business_group_name(AgsBusinessGroup *business_group,
					   GObject *security_context,
					   gchar *login,
					   gchar *security_token,
					   gchar *group_id,
					   GError **error)
{
  AgsBusinessGroupInterface *business_group_interface;

  g_return_val_if_fail(AGS_IS_BUSINESS_GROUP(business_group), NULL);
  business_group_interface = AGS_BUSINESS_GROUP_GET_INTERFACE(business_group);
  g_return_val_if_fail(business_group_interface->get_business_group_name, NULL);

  return(business_group_interface->get_business_group_name(business_group,
							   security_context,
							   login,
							   security_token,
							   group_id,
							   error));
}

/**
 * ags_business_group_set_user_id:
 * @business_group: the #AgsBusinessGroup
 * @security_context: the #AgsSecurityContext
 * @login: the login
 * @security_token: the security token
 * @business_group_name: the business group name
 * @user_id: the string array containing ids of users
 * @error: the #GError-struct
 *
 * Set business group ids of users as string array.
 *
 * Since: 2.0.0
 */
void
ags_business_group_set_user_id(AgsBusinessGroup *business_group,
			       GObject *security_context,
			       gchar *login,
			       gchar *security_token,
			       gchar *business_group_name,
			       gchar** user_id,
			       GError **error)
{
  AgsBusinessGroupInterface *business_group_interface;

  g_return_if_fail(AGS_IS_BUSINESS_GROUP(business_group));
  business_group_interface = AGS_BUSINESS_GROUP_GET_INTERFACE(business_group);
  g_return_if_fail(business_group_interface->set_user_id);
  business_group_interface->set_user_id(business_group,
					security_context,
					login,
					security_token,
					business_group_name,
					user_id,
					error);
}

/**
 * ags_business_group_get_user_id:
 * @business_group: the #AgsBusinessGroup
 * @security_context: the #AgsSecurityContext
 * @login: the login
 * @security_token: the security token
 * @business_group_name: the business group name
 * @error: the #GError-struct
 *
 * Get business group name.
 *
 * Returns: the business group's ids of users as string array
 * 
 * Since: 2.0.0
 */
gchar**
ags_business_group_get_user_id(AgsBusinessGroup *business_group,
			       GObject *security_context,
			       gchar *login,
			       gchar *security_token,
			       gchar *business_group_name,
			       GError **error)
{
  AgsBusinessGroupInterface *business_group_interface;

  g_return_val_if_fail(AGS_IS_BUSINESS_GROUP(business_group), NULL);
  business_group_interface = AGS_BUSINESS_GROUP_GET_INTERFACE(business_group);
  g_return_val_if_fail(business_group_interface->get_user_id, NULL);

  return(business_group_interface->get_user_id(business_group,
					       security_context,
					       login,
					       security_token,
					       business_group_name,
					       error));
}

/**
 * ags_business_group_set_context_path_with_read_permission:
 * @business_group: the #AgsBusinessGroup
 * @security_context: the #AgsSecurityContext
 * @login: the login
 * @security_token: the security token
 * @business_group_name: the business group name
 * @context_path: the string array containing context paths
 * @error: the #GError-struct
 *
 * Set business group context paths as string array.
 *
 * Since: 2.0.0
 */
void
ags_business_group_set_context_path_with_read_permission(AgsBusinessGroup *business_group,
							 GObject *security_context,
							 gchar *login,
							 gchar *security_token,
							 gchar *business_group_name,
							 gchar **context_path,
							 GError **error)
{
  AgsBusinessGroupInterface *business_group_interface;

  g_return_if_fail(AGS_IS_BUSINESS_GROUP(business_group));
  business_group_interface = AGS_BUSINESS_GROUP_GET_INTERFACE(business_group);
  g_return_if_fail(business_group_interface->set_context_path_with_read_permission);
  business_group_interface->set_context_path_with_read_permission(business_group,
								  security_context,
								  login,
								  security_token,
								  business_group_name,
								  context_path,
								  error);
}

/**
 * ags_business_group_get_context_path_with_read_permission:
 * @business_group: the #AgsBusinessGroup
 * @security_context: the #AgsSecurityContext
 * @login: the login
 * @security_token: the security token
 * @business_group_name: the business group name
 * @error: the #GError-struct
 *
 * Get business group name.
 *
 * Returns: the business group's context paths as string array
 * 
 * Since: 2.0.0
 */
gchar**
ags_business_group_get_context_path_with_read_permission(AgsBusinessGroup *business_group,
							 GObject *security_context,
							 gchar *login,
							 gchar *security_token,
							 gchar *business_group_name,
							 GError **error)
{
  AgsBusinessGroupInterface *business_group_interface;

  g_return_val_if_fail(AGS_IS_BUSINESS_GROUP(business_group), NULL);
  business_group_interface = AGS_BUSINESS_GROUP_GET_INTERFACE(business_group);
  g_return_val_if_fail(business_group_interface->get_context_path_with_read_permission, NULL);

  return(business_group_interface->get_context_path_with_read_permission(business_group,
									 security_context,
									 login,
									 security_token,
									 business_group_name,
									 error));
}

/**
 * ags_business_group_set_context_path_with_write_permission:
 * @business_group: the #AgsBusinessGroup
 * @security_context: the #AgsSecurityContext
 * @login: the login
 * @security_token: the security token
 * @business_group_name: the business group name
 * @context_path: the string array containing context paths
 * @error: the #GError-struct
 *
 * Set business group context paths as string array.
 *
 * Since: 2.0.0
 */
void
ags_business_group_set_context_path_with_write_permission(AgsBusinessGroup *business_group,
							  GObject *security_context,
							  gchar *login,
							  gchar *security_token,
							  gchar *business_group_name,
							  gchar **context_path,
							  GError **error)
{
  AgsBusinessGroupInterface *business_group_interface;

  g_return_if_fail(AGS_IS_BUSINESS_GROUP(business_group));
  business_group_interface = AGS_BUSINESS_GROUP_GET_INTERFACE(business_group);
  g_return_if_fail(business_group_interface->set_context_path_with_write_permission);
  business_group_interface->set_context_path_with_write_permission(business_group,
								   security_context,
								   login,
								   security_token,
								   business_group_name,
								   context_path,
								   error);
}

/**
 * ags_business_group_get_context_path_with_write_permission:
 * @business_group: the #AgsBusinessGroup
 * @security_context: the #AgsSecurityContext
 * @login: the login
 * @security_token: the security token
 * @business_group_name: the business group name
 * @error: the #GError-struct
 *
 * Get business group name.
 *
 * Returns: the business group's context paths as string array
 * 
 * Since: 2.0.0
 */
gchar**
ags_business_group_get_context_path_with_write_permission(AgsBusinessGroup *business_group,
							  GObject *security_context,
							  gchar *login,
							  gchar *security_token,
							  gchar *business_group_name,
							  GError **error)
{
  AgsBusinessGroupInterface *business_group_interface;

  g_return_val_if_fail(AGS_IS_BUSINESS_GROUP(business_group), NULL);
  business_group_interface = AGS_BUSINESS_GROUP_GET_INTERFACE(business_group);
  g_return_val_if_fail(business_group_interface->get_context_path_with_write_permission, NULL);

  return(business_group_interface->get_context_path_with_write_permission(business_group,
									  security_context,
									  login,
									  security_token,
									  business_group_name,
									  error));
}

/**
 * ags_business_group_set_context_path_with_execute_permission:
 * @business_group: the #AgsBusinessGroup
 * @security_context: the #AgsSecurityContext
 * @login: the login
 * @security_token: the security token
 * @business_group_name: the business group name
 * @context_path: the string array containing context paths
 * @error: the #GError-struct
 *
 * Set business group context paths as string array.
 *
 * Since: 2.0.0
 */
void
ags_business_group_set_context_path_with_execute_permission(AgsBusinessGroup *business_group,
							    GObject *security_context,
							    gchar *login,
							    gchar *security_token,
							    gchar *business_group_name,
							    gchar **context_path,
							    GError **error)
{
  AgsBusinessGroupInterface *business_group_interface;

  g_return_if_fail(AGS_IS_BUSINESS_GROUP(business_group));
  business_group_interface = AGS_BUSINESS_GROUP_GET_INTERFACE(business_group);
  g_return_if_fail(business_group_interface->set_context_path_with_execute_permission);
  business_group_interface->set_context_path_with_execute_permission(business_group,
								     security_context,
								     login,
								     security_token,
								     business_group_name,
								     context_path,
								     error);
}

/**
 * ags_business_group_get_context_path_with_execute_permission:
 * @business_group: the #AgsBusinessGroup
 * @security_context: the #AgsSecurityContext
 * @login: the login
 * @security_token: the security token
 * @business_group_name: the business group name
 * @error: the #GError-struct
 *
 * Get business group name.
 *
 * Returns: the business group's context paths as string array
 * 
 * Since: 2.0.0
 */
gchar**
ags_business_group_get_context_path_with_execute_permission(AgsBusinessGroup *business_group,
							    GObject *security_context,
							    gchar *login,
							    gchar *security_token,
							    gchar *business_group_name,
							    GError **error)
{
  AgsBusinessGroupInterface *business_group_interface;

  g_return_val_if_fail(AGS_IS_BUSINESS_GROUP(business_group), NULL);
  business_group_interface = AGS_BUSINESS_GROUP_GET_INTERFACE(business_group);
  g_return_val_if_fail(business_group_interface->get_context_path_with_execute_permission, NULL);

  return(business_group_interface->get_context_path_with_execute_permission(business_group,
									    security_context,
									    login,
									    security_token,
									    business_group_name,
									    error));
}
