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

#include <ags/X/ags_notation_editor.h>
#include <ags/X/ags_notation_editor_callbacks.h>

#include <ags/libags.h>
#include <ags/libags-audio.h>
#include <ags/libags-gui.h>

#include <ags/X/ags_window.h>

#include <ags/X/machine/ags_drum.h>
#include <ags/X/machine/ags_matrix.h>

#include <ags/X/thread/ags_gui_thread.h>

#include <libxml/tree.h>
#include <libxml/xpath.h>

#include <math.h>

#include <ags/config.h>
#include <ags/i18n.h>

void ags_notation_editor_class_init(AgsNotationEditorClass *notation_editor);
void ags_notation_editor_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_notation_editor_init(AgsNotationEditor *notation_editor);
void ags_notation_editor_set_property(GObject *gobject,
				      guint prop_id,
				      const GValue *value,
				      GParamSpec *param_spec);
void ags_notation_editor_get_property(GObject *gobject,
				      guint prop_id,
				      GValue *value,
				      GParamSpec *param_spec);
void ags_notation_editor_connect(AgsConnectable *connectable);
void ags_notation_editor_disconnect(AgsConnectable *connectable);
void ags_notation_editor_finalize(GObject *gobject);

void ags_notation_editor_real_machine_changed(AgsNotationEditor *notation_editor,
					      AgsMachine *machine);

void ags_notation_edit_play_channel(AgsNotationEdit *notation_edit,
				    AgsChannel *channel,
				    AgsNote *note);

enum{
  MACHINE_CHANGED,
  LAST_SIGNAL,
};

enum{
  PROP_0,
  PROP_SOUNDCARD,
};

static gpointer ags_notation_editor_parent_class = NULL;
static guint notation_editor_signals[LAST_SIGNAL];

/**
 * SECTION:ags_notation_editor
 * @short_description: A composite widget to edit notation
 * @title: AgsNotationEditor
 * @section_id:
 * @include: ags/X/ags_notation_editor.h
 *
 * #AgsNotationEditor is a composite widget to edit notation. You may select machines
 * or change editor tool to do notation.
 */

