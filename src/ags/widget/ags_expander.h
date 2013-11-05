/* AGS - Advanced GTK Sequencer
 * Copyright (C) 2013 Joël Krähemann
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

#ifndef __AGS_EXPANDER_H__
#define __AGS_EXPANDER_H__

#include <glib.h>
#include <glib-object.h>

#include <gtk/gtk.h>

#define AGS_TYPE_EXPANDER                (ags_expander_get_type())
#define AGS_EXPANDER(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_EXPANDER, AgsExpander))
#define AGS_EXPANDER_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST((class), AGS_TYPE_EXPANDER, AgsExpanderClass))
#define AGS_IS_EXPANDER(obj)             (G_TYPE_CHECK_INSTANCE_TYPE((obj), AGS_TYPE_EXPANDER))
#define AGS_IS_EXPANDER_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE((class), AGS_TYPE_EXPANDER))
#define AGS_EXPANDER_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS((obj), AGS_TYPE_EXPANDER, AgsExpanderClass))

#define AGS_EXPANDER_CHILD(ptr) ((AgsExpanderChild *)(ptr))

typedef struct _AgsExpander AgsExpander;
typedef struct _AgsExpanderClass AgsExpanderClass;
typedef struct _AgsExpanderChild AgsExpanderChild;

struct _AgsExpander
{
  GtkExpander expander;

  GtkTable *table;

  GList *children;
};

struct _AgsExpanderClass
{
  GtkExpanderClass expander;
};

struct _AgsExpanderChild
{
  GtkWidget *child;

  guint x;
  guint y;

  guint width;
  guint height;
};

GType ags_expander_get_type(void);

AgsExpanderChild* ags_expander_child_alloc(guint x, guint y,
					   guint width, guint height);
AgsExpanderChild* ags_expander_child_find(AgsExpander *expander,
					  GtkWidget *child);

void ags_expander_add(AgsExpander *expander,
		      GtkWidget *widget,
		      guint x, guint y,
		      guint width, guint height);
void ags_expander_remove(AgsExpander *expander,
			 GtkWidget *widget);

AgsExpander* ags_expander_new(guint width, guint height);

#endif /*__AGS_EXPANDER_H__*/
