/* GSequencer - Advanced GTK Sequencer
 * Copyright (C) 2005-2019 Joël Krähemann
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

#include <ags/X/machine/ags_pitch_sampler_callbacks.h>

#include <ags/libags.h>
#include <ags/libags-audio.h>
#include <ags/libags-gui.h>

#include <ags/X/ags_ui_provider.h>
#include <ags/X/ags_window.h>

#include <ags/i18n.h>

void
ags_pitch_sampler_parent_set_callback(GtkWidget *widget, GtkObject *old_parent, AgsPitchSampler *pitch_sampler)
{
  AgsWindow *window;

  gchar *str;
  
  if(old_parent != NULL){
    return;
  }

  window = AGS_WINDOW(gtk_widget_get_ancestor((GtkWidget *) pitch_sampler, AGS_TYPE_WINDOW));

  str = g_strdup_printf("Default %d",
			ags_window_find_machine_counter(window, AGS_TYPE_PITCH_SAMPLER)->counter);

  g_object_set(AGS_MACHINE(pitch_sampler),
	       "machine-name", str,
	       NULL);

  ags_window_increment_machine_counter(window,
				       AGS_TYPE_PITCH_SAMPLER);
  g_free(str);
}

void
ags_pitch_sampler_add_callback(GtkButton *button, AgsPitchSampler *pitch_sampler)
{
  //TODO:JK: implement me
}

void
ags_pitch_sampler_remove_callback(GtkButton *button, AgsPitchSampler *pitch_sampler)
{
  //TODO:JK: implement me
}

void
ags_pitch_sampler_auto_update_callback(GtkToggleButton *toggle, AgsPitchSampler *pitch_sampler)
{
  //TODO:JK: implement me
}

void
ags_pitch_sampler_update_callback(GtkButton *button, AgsPitchSampler *pitch_sampler)
{
  //TODO:JK: implement me
}

void
ags_pitch_sampler_enable_lfo_callback(GtkToggleButton *toggle, AgsPitchSampler *pitch_sampler)
{
  //TODO:JK: implement me
}

void
ags_pitch_sampler_lfo_freq_callback(GtkSpinButton *spin_button, AgsPitchSampler *pitch_sampler)
{
  //TODO:JK: implement me
}

void
ags_pitch_sampler_lfo_phase_callback(GtkSpinButton *spin_button, AgsPitchSampler *pitch_sampler)
{
  //TODO:JK: implement me
}

void
ags_pitch_sampler_lfo_depth_callback(GtkSpinButton *spin_button, AgsPitchSampler *pitch_sampler)
{
  //TODO:JK: implement me
}

void
ags_pitch_sampler_lfo_tuning_callback(GtkSpinButton *spin_button, AgsPitchSampler *pitch_sampler)
{
  //TODO:JK: implement me
}

