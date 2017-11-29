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

#include <ags/object/ags_connectable.h>
#include <ags/object/ags_soundcard.h>

#include <ags/thread/ags_mutex_manager.h>

#include <ags/audio/ags_channel.h>
#include <ags/audio/ags_output.h>
#include <ags/audio/ags_input.h>

#include <ags/X/ags_window.h>

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
  static GType ags_type_notation_editor = 0;

  if(!ags_type_notation_editor){
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
  }

  return(ags_type_notation_editor);
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
  GtkHPaned *paned;
  GtkScrolledWindow *scrolled_window;
  GtkTable *table;
  
  notation_editor->flags = 0;

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
		     FALSE, FALSE, 0);

  /* machine selector */
  paned = (GtkHPaned *) gtk_hpaned_new();
  gtk_box_pack_start((GtkBox *) notation_editor,
		     (GtkWidget *) paned,
		     TRUE, TRUE, 0);

  scrolled_window = (GtkScrolledWindow *) gtk_scrolled_window_new(NULL, NULL);
  gtk_paned_pack1((GtkPaned *) paned, (GtkWidget *) scrolled_window, FALSE, TRUE);

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
  table = (GtkTable *) gtk_table_new(3, 2,
				     FALSE);
  gtk_paned_pack2((GtkPaned *) paned,
		  (GtkWidget *) table,
		  TRUE, FALSE);

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

  /* piano */
  notation_editor->piano_viewport = gtk_viewport_new(NULL,
						     NULL);
  gtk_table_attach(table,
		   (GtkWidget *) notation_editor->notebook,
		   1, 2,
		   1, 2,
		   GTK_FILL, GTK_FILL,
		   0, 0);

  notation_editor->piano = ags_piano_new();
  gtk_container_add(notation_editor->piano_viewport,
		    notation_editor->piano);

  /* notation edit */
  notation_editor->notation_edit = ags_notation_edit_new();
  gtk_table_attach(table,
		   (GtkWidget *) notation_editor->notation_edit,
		   1, 2,
		   2, 3,
		   GTK_FILL|GTK_EXPAND, GTK_FILL,|GTK_EXPAND
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
  //TODO:JK: implement me
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

  if(notation_editor->selected_machine != NULL){
    AgsMutexManager *mutex_manager;

    pthread_mutex_t *application_mutex;
    pthread_mutex_t *audio_mutex;
  
    mutex_manager = ags_mutex_manager_get_instance();
    application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);
    
    pthread_mutex_lock(application_mutex);

    audio_mutex = ags_mutex_manager_lookup(mutex_manager,
					   (GObject *) notation_editor->selected_machine->audio);
  
    pthread_mutex_unlock(application_mutex);

    pthread_mutex_lock(audio_mutex);
    
    machine = notation_editor->selected_machine;

    list_notation = machine->audio->notation;
    i = 0;

    while((i = ags_notebook_next_active_tab(notation_editor->notebook,
					    i)) != -1){
      list_notation = g_list_nth(machine->audio->notation,
				 i);
      ags_notation_add_all_to_selection(AGS_NOTATION(list_notation->data));

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
  xmlNode *audio_node, *notation_node;
  
  gchar *buffer;

  guint position_x, position_y;
  gint first_x, last_x;
  gboolean paste_from_position;

  auto gint ags_notation_editor_paste_read_notation();
  
  gint ags_notation_editor_paste_read_notation(){
    xmlXPathContext *xpathCtxt;
    xmlXPathObject *xpathObj;
    xmlNodeSet *nodes;
    GList *notation_list;

    guint first_x;
    
    xpathCtxt = xmlXPathNewContext(clipboard);
    xpathObj = xmlXPathEvalExpression("/audio/notation", xpathCtxt);

    first_x = -1;
    
    if(xpathObj != NULL){
      int i, j, size;
      guint audio_channel;
      guint current_x;
      
      nodes = xpathObj->nodesetval;
      size = (nodes != NULL) ? nodes->nodeNr: 0;
      j = 0;

      for(i = 0; i < size; i++){

	j = ags_notebook_next_active_tab(notation_editor->notebook,
					 j);

	if(j == -1){
	  break;
	}
	
	//	audio_channel = (guint) g_ascii_strtoull(xmlGetProp(nodes->nodeTab[i],
	//						    "audio-channel"),
	//					 NULL,
	//					 10);
	notation_list = g_list_nth(machine->audio->notation,
				   j);
	j++;
	
	if(notation_list == NULL){
	  break;
	}
	
	if(paste_from_position){
	  xmlNode *child;

	  guint x_boundary;
	  
	  ags_notation_insert_from_clipboard(AGS_NOTATION(notation_list->data),
					     nodes->nodeTab[i],
					     TRUE, position_x,
					     TRUE, position_y);

	  /* get boundaries */
	  child = nodes->nodeTab[i]->children;
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

	  x_boundary = g_ascii_strtoull(xmlGetProp(nodes->nodeTab[i],
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

	  ags_notation_insert_from_clipboard(AGS_NOTATION(notation_list->data),
					     nodes->nodeTab[i],
					     FALSE, 0,
					     FALSE, 0);

	  /* get boundaries */
	  child = nodes->nodeTab[i]->children;
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
      }

      xmlXPathFreeObject(xpathObj);
    }

    return(first_x);
  }
  
  if((machine = notation_editor->selected_machine) != NULL){
    AgsMutexManager *mutex_manager;

    pthread_mutex_t *application_mutex;
    pthread_mutex_t *audio_mutex;

    notation_edit = notation_editor->notation_edit;
    
    mutex_manager = ags_mutex_manager_get_instance();
    application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);
    
    pthread_mutex_lock(application_mutex);  
    
    audio_mutex = ags_mutex_manager_lookup(mutex_manager,
					   (GObject *) notation_editor->selected_machine->audio);
  
    pthread_mutex_unlock(application_mutex);

    pthread_mutex_lock(audio_mutex);

    /* get clipboard */
    buffer = gtk_clipboard_wait_for_text(gtk_clipboard_get(GDK_SELECTION_CLIPBOARD));
    
    if(buffer == NULL){
      pthread_mutex_unlock(audio_mutex);

      return;
    }

    /* get position */
    if(notation_editor->toolbar->selected_edit_mode == notation_editor->toolbar->position){
      last_x = 0;
      paste_from_position = TRUE;

      position_x = notation_editor->notation_edit->selected_x;
      position_y = notation_editor->notation_edit->selected_y;
      
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
    while(audio_node != NULL){
      if(audio_node->type == XML_ELEMENT_NODE && !xmlStrncmp("audio", audio_node->name, 6)){

	notation_node = audio_node->children;
	
	first_x = ags_notation_editor_paste_read_notation();
	
	break;
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

      big_step = (guint) ceil((double) last_x * notation_edit->minimum_note_length) / notation_edit->minimum_note_length + (notation_edit->selected_x % (guint) (1 / AGS_NOTATION_MINIMUM_NOTE_LENGTH));
      small_step = (guint) big_step - (1 / notation_edit->minimum_note_length);
	
      if(small_step < last_x){
	notation_editor->notation_edit->selected_x = big_step;
      }else{
	notation_editor->notation_edit->selected_x = small_step;
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
  xmlNode *audio_node, *notation_node;

  GList *list_notation;

  xmlChar *buffer;

  int size;
  gint i;

  if(notation_editor->selected_machine != NULL){
    AgsMutexManager *mutex_manager;

    pthread_mutex_t *application_mutex;
    pthread_mutex_t *audio_mutex;

    machine = notation_editor->selected_machine;
  
    mutex_manager = ags_mutex_manager_get_instance();
    application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);
    
    pthread_mutex_lock(application_mutex);

    audio_mutex = ags_mutex_manager_lookup(mutex_manager,
					   (GObject *) machine->audio);
  
    pthread_mutex_unlock(application_mutex);

    /* create document */
    clipboard = xmlNewDoc(BAD_CAST XML_DEFAULT_VERSION);

    /* create root node */
    audio_node = xmlNewNode(NULL, BAD_CAST "audio");
    xmlDocSetRootElement(clipboard, audio_node);

    /* create notation nodes */
    list_notation = machine->audio->notation;
    i = 0;

    while((i = ags_notebook_next_active_tab(notation_editor->notebook,
							   i)) != -1){
      pthread_mutex_lock(audio_mutex);

      list_notation = g_list_nth(machine->audio->notation,
				 i);

      notation_node = ags_notation_copy_selection(AGS_NOTATION(list_notation->data));

      pthread_mutex_unlock(audio_mutex);

      xmlAddChild(audio_node, notation_node);

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

  xmlDocPtr clipboard;
  xmlNodePtr audio_node, notation_node;

  GList *list_notation;

  xmlChar *buffer;
  int size;
  gint i;

  if(notation_editor->selected_machine != NULL){
    AgsMutexManager *mutex_manager;

    pthread_mutex_t *application_mutex;
    pthread_mutex_t *audio_mutex;

    machine = notation_editor->selected_machine;

    mutex_manager = ags_mutex_manager_get_instance();
    application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);
    
    pthread_mutex_lock(application_mutex);

    audio_mutex = ags_mutex_manager_lookup(mutex_manager,
					   (GObject *) machine->audio);
  
    pthread_mutex_unlock(application_mutex);

    /* create document */
    clipboard = xmlNewDoc(BAD_CAST XML_DEFAULT_VERSION);

    /* create root node */
    audio_node = xmlNewNode(NULL, BAD_CAST "audio");
    xmlDocSetRootElement(clipboard, audio_node);

    /* create notation nodes */
    pthread_mutex_lock(audio_mutex);

    list_notation = machine->audio->notation;

    pthread_mutex_unlock(audio_mutex);

    i = 0;
    
    while((i = ags_notebook_next_active_tab(notation_editor->notebook,
					    i)) != -1){
      pthread_mutex_lock(audio_mutex);
      
      list_notation = g_list_nth(machine->audio->notation,
				 i);


      notation_node = ags_notation_cut_selection(AGS_NOTATION(list_notation->data));

      pthread_mutex_unlock(audio_mutex);

      xmlAddChild(audio_node, notation_node);

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

  auto void ags_notation_editor_invert_notation(AgsNotation *notation);

  void ags_notation_editor_invert_notation(AgsNotation *notation){
    GList *note;
    guint lower, upper;

    note = notation->notes;

    if(note == NULL){
      return;
    }

    /* retrieve upper and lower */
    upper = 0;
    lower = G_MAXUINT;
    
    while(note != NULL){
      if(AGS_NOTE(note->data)->y < lower){
	lower = AGS_NOTE(note->data)->y;
      }

      if(AGS_NOTE(note->data)->y > upper){
	upper = AGS_NOTE(note->data)->y;
      }
      
      note = note->next;
    }

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
  
  if(notation_editor->selected_machine != NULL){
    AgsMutexManager *mutex_manager;

    pthread_mutex_t *application_mutex;
    pthread_mutex_t *audio_mutex;

    mutex_manager = ags_mutex_manager_get_instance();
    application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);
    
    pthread_mutex_lock(application_mutex);

    audio_mutex = ags_mutex_manager_lookup(mutex_manager,
					   (GObject *) notation_editor->selected_machine->audio);
  
    pthread_mutex_unlock(application_mutex);

    pthread_mutex_lock(audio_mutex);

    machine = notation_editor->selected_machine;

    /* create notation nodes */
    list_notation = machine->audio->notation;
    i = 0;
    
    while((i = ags_notebook_next_active_tab(notation_editor->notebook,
					    i)) != -1){
      list_notation = g_list_nth(machine->audio->notation,
				 i);

      ags_notation_editor_invert_notation(AGS_NOTATION(list_notation->data));
      
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
