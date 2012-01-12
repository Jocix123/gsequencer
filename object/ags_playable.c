/* AGS - Advanced GTK Sequencer
 * Copyright (C) 2005-2011 Joël Krähemann
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <ags/object/ags_playable.h>

void ags_playable_base_init(AgsPlayableInterface *interface);

GType
ags_playable_get_type()
{
  static GType ags_type_playable = 0;

  if(!ags_type_playable){
    static const GTypeInfo ags_playable_info = {
      sizeof(AgsPlayableInterface),
      (GBaseInitFunc) ags_playable_base_init,
      NULL, /* base_finalize */
    };

    ags_type_playable = g_type_register_static(G_TYPE_INTERFACE,
					       "AgsPlayable\0", &ags_playable_info,
					       0);
  }

  return(ags_type_playable);
}


GQuark
ags_channel_error_quark()
{
  return(g_quark_from_static_string("ags-playable-error-quark\0"));
}

void
ags_playable_base_init(AgsPlayableInterface *interface)
{
  /* empty */
}

gboolean
ags_playable_open(AgsPlayable *playable, gchar *name)
{
  AgsPlayableInterface *playable_interface;
  gboolean ret_val;

  g_return_val_if_fail(AGS_IS_PLAYABLE(playable), FALSE);
  playable_interface = AGS_PLAYABLE_GET_INTERFACE(playable);
  g_return_val_if_fail(playable_interface->open, FALSE);
  ret_val = playable_interface->open(playable, name);

  return(ret_val);
}

guint
ags_playable_level_count(AgsPlayable *playable)
{
  AgsPlayableInterface *playable_interface;
  guint ret_val;

  g_return_val_if_fail(AGS_IS_PLAYABLE(playable), 0);
  playable_interface = AGS_PLAYABLE_GET_INTERFACE(playable);
  g_return_val_if_fail(playable_interface->level_count, 0);
  ret_val = playable_interface->level_count(playable);

  return(ret_val);
}

gchar**
ags_playable_sublevel_names(AgsPlayable *playable)
{
  AgsPlayableInterface *playable_interface;
  gchar **ret_val;

  g_return_val_if_fail(AGS_IS_PLAYABLE(playable), NULL);
  playable_interface = AGS_PLAYABLE_GET_INTERFACE(playable);
  g_return_val_if_fail(playable_interface->sublevel_names, NULL);
  ret_val = playable_interface->sublevel_names(playable);

  return(ret_val);
}

/**
 * ags_ipatch_sf2_reader_level_select:
 * @playable an #AgsPlayable
 * @nth_level of type guint
 * @sublevel_name a gchar pointer
 * @error an error that may occure
 *
 * Select a level in an monolythic file where @nth_level and @sublevel_name are equivalent.
 * If @sublevel_name is NULL @nth_level will be chosen.
 */
void
ags_playable_level_select(AgsPlayable *playable, guint nth_level, gchar *sublevel_name, GError **error)
{
  AgsPlayableInterface *playable_interface;

  g_return_if_fail(AGS_IS_PLAYABLE(playable));
  playable_interface = AGS_PLAYABLE_GET_INTERFACE(playable);
  g_return_if_fail(playable_interface->level_select);
  playable_interface->level_select(playable, nth_level, sublevel_name, error);
}

void
ags_playable_iter_start(AgsPlayable *playable)
{
  AgsPlayableInterface *playable_interface;

  g_return_if_fail(AGS_IS_PLAYABLE(playable));
  playable_interface = AGS_PLAYABLE_GET_INTERFACE(playable);
  g_return_if_fail(playable_interface->iter_start);
  playable_interface->iter_start(playable);
}

gboolean
ags_playable_iter_next(AgsPlayable *playable)
{
  AgsPlayableInterface *playable_interface;
  gboolean ret_val;

  g_return_val_if_fail(AGS_IS_PLAYABLE(playable), FALSE);
  playable_interface = AGS_PLAYABLE_GET_INTERFACE(playable);
  g_return_val_if_fail(playable_interface->iter_next, FALSE);
  ret_val = playable_interface->iter_next(playable);

  return(ret_val);
}

void
ags_playable_info(AgsPlayable *playable, guint *channels, guint *frames, guint *loop_start, guint *loop_end)
{
  AgsPlayableInterface *playable_interface;

  g_return_if_fail(AGS_IS_PLAYABLE(playable));
  playable_interface = AGS_PLAYABLE_GET_INTERFACE(playable);
  g_return_if_fail(playable_interface->info);
  playable_interface->info(playable, channels, frames, loop_start, loop_end);
}

short*
ags_playable_read(AgsPlayable *playable, guint channel)
{
  AgsPlayableInterface *playable_interface;

  g_return_if_fail(AGS_IS_PLAYABLE(playable));
  playable_interface = AGS_PLAYABLE_GET_INTERFACE(playable);
  g_return_if_fail(playable_interface->read);
  playable_interface->read(playable, channel);
}

void
ags_playable_close(AgsPlayable *playable)
{
  AgsPlayableInterface *playable_interface;

  g_return_if_fail(AGS_IS_PLAYABLE(playable));
  playable_interface = AGS_PLAYABLE_GET_INTERFACE(playable);
  g_return_if_fail(playable_interface->close);
  playable_interface->close(playable);
}

