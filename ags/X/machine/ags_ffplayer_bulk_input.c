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

#include <ags/X/machine/ags_ffplayer_bulk_input.h>
#include <ags/X/machine/ags_ffplayer_bulk_input_callbacks.h>

void ags_ffplayer_bulk_input_class_init(AgsFFPlayerBulkInputClass *ffplayer_bulk_input);
void ags_ffplayer_bulk_input_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_ffplayer_bulk_input_init(AgsFFPlayerBulkInput *ffplayer_bulk_input);

void ags_ffplayer_bulk_input_connect(AgsConnectable *connectable);
void ags_ffplayer_bulk_input_disconnect(AgsConnectable *connectable);

/**
 * SECTION:ags_ffplayer_bulk_input
 * @short_description: A composite widget to visualize a bunch of #AgsChannel
 * @title: AgsFFPlayerBulkInput
 * @section_id:
 * @include: ags/X/machine/gs_ffplayer_bulk_input.h
 *
 * #AgsFFPlayerBulkInput is a composite widget to visualize one #AgsChannel. It should be
 * packed by an #AgsFFPlayerBulkInput.
 */

static gpointer ags_ffplayer_bulk_input_parent_class = NULL;
static AgsConnectableInterface *ags_ffplayer_bulk_input_parent_connectable_interface;

GType
ags_ffplayer_bulk_input_get_type(void)
{
  static volatile gsize g_define_type_id__volatile = 0;

  if(g_once_init_enter (&g_define_type_id__volatile)){
    GType ags_type_ffplayer_bulk_input = 0;

    static const GTypeInfo ags_ffplayer_bulk_input_info = {
      sizeof(AgsFFPlayerBulkInputClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_ffplayer_bulk_input_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof(AgsFFPlayerBulkInput),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_ffplayer_bulk_input_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_ffplayer_bulk_input_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_ffplayer_bulk_input = g_type_register_static(AGS_TYPE_EFFECT_BULK,
							  "AgsFFPlayerBulkInput", &ags_ffplayer_bulk_input_info,
							  0);

    g_type_add_interface_static(ags_type_ffplayer_bulk_input,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);

    g_once_init_leave(&g_define_type_id__volatile, ags_type_ffplayer_bulk_input);
  }

  return g_define_type_id__volatile;
}

void
ags_ffplayer_bulk_input_class_init(AgsFFPlayerBulkInputClass *ffplayer_bulk_input)
{
  ags_ffplayer_bulk_input_parent_class = g_type_class_peek_parent(ffplayer_bulk_input);
}

void
ags_ffplayer_bulk_input_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_ffplayer_bulk_input_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->is_ready = NULL;
  connectable->is_connected = NULL;
  connectable->connect = ags_ffplayer_bulk_input_connect;
  connectable->disconnect = ags_ffplayer_bulk_input_disconnect;
}

void
ags_ffplayer_bulk_input_init(AgsFFPlayerBulkInput *ffplayer_bulk_input)
{
  AGS_EFFECT_BULK(ffplayer_bulk_input)->channel_type = AGS_TYPE_INPUT;
}

void
ags_ffplayer_bulk_input_connect(AgsConnectable *connectable)
{
  if((AGS_EFFECT_BULK_CONNECTED & (AGS_EFFECT_BULK(connectable)->flags)) != 0){
    return;
  }

  ags_ffplayer_bulk_input_parent_connectable_interface->connect(connectable);

  //TODO:JK: implement me
}

void
ags_ffplayer_bulk_input_disconnect(AgsConnectable *connectable)
{
  if((AGS_EFFECT_BULK_CONNECTED & (AGS_EFFECT_BULK(connectable)->flags)) == 0){
    return;
  }

  ags_ffplayer_bulk_input_parent_connectable_interface->disconnect(connectable);

  //TODO:JK: implement me
}

/**
 * ags_ffplayer_bulk_input_new:
 * @audio: the #AgsAudio to visualize
 * @channel_type: either %AGS_TYPE_INPUT or %AGS_TYPE_OUTPUT
 *
 * Creates an #AgsFFPlayerBulkInput
 *
 * Returns: a new #AgsFFPlayerBulkInput
 *
 * Since: 3.0.0
 */
AgsFFPlayerBulkInput*
ags_ffplayer_bulk_input_new(AgsAudio *audio,
			    GType channel_type)
{
  AgsFFPlayerBulkInput *ffplayer_bulk_input;

  ffplayer_bulk_input = (AgsFFPlayerBulkInput *) g_object_new(AGS_TYPE_FFPLAYER_BULK_INPUT,
							      "audio", audio,
							      "channel-type", channel_type,
							      NULL);

  return(ffplayer_bulk_input);
}
