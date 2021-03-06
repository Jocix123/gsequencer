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

#include <ags/X/editor/ags_wave_edit.h>
#include <ags/X/editor/ags_wave_edit_callbacks.h>

#include <ags/X/ags_ui_provider.h>
#include <ags/X/ags_window.h>
#include <ags/X/ags_navigation.h>
#include <ags/X/ags_wave_window.h>
#include <ags/X/ags_wave_editor.h>

#include <gdk/gdkkeysyms.h>
#include <atk/atk.h>

#include <cairo.h>
#include <math.h>

static GType ags_accessible_wave_edit_get_type(void);
void ags_wave_edit_class_init(AgsWaveEditClass *wave_edit);
void ags_accessible_wave_edit_class_init(AtkObject *object);
void ags_accessible_wave_edit_action_interface_init(AtkActionIface *action);
void ags_wave_edit_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_wave_edit_init(AgsWaveEdit *wave_edit);
void ags_wave_edit_set_property(GObject *gobject,
				guint prop_id,
				const GValue *value,
				GParamSpec *param_spec);
void ags_wave_edit_get_property(GObject *gobject,
				guint prop_id,
				GValue *value,
				GParamSpec *param_spec);
void ags_wave_edit_finalize(GObject *gobject);

void ags_wave_edit_connect(AgsConnectable *connectable);
void ags_wave_edit_disconnect(AgsConnectable *connectable);

AtkObject* ags_wave_edit_get_accessible(GtkWidget *widget);

gboolean ags_accessible_wave_edit_do_action(AtkAction *action,
					    gint i);
gint ags_accessible_wave_edit_get_n_actions(AtkAction *action);
const gchar* ags_accessible_wave_edit_get_description(AtkAction *action,
						      gint i);
const gchar* ags_accessible_wave_edit_get_name(AtkAction *action,
					       gint i);
const gchar* ags_accessible_wave_edit_get_keybinding(AtkAction *action,
						     gint i);
gboolean ags_accessible_wave_edit_set_description(AtkAction *action,
						  gint i);
gchar* ags_accessible_wave_edit_get_localized_name(AtkAction *action,
						   gint i);

void ags_wave_edit_get_preferred_width(GtkWidget *widget,
				       gint *minimal_width,
				       gint *natural_width);
void ags_wave_edit_get_preferred_height(GtkWidget *widget,
					gint *minimal_height,
					gint *natural_height);
void ags_wave_edit_size_allocate(GtkWidget *widget,
				 GtkAllocation *allocation);
void ags_wave_edit_show(GtkWidget *widget);
void ags_wave_edit_show_all(GtkWidget *widget);

gboolean ags_wave_edit_auto_scroll_timeout(GtkWidget *widget);

/**
 * SECTION:ags_wave_edit
 * @short_description: edit audio data
 * @title: AgsWaveEdit
 * @section_id:
 * @include: ags/X/editor/ags_wave_edit.h
 *
 * The #AgsWaveEdit lets you edit audio data.
 */

static gpointer ags_wave_edit_parent_class = NULL;

static GQuark quark_accessible_object = 0;

GHashTable *ags_wave_edit_auto_scroll = NULL;

enum{
  PROP_0,
  PROP_LINE,
};

