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

#include <ags/X/machine/ags_dssi_bridge_callbacks.h>

#include <ags/libags.h>
#include <ags/libags-audio.h>
#include <ags/libags-gui.h>

#include <ags/X/ags_window.h>
#include <ags/X/ags_effect_bridge.h>
#include <ags/X/ags_effect_bulk.h>
#include <ags/X/ags_bulk_member.h>

void
ags_dssi_bridge_parent_set_callback(GtkWidget *widget, GtkObject *old_parent, AgsDssiBridge *dssi_bridge)
{
  AgsWindow *window;

  gchar *str;

  if(old_parent != NULL){
    return;
  }

  window = AGS_WINDOW(gtk_widget_get_toplevel(widget));

  str = g_strdup_printf("Default %d",
			ags_window_find_machine_counter(window, AGS_TYPE_DSSI_BRIDGE)->counter);

  g_object_set(AGS_MACHINE(dssi_bridge),
	       "machine-name", str,
	       NULL);

  ags_window_increment_machine_counter(window,
				       AGS_TYPE_DSSI_BRIDGE);

  g_free(str);
}

void
ags_dssi_bridge_program_changed_callback(GtkComboBox *combo_box, AgsDssiBridge *dssi_bridge)
{
  GtkTreeIter iter;

  if(gtk_combo_box_get_active_iter(combo_box,
				   &iter)){
    AgsLadspaConversion *ladspa_conversion;

    AgsChannel *channel;
   
    GList *bulk_member, *bulk_member_start;
    GList *start_recall, *recall;
  
    gchar *name;
    gchar *specifier;
    
    LADSPA_PortDescriptor *port_descriptor;

    guint bank, program;
    unsigned long i;

    /* get program */
    gtk_tree_model_get(gtk_combo_box_get_model(combo_box), &iter,
		       0, &name,
		       1, &bank,
		       2, &program,
		       -1);
    dssi_bridge->dssi_descriptor->select_program(dssi_bridge->ladspa_handle,
						 (unsigned long) bank,
						 (unsigned long) program);


#ifdef AGS_DEBUG
    g_message("%d %d", bank, program);
#endif
    
    /* update ports */
    channel = AGS_MACHINE(dssi_bridge)->audio->input;
    port_descriptor = dssi_bridge->dssi_descriptor->LADSPA_Plugin->PortDescriptors;
    
    while(channel != NULL){
      g_object_get(channel,
		   "recall", &start_recall,
		   NULL);
      
      recall = start_recall;

      while((recall = ags_recall_find_type(recall, AGS_TYPE_RECALL_DSSI)) != NULL){
	GList *start_port, *port;
	
	g_object_set(recall->data,
		     "bank", bank,
		     "program", program,
		     NULL);

	g_object_get(recall->data,
		     "port", &start_port,
		     NULL);
	
	for(i = 0; i < dssi_bridge->dssi_descriptor->LADSPA_Plugin->PortCount; i++){
	  if(LADSPA_IS_PORT_CONTROL(port_descriptor[i])){
	    if(LADSPA_IS_PORT_INPUT(port_descriptor[i]) ||
	       LADSPA_IS_PORT_OUTPUT(port_descriptor[i])){
	      specifier = dssi_bridge->dssi_descriptor->LADSPA_Plugin->PortNames[i];

	      port = start_port;

	      while(port != NULL){
		if(!g_strcmp0(AGS_PORT(port->data)->specifier,
			      specifier)){
		  GValue value = {0,};

#ifdef AGS_DEBUG
		  g_message("%s %f", specifier, dssi_bridge->port_values[i]);
#endif
		  
		  g_value_init(&value,
			       G_TYPE_FLOAT);
		  
		  g_value_set_float(&value,
				    dssi_bridge->port_values[i]);
		  ags_port_safe_write_raw(port->data,
					  &value);
		
		  break;
		}
	
		port = port->next;
	      }
	    }
	  }
	}

	g_list_free_full(start_port,
			 g_object_unref);

	/* iterate */
	recall = recall->next;
      }

      g_list_free_full(start_recall,
		       g_object_unref);
      
      /* iterate */
      g_object_get(channel,
		   "next", &channel,
		   NULL);

      if(channel != NULL){
	g_object_unref(channel);
      }
    }

    /* update UI */
    bulk_member_start = gtk_container_get_children((GtkContainer *) AGS_EFFECT_BULK(AGS_EFFECT_BRIDGE(AGS_MACHINE(dssi_bridge)->bridge)->bulk_input)->table);
  
    for(i = 0; i < dssi_bridge->dssi_descriptor->LADSPA_Plugin->PortCount; i++){
      /* find bulk member */
      bulk_member = bulk_member_start;

      specifier = dssi_bridge->dssi_descriptor->LADSPA_Plugin->PortNames[i];

#ifdef AGS_DEBUG
      g_message("%s", specifier);
#endif
      
      while(bulk_member != NULL){
	if(AGS_IS_BULK_MEMBER(bulk_member->data) &&
	   !g_strcmp0(AGS_BULK_MEMBER(bulk_member->data)->specifier,
		      specifier)){
	  GtkWidget *child_widget;

	  AGS_BULK_MEMBER(bulk_member->data)->flags |= AGS_BULK_MEMBER_NO_UPDATE;

	  child_widget = gtk_bin_get_child((GtkBin *) AGS_BULK_MEMBER(bulk_member->data));
	  
	  ladspa_conversion = (AgsLadspaConversion *) AGS_BULK_MEMBER(bulk_member->data)->conversion;
	  
	  if(GTK_IS_TOGGLE_BUTTON(child_widget)){
	    if(dssi_bridge->port_values[i] == 0.0){
	      gtk_toggle_button_set_active((GtkToggleButton *) child_widget,
					   FALSE);
	    }else{
	      gtk_toggle_button_set_active((GtkToggleButton *) child_widget,
					   TRUE);
	    }
	  }else if(AGS_IS_DIAL(child_widget)){
	    gdouble val;

	    val = dssi_bridge->port_values[i];
	    
	    if(ladspa_conversion != NULL){
	      //	      val = ags_ladspa_conversion_convert(ladspa_conversion,
	      //				  dssi_bridge->port_values[i],
	      //				  TRUE);
	    }
	    
	    AGS_DIAL(child_widget)->adjustment->value = val;
	    ags_dial_draw((AgsDial *) child_widget);

#ifdef AGS_DEBUG
	    g_message(" --- %f", dssi_bridge->port_values[i]);
#endif
	  }
	
	  AGS_BULK_MEMBER(bulk_member->data)->flags &= (~AGS_BULK_MEMBER_NO_UPDATE);

	  break;
	}

	bulk_member = bulk_member->next;
      }
    }

    g_list_free(bulk_member_start);
  }
}
