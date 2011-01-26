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

#ifndef __AGS_EDITOR_H__
#define __AGS_EDITOR_H__

#include <glib.h>
#include <glib-object.h>

#include <gtk/gtk.h>

#include <ags/audio/ags_audio.h>

#include <ags/X/editor/ags_toolbar.h>
#include <ags/X/editor/ags_notebook.h>
#include <ags/X/editor/ags_meter.h>

#include <ags/X/ags_machine.h>

#define AGS_TYPE_EDITOR                (ags_editor_get_type ())
#define AGS_EDITOR(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_EDITOR, AgsEditor))
#define AGS_EDITOR_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST((class), AGS_TYPE_EDITOR, AgsEditorClass))
#define AGS_IS_EDITOR(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AGS_TYPE_EDITOR))
#define AGS_IS_EDITOR_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE ((class), AGS_TYPE_EDITOR))
#define AGS_EDITOR_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), AGS_TYPE_EDITOR, AgsEditorClass))

#define AGS_EDITOR_DEFAULT "default\0"

typedef struct _AgsEditor AgsEditor;
typedef struct _AgsEditorClass AgsEditorClass;

typedef enum{
  AGS_EDITOR_RESET_VSCROLLBAR,
  AGS_EDITOR_RESET_HSCROLLBAR,
}AgsEditorFlags;

struct _AgsEditor
{
  GtkVBox vbox;

  guint flags;

  GtkVBox *index_radio;
  GtkRadioButton *selected;

  AgsToolbar *toolbar;

  AgsNotebook *notebook;

  AgsMeter *meter;
  GtkDrawingArea *drawing_area;

  GtkMenu *popup;

  struct _AgsEditorControl{ // values retrieved by mouse pressed and released callback
    AgsNote note;

    guint x0;
    guint y0;
    guint x1;
    guint y1;
  }control;

  guint width;
  guint height;
  guint map_width;
  guint map_height;

  guint control_height;
  guint control_margin_y;

  guint y0;
  guint y1;

  guint nth_y;
  guint stop_y;

  struct _AgsEditorControlCurrent{ // values for drawing refering to current tic and zoom
    guint control_count;

    guint control_width;

    guint x0;
    guint x1;

    guint nth_x;
  }control_current;

  struct _AgsEditorControlUnit{ // values for drawing refering to smallest tic and current zoom
    guint control_count;

    guint control_width;

    guint x0;
    guint x1;

    guint nth_x;
    guint stop_x;
  }control_unit;

  GtkVScrollbar *vscrollbar;
  GtkHScrollbar *hscrollbar;
};

struct _AgsEditorClass
{
  GtkVBoxClass vbox;

  void (*change_machine)(AgsEditor *editor, AgsMachine *machine);
};

GType ags_editor_get_type(void);

void ags_editor_connect(AgsEditor *editor);

void ags_editor_change_machine(AgsEditor *editor, AgsMachine *machine);

void ags_editor_draw_segment(AgsEditor *editor);
void ags_editor_draw_notation(AgsEditor *editor);

void ags_editor_reset(AgsEditor *editor);

AgsEditor* ags_editor_new();

#endif /*__AGS_EDITOR_H__*/
