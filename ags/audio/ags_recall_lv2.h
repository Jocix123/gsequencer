/* GSequencer - Advanced GTK Sequencer
 * Copyright (C) 2005-2015 Joël Krähemann
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

#ifndef __AGS_RECALL_LV2_H__
#define __AGS_RECALL_LV2_H__

#include <glib.h>
#include <glib-object.h>

#include <lv2.h>

#include <ags/libags.h>

#include <ags/plugin/ags_lv2_plugin.h>

#include <ags/audio/ags_recall.h>
#include <ags/audio/ags_recall_channel_run.h>
#include <ags/audio/ags_channel.h>

#define AGS_TYPE_RECALL_LV2                (ags_recall_lv2_get_type())
#define AGS_RECALL_LV2(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_RECALL_LV2, AgsRecallLv2))
#define AGS_RECALL_LV2_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST((class), AGS_TYPE_RECALL_LV2, AgsRecallLv2Class))
#define AGS_IS_RECALL_LV2(obj)             (G_TYPE_CHECK_INSTANCE_TYPE((obj), AGS_TYPE_RECALL_LV2))
#define AGS_IS_RECALL_LV2_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE((class), AGS_TYPE_RECALL_LV2))
#define AGS_RECALL_LV2_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS((obj), AGS_TYPE_RECALL_LV2, AgsRecallLv2Class))

typedef struct _AgsRecallLv2 AgsRecallLv2;
typedef struct _AgsRecallLv2Class AgsRecallLv2Class;

/**
 * AgsRecallLv2FLags:
 * @AGS_RECALL_LV2_HAS_EVENT_PORT: has event port
 * @AGS_RECALL_LV2_HAS_ATOM_PORT: has atom port
 * @AGS_RECALL_LV2_HAS_WORKER: has worker
 * 
 * Enum values to control the behavior or indicate internal state of #AgsRecallLv2 by
 * enable/disable as flags.
 */
typedef enum{
  AGS_RECALL_LV2_HAS_EVENT_PORT   = 1,
  AGS_RECALL_LV2_HAS_ATOM_PORT    = 1 <<  1,
  AGS_RECALL_LV2_HAS_WORKER       = 1 <<  2,
}AgsRecallLv2FLags;

struct _AgsRecallLv2
{
  AgsRecallChannel recall_channel;

  guint flags;
  
  AgsTurtle *turtle;
  
  gchar *filename;
  gchar *effect;
  gchar *uri;
  uint32_t index;

  AgsLv2Plugin *plugin;
  LV2_Descriptor *plugin_descriptor;

  uint32_t *input_port;
  uint32_t input_lines;

  uint32_t *output_port;
  uint32_t output_lines;

  uint32_t event_port;
  uint32_t atom_port;

  uint32_t bank;
  uint32_t program;
};

struct _AgsRecallLv2Class
{
  AgsRecallChannelClass recall_channel;
};

GType ags_recall_lv2_get_type();

void ags_recall_lv2_load(AgsRecallLv2 *recall_lv2);
GList* ags_recall_lv2_load_ports(AgsRecallLv2 *recall_lv2);
void ags_recall_lv2_load_conversion(AgsRecallLv2 *recall_lv2,
				    GObject *port,
				    gpointer port_descriptor);

GList* ags_recall_lv2_find(GList *recall,
			   gchar *filename, gchar *uri);

AgsRecallLv2* ags_recall_lv2_new(AgsChannel *source,
				 AgsTurtle *turtle,
				 gchar *filename,
				 gchar *effect,
				 gchar *uri,
				 uint32_t index);

#endif /*__AGS_RECALL_LV2_H__*/