GType
ags_notation_editor_get_type(void)
{
  static volatile gsize g_define_type_id__volatile = 0;

  if(g_once_init_enter (&g_define_type_id__volatile)){
    GType ags_type_notation_editor;

    static const GTypeInfo ags_notation_editor_info = {
      sizeof (AgsNotationEditorClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_notation_editor_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsNotationEditor),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_notation_editor_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_notation_editor_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_notation_editor = g_type_register_static(GTK_TYPE_VBOX,
						      "AgsNotationEditor", &ags_notation_editor_info,
						      0);
    
    g_type_add_interface_static(ags_type_notation_editor,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);

    g_once_init_leave (&g_define_type_id__volatile, ags_type_notation_editor);
  }

  return g_define_type_id__volatile;
}

void
ags_notation_editor_connectable_interface_init(AgsConnectableInterface *connectable)
{
  connectable->is_ready = NULL;
  connectable->is_connected = NULL;
  connectable->connect = ags_notation_editor_connect;
  connectable->disconnect = ags_notation_editor_disconnect;
}

void
ags_notation_editor_class_init(AgsNotationEditorClass *notation_editor)
{
  GObjectClass *gobject;
  GParamSpec *param_spec;

  ags_notation_editor_parent_class = g_type_class_peek_parent(notation_editor);

  /* GObjectClass */
  gobject = (GObjectClass *) notation_editor;

  gobject->set_property = ags_notation_editor_set_property;
  gobject->get_property = ags_notation_editor_get_property;

  gobject->finalize = ags_notation_editor_finalize;
  
  /* properties */
  /**
   * AgsNotationEditor:soundcard:
   *
   * The assigned #AgsSoundcard acting as default sink.
   * 
   * Since: 1.2.0
   */
  param_spec = g_param_spec_object("soundcard",
				   i18n_pspec("assigned soundcard"),
				   i18n_pspec("The soundcard it is assigned with"),
				   G_TYPE_OBJECT,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_SOUNDCARD,
				  param_spec);

  /* AgsEditorClass */
  notation_editor->machine_changed = ags_notation_editor_real_machine_changed;

  /* signals */
  /**
   * AgsEditor::machine-changed:
   * @editor: the object to change machine.
   * @machine: the #AgsMachine to set
   *
   * The ::machine-changed signal notifies about changed machine.
   * 
   * Since: 1.2.0
   */
  notation_editor_signals[MACHINE_CHANGED] =
    g_signal_new("machine-changed",
                 G_TYPE_FROM_CLASS(notation_editor),
                 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET(AgsNotationEditorClass, machine_changed),
                 NULL, NULL,
                 g_cclosure_marshal_VOID__OBJECT,
                 G_TYPE_NONE, 1,
		 G_TYPE_OBJECT);
}

void
ags_notation_editor_init(AgsNotationEditor *notation_editor)
{
  GtkViewport *viewport;
  GtkScrolledWindow *scrolled_window;
  GtkTable *table;
  
  notation_editor->flags = (AGS_NOTATION_EDITOR_PASTE_MATCH_AUDIO_CHANNEL |
			    AGS_NOTATION_EDITOR_PASTE_NO_DUPLICATES);

  notation_editor->version = AGS_NOTATION_EDITOR_DEFAULT_VERSION;
  notation_editor->build_id = AGS_NOTATION_EDITOR_DEFAULT_BUILD_ID;

  /* offset */
  notation_editor->tact_counter = 0;
  notation_editor->current_tact = 0.0;

  /* soundcard */
  notation_editor->soundcard = NULL;

  /* notation toolbar */
  notation_editor->notation_toolbar = ags_notation_toolbar_new();
  gtk_box_pack_start((GtkBox *) notation_editor,
		     (GtkWidget *) notation_editor->notation_toolbar,
		     FALSE, FALSE,
		     0);

  /* paned */
  notation_editor->paned = (GtkHPaned *) gtk_hpaned_new();
  gtk_box_pack_start((GtkBox *) notation_editor,
		     (GtkWidget *) notation_editor->paned,
		     TRUE, TRUE, 0);

  /* machine selector */
  viewport = gtk_viewport_new(NULL,
			      NULL);
  g_object_set(viewport,
	       "shadow-type", GTK_SHADOW_NONE,
	       NULL);
  gtk_paned_pack1((GtkPaned *) notation_editor->paned,
		  (GtkWidget *) viewport,
		  FALSE, TRUE);

  scrolled_window = (GtkScrolledWindow *) gtk_scrolled_window_new(NULL, NULL);
  gtk_container_add(viewport,
		    scrolled_window);

  notation_editor->machine_selector = g_object_new(AGS_TYPE_MACHINE_SELECTOR,
						   "homogeneous", FALSE,
						   "spacing", 0,
						   NULL);
  notation_editor->machine_selector->flags |= (AGS_MACHINE_SELECTOR_SHOW_REVERSE_MAPPING |
					       AGS_MACHINE_SELECTOR_SHOW_SHIFT_PIANO |
					       AGS_MACHINE_SELECTOR_NOTATION);
  gtk_label_set_label(notation_editor->machine_selector->label,
		      i18n("notation"));
  
  notation_editor->machine_selector->popup = ags_machine_selector_popup_new(notation_editor->machine_selector);
  g_object_set(notation_editor->machine_selector->menu_button,
	       "menu", notation_editor->machine_selector->popup,
	       NULL);
  
  gtk_scrolled_window_add_with_viewport(scrolled_window,
					(GtkWidget *) notation_editor->machine_selector);

  /* selected machine */
  notation_editor->selected_machine = NULL;

  /* table */
  viewport = gtk_viewport_new(NULL,
			      NULL);
  g_object_set(viewport,
	       "shadow-type", GTK_SHADOW_NONE,
	       NULL);
  gtk_paned_pack2((GtkPaned *) notation_editor->paned,
		  (GtkWidget *) viewport,
		  TRUE, TRUE);

  table = (GtkTable *) gtk_table_new(3, 2,
				     FALSE);
  gtk_container_add(viewport,
		    table);
  
  /* notebook */
  notation_editor->notebook = g_object_new(AGS_TYPE_NOTEBOOK,
					   "homogeneous", FALSE,
					   "spacing", 0,
					   "prefix", i18n("channel"),
					   NULL);
  gtk_table_attach(table,
		   (GtkWidget *) notation_editor->notebook,
		   0, 3,
		   0, 1,
		   GTK_FILL|GTK_EXPAND, GTK_FILL,
		   0, 0);

  /* scrolled piano */
  notation_editor->scrolled_piano = ags_scrolled_piano_new();
  g_object_set(notation_editor->scrolled_piano,
	       "margin-top", (guint) ((AGS_RULER_FONT_SIZE + (2 * AGS_RULER_FREE_SPACE) + AGS_RULER_LARGE_STEP) - (ceil(AGS_PIANO_DEFAULT_KEY_HEIGHT / 4.0))),
	       NULL);
  gtk_table_attach(table,
		   (GtkWidget *) notation_editor->scrolled_piano,
		   0, 1,
		   1, 2,
		   GTK_FILL, GTK_FILL,
		   0, 0);

  /* notation edit */
  notation_editor->notation_edit = ags_notation_edit_new();
  gtk_table_attach(table,
		   (GtkWidget *) notation_editor->notation_edit,
		   1, 2,
		   1, 2,
		   GTK_FILL|GTK_EXPAND, GTK_FILL|GTK_EXPAND,
		   0, 0);
}

void
ags_notation_editor_set_property(GObject *gobject,
				 guint prop_id,
				 const GValue *value,
				 GParamSpec *param_spec)
{
  AgsNotationEditor *notation_editor;

  notation_editor = AGS_NOTATION_EDITOR(gobject);

  switch(prop_id){
  case PROP_SOUNDCARD:
    {
      GObject *soundcard;

      soundcard = g_value_get_object(value);

      if(notation_editor->soundcard == soundcard){
	return;
      }

      if(notation_editor->soundcard != NULL){
	g_object_unref(notation_editor->soundcard);
      }
      
      if(soundcard != NULL){
	g_object_ref(soundcard);
      }
      
      notation_editor->soundcard = soundcard;
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_notation_editor_get_property(GObject *gobject,
				 guint prop_id,
				 GValue *value,
				 GParamSpec *param_spec)
{
  AgsNotationEditor *notation_editor;

  notation_editor = AGS_NOTATION_EDITOR(gobject);

  switch(prop_id){
  case PROP_SOUNDCARD:
    {
      g_value_set_object(value, notation_editor->soundcard);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_notation_editor_connect(AgsConnectable *connectable)
{
  AgsNotationEditor *notation_editor;

  notation_editor = AGS_NOTATION_EDITOR(connectable);

  if((AGS_NOTATION_EDITOR_CONNECTED & (notation_editor->flags)) != 0){
    return;
  }

  notation_editor->flags |= AGS_NOTATION_EDITOR_CONNECTED;  
  
  g_signal_connect((GObject *) notation_editor->machine_selector, "changed",
		   G_CALLBACK(ags_notation_editor_machine_changed_callback), (gpointer) notation_editor);

  /* toolbar */
  ags_connectable_connect(AGS_CONNECTABLE(notation_editor->notation_toolbar));

  /* machine selector */
  ags_connectable_connect(AGS_CONNECTABLE(notation_editor->machine_selector));

  /* notation edit */
  ags_connectable_connect(AGS_CONNECTABLE(notation_editor->notation_edit));
}

void
ags_notation_editor_disconnect(AgsConnectable *connectable)
{
  AgsNotationEditor *notation_editor;

  notation_editor = AGS_NOTATION_EDITOR(connectable);

  /* notation toolbar */
  ags_connectable_disconnect(AGS_CONNECTABLE(notation_editor->notation_toolbar)); 

  /* machine selector */
  ags_connectable_disconnect(AGS_CONNECTABLE(notation_editor->machine_selector));

  /* notation edit */
  ags_connectable_disconnect(AGS_CONNECTABLE(notation_editor->notation_edit));
}

void
ags_notation_editor_finalize(GObject *gobject)
{
  AgsNotationEditor *notation_editor;

  notation_editor = AGS_NOTATION_EDITOR(gobject);

  if(notation_editor->soundcard != NULL){
    g_object_unref(notation_editor->soundcard);
  }
  
  /* call parent */
  G_OBJECT_CLASS(ags_notation_editor_parent_class)->finalize(gobject);
}

void
ags_notation_editor_real_machine_changed(AgsNotationEditor *notation_editor,
					 AgsMachine *machine)
{
  AgsMachine *old_machine;

  AgsMutexManager *mutex_manager;

  GList *tab;

  guint length;
  guint audio_channels;
  guint i;

  pthread_mutex_t *application_mutex;
  pthread_mutex_t *audio_mutex;

  mutex_manager = ags_mutex_manager_get_instance();
  application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);

  /* disconnect set pads - old */
  old_machine = notation_editor->selected_machine;

  if(old_machine != NULL){
    g_object_disconnect(old_machine,
			"any_signal::resize-audio-channels",
			G_CALLBACK(ags_notation_editor_resize_audio_channels_callback),
			(gpointer) notation_editor,
			"any_signal::resize-pads",
			G_CALLBACK(ags_notation_editor_resize_pads_callback),
			(gpointer) notation_editor,
			NULL);
  }

  /* get audio mutex */
  if(machine != NULL){
    pthread_mutex_lock(application_mutex);
  
    audio_mutex = ags_mutex_manager_lookup(mutex_manager,
					   (GObject *) machine->audio);
  
    pthread_mutex_unlock(application_mutex);
  }
  
  /* notebook - remove tabs */
  length = g_list_length(notation_editor->notebook->tab);
  
  for(i = 0; i < length; i++){
    ags_notebook_remove_tab(notation_editor->notebook,
			    0);
  }

  /* check pattern mode */
  if(AGS_IS_DRUM(machine) ||
     AGS_IS_MATRIX(machine)){
    notation_editor->flags |= AGS_NOTATION_EDITOR_PATTERN_MODE;
  }else{
    notation_editor->flags &= (~AGS_NOTATION_EDITOR_PATTERN_MODE);
  }
  
  /* notebook - add tabs */
  if(machine != NULL){
    pthread_mutex_lock(audio_mutex);

    audio_channels = machine->audio->audio_channels;
    
    pthread_mutex_unlock(audio_mutex);

    for(i = 0; i < audio_channels; i++){
      ags_notebook_insert_tab(notation_editor->notebook,
			      i);

      tab = notation_editor->notebook->tab;
      gtk_toggle_button_set_active(AGS_NOTEBOOK_TAB(tab->data)->toggle,
				   TRUE);
    }
  }

  /* piano */
  if(machine != NULL){
    guint channel_count;

    /* get channel count */
    pthread_mutex_lock(audio_mutex);

#if 0
    if((AGS_AUDIO_NOTATION_DEFAULT & (machine->audio->flags)) != 0){
      channel_count = machine->audio->input_pads;
    }else{
      channel_count = machine->audio->output_pads;
    }
#else
    channel_count = machine->audio->input_pads;
#endif
    
    pthread_mutex_unlock(audio_mutex);

    /* apply channel count */
    g_object_set(notation_editor->scrolled_piano->piano,
		 "key-count", channel_count,
		 NULL);
  }else{
    /* apply default */
    g_object_set(notation_editor->scrolled_piano->piano,
		 "key-count", AGS_PIANO_DEFAULT_KEY_COUNT,
		 NULL);
  }
  
  gtk_widget_queue_draw(notation_editor->scrolled_piano->piano);
  
  /* selected machine */
  notation_editor->selected_machine = machine;

  /* reset scrollbars */
  ags_notation_edit_reset_vscrollbar(notation_editor->notation_edit);
  ags_notation_edit_reset_hscrollbar(notation_editor->notation_edit);

  /* redraw */
  gtk_widget_queue_draw(notation_editor->notation_edit);

  /* connect set-pads - new */
  if(machine != NULL){
    g_signal_connect_after(machine, "resize-audio-channels",
			   G_CALLBACK(ags_notation_editor_resize_audio_channels_callback), notation_editor);

    g_signal_connect_after(machine, "resize-pads",
			   G_CALLBACK(ags_notation_editor_resize_pads_callback), notation_editor);
  }  
}

/**
 * ags_notation_editor_machine_changed:
 * @notation_editor: an #AgsNotationEditor
 * @machine: the new #AgsMachine
 *
 * Is emitted as machine changed of notation_editor.
 *
 * Since: 1.2.0
 */
void
ags_notation_editor_machine_changed(AgsNotationEditor *notation_editor,
				    AgsMachine *machine)
{
  g_return_if_fail(AGS_IS_NOTATION_EDITOR(notation_editor));

  g_object_ref((GObject *) notation_editor);
  g_signal_emit((GObject *) notation_editor,
		notation_editor_signals[MACHINE_CHANGED], 0,
		machine);
  g_object_unref((GObject *) notation_editor);
}

/**
 * ags_notation_editor_add_note:
 * @notation_editor: an #AgsNotationEditor
 * @note: the #AgsNote to add
 * 
 * Add note.
 *
 * Since: 1.2.0
 */
void
ags_notation_editor_add_note(AgsNotationEditor *notation_editor,
			     AgsNote *note)
{
  AgsMachine *machine;
  
  AgsNotation *notation;

  AgsTimestamp *timestamp;

  GList *list_notation;

  gint i;
  
  if(!AGS_IS_NOTATION_EDITOR(notation_editor) ||
     !AGS_IS_NOTE(note)){
    return;
  }

  if(notation_editor->selected_machine != NULL){
    AgsMutexManager *mutex_manager;

    pthread_mutex_t *application_mutex;
    pthread_mutex_t *audio_mutex;

    machine = notation_editor->selected_machine;
  
    mutex_manager = ags_mutex_manager_get_instance();
    application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);

    /* get audio mutex */
    pthread_mutex_lock(application_mutex);

    audio_mutex = ags_mutex_manager_lookup(mutex_manager,
					   (GObject *) machine->audio);
  
    pthread_mutex_unlock(application_mutex);

    /* check all active tabs */
    timestamp = ags_timestamp_new();

    timestamp->flags &= (~AGS_TIMESTAMP_UNIX);
    timestamp->flags |= AGS_TIMESTAMP_OFFSET;
    
    timestamp->timer.ags_offset.offset = (guint64) AGS_NOTATION_DEFAULT_OFFSET * floor((double) note->x[0] / (double) AGS_NOTATION_DEFAULT_OFFSET);

    pthread_mutex_lock(audio_mutex);

    i = 0;

    while((i = ags_notebook_next_active_tab(notation_editor->notebook,
					    i)) != -1){
      AgsNote *new_note;
      
      list_notation = machine->audio->notation;
      list_notation = ags_notation_find_near_timestamp(list_notation, i,
						       timestamp);

      if(list_notation != NULL){
	notation = list_notation->data;
      }else{
	notation = ags_notation_new(machine->audio,
				    i);
	notation->timestamp->timer.ags_offset.offset = timestamp->timer.ags_offset.offset;
	
	machine->audio->notation = ags_notation_add(machine->audio->notation,
						    notation);
      }

      new_note = ags_note_duplicate(note);
      ags_notation_add_note(notation,
			    new_note,
			    FALSE);

      i++;
    }
    
    pthread_mutex_unlock(audio_mutex);

    gtk_widget_queue_draw(notation_editor->notation_edit);
  }
}

/**
 * ags_notation_editor_delete_note:
 * @notation_editor: an #AgsNotationEditor
 * @x: point x
 * @y: point y
 * 
 * Delete note.
 *
 * Since: 1.2.0
 */
void
ags_notation_editor_delete_note(AgsNotationEditor *notation_editor,
				guint x, guint y)
{
  AgsMachine *machine;
  
  AgsNotation *notation;

  AgsTimestamp *timestamp;

  GList *list_notation;

  gint i;

  if(!AGS_IS_NOTATION_EDITOR(notation_editor)){
    return;
  }
  
  if(notation_editor->selected_machine != NULL){
    AgsMutexManager *mutex_manager;

    pthread_mutex_t *application_mutex;
    pthread_mutex_t *audio_mutex;

    machine = notation_editor->selected_machine;
  
    mutex_manager = ags_mutex_manager_get_instance();
    application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);

    /* get audio mutex */
    pthread_mutex_lock(application_mutex);

    audio_mutex = ags_mutex_manager_lookup(mutex_manager,
					   (GObject *) machine->audio);
  
    pthread_mutex_unlock(application_mutex);

    /* check all active tabs */
    timestamp = ags_timestamp_new();

    timestamp->flags &= (~AGS_TIMESTAMP_UNIX);
    timestamp->flags |= AGS_TIMESTAMP_OFFSET;
    
    timestamp->timer.ags_offset.offset = AGS_NOTATION_DEFAULT_OFFSET * floor(x / AGS_NOTATION_DEFAULT_OFFSET);

    pthread_mutex_lock(audio_mutex);

    i = 0;

    while((i = ags_notebook_next_active_tab(notation_editor->notebook,
					    i)) != -1){
      AgsNote *new_note;
      
      list_notation = machine->audio->notation;
      list_notation = ags_notation_find_near_timestamp(list_notation, i,
						       timestamp);

      if(list_notation != NULL){
	notation = list_notation->data;
      }else{
	i++;
	
	continue;
      }

      ags_notation_remove_note_at_position(notation,
					   x, y);

      i++;
    }
    
    pthread_mutex_unlock(audio_mutex);

    gtk_widget_queue_draw(notation_editor->notation_edit);
  }
}

/**
 * ags_notation_editor_select_region:
 * @notation_editor: an #AgsNotationEditor
 * @x0: point x0
 * @y0: point y0
 * @x1: point x1
 * @y1: point y1
 * 
 * Select region.
 *
 * Since: 1.2.0
 */
void
ags_notation_editor_select_region(AgsNotationEditor *notation_editor,
				  guint x0, guint y0,
				  guint x1, guint y1)
{
  AgsMachine *machine;
  
  AgsNotation *notation;

  AgsTimestamp *timestamp;

  GList *list_notation;

  gint i;

  if(!AGS_IS_NOTATION_EDITOR(notation_editor)){
    return;
  }
  
  if(notation_editor->selected_machine != NULL){
    AgsMutexManager *mutex_manager;

    pthread_mutex_t *application_mutex;
    pthread_mutex_t *audio_mutex;

    machine = notation_editor->selected_machine;
  
    mutex_manager = ags_mutex_manager_get_instance();
    application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);

    /* get audio mutex */
    pthread_mutex_lock(application_mutex);

    audio_mutex = ags_mutex_manager_lookup(mutex_manager,
					   (GObject *) machine->audio);
  
    pthread_mutex_unlock(application_mutex);

    /* swap values if needed */
    if(x0 > x1){
      guint tmp;

      tmp = x0;
      
      x0 = x1;
      x1 = tmp;
    }

    if(y0 > y1){
      guint tmp;

      tmp = y0;
      
      y0 = y1;
      y1 = tmp;
    }

    /* check all active tabs */
    timestamp = ags_timestamp_new();

    timestamp->flags &= (~AGS_TIMESTAMP_UNIX);
    timestamp->flags |= AGS_TIMESTAMP_OFFSET;
    
    pthread_mutex_lock(audio_mutex);

    i = 0;

    while((i = ags_notebook_next_active_tab(notation_editor->notebook,
					    i)) != -1){      
      list_notation = machine->audio->notation;
      
      timestamp->timer.ags_offset.offset = AGS_NOTATION_DEFAULT_OFFSET * floor(x0 / AGS_NOTATION_DEFAULT_OFFSET);
      
      while((list_notation = ags_notation_find_near_timestamp(list_notation, i,
							      timestamp)) != NULL &&
	    timestamp->timer.ags_offset.offset < (AGS_NOTATION_DEFAULT_OFFSET * floor(x1 / AGS_NOTATION_DEFAULT_OFFSET)) + AGS_NOTATION_DEFAULT_OFFSET){
	ags_notation_add_region_to_selection(list_notation->data,
					     x0, y0,
					     x1, y1,
					     TRUE);
	
	/* iterate */
	timestamp->timer.ags_offset.offset += AGS_NOTATION_DEFAULT_OFFSET;
	
	list_notation = list_notation->next;
      }
      
      i++;
    }
    
    pthread_mutex_unlock(audio_mutex);

    gtk_widget_queue_draw(notation_editor->notation_edit);
  }
}

void
ags_notation_edit_play_channel(AgsNotationEdit *notation_edit,
			       AgsChannel *channel,
			       AgsNote *note)
{
  AgsWindow *window;
  
  AgsGuiThread *gui_thread;

  AgsStartSoundcard *start_soundcard;
  AgsInitChannel *init_channel;
  AgsAppendChannel *append_channel;
  
  AgsMutexManager *mutex_manager;
  AgsThread *main_loop;

  AgsApplicationContext *application_context;
  GObject *soundcard;

  GList *task;
  
  pthread_mutex_t *application_mutex;
  pthread_mutex_t *channel_mutex;

  window = (AgsWindow *) gtk_widget_get_toplevel((GtkWidget *) notation_edit);

  application_context = (AgsApplicationContext *) window->application_context;
  
  mutex_manager = ags_mutex_manager_get_instance();
  application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);

  /* get threads */
  pthread_mutex_lock(application_mutex);

  main_loop = (AgsThread *) application_context->main_loop;

  pthread_mutex_unlock(application_mutex);

  gui_thread = (AgsGuiThread *) ags_thread_find_type(main_loop,
						     AGS_TYPE_GUI_THREAD);
  
  /* lookup channel mutex */
  pthread_mutex_lock(application_mutex);
  
  channel_mutex = ags_mutex_manager_lookup(mutex_manager,
					   (GObject *) channel);
  
  pthread_mutex_unlock(application_mutex);

  /* get soundcard */
  pthread_mutex_lock(channel_mutex);

  soundcard = channel->soundcard;
  
  pthread_mutex_unlock(channel_mutex);

  if(soundcard == NULL){
    return;
  }

  /* create tasks */
  task = NULL;

  /* init channel for playback */
  init_channel = ags_init_channel_new(channel, FALSE,
				      TRUE, FALSE, FALSE);
  g_signal_connect_after(G_OBJECT(init_channel), "launch",
			 G_CALLBACK(ags_notation_editor_init_channel_launch_callback), note);
  task = g_list_prepend(task,
			init_channel);
    
  /* append channel for playback */
  append_channel = ags_append_channel_new((GObject *) main_loop,
					  (GObject *) channel);
  task = g_list_prepend(task,
			append_channel);

  /* create start task */
  start_soundcard = ags_start_soundcard_new(application_context);
  task = g_list_prepend(task,
			start_soundcard);

  /* perform playback */
  task = g_list_reverse(task);
  ags_gui_thread_schedule_task_list(gui_thread,
				    task);
}

/**
 * ags_notation_editor_do_feedback:
 * @notation_editor: an #AgsNotationEditor
 * 
 * Do playback feedback.
 *
 * Since: 1.2.0
 */
void
ags_notation_editor_do_feedback(AgsNotationEditor *notation_editor)
{
  AgsNotationEdit *notation_edit;
  AgsMachine *machine;
  
  AgsNotation *notation;

  AgsTimestamp *timestamp;

  GList *list_notation;

  gint i;

  if(!AGS_IS_NOTATION_EDITOR(notation_editor)){
    return;
  }
  
  if(notation_editor->selected_machine != NULL){
    AgsChannel *output, *input;
    AgsChannel *channel;
    
    AgsMutexManager *mutex_manager;

    guint audio_flags;
    guint output_pads, input_pads;

    pthread_mutex_t *application_mutex;
    pthread_mutex_t *audio_mutex;

    notation_edit = notation_editor->notation_edit;
    machine = notation_editor->selected_machine;
  
    mutex_manager = ags_mutex_manager_get_instance();
    application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);

    /* get audio mutex */
    pthread_mutex_lock(application_mutex);

    audio_mutex = ags_mutex_manager_lookup(mutex_manager,
					   (GObject *) machine->audio);
  
    pthread_mutex_unlock(application_mutex);

    /* check all active tabs */
    timestamp = ags_timestamp_new();

    timestamp->flags &= (~AGS_TIMESTAMP_UNIX);
    timestamp->flags |= AGS_TIMESTAMP_OFFSET;
    
    timestamp->timer.ags_offset.offset = AGS_NOTATION_DEFAULT_OFFSET * floor(notation_edit->cursor_position_x / AGS_NOTATION_DEFAULT_OFFSET);

    i = 0;

    pthread_mutex_lock(audio_mutex);

    audio_flags = machine->audio->flags;

    output_pads = machine->audio->output_pads;
    input_pads = machine->audio->input_pads;

    output = machine->audio->output;
    input = machine->audio->input;
    
    pthread_mutex_unlock(audio_mutex);

    while((i = ags_notebook_next_active_tab(notation_editor->notebook,
					    i)) != -1){
      AgsNote *current_note;
      
      pthread_mutex_lock(audio_mutex);
      
      list_notation = machine->audio->notation;
      list_notation = ags_notation_find_near_timestamp(list_notation, i,
						       timestamp);

      if(list_notation != NULL){
	notation = list_notation->data;
      }else{
	i++;
	
	pthread_mutex_unlock(audio_mutex);
	
	continue;
      }

      current_note = ags_notation_find_point(list_notation->data,
					     notation_edit->cursor_position_x, notation_edit->cursor_position_y,
					     FALSE);
    
      pthread_mutex_unlock(audio_mutex);

      if(current_note != NULL){
#if 0
	if((AGS_AUDIO_NOTATION_DEFAULT & audio_flags) == 0){
	  channel = ags_channel_nth(output,
				    i);
	}else{
	  channel = ags_channel_nth(input,
				    i);
	}
#else
	channel = ags_channel_nth(input,
				  i);
#endif
	
	if((AGS_AUDIO_REVERSE_MAPPING & audio_flags) != 0){
	  channel = ags_channel_pad_nth(channel,
					(((AGS_AUDIO_NOTATION_DEFAULT & audio_flags) == 0) ? output_pads: input_pads) - notation_edit->cursor_position_y - 1);
	}else{
	  channel = ags_channel_pad_nth(channel,
					notation_edit->cursor_position_y);
	}

	ags_notation_edit_play_channel(notation_edit,
				       channel,
				       current_note);
      }
      
      i++;
    }
  }
}

/**
 * ags_notation_editor_select_all:
 * @notation_editor: an #AgsNotationEditor
 *
 * Is emitted as machine changed of notation_editor.
 *
 * Since: 1.2.0
 */
void
ags_notation_editor_select_all(AgsNotationEditor *notation_editor)
{
  AgsMachine *machine;
      
  GList *list_notation;

  gint i;

  if(!AGS_IS_NOTATION_EDITOR(notation_editor)){
    return;
  }
  
  if(notation_editor->selected_machine != NULL){
    AgsMutexManager *mutex_manager;

    pthread_mutex_t *application_mutex;
    pthread_mutex_t *audio_mutex;

    machine = notation_editor->selected_machine;
  
    mutex_manager = ags_mutex_manager_get_instance();
    application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);
    
    /* get audio mutex */
    pthread_mutex_lock(application_mutex);

    audio_mutex = ags_mutex_manager_lookup(mutex_manager,
					   (GObject *) machine->audio);
  
    pthread_mutex_unlock(application_mutex);

    /* check all active tabs */
    pthread_mutex_lock(audio_mutex);

    i = 0;
    
    while((i = ags_notebook_next_active_tab(notation_editor->notebook,
					    i)) != -1){
      list_notation = machine->audio->notation;
      
      while((list_notation = ags_notation_find_near_timestamp(list_notation, i,
							      NULL)) != NULL){
	ags_notation_add_all_to_selection(AGS_NOTATION(list_notation->data));
	
	list_notation = list_notation->next;
      }

      i++;
    }
    
    pthread_mutex_unlock(audio_mutex);

    gtk_widget_queue_draw(notation_editor->notation_edit);
  }
}

