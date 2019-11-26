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

#ifndef __AGS_PASSWORD_STORE_MANAGER_H__
#define __AGS_PASSWORD_STORE_MANAGER_H__

#include <glib.h>
#include <glib-object.h>

#include <ags/server/security/ags_password_store.h>

#define AGS_TYPE_PASSWORD_STORE_MANAGER                (ags_password_store_manager_get_type())
#define AGS_PASSWORD_STORE_MANAGER(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_PASSWORD_STORE_MANAGER, AgsPasswordStoreManager))
#define AGS_PASSWORD_STORE_MANAGER_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST((class), AGS_TYPE_PASSWORD_STORE_MANAGER, AgsPasswordStoreManagerClass))
#define AGS_IS_PASSWORD_STORE_MANAGER(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AGS_TYPE_PASSWORD_STORE_MANAGER))
#define AGS_IS_PASSWORD_STORE_MANAGER_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE ((class), AGS_TYPE_PASSWORD_STORE_MANAGER))
#define AGS_PASSWORD_STORE_MANAGER_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), AGS_TYPE_PASSWORD_STORE_MANAGER, AgsPasswordStoreManagerClass))

typedef struct _AgsPasswordStoreManager AgsPasswordStoreManager;
typedef struct _AgsPasswordStoreManagerClass AgsPasswordStoreManagerClass;

struct _AgsPasswordStoreManager
{
  GObject gobject;
  
  GList *password_store;
};

struct _AgsPasswordStoreManagerClass
{
  GObjectClass gobject;
};

GType ags_password_store_manager_get_type(void);

GList* ags_password_store_manager_get_password_store(AgsPasswordStoreManager *password_store_manager);

void ags_password_store_manager_add_password_store(AgsPasswordStoreManager *password_store_manager,
						   GObject *password_store);
void ags_password_store_manager_remove_password_store(AgsPasswordStoreManager *password_store_manager,
						      GObject *password_store);

/*  */
gboolean ags_password_store_manager_check_password(AgsPasswordStoreManager *password_store_manager,
						   gchar *login,
						   gchar *password);

/*  */
AgsPasswordStoreManager* ags_password_store_manager_get_instance();

AgsPasswordStoreManager* ags_password_store_manager_new();

#endif /*__AGS_PASSWORD_STORE_MANAGER_H__*/
