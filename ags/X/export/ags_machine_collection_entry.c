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

#include <ags/X/export/ags_machine_collection_entry.h>

#include <ags/libags.h>
#include <ags/libags-audio.h>

#include <ags/X/ags_window.h>
#include <ags/X/ags_machine.h>

#include <ags/X/export/ags_midi_export_wizard.h>

#include <math.h>

#include <ags/i18n.h>

void ags_machine_collection_entry_class_init(AgsMachineCollectionEntryClass *machine_collection_entry);
void ags_machine_collection_entry_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_machine_collection_entry_applicable_interface_init(AgsApplicableInterface *applicable);
void ags_machine_collection_entry_init(AgsMachineCollectionEntry *machine_collection_entry);
void ags_machine_collection_entry_set_property(GObject *gobject,
					       guint prop_id,
					       const GValue *value,
					       GParamSpec *param_spec);
void ags_machine_collection_entry_get_property(GObject *gobject,
					       guint prop_id,
					       GValue *value,
					       GParamSpec *param_spec);

void ags_machine_collection_entry_connect(AgsConnectable *connectable);
void ags_machine_collection_entry_disconnect(AgsConnectable *connectable);

void ags_machine_collection_entry_set_update(AgsApplicable *applicable, gboolean update);
void ags_machine_collection_entry_apply(AgsApplicable *applicable);
void ags_machine_collection_entry_reset(AgsApplicable *applicable);

/**
 * SECTION:ags_machine_collection_entry
 * @short_description: pack pad editors.
 * @title: AgsMachineCollectionEntry
 * @section_id:
 * @include: ags/X/ags_machine_collection_entry.h
 *
 * #AgsMachineCollectionEntry is a wizard to export midi files and do machine mapping.
 */

enum{
  PROP_0,
  PROP_MACHINE,
};