/**
 * ags_notation_editor_paste:
 * @notation_editor: an #AgsNotationEditor
 *
 * Is emitted as machine changed of notation_editor.
 *
 * Since: 1.2.0
 */
void
ags_notation_editor_paste(AgsNotationEditor *notation_editor)
{
  AgsMachine *machine;
  AgsNotationEdit *notation_edit;
  
  AgsNotation *notation;

  xmlDoc *clipboard;
  xmlNode *audio_node;
  xmlNode *notation_list_node, *notation_node;
  xmlNode *timestamp_node;
  
  gchar *buffer;

  guint position_x, position_y;
  gint first_x, last_x;
  gboolean paste_from_position;

  auto gint ags_notation_editor_paste_notation_all(xmlNode *notation_node,
						   AgsTimestamp *timestamp,
						   gboolean match_channel, gboolean no_duplicates);
  auto gint ags_notation_editor_paste_notation(xmlNode *audio_node);

  gint ags_notation_editor_paste_notation_all(xmlNode *notation_node,
					      AgsTimestamp *timestamp,
					      gboolean match_channel, gboolean no_duplicates)
  {    
    AgsNotation *notation;
		
    GList *list_notation;
    
    gint first_x;
    guint current_x;
    gint i;

    first_x = -1;
    
    /*  */
    i = 0;
		
    while((i = ags_notebook_next_active_tab(notation_editor->notebook,
					    i)) != -1){		  
      list_notation = ags_notation_find_near_timestamp(machine->audio->notation, i,
						       timestamp);

      if(list_notation == NULL){
	notation = ags_notation_new(machine->audio,
				    i);
	notation->timestamp->timer.ags_offset.offset = timestamp->timer.ags_offset.offset;
	machine->audio->notation = ags_notation_add(machine->audio->notation,
						    notation);
	g_object_ref(notation);
      }else{
	notation = AGS_NOTATION(list_notation->data);
      }
		  
      if(paste_from_position){
	xmlNode *child;

	guint x_boundary;
	  
	ags_notation_insert_from_clipboard_extended(notation,
						    notation_node,
						    TRUE, position_x,
						    TRUE, position_y,
						    match_channel, no_duplicates);
		    
	/* get boundaries */
	child = notation_node->children;
	current_x = 0;
	  
	while(child != NULL){
	  if(child->type == XML_ELEMENT_NODE){
	    if(!xmlStrncmp(child->name,
			   "note",
			   5)){
	      guint tmp;

	      tmp = g_ascii_strtoull(xmlGetProp(child,
						"x1"),
				     NULL,
				     10);

	      if(tmp > current_x){
		current_x = tmp;
	      }
	    }
	  }

	  child = child->next;
	}

	x_boundary = g_ascii_strtoull(xmlGetProp(notation_node,
						 "x_boundary"),
				      NULL,
				      10);


	if(first_x == -1 || x_boundary < first_x){
	  first_x = x_boundary;
	}
	  
	if(position_x > x_boundary){
	  current_x += (position_x - x_boundary);
	}else{
	  current_x -= (x_boundary - position_x);
	}
	  
	if(current_x > last_x){
	  last_x = current_x;
	}	
      }else{
	xmlNode *child;

	ags_notation_insert_from_clipboard(notation,
					   notation_node,
					   FALSE, 0,
					   FALSE, 0);

	/* get boundaries */
	child = notation_node->children;
	current_x = 0;
	  
	while(child != NULL){
	  if(child->type == XML_ELEMENT_NODE){
	    if(!xmlStrncmp(child->name,
			   "note",
			   5)){
	      guint tmp;

	      tmp = g_ascii_strtoull(xmlGetProp(child,
						"x1"),
				     NULL,
				     10);

	      if(tmp > current_x){
		current_x = tmp;
	      }
	    }
	  }

	  child = child->next;
	}

	if(current_x > last_x){
	  last_x = current_x;
	}
      }
		  
      i++;
    }

    return(first_x);
  }
  
  gint ags_notation_editor_paste_notation(xmlNode *audio_node){
    AgsTimestamp *timestamp;

    gint first_x;
    gboolean match_channel, no_duplicates;

    first_x = -1;

    match_channel = ((AGS_NOTATION_EDITOR_PASTE_MATCH_AUDIO_CHANNEL & (notation_editor->flags)) != 0) ? TRUE: FALSE;
    no_duplicates = ((AGS_NOTATION_EDITOR_PASTE_NO_DUPLICATES & (notation_editor->flags)) != 0) ? TRUE: FALSE;

    /* timestamp */
    timestamp = ags_timestamp_new();

    timestamp->flags &= (~AGS_TIMESTAMP_UNIX);
    timestamp->flags |= AGS_TIMESTAMP_OFFSET;
    
    timestamp->timer.ags_offset.offset = 0;
    
    /* paste notation */
    notation_list_node = audio_node->children;

    while(notation_list_node != NULL){
      if(notation_list_node->type == XML_ELEMENT_NODE){
	if(!xmlStrncmp(notation_list_node->name,
		       "notation-list",
		       14)){
	  notation_node = notation_list_node->children;
	  
	  while(notation_node != NULL){
	    if(notation_node->type == XML_ELEMENT_NODE){
	      if(!xmlStrncmp(notation_node->name,
			     "notation",
			     9)){
		guint64 offset;
		
		timestamp_node = notation_node->children;
		offset = 0;
	  
		while(timestamp_node != NULL){
		  if(timestamp_node->type == XML_ELEMENT_NODE){
		    if(!xmlStrncmp(timestamp_node->name,
				   "timestamp",
				   10)){
		      offset = g_ascii_strtoull(xmlGetProp(timestamp_node,
							   "offset"),
						NULL,
						10);
		      
		      break;
		    }
		  }

		  timestamp_node = timestamp_node->next;
		}     

		/* 1st attempt */
		timestamp->timer.ags_offset.offset = offset;
		
		first_x = ags_notation_editor_paste_notation_all(notation_node,
								 timestamp,
								 match_channel, no_duplicates);

		/* 2nd attempt */
		timestamp->timer.ags_offset.offset = offset + AGS_NOTATION_DEFAULT_OFFSET;

		ags_notation_editor_paste_notation_all(notation_node,
						       timestamp,
						       match_channel, no_duplicates);
	      }
	    }

	    notation_node = notation_node->next;
	  }	  
	}
      }

      notation_list_node = notation_list_node->next;
    }    

    g_object_unref(timestamp);

    return(first_x);
  }

  if(!AGS_IS_NOTATION_EDITOR(notation_editor)){
    return;
  }
  
  if((machine = notation_editor->selected_machine) != NULL){
    AgsMutexManager *mutex_manager;

    pthread_mutex_t *application_mutex;
    pthread_mutex_t *audio_mutex;

    notation_edit = notation_editor->notation_edit;
    
    mutex_manager = ags_mutex_manager_get_instance();
    application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);
    
    /* get audio mutex */
    pthread_mutex_lock(application_mutex);  
    
    audio_mutex = ags_mutex_manager_lookup(mutex_manager,
					   (GObject *) notation_editor->selected_machine->audio);
  
    pthread_mutex_unlock(application_mutex);

    /* get clipboard */
    buffer = gtk_clipboard_wait_for_text(gtk_clipboard_get(GDK_SELECTION_CLIPBOARD));
    
    if(buffer == NULL){
      return;
    }

    /* get position */
    if(notation_editor->notation_toolbar->selected_edit_mode == notation_editor->notation_toolbar->position){
      last_x = 0;
      paste_from_position = TRUE;

      position_x = notation_editor->notation_edit->cursor_position_x;
      position_y = notation_editor->notation_edit->cursor_position_y;
      
#ifdef DEBUG
      printf("pasting at position: [%u,%u]\n", position_x, position_y);
#endif
    }else{
      paste_from_position = FALSE;
    }

    /* get xml tree */
    clipboard = xmlReadMemory(buffer, strlen(buffer),
			      NULL, "UTF-8",
			      0);
    audio_node = xmlDocGetRootElement(clipboard);

    first_x = -1;
    
    /* iterate xml tree */
    pthread_mutex_lock(audio_mutex);

    while(audio_node != NULL){
      if(audio_node->type == XML_ELEMENT_NODE){
	if(!xmlStrncmp("audio", audio_node->name, 6)){
	  notation_node = audio_node->children;
	  
	  first_x = ags_notation_editor_paste_notation(audio_node);
	
	  break;
	}
      }
      
      audio_node = audio_node->next;
    }

    if(first_x == -1){
      first_x = 0;
    }
    
    xmlFreeDoc(clipboard); 

    pthread_mutex_unlock(audio_mutex);

    if(paste_from_position){
      gint big_step, small_step;

      //TODO:JK: implement me
      big_step = (guint) ceil((double) last_x / 16.0) * 16.0 + (notation_edit->cursor_position_x % (guint) 16);
      small_step = (guint) big_step - 16;
	
      if(small_step < last_x){
	notation_editor->notation_edit->cursor_position_x = big_step;
      }else{
	notation_editor->notation_edit->cursor_position_x = small_step;
      }
    }

    gtk_widget_queue_draw(notation_editor->notation_edit);
  }
}

