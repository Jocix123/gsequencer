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

#ifndef __AGS_VOLUME_CHANNEL_H__
#define __AGS_VOLUME_CHANNEL_H__

#include <glib.h>
#include <glib-object.h>

#include <ags/audio/ags_recall.h>

#include <ags/audio/ags_channel.h>

#define AGS_TYPE_VOLUME_CHANNEL            (ags_volume_channel_get_type())
#define AGS_VOLUME_CHANNEL(obj)            (G_TYPE_CHECK_INSTANCE_CAST(obj, AGS_TYPE_VOLUME_CHANNEL, AgsVolumeChannel))
#define AGS_VOLUME_CHANNEL_CLASS(class)    (G_TYPE_CHECK_INSTANCE_CAST(class, AGS_TYPE_VOLUME_CHANNEL, AgsVolumeChannelClass))

typedef struct _AgsVolumeChannel AgsVolumeChannel;
typedef struct _AgsVolumeChannelClass AgsVolumeChannelClass;

struct _AgsVolumeChannel{
  AgsRecall recall;

  AgsChannel *channel;
  gulong channel_recycling_changed_handler;

  gdouble *volume;
};

struct _AgsVolumeChannelClass{
  AgsRecallClass recall;
};

GType ags_volume_channel_get_type();

AgsVolumeChannel* ags_volume_channel_new(AgsChannel *channel, gdouble *volume);

#endif /*__AGS_VOLUME_CHANNEL_H__*/
