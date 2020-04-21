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

#include <ags/audio/ags_port_util.h>

#include <ags/plugin/ags_ladspa_conversion.h>

/**
 * SECTION:ags_port_util
 * @short_description: port util
 * @title: AgsPortUtil
 * @section_id:
 * @include: ags/audio/ags_port_util.h
 *
 * Utility functions to deal with #AgsPort.
 */

/**
 * ags_port_util_load_ladspa_conversion:
 * @port: the #AgsPort
 * @plugin_port: the #AgsPluginPort
 * 
 * Loads conversion object by using @plugin_port and sets in on @port.
 * 
 * Since: 3.3.0
 */
void
ags_port_util_load_ladspa_conversion(AgsPort *port,
				     AgsPluginPort *plugin_port)
{
  AgsLadspaConversion *ladspa_conversion;

  if(!AGS_IS_PORT(port) ||
     !AGS_IS_PLUGIN_PORT(plugin_port)){
    return;
  }

  ladspa_conversion = NULL;

  if(ags_plugin_port_test_flags(plugin_port,
				AGS_PLUGIN_PORT_BOUNDED_BELOW)){
    if(!AGS_IS_LADSPA_CONVERSION(ladspa_conversion)){
      ladspa_conversion = ags_ladspa_conversion_new();
    }

    ladspa_conversion->flags |= AGS_LADSPA_CONVERSION_BOUNDED_BELOW;
  }

  if(ags_plugin_port_test_flags(plugin_port,
				AGS_PLUGIN_PORT_BOUNDED_ABOVE)){
    if(!AGS_IS_LADSPA_CONVERSION(ladspa_conversion)){
      ladspa_conversion = ags_ladspa_conversion_new();
    }

    ladspa_conversion->flags |= AGS_LADSPA_CONVERSION_BOUNDED_ABOVE;
  }
  
  if(ags_plugin_port_test_flags(plugin_port,
				AGS_PLUGIN_PORT_SAMPLERATE)){
    if(!AGS_IS_LADSPA_CONVERSION(ladspa_conversion)){
      ladspa_conversion = ags_ladspa_conversion_new();
    }
        
    ladspa_conversion->flags |= AGS_LADSPA_CONVERSION_SAMPLERATE;
  }

  if(ags_plugin_port_test_flags(plugin_port,
				AGS_PLUGIN_PORT_LOGARITHMIC)){
    if(!AGS_IS_LADSPA_CONVERSION(ladspa_conversion)){
      ladspa_conversion = ags_ladspa_conversion_new();
    }
    
    ladspa_conversion->flags |= AGS_LADSPA_CONVERSION_LOGARITHMIC;
  }

  if(ladspa_conversion != NULL){
    g_object_set(port,
		 "conversion", ladspa_conversion,
		 NULL);
  }
}