/**
 * ags_notation_editor_copy:
 * @notation_editor: an #AgsNotationEditor
 *
 * Is emitted as machine changed of notation_editor.
 *
 * Since: 1.2.0
 */
void
ags_notation_editor_copy(AgsNotationEditor *notation_editor)
{
  AgsMachine *machine;
  
  AgsNotation *notation;

  xmlDoc *clipboard;
  xmlNode *audio_node, *notation_list_node, *notation_node;

  GList *list_notation;

  xmlChar *buffer;

  int size;
  gint i;

  if(!AGS_IS_NOTATION_EDITOR(notation_editor)){
    return;
  }
  
  if(notation_editor->selected_machine != NULL){
    AgsMutexManager *mutex_manager;

    pthread_mutex_t *application_mutex;
    pthread_mutex_t *audio_mutex;

    machine = notation_editor->selected_machine;
  
    mutex_manager = ags_mutex_manager_get_instance();
    application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);
    
    /* get audio mutex */
    pthread_mutex_lock(application_mutex);

    audio_mutex = ags_mutex_manager_lookup(mutex_manager,
					   (GObject *) machine->audio);
  
    pthread_mutex_unlock(application_mutex);

    /* create document */
    clipboard = xmlNewDoc(BAD_CAST XML_DEFAULT_VERSION);

    /* create root node */
    audio_node = xmlNewNode(NULL,
			    BAD_CAST "audio");
    xmlDocSetRootElement(clipboard, audio_node);

    notation_list_node = xmlNewNode(NULL,
				    BAD_CAST "notation-list");
    xmlAddChild(audio_node,
		notation_list_node);

    /* create notation nodes */
    list_notation = machine->audio->notation;
    i = 0;

    while((i = ags_notebook_next_active_tab(notation_editor->notebook,
					    i)) != -1){
      pthread_mutex_lock(audio_mutex);

      list_notation = machine->audio->notation;

      /* copy */
      while((list_notation = ags_notation_find_near_timestamp(list_notation, i,
							      NULL)) != NULL){
	notation_node = ags_notation_copy_selection(AGS_NOTATION(list_notation->data));
	xmlAddChild(notation_list_node,
		    notation_node);
	
	list_notation = list_notation->next;
      }

      pthread_mutex_unlock(audio_mutex);

      i++;
    }
    
    /* write to clipboard */
    xmlDocDumpFormatMemoryEnc(clipboard, &buffer, &size, "UTF-8", TRUE);
    gtk_clipboard_set_text(gtk_clipboard_get(GDK_SELECTION_CLIPBOARD),
			   buffer, size);
    gtk_clipboard_store(gtk_clipboard_get(GDK_SELECTION_CLIPBOARD));

    xmlFreeDoc(clipboard);
  }
}