GType
ags_wave_edit_get_type(void)
{
  static volatile gsize g_define_type_id__volatile = 0;

  if(g_once_init_enter (&g_define_type_id__volatile)){
    GType ags_type_wave_edit = 0;

    static const GTypeInfo ags_wave_edit_info = {
      sizeof (AgsWaveEditClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_wave_edit_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsWaveEdit),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_wave_edit_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_wave_edit_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_wave_edit = g_type_register_static(GTK_TYPE_TABLE,
						"AgsWaveEdit", &ags_wave_edit_info,
						0);
    
    g_type_add_interface_static(ags_type_wave_edit,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);

    g_once_init_leave(&g_define_type_id__volatile, ags_type_wave_edit);
  }

  return g_define_type_id__volatile;
}

static GType
ags_accessible_wave_edit_get_type(void)
{
  static GType ags_type_accessible_wave_edit = 0;

  if(!ags_type_accessible_wave_edit){
    const GTypeInfo ags_accesssible_wave_edit_info = {
      sizeof(GtkAccessibleClass),
      NULL,           /* base_init */
      NULL,           /* base_finalize */
      (GClassInitFunc) ags_accessible_wave_edit_class_init,
      NULL,           /* class_finalize */
      NULL,           /* class_data */
      sizeof(GtkAccessible),
      0,             /* n_preallocs */
      NULL, NULL
    };

    static const GInterfaceInfo atk_action_interface_info = {
      (GInterfaceInitFunc) ags_accessible_wave_edit_action_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };
    
    ags_type_accessible_wave_edit = g_type_register_static(GTK_TYPE_ACCESSIBLE,
							   "AgsAccessibleWaveEdit", &ags_accesssible_wave_edit_info,
							   0);

    g_type_add_interface_static(ags_type_accessible_wave_edit,
				ATK_TYPE_ACTION,
				&atk_action_interface_info);
  }
  
  return(ags_type_accessible_wave_edit);
}


void
ags_wave_edit_class_init(AgsWaveEditClass *wave_edit)
{
  GtkWidgetClass *widget;

  GObjectClass *gobject;

  GParamSpec *param_spec;

  ags_wave_edit_parent_class = g_type_class_peek_parent(wave_edit);

  /* GObjectClass */
  gobject = G_OBJECT_CLASS(wave_edit);

  gobject->set_property = ags_wave_edit_set_property;
  gobject->get_property = ags_wave_edit_get_property;

  gobject->finalize = ags_wave_edit_finalize;

  /* properties */
  /**
   * AgsWaveEdit:line:
   *
   * The wave edit's line.
   * 
   * Since: 3.0.0
   */
  param_spec = g_param_spec_uint("line",
				 "line",
				 "The line of wave edit",
				 0,
				 G_MAXUINT32,
				 0,
				 G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_LINE,
				  param_spec);

  /* GtkWidgetClass */
  widget = (GtkWidgetClass *) wave_edit;

//  widget->get_preferred_width = ags_wave_edit_get_preferred_width;
//  widget->get_preferred_height = ags_wave_edit_get_preferred_height;
//  widget->size_allocate = ags_wave_edit_size_allocate;
//  widget->draw = ags_wave_edit_draw;
  widget->show = ags_wave_edit_show;
  widget->show_all = ags_wave_edit_show_all;
}

void
ags_wave_edit_connectable_interface_init(AgsConnectableInterface *connectable)
{
  connectable->is_ready = NULL;
  connectable->is_connected = NULL;
  connectable->connect = ags_wave_edit_connect;
  connectable->disconnect = ags_wave_edit_disconnect;
}

void
ags_accessible_wave_edit_class_init(AtkObject *object)
{
  quark_accessible_object = g_quark_from_static_string("ags-accessible-object");
}

void
ags_accessible_wave_edit_action_interface_init(AtkActionIface *action)
{
  action->do_action = ags_accessible_wave_edit_do_action;
  action->get_n_actions = ags_accessible_wave_edit_get_n_actions;
  action->get_description = ags_accessible_wave_edit_get_description;
  action->get_name = ags_accessible_wave_edit_get_name;
  action->get_keybinding = ags_accessible_wave_edit_get_keybinding;
  action->set_description = ags_accessible_wave_edit_set_description;
  action->get_localized_name = ags_accessible_wave_edit_get_localized_name;
}

void
ags_wave_edit_init(AgsWaveEdit *wave_edit)
{
  GtkAdjustment *adjustment;

  AgsApplicationContext *application_context;
  
  gdouble gui_scale_factor;

  application_context = ags_application_context_get_instance();

  g_object_set(wave_edit,
	       "can-focus", FALSE,
	       "n-columns", 3,
	       "n-rows", 4,
	       "homogeneous", FALSE,
	       NULL);

  wave_edit->flags = 0;
  wave_edit->mode = AGS_WAVE_EDIT_NO_EDIT_MODE;

  wave_edit->button_mask = 0;
  wave_edit->key_mask = 0;

  /* scale factor */
  gui_scale_factor = ags_ui_provider_get_gui_scale_factor(AGS_UI_PROVIDER(application_context));

  wave_edit->line = 0;
  
  wave_edit->note_offset = 0;
  wave_edit->note_offset_absolute = 0;

  wave_edit->control_width = (guint) (gui_scale_factor * AGS_WAVE_EDIT_DEFAULT_CONTROL_WIDTH);
  wave_edit->control_height = (guint) (gui_scale_factor * AGS_WAVE_EDIT_DEFAULT_CONTROL_HEIGHT);
  
  wave_edit->cursor_position_x = AGS_WAVE_EDIT_DEFAULT_CURSOR_POSITION_X;
  wave_edit->cursor_position_y = AGS_WAVE_EDIT_DEFAULT_CURSOR_POSITION_Y;

  wave_edit->selected_buffer_border = AGS_WAVE_EDIT_DEFAULT_SELECTED_BUFFER_BORDER;
  
  wave_edit->selection_x0 = 0;
  wave_edit->selection_x1 = 0;
  wave_edit->selection_y0 = 0;
  wave_edit->selection_y1 = 0;

  wave_edit->ruler = ags_ruler_new();
  g_object_set(wave_edit->ruler,
	       "height-request", (gint) (gui_scale_factor * AGS_RULER_DEFAULT_HEIGHT),
	       "font-size",  (guint) (gui_scale_factor * wave_edit->ruler->font_size),
	       "step", (guint) (gui_scale_factor * AGS_RULER_DEFAULT_STEP),
	       "large-step", (guint) (gui_scale_factor * AGS_RULER_DEFAULT_LARGE_STEP),
	       "small-step", (guint) (gui_scale_factor * AGS_RULER_DEFAULT_SMALL_STEP),
	       "no-show-all", TRUE,
	       NULL);
  gtk_widget_set_size_request((GtkWidget *) wave_edit->ruler,
			      -1,
			      (gint) (gui_scale_factor * AGS_RULER_DEFAULT_HEIGHT));
  gtk_table_attach(GTK_TABLE(wave_edit),
		   (GtkWidget *) wave_edit->ruler,
		   0, 1,
		   0, 1,
		   GTK_FILL|GTK_EXPAND,
		   GTK_FILL,
		   0, 0);

  wave_edit->lower = AGS_WAVE_EDIT_DEFAULT_LOWER;
  wave_edit->upper = AGS_WAVE_EDIT_DEFAULT_UPPER;

  wave_edit->default_value = AGS_WAVE_EDIT_DEFAULT_VALUE;

  wave_edit->drawing_area = (GtkDrawingArea *) gtk_drawing_area_new();
  gtk_widget_set_events(GTK_WIDGET (wave_edit->drawing_area), GDK_EXPOSURE_MASK
			| GDK_LEAVE_NOTIFY_MASK
			| GDK_BUTTON_PRESS_MASK
			| GDK_BUTTON_RELEASE_MASK
			| GDK_POINTER_MOTION_MASK
			| GDK_POINTER_MOTION_HINT_MASK
			| GDK_CONTROL_MASK
			| GDK_KEY_PRESS_MASK
			| GDK_KEY_RELEASE_MASK);
  gtk_widget_set_can_focus((GtkWidget *) wave_edit->drawing_area,
			   TRUE);
    
  gtk_widget_set_size_request((GtkWidget *) wave_edit->drawing_area,
			      -1, (gint) (gui_scale_factor * AGS_LEVEL_DEFAULT_LEVEL_HEIGHT));
  gtk_table_attach(GTK_TABLE(wave_edit),
		   (GtkWidget *) wave_edit->drawing_area,
		   0, 1,
		   1, 2,
		   GTK_FILL|GTK_EXPAND,
		   GTK_FILL|GTK_EXPAND,
		   0, 0);

  wave_edit->wave_data = NULL;
  wave_edit->stride = -1;
  
  /* vscrollbar */
  adjustment = (GtkAdjustment *) gtk_adjustment_new(0.0, 0.0, 1.0, 1.0, wave_edit->control_height, 1.0);
  wave_edit->vscrollbar = (GtkVScrollbar *) gtk_vscrollbar_new(adjustment);
  g_object_set(wave_edit->vscrollbar,
	       "no-show-all", TRUE,
	       NULL);
  gtk_widget_set_size_request((GtkWidget *) wave_edit->vscrollbar,
			      -1, (gint) (gui_scale_factor * AGS_LEVEL_DEFAULT_LEVEL_HEIGHT));
  gtk_table_attach(GTK_TABLE(wave_edit),
		   (GtkWidget *) wave_edit->vscrollbar,
		   1, 2,
		   1, 2,
		   GTK_FILL, GTK_FILL,
		   0, 0);

  /* hscrollbar */
  adjustment = (GtkAdjustment *) gtk_adjustment_new(0.0, 0.0, 1.0, 1.0, (gdouble) wave_edit->control_width, 1.0);
  wave_edit->hscrollbar = (GtkHScrollbar *) gtk_hscrollbar_new(adjustment);
  g_object_set(wave_edit->hscrollbar,
	       "no-show-all", TRUE,
	       NULL);
  gtk_widget_set_size_request((GtkWidget *) wave_edit->hscrollbar,
			      -1, -1);
  gtk_table_attach(GTK_TABLE(wave_edit),
		   (GtkWidget *) wave_edit->hscrollbar,
		   0, 1,
		   2, 3,
		   GTK_FILL, GTK_FILL,
		   0, 0);

  /* auto-scroll */
  if(ags_wave_edit_auto_scroll == NULL){
    ags_wave_edit_auto_scroll = g_hash_table_new_full(g_direct_hash, g_direct_equal,
						      NULL,
						      NULL);
  }

  g_hash_table_insert(ags_wave_edit_auto_scroll,
		      wave_edit, ags_wave_edit_auto_scroll_timeout);
  g_timeout_add(1000 / 30, (GSourceFunc) ags_wave_edit_auto_scroll_timeout, (gpointer) wave_edit);
}

void
ags_wave_edit_set_property(GObject *gobject,
				 guint prop_id,
				 const GValue *value,
				 GParamSpec *param_spec)
{
  AgsWaveEdit *wave_edit;

  wave_edit = AGS_WAVE_EDIT(gobject);

  switch(prop_id){
  case PROP_LINE:
    {
      wave_edit->line = g_value_get_uint(value);

      gtk_widget_queue_draw((GtkWidget *) wave_edit);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_wave_edit_get_property(GObject *gobject,
				 guint prop_id,
				 GValue *value,
				 GParamSpec *param_spec)
{
  AgsWaveEdit *wave_edit;

  wave_edit = AGS_WAVE_EDIT(gobject);

  switch(prop_id){
  case PROP_LINE:
    {
      g_value_set_uint(value,
		       wave_edit->line);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_wave_edit_finalize(GObject *gobject)
{
  AgsWaveEdit *wave_edit;
  
  wave_edit = AGS_WAVE_EDIT(gobject);
  
  /* remove auto scroll */
  g_hash_table_remove(ags_wave_edit_auto_scroll,
		      wave_edit);

  /* call parent */
  G_OBJECT_CLASS(ags_wave_edit_parent_class)->finalize(gobject);
}

void
ags_wave_edit_connect(AgsConnectable *connectable)
{
  AgsWaveEdit *wave_edit;

  wave_edit = AGS_WAVE_EDIT(connectable);

  if((AGS_WAVE_EDIT_CONNECTED & (wave_edit->flags)) != 0){
    return;
  }
  
  wave_edit->flags |= AGS_WAVE_EDIT_CONNECTED;

  /* drawing area */
  g_signal_connect(G_OBJECT(wave_edit->drawing_area), "draw",
		   G_CALLBACK(ags_wave_edit_draw_callback), (gpointer) wave_edit);

  g_signal_connect_after((GObject *) wave_edit->drawing_area, "configure_event",
			 G_CALLBACK(ags_wave_edit_drawing_area_configure_event), (gpointer) wave_edit);

  g_signal_connect((GObject *) wave_edit->drawing_area, "button_press_event",
		   G_CALLBACK(ags_wave_edit_drawing_area_button_press_event), (gpointer) wave_edit);

  g_signal_connect((GObject *) wave_edit->drawing_area, "button_release_event",
		   G_CALLBACK(ags_wave_edit_drawing_area_button_release_event), (gpointer) wave_edit);
  
  g_signal_connect((GObject *) wave_edit->drawing_area, "motion_notify_event",
		   G_CALLBACK(ags_wave_edit_drawing_area_motion_notify_event), (gpointer) wave_edit);

  g_signal_connect((GObject *) wave_edit->drawing_area, "key_press_event",
		   G_CALLBACK(ags_wave_edit_drawing_area_key_press_event), (gpointer) wave_edit);

  g_signal_connect((GObject *) wave_edit->drawing_area, "key_release_event",
		   G_CALLBACK(ags_wave_edit_drawing_area_key_release_event), (gpointer) wave_edit);

  /* scrollbars */
  g_signal_connect_after((GObject *) wave_edit->vscrollbar, "value-changed",
			 G_CALLBACK(ags_wave_edit_vscrollbar_value_changed), (gpointer) wave_edit);

  g_signal_connect_after((GObject *) wave_edit->hscrollbar, "value-changed",
			 G_CALLBACK(ags_wave_edit_hscrollbar_value_changed), (gpointer) wave_edit);
}

void
ags_wave_edit_disconnect(AgsConnectable *connectable)
{
  AgsWaveEdit *wave_edit;

  wave_edit = AGS_WAVE_EDIT(connectable);

  if((AGS_WAVE_EDIT_CONNECTED & (wave_edit->flags)) == 0){
    return;
  }
  
  wave_edit->flags &= (~AGS_WAVE_EDIT_CONNECTED);

  /* drawing area */
  g_object_disconnect((GObject *) wave_edit->drawing_area,
		      "any_signal::draw",
		      G_CALLBACK(ags_wave_edit_draw_callback),
		      (gpointer) wave_edit,
		      "any_signal::configure_event",
		      G_CALLBACK(ags_wave_edit_drawing_area_configure_event),
		      wave_edit,
		      "any_signal::button_press_event",
		      G_CALLBACK(ags_wave_edit_drawing_area_button_press_event),
		      wave_edit,
		      "any_signal::button_release_event",
		      G_CALLBACK(ags_wave_edit_drawing_area_button_release_event),
		      wave_edit,
		      "any_signal::motion_notify_event",
		      G_CALLBACK(ags_wave_edit_drawing_area_motion_notify_event),
		      wave_edit,
		      "any_signal::key_press_event",
		      G_CALLBACK(ags_wave_edit_drawing_area_key_press_event),
		      wave_edit,
		      "any_signal::key_release_event",
		      G_CALLBACK(ags_wave_edit_drawing_area_key_release_event),
		      wave_edit,
		      NULL);

  /* scrollbars */
  g_object_disconnect((GObject *) wave_edit->vscrollbar,
		      "any_signal::value-changed",
		      G_CALLBACK(ags_wave_edit_vscrollbar_value_changed),
		      (gpointer) wave_edit,
		      NULL);

  g_object_disconnect((GObject *) wave_edit->hscrollbar,
		      "any_signal::value-changed",
		      G_CALLBACK(ags_wave_edit_hscrollbar_value_changed),
		      (gpointer) wave_edit,
		      NULL);
}

AtkObject*
ags_wave_edit_get_accessible(GtkWidget *widget)
{
  AtkObject* accessible;

  accessible = g_object_get_qdata(G_OBJECT(widget),
				  quark_accessible_object);
  
  if(!accessible){
    accessible = g_object_new(ags_accessible_wave_edit_get_type(),
			      NULL);
    
    g_object_set_qdata(G_OBJECT(widget),
		       quark_accessible_object,
		       accessible);
    gtk_accessible_set_widget(GTK_ACCESSIBLE(accessible),
			      widget);
  }
  
  return(accessible);
}

gboolean
ags_accessible_wave_edit_do_action(AtkAction *action,
				   gint i)
{
  AgsWaveEdit *wave_edit;
  
  GdkEventKey *key_press, *key_release;
  GdkEventKey *modifier_press, *modifier_release;
  GdkEventKey *second_level_press, *second_level_release;
  
  if(!(i >= 0 && i < 7)){
    return(FALSE);
  }

  wave_edit = (AgsWaveEdit *) gtk_accessible_get_widget(GTK_ACCESSIBLE(action));
  
  key_press = (GdkEventKey *) gdk_event_new(GDK_KEY_PRESS);
  key_release = (GdkEventKey *) gdk_event_new(GDK_KEY_RELEASE);

  /* create modifier */
  modifier_press = (GdkEventKey *) gdk_event_new(GDK_KEY_PRESS);
  modifier_release = (GdkEventKey *) gdk_event_new(GDK_KEY_RELEASE);
  
  modifier_press->keyval =
    modifier_release->keyval = GDK_KEY_Control_R;

  /* create second level */
  second_level_press = (GdkEventKey *) gdk_event_new(GDK_KEY_PRESS);
  second_level_release = (GdkEventKey *) gdk_event_new(GDK_KEY_RELEASE);
  
  second_level_press->keyval =
    second_level_release->keyval = GDK_KEY_Shift_R;

  switch(i){
  case 0:
    {
      key_press->keyval =
	key_release->keyval = GDK_KEY_Left;
      
      /* send event */
      gtk_widget_event((GtkWidget *) wave_edit->drawing_area,
		       (GdkEvent *) key_press);
      gtk_widget_event((GtkWidget *) wave_edit->drawing_area,
		       (GdkEvent *) key_release);
    }
    break;
  case 1:
    {
      key_press->keyval =
	key_release->keyval = GDK_KEY_Right;
      
      /* send event */
      gtk_widget_event((GtkWidget *) wave_edit->drawing_area,
		       (GdkEvent *) key_press);
      gtk_widget_event((GtkWidget *) wave_edit->drawing_area,
		       (GdkEvent *) key_release);
    }
    break;
  case 2:
    {
      key_press->keyval =
	key_release->keyval = GDK_KEY_Left;
      
      /* send event */
      gtk_widget_event((GtkWidget *) wave_edit->drawing_area,
		       (GdkEvent *) second_level_press);
      gtk_widget_event((GtkWidget *) wave_edit->drawing_area,
		       (GdkEvent *) key_press);
      gtk_widget_event((GtkWidget *) wave_edit->drawing_area,
		       (GdkEvent *) key_release);
      gtk_widget_event((GtkWidget *) wave_edit->drawing_area,
		       (GdkEvent *) second_level_release);
    }
    break;
  case 3:
    {
      key_press->keyval =
	key_release->keyval = GDK_KEY_Right;
      
      /* send event */
      gtk_widget_event((GtkWidget *) wave_edit->drawing_area,
		       (GdkEvent *) second_level_press);
      gtk_widget_event((GtkWidget *) wave_edit->drawing_area,
		       (GdkEvent *) key_press);
      gtk_widget_event((GtkWidget *) wave_edit->drawing_area,
		       (GdkEvent *) key_release);
      gtk_widget_event((GtkWidget *) wave_edit->drawing_area,
		       (GdkEvent *) second_level_release);
    }
    break;
  case 4:
    {
      key_press->keyval =
	key_release->keyval = GDK_KEY_c;

      /* send event */
      gtk_widget_event((GtkWidget *) wave_edit->drawing_area,
		       (GdkEvent *) modifier_press);
      gtk_widget_event((GtkWidget *) wave_edit->drawing_area,
		       (GdkEvent *) key_press);
      gtk_widget_event((GtkWidget *) wave_edit->drawing_area,
		       (GdkEvent *) key_release);
      gtk_widget_event((GtkWidget *) wave_edit->drawing_area,
		       (GdkEvent *) modifier_release);      
    }    
    break;
  case 5:
    {
      key_press->keyval =
	key_release->keyval = GDK_KEY_x;

      /* send event */
      gtk_widget_event((GtkWidget *) wave_edit->drawing_area,
		       (GdkEvent *) modifier_press);
      gtk_widget_event((GtkWidget *) wave_edit->drawing_area,
		       (GdkEvent *) key_press);
      gtk_widget_event((GtkWidget *) wave_edit->drawing_area,
		       (GdkEvent *) key_release);
      gtk_widget_event((GtkWidget *) wave_edit->drawing_area,
		       (GdkEvent *) modifier_release);      
    }
    break;
  case 6:
    {
      key_press->keyval =
	key_release->keyval = GDK_KEY_v;

      /* send event */
      gtk_widget_event((GtkWidget *) wave_edit->drawing_area,
		       (GdkEvent *) modifier_press);
      gtk_widget_event((GtkWidget *) wave_edit->drawing_area,
		       (GdkEvent *) key_press);
      gtk_widget_event((GtkWidget *) wave_edit->drawing_area,
		       (GdkEvent *) key_release);
      gtk_widget_event((GtkWidget *) wave_edit->drawing_area,
		       (GdkEvent *) modifier_release);      
    }
    break;
  }

  return(TRUE);
}

gint
ags_accessible_wave_edit_get_n_actions(AtkAction *action)
{
  return(7);
}

const gchar*
ags_accessible_wave_edit_get_description(AtkAction *action,
					 gint i)
{
  static const gchar *actions[] = {
    "move cursor left",
    "move cursor right",
    "move cursor small left",
    "move cursor small right",
    "copy wave to clipboard",
    "cut wave to clipboard",
    "paste wave from clipboard",
  };

  if(i >= 0 && i < 7){
    return(actions[i]);
  }else{
    return(NULL);
  }
}

const gchar*
ags_accessible_wave_edit_get_name(AtkAction *action,
				  gint i)
{
  static const gchar *actions[] = {
    "left",
    "right",
    "small-left",
    "small-right",
    "copy",
    "cut",
    "paste",
  };
  
  if(i >= 0 && i < 7){
    return(actions[i]);
  }else{
    return(NULL);
  }
}

const gchar*
ags_accessible_wave_edit_get_keybinding(AtkAction *action,
					gint i)
{
  static const gchar *actions[] = {
    "left",
    "right",
    "Shft+Left",
    "Shft+Right",
    "Ctrl+c",
    "Ctrl+x",
    "Ctrl+v",
  };
  
  if(i >= 0 && i < 7){
    return(actions[i]);
  }else{
    return(NULL);
  }
}

gboolean
ags_accessible_wave_edit_set_description(AtkAction *action,
					 gint i)
{
  //TODO:JK: implement me

  return(FALSE);
}

gchar*
ags_accessible_wave_edit_get_localized_name(AtkAction *action,
					    gint i)
{
  //TODO:JK: implement me

  return(NULL);
}

void
ags_wave_edit_get_preferred_width(GtkWidget *widget,
				  gint *minimal_width,
				  gint *natural_width)
{
  minimal_width =
    natural_width = NULL;
}

void
ags_wave_edit_get_preferred_height(GtkWidget *widget,
				   gint *minimal_height,
				   gint *natural_height)
{  
  minimal_height =
    natural_height = NULL;
}

void
ags_wave_edit_size_allocate(GtkWidget *widget,
			    GtkAllocation *allocation)
{
  AgsWaveEdit *wave_edit;

  GdkWindow *window;

  AgsApplicationContext *application_context;

  GtkAllocation child_allocation;
  
  gdouble gui_scale_factor;

  wave_edit = AGS_WAVE_EDIT(widget);

  application_context = ags_application_context_get_instance();

  /* scale factor */
  gui_scale_factor = ags_ui_provider_get_gui_scale_factor(AGS_UI_PROVIDER(application_context));
  
  allocation->height = (gint) (gui_scale_factor * AGS_LEVEL_DEFAULT_LEVEL_HEIGHT);
  
  child_allocation.x = allocation->x;
  child_allocation.y = allocation->y;
  
  child_allocation.width = allocation->width;
  child_allocation.height = (gint) (gui_scale_factor * AGS_LEVEL_DEFAULT_LEVEL_HEIGHT);

  gtk_widget_size_allocate((GtkWidget *) wave_edit->drawing_area,
  			   &child_allocation);

  window = gtk_widget_get_window((GtkWidget *) wave_edit->drawing_area);
  gdk_window_move(window,
  		  allocation->x, allocation->y);
}

void
ags_wave_edit_show(GtkWidget *widget)
{
  AgsWaveEdit *wave_edit;

  GtkAllocation allocation;
  
  wave_edit = AGS_WAVE_EDIT(widget);

  /* call parent */
  GTK_WIDGET_CLASS(ags_wave_edit_parent_class)->show(widget);

  gtk_widget_get_allocation(GTK_WIDGET(wave_edit->drawing_area),
			    &allocation);
  
  gtk_widget_show((GtkWidget *) wave_edit->drawing_area);
  
  if((AGS_WAVE_EDIT_SHOW_RULER & (wave_edit->flags)) != 0){
    gtk_widget_show((GtkWidget *) wave_edit->ruler);
  }

  if((AGS_WAVE_EDIT_SHOW_VSCROLLBAR & (wave_edit->flags)) != 0){
    gtk_widget_show((GtkWidget *) wave_edit->vscrollbar);
  }

  if((AGS_WAVE_EDIT_SHOW_HSCROLLBAR & (wave_edit->flags)) != 0){
    gtk_widget_show((GtkWidget *) wave_edit->hscrollbar);
  }
}

void
ags_wave_edit_show_all(GtkWidget *widget)
{
  AgsWaveEdit *wave_edit;

  wave_edit = AGS_WAVE_EDIT(widget);

  /* call parent */
  GTK_WIDGET_CLASS(ags_wave_edit_parent_class)->show_all(widget);

  gtk_widget_show_all((GtkWidget *) wave_edit->drawing_area);

  if((AGS_WAVE_EDIT_SHOW_RULER & (wave_edit->flags)) != 0){
    gtk_widget_show((GtkWidget *) wave_edit->ruler);
  }

  if((AGS_WAVE_EDIT_SHOW_VSCROLLBAR & (wave_edit->flags)) != 0){
    gtk_widget_show((GtkWidget *) wave_edit->vscrollbar);
  }

  if((AGS_WAVE_EDIT_SHOW_HSCROLLBAR & (wave_edit->flags)) != 0){
    gtk_widget_show((GtkWidget *) wave_edit->hscrollbar);
  }
}

gboolean
ags_wave_edit_auto_scroll_timeout(GtkWidget *widget)
{
  if(g_hash_table_lookup(ags_wave_edit_auto_scroll,
			 widget) != NULL){
    AgsWaveEditor *wave_editor;
    AgsWaveEdit *wave_edit;

    GtkAdjustment *hscrollbar_adjustment;

    GObject *output_soundcard;
    
    double x;
    
    wave_edit = AGS_WAVE_EDIT(widget);

    if((AGS_WAVE_EDIT_AUTO_SCROLL & (wave_edit->flags)) == 0){
      return(TRUE);
    }
    
    wave_editor = (AgsWaveEditor *) gtk_widget_get_ancestor((GtkWidget *) wave_edit,
							    AGS_TYPE_WAVE_EDITOR);
    
    if(wave_editor->selected_machine == NULL){
      return(TRUE);
    }

    hscrollbar_adjustment = gtk_range_get_adjustment(GTK_RANGE(wave_edit->hscrollbar));
    
    /* reset offset */
    g_object_get(wave_editor->selected_machine->audio,
		 "output-soundcard", &output_soundcard,
		 NULL);
    
    wave_edit->note_offset = ags_soundcard_get_note_offset(AGS_SOUNDCARD(output_soundcard));
    wave_edit->note_offset_absolute = ags_soundcard_get_note_offset_absolute(AGS_SOUNDCARD(output_soundcard));

    /* reset scrollbar */
    x = ((wave_edit->note_offset * wave_edit->control_width) / (AGS_WAVE_EDITOR_MAX_CONTROLS * wave_edit->control_width)) * gtk_adjustment_get_upper(hscrollbar_adjustment);
    
    gtk_range_set_value(GTK_RANGE(wave_edit->hscrollbar),
			x);

    g_object_unref(output_soundcard);
    
    return(TRUE);
  }else{
    return(FALSE);
  }
}

void
ags_wave_edit_reset_vscrollbar(AgsWaveEdit *wave_edit)
{
  AgsWaveEditor *wave_editor;

  GtkAdjustment *adjustment;

  AgsApplicationContext *application_context;
  
  GtkAllocation allocation;
  
  gdouble gui_scale_factor;
  double varea_height;
  gdouble upper, old_upper;
  
  if(!AGS_IS_WAVE_EDIT(wave_edit)){
    return;
  }

  wave_editor = (AgsWaveEditor *) gtk_widget_get_ancestor((GtkWidget *) wave_edit,
							  AGS_TYPE_WAVE_EDITOR);

  application_context = ags_application_context_get_instance();

  /* scale factor */
  gui_scale_factor = ags_ui_provider_get_gui_scale_factor(AGS_UI_PROVIDER(application_context));

  if(wave_editor->selected_machine == NULL){
    return;
  }

  adjustment = gtk_range_get_adjustment(GTK_RANGE(wave_edit->vscrollbar));

  gtk_widget_get_allocation(GTK_WIDGET(wave_edit->drawing_area),
			    &allocation);
  
  /* upper */
  old_upper = gtk_adjustment_get_upper(adjustment); 

  varea_height = (wave_edit->step_count * wave_edit->control_height);
  upper = varea_height - allocation.height;

  if(upper < 0.0){
    upper = 0.0;
  }
	   
  gtk_adjustment_set_upper(adjustment,
			   upper);

  /* reset value */
  if(old_upper != 0.0){
    gtk_adjustment_set_value(adjustment,
			     gtk_adjustment_get_value(adjustment) / old_upper * upper);
  }
}

void
ags_wave_edit_reset_hscrollbar(AgsWaveEdit *wave_edit)
{
  AgsWaveEditor *wave_editor;
  AgsWaveToolbar *wave_toolbar;

  GtkAdjustment *adjustment;

  AgsApplicationContext *application_context;
  
  GtkAllocation allocation;

  gdouble gui_scale_factor;
  double zoom_factor, zoom;
  double zoom_correction;
  guint map_width;
  gdouble upper, old_upper;
  
  if(!AGS_IS_WAVE_EDIT(wave_edit)){
    return;
  }

  wave_editor = (AgsWaveEditor *) gtk_widget_get_ancestor((GtkWidget *) wave_edit,
							  AGS_TYPE_WAVE_EDITOR);

  application_context = ags_application_context_get_instance();

  /* scale factor */
  gui_scale_factor = ags_ui_provider_get_gui_scale_factor(AGS_UI_PROVIDER(application_context));

  if(wave_editor->selected_machine == NULL){
    return;
  }

  wave_toolbar = wave_editor->wave_toolbar;

  gtk_widget_get_allocation(GTK_WIDGET(wave_edit->drawing_area),
			    &allocation);
  
  /* adjustment */
  adjustment = gtk_range_get_adjustment(GTK_RANGE(wave_edit->hscrollbar));

  /* zoom */
  zoom_factor = exp2(6.0 - (double) gtk_combo_box_get_active((GtkComboBox *) wave_toolbar->zoom));
  zoom = exp2((double) gtk_combo_box_get_active((GtkComboBox *) wave_toolbar->zoom) - 2.0);

  /* upper */
  old_upper = gtk_adjustment_get_upper(adjustment);
  
  zoom_correction = 1.0 / 16;

//  map_width = ((double) AGS_WAVE_EDITOR_MAX_CONTROLS * zoom * zoom_correction);
  map_width = (gui_scale_factor * 64.0) * (16.0 * 16.0 * 1200.0) * zoom * zoom_correction;
  upper = map_width - allocation.width;

  if(upper < 0.0){    
    upper = 0.0;
  }

  gtk_adjustment_set_upper(adjustment,
			   upper);

  /* ruler */
  wave_edit->ruler->factor = zoom_factor;
  wave_edit->ruler->precision = zoom;
  wave_edit->ruler->scale_precision = 1.0 / zoom;

  gtk_adjustment_set_upper(wave_edit->ruler->adjustment,
			   upper / wave_edit->control_width);

  /* reset value */
  if(old_upper != 0.0){
#if 0
    gtk_adjustment_set_value(adjustment,
			     gtk_adjustment_get_value(adjustment) / old_upper * upper);
#endif
  }
}

void
ags_wave_edit_draw_segment(AgsWaveEdit *wave_edit, cairo_t *cr)
{
  AgsWaveEditor *wave_editor;
  AgsWaveToolbar *wave_toolbar;
  
  GtkStyleContext *wave_edit_style_context;

  GtkAdjustment *hscrollbar_adjustment;
  
  AgsApplicationContext *application_context;

  GtkAllocation allocation;
  
  GdkRGBA *fg_color;
  GdkRGBA *bg_color;
  GdkRGBA *border_color;

  gdouble gui_scale_factor;
  gdouble x_offset, y_offset;
  gdouble translated_ground;
  double tact;
  gdouble y;
  gdouble map_height;
  gdouble width, height;
  guint control_width;
  guint i, j;
  guint j_set;

  GValue value = {0,};

  const static double quarter_dashes = {
    0.25,
  };

  const static double segment_dashes = {
    0.5,
  };
  
  if(!AGS_IS_WAVE_EDIT(wave_edit)){
    return;
  }

  application_context = ags_application_context_get_instance();

  wave_editor = (AgsWaveEditor *) gtk_widget_get_ancestor((GtkWidget *) wave_edit,
							  AGS_TYPE_WAVE_EDITOR);

  if(wave_editor->selected_machine == NULL){
    return;
  }

  wave_toolbar = wave_editor->wave_toolbar;

  gtk_widget_get_allocation(GTK_WIDGET(wave_edit->drawing_area),
			    &allocation);
  
  /* scale factor */
  gui_scale_factor = ags_ui_provider_get_gui_scale_factor(AGS_UI_PROVIDER(application_context));
  
  /* dimension and offset */
  x_offset = gtk_range_get_value(GTK_RANGE(wave_edit->hscrollbar));
  y_offset = gtk_range_get_value(GTK_RANGE(wave_edit->vscrollbar));

  y = (gdouble) 0.0;

  width = (gdouble) allocation.width;
  height = (gdouble) allocation.height;

  /* style context */
  wave_edit_style_context = gtk_widget_get_style_context(GTK_WIDGET(wave_edit->drawing_area));

  gtk_style_context_get_property(wave_edit_style_context,
				 "color",
				 GTK_STATE_FLAG_NORMAL,
				 &value);

  fg_color = g_value_dup_boxed(&value);
  g_value_unset(&value);

  gtk_style_context_get_property(wave_edit_style_context,
				 "background-color",
				 GTK_STATE_FLAG_NORMAL,
				 &value);

  bg_color = g_value_dup_boxed(&value);
  g_value_unset(&value);

  gtk_style_context_get_property(wave_edit_style_context,
				 "border-color",
				 GTK_STATE_FLAG_NORMAL,
				 &value);

  border_color = g_value_dup_boxed(&value);
  g_value_unset(&value);

  /* push group */
  cairo_push_group(cr);

  /* background */
  cairo_set_source_rgba(cr,
			bg_color->red,
			bg_color->green,
			bg_color->blue,
			bg_color->alpha);
  cairo_rectangle(cr, 0.0, y, width, height);
  cairo_fill(cr);

  /* background border */
  cairo_set_source_rgba(cr,
			border_color->red,
			border_color->green,
			border_color->blue,
			border_color->alpha);
  cairo_set_line_width(cr, 1.0);
  cairo_rectangle(cr, 0.0, y, width, height);
  cairo_stroke(cr);

  cairo_set_line_width(cr, 1.0);

  tact = exp2((double) gtk_combo_box_get_active(wave_toolbar->zoom) - 2.0);
  
  map_height = (gdouble) height;

  control_width = (gint) (gui_scale_factor * (gdouble) AGS_WAVE_EDIT_DEFAULT_CONTROL_WIDTH);
  i = control_width - (guint) x_offset % control_width;
  
  cairo_set_source_rgba(cr,
			fg_color->red,
			fg_color->blue,
			fg_color->green,
			fg_color->alpha);

  if(i < width &&
     tact > 1.0 ){
    j_set = ((guint) x_offset / control_width + 1) % ((guint) tact);

    cairo_set_dash(cr,
		   &segment_dashes,
		   1,
		   0.0);

    if(j_set != 0){
      j = j_set;
      goto ags_wave_edit_draw_segment0;
    }
  }

  for(; i < width; ){
    cairo_set_dash(cr,
		   NULL,
		   0,
		   0.0);
    
    cairo_move_to(cr, (double) i, y);
    cairo_line_to(cr, (double) i, y + height);
    cairo_stroke(cr);
    
    i += control_width;
    
    cairo_set_dash(cr,
		   &segment_dashes,
		   1,
		   0.0);
    
    for(j = 1; i < width && j < tact; j++){
    ags_wave_edit_draw_segment0:
      cairo_move_to(cr, (double) i, y);
      cairo_line_to(cr, (double) i, y + height);
      cairo_stroke(cr);
      
      i += control_width;
    }
  }

  cairo_set_source_rgba(cr,
			bg_color->red,
			bg_color->green,
			bg_color->blue,
			bg_color->alpha);

  /* middle */
  if(map_height * 0.5 < height){
    cairo_move_to(cr,
		  0.0, y + map_height * 0.5);
    cairo_line_to(cr,
		  width, y + map_height * 0.5);
    cairo_stroke(cr);
  }
  
  /* set dash */
  cairo_set_dash(cr,
		 &quarter_dashes,
		 1,
		 0.0);

  /* lower quarter */
  if(map_height * 0.25 < height){
    cairo_move_to(cr,
		  0.0, y + map_height * 0.25);
    cairo_line_to(cr,
		  width, y + map_height * 0.25);
    cairo_stroke(cr);
  }
  
  /* upper quarter */
  if(map_height * 0.75 < height){
    cairo_move_to(cr,
		  0.0, y + map_height * 0.75);
    cairo_line_to(cr,
		  width, y + map_height * 0.75);
    cairo_stroke(cr);
  }
  
  /* complete */
  cairo_pop_group_to_source(cr);
  cairo_paint(cr);
      
  cairo_surface_mark_dirty(cairo_get_target(cr));

  g_boxed_free(GDK_TYPE_RGBA, fg_color);
  g_boxed_free(GDK_TYPE_RGBA, bg_color);
  g_boxed_free(GDK_TYPE_RGBA, border_color);
}

void
ags_wave_edit_draw_position(AgsWaveEdit *wave_edit, cairo_t *cr)
{
  AgsWaveEditor *wave_editor;
  AgsWaveToolbar *wave_toolbar;

  GtkStyleContext *wave_edit_style_context;

  AgsApplicationContext *application_context;

  GdkRGBA *fg_color_active;

  gdouble gui_scale_factor;
  gdouble tact;
  guint control_width;
  double position;
  double x, y;
  double width, height;
  gboolean height_fits;

  GValue value = {0,};

  if(!AGS_IS_WAVE_EDIT(wave_edit)){
    return;
  }

  application_context = ags_application_context_get_instance();

  wave_editor = (AgsWaveEditor *) gtk_widget_get_ancestor((GtkWidget *) wave_edit,
							  AGS_TYPE_WAVE_EDITOR);

  if(wave_editor->selected_machine == NULL){
    return;
  }

  wave_toolbar = wave_editor->wave_toolbar;

  /* scale factor */
  gui_scale_factor = ags_ui_provider_get_gui_scale_factor(AGS_UI_PROVIDER(application_context));

  /* style context */
  wave_edit_style_context = gtk_widget_get_style_context(GTK_WIDGET(wave_edit->drawing_area));

  gtk_style_context_get_property(wave_edit_style_context,
				 "color",
				 GTK_STATE_FLAG_ACTIVE,
				 &value);

  fg_color_active = g_value_dup_boxed(&value);
  g_value_unset(&value);

  tact = exp2((double) gtk_combo_box_get_active(wave_toolbar->zoom) - 2.0);

  /* get offset and dimensions */
  control_width = (gint) (gui_scale_factor * (gdouble) AGS_WAVE_EDIT_DEFAULT_CONTROL_WIDTH);

  position = ((double) wave_edit->note_offset) * ((double) control_width);
  
  y = 0.0;
  x = (position) - (gtk_range_get_value(GTK_RANGE(wave_edit->hscrollbar)));

  width = (double) ((guint) (gui_scale_factor * AGS_WAVE_EDIT_DEFAULT_FADER_WIDTH));
  height = (double) ((guint) (gui_scale_factor * AGS_WAVE_EDIT_DEFAULT_HEIGHT));

  /* push group */
  cairo_push_group(cr);
  
  /* draw fader */
  cairo_set_source_rgba(cr,
			fg_color_active->red,
			fg_color_active->blue,
			fg_color_active->green,
			fg_color_active->alpha);

  cairo_rectangle(cr,
		  (double) x, (double) y,
		  (double) width, (double) height);
  cairo_fill(cr);

  /* complete */
  cairo_pop_group_to_source(cr);
  cairo_paint(cr);
      
//  cairo_surface_mark_dirty(cairo_get_target(cr));

  g_boxed_free(GDK_TYPE_RGBA, fg_color_active);
}

void
ags_wave_edit_draw_cursor(AgsWaveEdit *wave_edit, cairo_t *cr)
{
  AgsWaveEditor *wave_editor;
  AgsWaveToolbar *wave_toolbar;

  GtkStyleContext *wave_edit_style_context;

  AgsApplicationContext *application_context;

  GtkAllocation allocation;

  GdkRGBA *fg_color_focused;

  gdouble gui_scale_factor;
  double zoom, zoom_factor;
  double x, y;
  double width, height;
  gboolean height_fits;

  GValue value = {0,};

  if(!AGS_IS_WAVE_EDIT(wave_edit)){
    return;
  }

  application_context = ags_application_context_get_instance();

  wave_editor = (AgsWaveEditor *) gtk_widget_get_ancestor((GtkWidget *) wave_edit,
							  AGS_TYPE_WAVE_EDITOR);

  if(wave_editor->selected_machine == NULL){
    return;
  }

  wave_toolbar = wave_editor->wave_toolbar;

  /* scale factor */
  gui_scale_factor = ags_ui_provider_get_gui_scale_factor(AGS_UI_PROVIDER(application_context));

  /* style context */
  wave_edit_style_context = gtk_widget_get_style_context(GTK_WIDGET(wave_edit->drawing_area));

  gtk_style_context_get_property(wave_edit_style_context,
				 "color",
				 GTK_STATE_FLAG_FOCUSED,
				 &value);

  fg_color_focused = g_value_dup_boxed(&value);
  g_value_unset(&value);

  gtk_widget_get_allocation(GTK_WIDGET(wave_edit->drawing_area),
			    &allocation);
  
  /* zoom */
  zoom = exp2((double) gtk_combo_box_get_active((GtkComboBox *) wave_toolbar->zoom) - 2.0);
  zoom_factor = exp2(6.0 - (double) gtk_combo_box_get_active((GtkComboBox *) wave_toolbar->zoom));
  
  y = 0.0;
  x = (((double) wave_edit->cursor_position_x) - (gtk_range_get_value(GTK_RANGE(wave_edit->hscrollbar)))) /  zoom_factor;

  width = (double) ((guint) (gui_scale_factor * AGS_WAVE_EDIT_DEFAULT_FADER_WIDTH));
  height = (double) ((guint) (gui_scale_factor * AGS_WAVE_EDIT_DEFAULT_HEIGHT));

  /* push group */
  cairo_push_group(cr);
  
  /* draw cursor */
  cairo_set_source_rgba(cr,
			fg_color_focused->red,
			fg_color_focused->blue,
			fg_color_focused->green,
			fg_color_focused->alpha);

  cairo_rectangle(cr,
		  (double) x, (double) y,
		  (double) width, (double) height);
  cairo_fill(cr);

  /* complete */
  cairo_pop_group_to_source(cr);
  cairo_paint(cr);
      
  cairo_surface_mark_dirty(cairo_get_target(cr));

  g_boxed_free(GDK_TYPE_RGBA, fg_color_focused);
}

void
ags_wave_edit_draw_selection(AgsWaveEdit *wave_edit, cairo_t *cr)
{
  AgsWaveEditor *wave_editor;
  AgsWaveToolbar *wave_toolbar;

  GtkStyleContext *wave_edit_style_context;

  AgsApplicationContext *application_context;

  GtkAllocation allocation;
  
  GdkRGBA *fg_color_prelight;
  
  double zoom, zoom_factor;
  double x, y;
  double width, height;

  GValue value = {0,};

  if(!AGS_IS_WAVE_EDIT(wave_edit)){
    return;
  }

  wave_editor = (AgsWaveEditor *) gtk_widget_get_ancestor((GtkWidget *) wave_edit,
							  AGS_TYPE_WAVE_EDITOR);
  
  wave_toolbar = wave_editor->wave_toolbar;

  application_context = ags_application_context_get_instance();

  /* style context */
  wave_edit_style_context = gtk_widget_get_style_context(GTK_WIDGET(wave_edit->drawing_area));

  gtk_style_context_get_property(wave_edit_style_context,
				 "color",
				 GTK_STATE_FLAG_PRELIGHT,
				 &value);

  fg_color_prelight = g_value_dup_boxed(&value);
  g_value_unset(&value);

  gtk_widget_get_allocation(GTK_WIDGET(wave_edit->drawing_area),
			    &allocation);

  /* zoom */
  zoom = exp2((double) gtk_combo_box_get_active((GtkComboBox *) wave_toolbar->zoom) - 2.0);
  zoom_factor = exp2(6.0 - (double) gtk_combo_box_get_active((GtkComboBox *) wave_toolbar->zoom));

  /* get offset and dimensions */
  if(wave_edit->selection_x0 < wave_edit->selection_x1){
    x = (((double) wave_edit->selection_x0) - gtk_range_get_value(GTK_RANGE(wave_edit->hscrollbar))) / zoom_factor;
    width = ((double) wave_edit->selection_x1 - (double) wave_edit->selection_x0) / zoom_factor;
  }else{
    x = (((double) wave_edit->selection_x1) - gtk_range_get_value(GTK_RANGE(wave_edit->hscrollbar))) / zoom_factor;
    width = ((double) wave_edit->selection_x0 - (double) wave_edit->selection_x1) / zoom_factor;
  }

  if(wave_edit->selection_y0 < wave_edit->selection_y1){
    y = ((double) wave_edit->selection_y0) - gtk_range_get_value(GTK_RANGE(wave_edit->vscrollbar));
    height = ((double) wave_edit->selection_y1 - (double) wave_edit->selection_y0);
  }else{
    y = ((double) wave_edit->selection_y1) - gtk_range_get_value(GTK_RANGE(wave_edit->vscrollbar));
    height = ((double) wave_edit->selection_y0 - (double) wave_edit->selection_y1);
  }

  /* clip */
  if(x < 0.0){
    width += x;

    x = 0.0;
  }else if(x > allocation.width){
    g_boxed_free(GDK_TYPE_RGBA, fg_color_prelight);
    
    return;
  }

  if(x + width > allocation.width){
    width = ((double) allocation.width) - x;
  }

  if(y < 0.0){
    height += y;

    y = 0.0;
  }else if(y > allocation.height){
    g_boxed_free(GDK_TYPE_RGBA, fg_color_prelight);

    return;
  }

  if(y + height > allocation.height){
    height = ((double) allocation.height) - y;
  }
    
  /* push group */
  cairo_push_group(cr);

  /* draw selection */
  cairo_set_source_rgba(cr,
			fg_color_prelight->red,
			fg_color_prelight->blue,
			fg_color_prelight->green,
			1.0 / 3.0);

  cairo_rectangle(cr,
		  x, y,
		  width, height);
  cairo_fill(cr);

  /* complete */
  cairo_pop_group_to_source(cr);
  cairo_paint(cr);
      
  //cairo_surface_mark_dirty(cairo_get_target(cr));
  
  g_boxed_free(GDK_TYPE_RGBA, fg_color_prelight);
}

void
ags_wave_edit_draw_buffer(AgsWaveEdit *wave_edit,
			  AgsBuffer *buffer,
			  cairo_t *cr,
			  gdouble bpm,
			  gdouble opacity)
{
  AgsWaveEditor *wave_editor;
  AgsWaveToolbar *wave_toolbar;

  GtkStyleContext *wave_edit_style_context;

  AgsApplicationContext *application_context;

  GtkAllocation allocation;

  GdkRGBA *fg_color;
  GdkRGBA *fg_color_selected;

  GObject *soundcard;
  
  cairo_t *i_cr;
  cairo_surface_t *surface;

  unsigned char *image_data, *bg_data;
  
  gdouble gui_scale_factor;
  gdouble tact;
  guint samplerate;
  guint buffer_size;
  guint format;
  guint x0, x1;
  guint x_cut;
  guint64 x;
  gdouble width, height;
  double zoom, zoom_factor;
  gdouble delay_factor;
  guint wave_data_width;
  guint i;

  GValue value = {0};

  GRecMutex *buffer_mutex;

  if(!AGS_IS_WAVE_EDIT(wave_edit) ||
     !AGS_IS_BUFFER(buffer)){
    return;
  }

  wave_editor = (AgsWaveEditor *) gtk_widget_get_ancestor((GtkWidget *) wave_edit,
							  AGS_TYPE_WAVE_EDITOR);

  if(wave_editor->selected_machine == NULL){
    return;
  }

  application_context = ags_application_context_get_instance();
  
  wave_toolbar = wave_editor->wave_toolbar;

  /* scale factor */
  gui_scale_factor = ags_ui_provider_get_gui_scale_factor(AGS_UI_PROVIDER(application_context));

  /* style context */
  wave_edit_style_context = gtk_widget_get_style_context(GTK_WIDGET(wave_edit->drawing_area));

  gtk_style_context_get_property(wave_edit_style_context,
				 "color",
				 GTK_STATE_FLAG_NORMAL,
				 &value);

  fg_color = g_value_dup_boxed(&value);
  g_value_unset(&value);

  gtk_style_context_get_property(wave_edit_style_context,
				 "color",
				 GTK_STATE_FLAG_SELECTED,
				 &value);

  fg_color_selected = g_value_dup_boxed(&value);
  g_value_unset(&value);
  
  gtk_widget_get_allocation(GTK_WIDGET(wave_edit->drawing_area),
			    &allocation);

  buffer_mutex = AGS_BUFFER_GET_OBJ_MUTEX(buffer);
  
  g_object_get(wave_editor->selected_machine->audio,
	       "output-soundcard", &soundcard,
	       NULL);

  g_object_unref(soundcard);
  
  /* zoom */
  zoom = exp2((double) gtk_combo_box_get_active((GtkComboBox *) wave_toolbar->zoom) - 2.0);
  zoom_factor = exp2(6.0 - (double) gtk_combo_box_get_active((GtkComboBox *) wave_toolbar->zoom));

  delay_factor = ags_soundcard_get_delay_factor(AGS_SOUNDCARD(soundcard));

  tact = exp2((double) gtk_combo_box_get_active(wave_toolbar->zoom) - 2.0);
  
  /* get visisble region */
  x0 = gtk_range_get_value(GTK_RANGE(wave_edit->hscrollbar));
  x1 = (gtk_range_get_value(GTK_RANGE(wave_edit->hscrollbar)) + allocation.width * zoom_factor);

  /* width and height */
  width = (gdouble) allocation.width;
  height = (gdouble) allocation.height;
  
  /* draw point */
  g_object_get(buffer,
	       "samplerate", &samplerate,
	       "buffer-size", &buffer_size,
	       "format", &format,
	       "x", &x,
	       NULL);

  x_cut = x0;

  if(((((double) (x) / samplerate * (bpm / 60.0) / delay_factor) * (gui_scale_factor * 64.0))) / zoom_factor - x_cut < 0.0 ||
     ((((double) (x) / samplerate * (bpm / 60.0) / delay_factor) * (gui_scale_factor * 64.0))) / zoom_factor - x_cut > allocation.width){
    g_boxed_free(GDK_TYPE_RGBA, fg_color);
    g_boxed_free(GDK_TYPE_RGBA, fg_color_selected);

    return;
  }
  
  wave_data_width = buffer_size;

  /*
  if(wave_edit->stride != cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, wave_data_width)){
    if(wave_edit->wave_data != NULL){
      free(wave_edit->wave_data);
    }

    wave_edit->stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, wave_data_width);
    wave_edit->wave_data = malloc(wave_edit->stride * AGS_WAVE_EDIT_DEFAULT_HEIGHT);
  }
  
  surface = cairo_image_surface_create_for_data(wave_edit->wave_data,
						CAIRO_FORMAT_ARGB32,
						wave_data_width,
						AGS_WAVE_EDIT_DEFAULT_HEIGHT,
						wave_edit->stride);

  i_cr = cairo_create(surface);
  */

  /* draw buffer */
  cairo_set_source_rgba(cr,
			fg_color->red,
			fg_color->blue,
			fg_color->green,
			opacity * fg_color->alpha);
  
  cairo_set_line_width(cr, 1.0);

  //  cairo_scale(cr,
  //	      1.0 / (zoom_factor * (((60.0 / bpm) * ((double) buffer_size / (double) samplerate)) * AGS_WAVE_EDIT_X_RESOLUTION)), 1.0);
  
  for(i = 0; i < buffer_size; i += (zoom_factor * 16)){
    double y0, y1;

    y0 = 0.0;
    y1 = 0.0;

    g_rec_mutex_lock(buffer_mutex);

    switch(format){
    case AGS_SOUNDCARD_SIGNED_8_BIT:
    {
      y1 = (double) ((gint8 *) buffer->data)[i] / exp2(7.0);
    }
    break;
    case AGS_SOUNDCARD_SIGNED_16_BIT:
    {
      y1 = (double) ((gint16 *) buffer->data)[i] / exp2(15.0);
    }
    break;
    case AGS_SOUNDCARD_SIGNED_24_BIT:
    {
      y1 = (double) ((gint32 *) buffer->data)[i] / exp2(23.0);
    }
    break;
    case AGS_SOUNDCARD_SIGNED_32_BIT:
    {
      y1 = (double) ((gint32 *) buffer->data)[i] / exp2(31.0);
    }
    break;
    case AGS_SOUNDCARD_SIGNED_64_BIT:
    {
      y1 = (double) ((gint64 *) buffer->data)[i] / exp2(63.0);
    }
    break;
    case AGS_SOUNDCARD_FLOAT:
    {
      y1 = (double) ((gfloat *) buffer->data)[i];
    }
    break;
    case AGS_SOUNDCARD_DOUBLE:
    {
      y1 = (double) ((gdouble *) buffer->data)[i];
    }
    break;
    }

    g_rec_mutex_unlock(buffer_mutex);

    y0 = 0.5 * height;
    y1 = (((y1 + 1.0) * height) / 2.0);
    
    cairo_move_to(cr,
		  ((((double) (x + i) / samplerate * (bpm / 60.0) / delay_factor) * (gui_scale_factor * 64.0))) / zoom_factor - x_cut, y0);
    cairo_line_to(cr,
		  ((((double) (x + i) / samplerate * (bpm / 60.0) / delay_factor) * (gui_scale_factor * 64.0))) / zoom_factor - x_cut, y1);
    cairo_stroke(cr);
  }  
  
  /* check buffer selected */
  if(ags_buffer_test_flags(buffer, AGS_BUFFER_IS_SELECTED)){
    /* draw selected buffer */
    cairo_set_source_rgba(cr,
			  fg_color_selected->red,
			  fg_color_selected->blue,
			  fg_color_selected->green,
			  opacity / 3.0);

    cairo_set_line_width(cr, 1.0 + (double) wave_edit->selected_buffer_border);

    for(i = 0; i < buffer_size; i += (zoom_factor * 16)){
      double y0, y1;

      y0 = 0.0;
      y1 = 0.0;

      g_rec_mutex_lock(buffer_mutex);

      switch(format){
      case AGS_SOUNDCARD_SIGNED_8_BIT:
      {
	y1 = (double) ((gint8 *) buffer->data)[i] / exp2(7.0);
      }
      break;
      case AGS_SOUNDCARD_SIGNED_16_BIT:
      {
	y1 = (double) ((gint16 *) buffer->data)[i] / exp2(15.0);
      }
      break;
      case AGS_SOUNDCARD_SIGNED_24_BIT:
      {
	y1 = (double) ((gint32 *) buffer->data)[i] / exp2(23.0);
      }
      break;
      case AGS_SOUNDCARD_SIGNED_32_BIT:
      {
	y1 = (double) ((gint32 *) buffer->data)[i] / exp2(31.0);
      }
      break;
      case AGS_SOUNDCARD_SIGNED_64_BIT:
      {
	y1 = (double) ((gint64 *) buffer->data)[i] / exp2(63.0);
      }
      break;
      case AGS_SOUNDCARD_FLOAT:
      {
	y1 = (double) ((gfloat *) buffer->data)[i];
      }
      break;
      case AGS_SOUNDCARD_DOUBLE:
      {
	y1 = (double) ((gdouble *) buffer->data)[i];
      }
      break;
      }

      g_rec_mutex_unlock(buffer_mutex);

      y0 = 0.5 * height;
      y1 = (((y1 + 1.0) * height) / 2.0);
    
      cairo_move_to(cr,
		    ((((double) (x + i) / samplerate * (bpm / 60.0) / delay_factor) * (gui_scale_factor * 64.0))) / zoom_factor - x_cut, y0);
      cairo_line_to(cr,
		    ((((double) (x + i) / samplerate * (bpm / 60.0) / delay_factor) * (gui_scale_factor * 64.0))) / zoom_factor - x_cut, y1);
      cairo_stroke(cr);
    }  
  }
  

  /* draw buffer */
  //  cairo_set_source_surface(cr, surface,
  //			   (bpm / (60.0 * (x / samplerate))) * AGS_WAVE_EDIT_X_RESOLUTION, 0.0);

  g_boxed_free(GDK_TYPE_RGBA, fg_color);
  g_boxed_free(GDK_TYPE_RGBA, fg_color_selected);
}

void
ags_wave_edit_draw_wave(AgsWaveEdit *wave_edit, cairo_t *cr)
{
  AgsWindow *window;
  AgsWaveWindow *wave_window;
  AgsWaveEditor *wave_editor;
  AgsWaveToolbar *wave_toolbar;

  AgsTimestamp *current_timestamp;    

  AgsApplicationContext *application_context;

  GObject *soundcard;
  
  GtkAllocation allocation;

  GList *start_list_wave, *list_wave;
  GList *start_list_buffer, *list_buffer;

  gdouble gui_scale_factor;
  double zoom, zoom_factor;
  gdouble delay_factor;
  gdouble opacity;
  guint line;
  guint samplerate;
  gdouble bpm;
  guint x0, x1;
  guint x_cut;
  
  if(!AGS_IS_WAVE_EDIT(wave_edit)){
    return;
  }

  wave_editor = (AgsWaveEditor *) gtk_widget_get_ancestor((GtkWidget *) wave_edit,
							  AGS_TYPE_WAVE_EDITOR);

  if(wave_editor->selected_machine == NULL){
    return;
  }

  application_context = ags_application_context_get_instance();
  
  wave_window = (AgsWaveWindow *) gtk_widget_get_ancestor((GtkWidget *) wave_editor,
							  AGS_TYPE_WAVE_WINDOW);
  window = (AgsWindow *) wave_window->parent_window;  

  wave_toolbar = wave_editor->wave_toolbar;

  /* scale factor */
  gui_scale_factor = ags_ui_provider_get_gui_scale_factor(AGS_UI_PROVIDER(application_context));
  
  gtk_widget_get_allocation(GTK_WIDGET(wave_edit->drawing_area),
			    &allocation);

  g_object_get(wave_editor->selected_machine->audio,
	       "output-soundcard", &soundcard,
	       NULL);

  g_object_unref(soundcard);

  bpm = gtk_spin_button_get_value(window->navigation->bpm);
  
  /* zoom */
  zoom = exp2((double) gtk_combo_box_get_active((GtkComboBox *) wave_toolbar->zoom) - 2.0);
  zoom_factor = exp2(6.0 - (double) gtk_combo_box_get_active((GtkComboBox *) wave_toolbar->zoom));

  delay_factor = ags_soundcard_get_delay_factor(AGS_SOUNDCARD(soundcard));
  
  /* get visisble region */
  x0 = (guint) gtk_range_get_value(GTK_RANGE(wave_edit->hscrollbar));
  x1 = ((guint) gtk_range_get_value(GTK_RANGE(wave_edit->hscrollbar)) + allocation.width * zoom_factor);

  x_cut = x0;
  
  /* draw wave */
  g_object_get(wave_editor->selected_machine->audio,
	       "wave", &start_list_wave,
	       NULL);

  list_wave = start_list_wave;
  line = wave_edit->line;

  opacity = gtk_spin_button_get_value(wave_editor->wave_toolbar->opacity);
  
  while(list_wave != NULL){
    AgsWave *wave;

    GList *start_list_buffer, *list_buffer;

    guint current_line;
    guint64 offset;
    
    wave = AGS_WAVE(list_wave->data);

    g_object_get(wave,
		 "timestamp", &current_timestamp,
		 "line", &current_line,
		 "samplerate", &samplerate,
		 NULL);

    g_object_unref(current_timestamp);
    
    if(current_line != line ||
       current_timestamp == NULL){
      list_wave = list_wave->next;

      continue;
    }
    
    offset = ags_timestamp_get_ags_offset(current_timestamp);
    
    if(((((double) (offset) / samplerate * (bpm / 60.0) / delay_factor) * (gui_scale_factor * 64.0))) / zoom_factor - x_cut > allocation.width){
      break;
    }

    g_object_get(wave,
		 "buffer", &start_list_buffer,
		 NULL);
      
    list_buffer = start_list_buffer;

    while(list_buffer != NULL){
      ags_wave_edit_draw_buffer(wave_edit,
				list_buffer->data,
				cr,
				bpm,
				opacity);

      /* iterate */
      list_buffer = list_buffer->next;
    }

    g_list_free_full(start_list_buffer,
		     g_object_unref);
      
    /* iterate */
    list_wave = list_wave->next;
  }
  
  //TODO:JK: implement me

  g_list_free_full(start_list_wave,
		   g_object_unref);
}

void
ags_wave_edit_draw(AgsWaveEdit *wave_edit, cairo_t *cr)
{
  AgsWaveEditor *wave_editor;

  wave_editor = (AgsWaveEditor *) gtk_widget_get_ancestor((GtkWidget *) wave_edit,
							  AGS_TYPE_WAVE_EDITOR);
  
  ags_wave_editor_reset_scrollbar(wave_editor);
  
  ags_wave_edit_reset_vscrollbar(wave_edit);
  ags_wave_edit_reset_hscrollbar(wave_edit);

  /* segment */
  ags_wave_edit_draw_segment(wave_edit, cr);

  /* wave */
  ags_wave_edit_draw_wave(wave_edit, cr);
  
  /* edit mode */
  switch(wave_edit->mode){
  case AGS_WAVE_EDIT_POSITION_CURSOR:
    {
      ags_wave_edit_draw_cursor(wave_edit, cr);
    }
    break;
  case AGS_WAVE_EDIT_SELECT_BUFFER:
    {
      ags_wave_edit_draw_selection(wave_edit, cr);
    }
    break;
  }

  /* fader */
  if((AGS_WAVE_EDIT_AUTO_SCROLL & (wave_edit->flags)) != 0){
    ags_wave_edit_draw_position(wave_edit, cr);
  }
}


/**
 * ags_wave_edit_new:
 * @line: the line
 *
 * Create a new instance of #AgsWaveEdit.
 *
 * Returns: the new #AgsWaveEdit
 *
 * Since: 3.0.0
 */
AgsWaveEdit*
ags_wave_edit_new(guint line)
{
  AgsWaveEdit *wave_edit;

  wave_edit = (AgsWaveEdit *) g_object_new(AGS_TYPE_WAVE_EDIT,
					   "line", line,
					   NULL);

  return(wave_edit);
}
