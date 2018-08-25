/* GSequencer - Advanced GTK Sequencer
 * Copyright (C) 2005-2017 Joël Krähemann
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

#ifndef __AGS_WAVE_EDITOR_CALLBACKS_H__
#define __AGS_WAVE_EDITOR_CALLBACKS_H__

#include <glib.h>
#include <glib-object.h>

#include <gtk/gtk.h>

#include <ags/X/ags_wave_editor.h>
#include <ags/X/ags_machine.h>

#include <ags/X/editor/ags_machine_selector.h>

gboolean ags_wave_editor_edit_expose_event(GtkWidget *widget, GdkEventExpose *event, AgsWaveEditor *wave_editor);
gboolean ags_wave_editor_edit_configure_event(GtkWidget *widget, GdkEventConfigure *event, AgsWaveEditor *wave_editor);

void ags_wave_editor_vscrollbar_value_changed(GtkRange *range, AgsWaveEditor *wave_editor);
void ags_wave_editor_hscrollbar_value_changed(GtkRange *range, AgsWaveEditor *wave_editor);

void ags_wave_editor_wave_edit_hscrollbar_value_changed(GtkRange *range, AgsWaveEditor *wave_editor);

void ags_wave_editor_machine_changed_callback(AgsMachineSelector *machine_selector, AgsMachine *machine,
					      AgsWaveEditor *wave_editor);

void ags_wave_editor_resize_audio_channels_callback(AgsMachine *machine, 
						    guint audio_channels, guint audio_channels_old,
						    AgsWaveEditor *wave_editor);
void ags_wave_editor_resize_pads_callback(AgsMachine *machine, GType channel_type,
					  guint pads, guint pads_old,
					  AgsWaveEditor *wave_editor);

#endif /*__AGS_WAVE_EDITOR_CALLBACKS_H__*/