/**
 * ags_notation_editor_cut:
 * @notation_editor: an #AgsNotationEditor
 *
 * Is emitted as machine changed of notation_editor.
 *
 * Since: 1.2.0
 */
void
ags_notation_editor_cut(AgsNotationEditor *notation_editor)
{
  AgsMachine *machine;
  
  AgsNotation *notation;

  xmlDoc *clipboard;
  xmlNode *audio_node;
  xmlNode *notation_list_node, *notation_node;

  GList *list_notation;

  xmlChar *buffer;
  int size;
  gint i;

  if(!AGS_IS_NOTATION_EDITOR(notation_editor)){
    return;
  }
  
  if(notation_editor->selected_machine != NULL){
    AgsMutexManager *mutex_manager;

    pthread_mutex_t *application_mutex;
    pthread_mutex_t *audio_mutex;

    machine = notation_editor->selected_machine;

    mutex_manager = ags_mutex_manager_get_instance();
    application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);
    
    /* get audio mutex */
    pthread_mutex_lock(application_mutex);

    audio_mutex = ags_mutex_manager_lookup(mutex_manager,
					   (GObject *) machine->audio);
  
    pthread_mutex_unlock(application_mutex);

    /* create document */
    clipboard = xmlNewDoc(BAD_CAST XML_DEFAULT_VERSION);

    /* create root node */
    audio_node = xmlNewNode(NULL,
			    BAD_CAST "audio");
    xmlDocSetRootElement(clipboard, audio_node);

    notation_list_node = xmlNewNode(NULL,
				    BAD_CAST "notation-list");
    xmlAddChild(audio_node,
		notation_list_node);

    /* create notation nodes */
    pthread_mutex_lock(audio_mutex);

    list_notation = machine->audio->notation;

    pthread_mutex_unlock(audio_mutex);

    i = 0;
    
    while((i = ags_notebook_next_active_tab(notation_editor->notebook,
					    i)) != -1){
      pthread_mutex_lock(audio_mutex);
      
      list_notation = machine->audio->notation;

      /* cut */
      while((list_notation = ags_notation_find_near_timestamp(list_notation, i,
							      NULL)) != NULL){
	notation_node = ags_notation_cut_selection(AGS_NOTATION(list_notation->data));
	xmlAddChild(notation_list_node,
		    notation_node);
	
	list_notation = list_notation->next;
      }
      
      pthread_mutex_unlock(audio_mutex);

      i++;
    }

    gtk_widget_queue_draw(notation_editor->notation_edit);

    /* write to clipboard */
    xmlDocDumpFormatMemoryEnc(clipboard, &buffer, &size, "UTF-8", TRUE);
    gtk_clipboard_set_text(gtk_clipboard_get(GDK_SELECTION_CLIPBOARD),
			   buffer, size);
    gtk_clipboard_store(gtk_clipboard_get(GDK_SELECTION_CLIPBOARD));

    xmlFreeDoc(clipboard);
  }
}

