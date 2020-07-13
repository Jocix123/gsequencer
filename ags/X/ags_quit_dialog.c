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

#include <ags/X/ags_quit_dialog.h>
#include <ags/X/ags_quit_dialog_callbacks.h>

#include <ags/X/ags_dialog.h>
#include <ags/X/ags_navigation.h>
#include <ags/X/ags_quit_soundcard.h>

#include <stdlib.h>
#include <ags/i18n.h>

void ags_quit_dialog_class_init(AgsQuitDialogClass *quit_dialog);
void ags_quit_dialog_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_quit_dialog_init(AgsQuitDialog *quit_dialog);
void ags_quit_dialog_finalize(GObject *gobject);

void ags_quit_dialog_connect(AgsConnectable *connectable);
void ags_quit_dialog_disconnect(AgsConnectable *connectable);

/**
 * SECTION:ags_quit_dialog
 * @short_description: The quit dialog
 * @title: AgsQuitDialog
 * @section_id:
 * @include: ags/X/ags_quit_dialog.h
 *
 * #AgsQuitDialog is shown as terminating GSequencer.
 */

static gpointer ags_quit_dialog_parent_class = NULL;

GType
ags_quit_dialog_get_type()
{
  static volatile gsize g_define_type_id__volatile = 0;

  if(g_once_init_enter (&g_define_type_id__volatile)){
    GType ags_type_quit_dialog = 0;

    static const GTypeInfo ags_quit_dialog_info = {
      sizeof (AgsQuitDialogClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_quit_dialog_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsQuitDialog),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_quit_dialog_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_quit_dialog_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_quit_dialog = g_type_register_static(GTK_TYPE_DIALOG,
						  "AgsQuitDialog", &ags_quit_dialog_info,
						  0);
    
    g_type_add_interface_static(ags_type_quit_dialog,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);

    g_once_init_leave(&g_define_type_id__volatile, ags_type_quit_dialog);
  }

  return g_define_type_id__volatile;
}

void
ags_quit_dialog_class_init(AgsQuitDialogClass *quit_dialog)
{
  GObjectClass *gobject;

  ags_quit_dialog_parent_class = g_type_class_peek_parent(quit_dialog);

  /* GObjectClass */
  gobject = (GObjectClass *) quit_dialog;

  gobject->finalize = ags_quit_dialog_finalize;
}

void
ags_quit_dialog_connectable_interface_init(AgsConnectableInterface *connectable)
{
  connectable->get_uuid = NULL;
  connectable->has_resource = NULL;

  connectable->is_ready = NULL;
  connectable->add_to_registry = NULL;
  connectable->remove_from_registry = NULL;

  connectable->list_resource = NULL;

  connectable->xml_compose = NULL;
  connectable->xml_parse = NULL;

  connectable->is_connected = NULL;
  connectable->connect = ags_quit_dialog_connect;
  connectable->disconnect = ags_quit_dialog_disconnect;

  connectable->connect_connection = NULL;
  connectable->disconnect_connection = NULL;
}

void
ags_quit_dialog_init(AgsQuitDialog *quit_dialog)
{
}

void
ags_quit_dialog_connect(AgsConnectable *connectable)
{
  AgsQuitDialog *quit_dialog;

  quit_dialog = AGS_QUIT_DIALOG(connectable);

  if((AGS_QUIT_DIALOG_CONNECTED & (quit_dialog->flags)) != 0){
    return;
  }

  quit_dialog->flags |= AGS_QUIT_DIALOG_CONNECTED;
}

void
ags_quit_dialog_disconnect(AgsConnectable *connectable)
{
  AgsQuitDialog *quit_dialog;

  quit_dialog = AGS_QUIT_DIALOG(connectable);

  if((AGS_QUIT_DIALOG_CONNECTED & (quit_dialog->flags)) == 0){
    return;
  }

  quit_dialog->flags &= (~AGS_QUIT_DIALOG_CONNECTED);
}

void
ags_quit_dialog_finalize(GObject *gobject)
{
  AgsQuitDialog *quit_dialog;

  quit_dialog = (AgsQuitDialog *) gobject;
  
  G_OBJECT_CLASS(ags_quit_dialog_parent_class)->finalize(gobject);
}

/**
 * ags_quit_dialog_new:
 * 
 * Create a new instance of #AgsQuitDialog
 * 
 * Returns: the #AgsQuitDialog
 * 
 * Since: 3.5.0
 */
AgsQuitDialog*
ags_quit_dialog_new()
{
  AgsQuitDialog *quit_dialog;

  quit_dialog = (AgsQuitDialog *) g_object_new(AGS_TYPE_QUIT_DIALOG,
					       NULL);

  return(quit_dialog);
}
