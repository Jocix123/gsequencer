/* GSequencer - Advanced GTK Sequencer
 * Copyright (C) 2005-2020 Joël Krähemann
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

#ifndef __AGS_DSSI_MANAGER_H__
#define __AGS_DSSI_MANAGER_H__

#include <glib.h>
#include <glib-object.h>

#include <ags/libags.h>

#include <ags/plugin/ags_dssi_plugin.h>

G_BEGIN_DECLS

#define AGS_TYPE_DSSI_MANAGER                (ags_dssi_manager_get_type())
#define AGS_DSSI_MANAGER(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_DSSI_MANAGER, AgsDssiManager))
#define AGS_DSSI_MANAGER_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST((class), AGS_TYPE_DSSI_MANAGER, AgsDssiManagerClass))
#define AGS_IS_DSSI_MANAGER(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AGS_TYPE_DSSI_MANAGER))
#define AGS_IS_DSSI_MANAGER_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE ((class), AGS_TYPE_DSSI_MANAGER))
#define AGS_DSSI_MANAGER_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), AGS_TYPE_DSSI_MANAGER, AgsDssiManagerClass))

#define AGS_DSSI_MANAGER_GET_OBJ_MUTEX(obj) (&(((AgsDssiManager *) obj)->obj_mutex))

typedef struct _AgsDssiManager AgsDssiManager;
typedef struct _AgsDssiManagerClass AgsDssiManagerClass;

struct _AgsDssiManager
{
  GObject gobject;

  GRecMutex obj_mutex;
  
  GList *dssi_plugin_blacklist;
  GList *dssi_plugin;
};

struct _AgsDssiManagerClass
{
  GObjectClass gobject;
};

GType ags_dssi_manager_get_type(void);

gchar** ags_dssi_manager_get_default_path();
void ags_dssi_manager_set_default_path(gchar** default_path);

gchar** ags_dssi_manager_get_filenames(AgsDssiManager *dssi_manager);
AgsDssiPlugin* ags_dssi_manager_find_dssi_plugin(AgsDssiManager *dssi_manager,
						 gchar *filename, gchar *effect);
AgsDssiPlugin* ags_dssi_manager_find_dssi_plugin_with_fallback(AgsDssiManager *dssi_manager,
							       gchar *filename, gchar *effect);

void ags_dssi_manager_load_blacklist(AgsDssiManager *dssi_manager,
				     gchar *blacklist_filename);

void ags_dssi_manager_load_file(AgsDssiManager *dssi_manager,
				gchar *dssi_path,
				gchar *filename);
void ags_dssi_manager_load_default_directory(AgsDssiManager *dssi_manager);

/*  */
AgsDssiManager* ags_dssi_manager_get_instance();

AgsDssiManager* ags_dssi_manager_new();

G_END_DECLS

#endif /*__AGS_DSSI_MANAGER_H__*/