/**
 * ags_notation_editor_invert:
 * @notation_editor: an #AgsNotationEditor
 *
 * Invert all notation of @notation_editor's selected machine.
 *
 * Since: 1.2.0
 */
void
ags_notation_editor_invert(AgsNotationEditor *notation_editor)
{
  AgsMachine *machine;
  
  AgsNotation *notation;

  GList *list_notation;

  int size;
  gint i;

  auto void ags_notation_editor_get_boundary(AgsNotation *notation,
					     guint *lower, guint *upper);
  auto void ags_notation_editor_invert_notation(AgsNotation *notation,
						guint lower, guint upper);

  void ags_notation_editor_get_boundary(AgsNotation *notation,
					guint *lower, guint *upper)
  {
    GList *note;
    
    /* retrieve upper and lower */    
    while(note != NULL){
      if(AGS_NOTE(note->data)->y < lower[0]){
	lower[0] = AGS_NOTE(note->data)->y;
      }

      if(AGS_NOTE(note->data)->y > upper[0]){
	upper[0] = AGS_NOTE(note->data)->y;
      }
      
      note = note->next;
    }
  }
  
  void ags_notation_editor_invert_notation(AgsNotation *notation,
					   guint lower, guint upper)
  {
    GList *note;
    
    /* invert */
    note = notation->notes;

    while(note != NULL){
      if((gdouble) AGS_NOTE(note->data)->y < (gdouble) (upper - lower) / 2.0){
	AGS_NOTE(note->data)->y = (upper - (AGS_NOTE(note->data)->y - lower));
      }else if((gdouble) AGS_NOTE(note->data)->y > (gdouble) (upper - lower) / 2.0){
	AGS_NOTE(note->data)->y = (lower + (upper - AGS_NOTE(note->data)->y));
      }
      
      note = note->next;
    }
  }
  
  if(!AGS_IS_NOTATION_EDITOR(notation_editor)){
    return;
  }
  
  if(notation_editor->selected_machine != NULL){
    AgsMutexManager *mutex_manager;

    guint lower, upper;
    
    pthread_mutex_t *application_mutex;
    pthread_mutex_t *audio_mutex;

    mutex_manager = ags_mutex_manager_get_instance();
    application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);
    
    /* get audio mutex */
    pthread_mutex_lock(application_mutex);

    audio_mutex = ags_mutex_manager_lookup(mutex_manager,
					   (GObject *) notation_editor->selected_machine->audio);
  
    pthread_mutex_unlock(application_mutex);

    /* create notation nodes */
    pthread_mutex_lock(audio_mutex);

    machine = notation_editor->selected_machine;

    i = 0;
    
    while((i = ags_notebook_next_active_tab(notation_editor->notebook,
					    i)) != -1){
      /* get boundary */
      list_notation = machine->audio->notation;

      lower = G_MAXUINT;
      upper = 0;

      while(list_notation != NULL){
	list_notation = ags_notation_find_near_timestamp(list_notation, i,
							 NULL);
	ags_notation_editor_get_boundary(AGS_NOTATION(list_notation->data),
					 &lower, &upper);

	list_notation = list_notation->next;
      }
      
      /* invert */
      list_notation = machine->audio->notation;
      
      while((list_notation = ags_notation_find_near_timestamp(list_notation, i,
							      NULL)) != NULL){
	ags_notation_editor_invert_notation(AGS_NOTATION(list_notation->data),
					    lower, upper);

	list_notation = list_notation->next;
      }
      
      i++;
    }

    pthread_mutex_unlock(audio_mutex);
  
    gtk_widget_queue_draw(notation_editor->notation_edit);
  }
}

/**
 * ags_notation_editor_new:
 *
 * Creates an #AgsNotationEditor
 *
 * Returns: a new #AgsNotationEditor
 *
 * Since: 1.2.0
 */
AgsNotationEditor*
ags_notation_editor_new()
{
  AgsNotationEditor *notation_editor;

  notation_editor = (AgsNotationEditor *) g_object_new(AGS_TYPE_NOTATION_EDITOR,
						       NULL);

  return(notation_editor);
}