GType
ags_machine_collection_entry_get_type(void)
{
  static GType ags_type_machine_collection_entry = 0;

  if(!ags_type_machine_collection_entry){
    static const GTypeInfo ags_machine_collection_entry_info = {
      sizeof (AgsMachineCollectionEntryClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_machine_collection_entry_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsMachineCollectionEntry),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_machine_collection_entry_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_machine_collection_entry_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    static const GInterfaceInfo ags_applicable_interface_info = {
      (GInterfaceInitFunc) ags_machine_collection_entry_applicable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_machine_collection_entry = g_type_register_static(GTK_TYPE_TABLE,
							       "AgsMachineCollectionEntry", &ags_machine_collection_entry_info,
							       0);

    g_type_add_interface_static(ags_type_machine_collection_entry,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);

    g_type_add_interface_static(ags_type_machine_collection_entry,
				AGS_TYPE_APPLICABLE,
				&ags_applicable_interface_info);
  }
  
  return(ags_type_machine_collection_entry);
}

void
ags_machine_collection_entry_class_init(AgsMachineCollectionEntryClass *machine_collection_entry)
{
  GObjectClass *gobject;
  GParamSpec *param_spec;

  /* GObjectClass */
  gobject = (GObjectClass *) machine_collection_entry;

  gobject->set_property = ags_machine_collection_entry_set_property;
  gobject->get_property = ags_machine_collection_entry_get_property;

  /* properties */
  /**
   * AgsMachineCollectionEntry:machine:
   *
   * The assigned #AgsMachine.
   * 
   * Since: 2.0.0
   */
  param_spec = g_param_spec_object("machine",
				   i18n_pspec("assigned machine"),
				   i18n_pspec("The machine which this machine entry is assigned with"),
				   AGS_TYPE_MACHINE,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_MACHINE,
				  param_spec);
}

void
ags_machine_collection_entry_connectable_interface_init(AgsConnectableInterface *connectable)
{
  connectable->is_ready = NULL;
  connectable->is_connected = NULL;
  connectable->connect = ags_machine_collection_entry_connect;
  connectable->disconnect = ags_machine_collection_entry_disconnect;
}

void
ags_machine_collection_entry_applicable_interface_init(AgsApplicableInterface *applicable)
{
  applicable->set_update = ags_machine_collection_entry_set_update;
  applicable->apply = ags_machine_collection_entry_apply;
  applicable->reset = ags_machine_collection_entry_reset;
}

void
ags_machine_collection_entry_init(AgsMachineCollectionEntry *machine_collection_entry)
{
  GtkLabel *label;

  machine_collection_entry->flags = 0;
  
  gtk_table_resize((GtkTable *) machine_collection_entry,
		   4, 4);

  machine_collection_entry->machine = NULL;

  /* enabled */
  machine_collection_entry->enabled = (GtkCheckButton *) gtk_check_button_new_with_label(i18n("enabled"));
  gtk_table_attach((GtkTable *) machine_collection_entry,
		   (GtkWidget *) machine_collection_entry->enabled,
		   0, 4,
		   0, 1,
		   GTK_FILL, GTK_FILL,
		   0, 0);

  /* machine label */
  machine_collection_entry->label = (GtkLabel *) g_object_new(GTK_TYPE_LABEL,
							      "xalign", 0.0,
							      NULL);
  gtk_table_attach((GtkTable *) machine_collection_entry,
		   (GtkWidget *) machine_collection_entry->label,
		   0, 4,
		   1, 2,
		   GTK_FILL, GTK_FILL,
		   0, 0);

  /* instrument * /
  label = (GtkLabel *) g_object_new(GTK_TYPE_LABEL,
				    "label", "instrument: ",
				    "xalign", 0.0,
				    NULL);
  gtk_table_attach((GtkTable *) machine_collection_entry,
		   (GtkWidget *) label,
		   0, 2,
		   2, 3,
		   GTK_FILL, GTK_FILL,
		   0, 0);

  machine_collection_entry->instrument = (GtkEntry *) gtk_entry_new();
  gtk_table_attach((GtkTable *) machine_collection_entry,
		   (GtkWidget *) machine_collection_entry->instrument,
		   2, 4,
		   2, 3,
		   GTK_FILL, GTK_FILL,
		   0, 0);
  */
  machine_collection_entry->instrument = NULL;
  
  /* sequence */
  label = (GtkLabel *) g_object_new(GTK_TYPE_LABEL,
				    "label", i18n("sequence: "),
				    "xalign", 0.0,
				    NULL);
  gtk_table_attach((GtkTable *) machine_collection_entry,
		   (GtkWidget *) label,
		   0, 2,
		   3, 4,
		   GTK_FILL, GTK_FILL,
		   0, 0);

  machine_collection_entry->sequence = (GtkEntry *) gtk_entry_new();
  gtk_table_attach((GtkTable *) machine_collection_entry,
		   (GtkWidget *) machine_collection_entry->sequence,
		   2, 4,
		   3, 4,
		   GTK_FILL, GTK_FILL,
		   0, 0);
}

void
ags_machine_collection_entry_set_property(GObject *gobject,
					  guint prop_id,
					  const GValue *value,
					  GParamSpec *param_spec)
{
  AgsMachineCollectionEntry *machine_collection_entry;

  machine_collection_entry = AGS_MACHINE_COLLECTION_ENTRY(gobject);

  switch(prop_id){
  case PROP_MACHINE:
    {
      GtkWidget *machine;

      machine = (GtkWidget *) g_value_get_object(value);

      if(machine_collection_entry->machine == machine){
	return;
      }

      if(machine_collection_entry->machine != NULL){
	g_object_unref(machine_collection_entry->machine);
      }
      
      if(machine != NULL){
	g_object_ref(machine);

	/* fill in some fields */
	gtk_label_set_text(machine_collection_entry->label,
			   g_strdup_printf("%s: %s",
					   G_OBJECT_TYPE_NAME(machine),
					   AGS_MACHINE(machine)->machine_name));
	
	//	gtk_entry_set_text(machine_collection_entry->instrument,
	//		   G_OBJECT_TYPE_NAME(machine));

      	gtk_entry_set_text(machine_collection_entry->sequence,
			   AGS_MACHINE(machine)->machine_name);
      }

      machine_collection_entry->machine = machine;
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_machine_collection_entry_get_property(GObject *gobject,
					  guint prop_id,
					  GValue *value,
					  GParamSpec *param_spec)
{
  AgsMachineCollectionEntry *machine_collection_entry;

  machine_collection_entry = AGS_MACHINE_COLLECTION_ENTRY(gobject);

  switch(prop_id){
  case PROP_MACHINE:
    {
      g_value_set_object(value, machine_collection_entry->machine);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_machine_collection_entry_connect(AgsConnectable *connectable)
{
  AgsMachineCollectionEntry *machine_collection_entry;

  machine_collection_entry = AGS_MACHINE_COLLECTION_ENTRY(connectable);

  if((AGS_MACHINE_COLLECTION_ENTRY_CONNECTED & (machine_collection_entry->flags)) != 0){
    return;
  }

  machine_collection_entry->flags |= AGS_MACHINE_COLLECTION_ENTRY_CONNECTED;
}

void
ags_machine_collection_entry_disconnect(AgsConnectable *connectable)
{
  AgsMachineCollectionEntry *machine_collection_entry;

  machine_collection_entry = AGS_MACHINE_COLLECTION_ENTRY(connectable);

  if((AGS_MACHINE_COLLECTION_ENTRY_CONNECTED & (machine_collection_entry->flags)) == 0){
    return;
  }

  machine_collection_entry->flags &= (~AGS_MACHINE_COLLECTION_ENTRY_CONNECTED);
}

void
ags_machine_collection_entry_set_update(AgsApplicable *applicable, gboolean update)
{
  AgsMachineCollectionEntry *machine_collection_entry;

  machine_collection_entry = AGS_MACHINE_COLLECTION_ENTRY(applicable);
}

void
ags_machine_collection_entry_apply(AgsApplicable *applicable)
{
  AgsWindow *window;
  AgsMachine *machine;

  AgsMidiExportWizard *midi_export_wizard;
  AgsMachineCollectionEntry *machine_collection_entry;

  AgsMidiBuilder *midi_builder;

  AgsNote **check_match;
  
  GList *start_notation, *notation;
  GList *start_note, *note;
  
  gint key_on[128];
  gint key_off[128];
  gboolean key_active[128];

  gdouble bpm;
  guint division;
  guint delta_time;
  guint pulse_unit;
  guint notation_count;
  guint x, prev_x;
  guint i;
  gboolean initial_track;
  gboolean success;
  
  machine_collection_entry = AGS_MACHINE_COLLECTION_ENTRY(applicable);

  if(!gtk_toggle_button_get_active(machine_collection_entry->enabled)){
    return;
  }

  midi_export_wizard = gtk_widget_get_ancestor(machine_collection_entry,
					       AGS_TYPE_MIDI_EXPORT_WIZARD);

  window = midi_export_wizard->main_window;
  machine = machine_collection_entry->machine;

  g_object_get(machine->audio,
	       "notation", &start_notation,
	       NULL);
  
  midi_builder = midi_export_wizard->midi_builder;
  pulse_unit = midi_export_wizard->pulse_unit;

  bpm = midi_builder->midi_header->beat;
  division = midi_builder->midi_header->division;
  
  notation_count = g_list_length(start_notation);

  if(midi_builder->current_midi_track == NULL){
    initial_track = TRUE;
  }else{
    initial_track = FALSE;
  }
  
  /* append track */
  ags_midi_builder_append_track(midi_builder,
				gtk_entry_get_text(machine_collection_entry->sequence));

  /* append tempo */
  if(initial_track){
    //TODO:JK: improve hard-coded values
    ags_midi_builder_append_time_signature(midi_builder,
					   0,
					   4, 4,
					   4, 16);
    
    ags_midi_builder_append_tempo(midi_builder,
				  0,
				  (guint) round(60.0 * 1000000.0 / bpm));
  }
  
  /* put keys */
  check_match = (AgsNote **) malloc(notation_count * sizeof(AgsNote *));
  
  memset(key_on, -1, 128 * sizeof(gint));
  memset(key_off, -1, 128 * sizeof(gint));
  
  memset(key_active, FALSE, 128 * sizeof(gboolean));

  delta_time = 0;
  prev_x = 0;
  
  success = TRUE;

  while(success){
    guint note_x0, note_x1;
    guint check_x0, check_x1;
    guint note_y;
    guint check_y;
    
    notation = start_notation;

    for(i = 0; i < notation_count; i++){
      check_match[i] = NULL;
    }

    /* check key-on */    
    for(i = 0; notation != NULL; i++){
      g_object_get(notation->data,
		   "note", &start_note,
		   NULL);

      note = start_note;

      while(note != NULL){
	g_object_get(note->data,
		     "x0", &note_x0,
		     "y", &note_y,
		     NULL);
	
	if(note_y < 128){
	  if(key_on[note_y] < (gint) note_x0){
	    check_match[i] = AGS_NOTE(note->data);
	    
	    break;
	  }
	}
	
	note = note->next;
      }

      g_list_free(start_note);
      
      notation = notation->next;
    }

    /* check key-off */
    notation = start_notation;

    for(i = 0; notation != NULL; i++){
      g_object_get(notation->data,
		   "note", &start_note,
		   NULL);

      note = start_note;

      while(note != NULL){
	g_object_get(note->data,
		     "x0", &note_x0,
		     "x1", &note_x1,
		     "y", &note_y,
		     NULL);

	if(note_y < 128){
	  if(check_match[i] != NULL){
	    g_object_get(check_match[i],
			 "x0", &check_x0,
			 NULL);
	    
	    if(check_x0 < note_x1){
	      break;
	    }
	  }
	  
    	  if(key_off[note_y] < (gint) note_x1){
	    check_match[i] = AGS_NOTE(note->data);
	    
	    break;
	  }
	}
	
	note = note->next;
      }

      g_list_free(start_note);
      
      notation = notation->next;
    }

    /* find next pulse */
    x = G_MAXUINT;
    success = FALSE;
    
    for(i = 0; i < notation_count; i++){
      if(check_match[i] != NULL){
	g_object_get(check_match[i],
		     "x0", &check_x0,
		     "x1", &check_x1,
		     "y", &check_y,
		     NULL);
	
	if(!key_active[check_y]){
	  /* check key-on */
	  if(check_x0 < x){
	    x = check_x0;

	    success = TRUE;
	  }
	}else if(key_active[check_y]){
	  /* check key-off */
	  if(check_x1 < x){
	    x = check_x1;
	    
	    success = TRUE;
	  }
	}
      }
    }
    
    /* apply events matching pulse */
    if(success){
      for(i = 0; i < notation_count; i++){
	if(check_match[i] != NULL){
	  if(x > prev_x){
	    delta_time += (x - prev_x) * pulse_unit;
	  }else{
	    delta_time = 0;
	  }

	  g_object_get(check_match[i],
		       "x0", &check_x0,
		       "x1", &check_x1,
		       "y", &check_y,
		       NULL);

	  if(check_x0 == x){
	    if(key_on[check_y] != check_x0){
	      /* append key-on */
	      ags_midi_builder_append_key_on(midi_builder,
					     delta_time,
					     0,
					     check_y,
					     127);
	      
#ifdef AGS_DEBUG
	      g_message("key-on %d,%d - %d", check_x0, check_x1, check_y);
#endif
	    
	      key_on[check_y] = check_x0;
	      key_active[check_y] = TRUE;
	    }
	  }else if(check_x1 == x){
	    if(key_off[check_y] != check_x1){
	      /* append key-off */
	      ags_midi_builder_append_key_off(midi_builder,
					      delta_time,
					      0,
					      check_y,
					      127);	    

	      key_off[check_y] = check_x1;
	      key_active[check_y] = FALSE;
	    }
	  }

	  prev_x = x;
	}
      }
    }
  }

  g_list_free(start_notation);
}

void
ags_machine_collection_entry_reset(AgsApplicable *applicable)
{
  AgsMachineCollectionEntry *machine_collection_entry;

  machine_collection_entry = AGS_MACHINE_COLLECTION_ENTRY(applicable);
}

/**
 * ags_machine_collection_entry_new:
 * @machine: the #AgsMachine
 *
 * Create a new instance of #AgsMachineCollectionEntry
 *
 * Returns: the new #AgsMachineCollectionEntry
 *
 * Since: 2.0.0
 */
AgsMachineCollectionEntry*
ags_machine_collection_entry_new(GtkWidget *machine)
{
  AgsMachineCollectionEntry *machine_collection_entry;

  machine_collection_entry = (AgsMachineCollectionEntry *) g_object_new(AGS_TYPE_MACHINE_COLLECTION_ENTRY,
									"machine", machine,
									NULL);
  
  return(machine_collection_entry);
}
