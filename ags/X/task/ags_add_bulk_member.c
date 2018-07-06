/* GSequencer - Advanced GTK Sequencer
 * Copyright (C) 2005-2015 Joël Krähemann
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

#include <ags/X/task/ags_add_bulk_member.h>

#include <ags/object/ags_application_context.h>
#include <ags/object/ags_connectable.h>

#include <ags/thread/ags_mutex_manager.h>

#include <ags/audio/ags_audio.h>
#include <ags/audio/ags_channel.h>

#include <ags/X/ags_window.h>
#include <ags/X/ags_effect_bridge.h>
#include <ags/X/ags_effect_bulk.h>
#include <ags/X/ags_bulk_member.h>

#include <ags/X/thread/ags_gui_thread.h>

void ags_add_bulk_member_class_init(AgsAddBulkMemberClass *add_bulk_member);
void ags_add_bulk_member_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_add_bulk_member_init(AgsAddBulkMember *add_bulk_member);
void ags_add_bulk_member_connect(AgsConnectable *connectable);
void ags_add_bulk_member_disconnect(AgsConnectable *connectable);
void ags_add_bulk_member_finalize(GObject *gobject);

void ags_add_bulk_member_launch(AgsTask *task);

/**
 * SECTION:ags_add_bulk_member
 * @short_description: add line_member object to line
 * @title: AgsAddBulkMember
 * @section_id:
 * @include: ags/X/task/ags_add_bulk_member.h
 *
 * The #AgsAddBulkMember task addspacks #AgsLineMember to #AgsLine.
 */

static gpointer ags_add_bulk_member_parent_class = NULL;
static AgsConnectableInterface *ags_add_bulk_member_parent_connectable_interface;

GType
ags_add_bulk_member_get_type()
{
  static volatile gsize g_define_type_id__volatile = 0;

  if(g_once_init_enter (&g_define_type_id__volatile)){
    GType ags_type_add_bulk_member;

    static const GTypeInfo ags_add_bulk_member_info = {
      sizeof (AgsAddBulkMemberClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_add_bulk_member_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsAddBulkMember),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_add_bulk_member_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_add_bulk_member_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_add_bulk_member = g_type_register_static(AGS_TYPE_TASK,
						 "AgsAddBulkMember",
						 &ags_add_bulk_member_info,
						 0);

    g_type_add_interface_static(ags_type_add_bulk_member,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);

    g_once_init_leave (&g_define_type_id__volatile, ags_type_add_bulk_member);
  }

  return g_define_type_id__volatile;
}

void
ags_add_bulk_member_class_init(AgsAddBulkMemberClass *add_bulk_member)
{
  GObjectClass *gobject;
  AgsTaskClass *task;

  ags_add_bulk_member_parent_class = g_type_class_peek_parent(add_bulk_member);

  /* gobject */
  gobject = (GObjectClass *) add_bulk_member;

  gobject->finalize = ags_add_bulk_member_finalize;

  /* task */
  task = (AgsTaskClass *) add_bulk_member;

  task->launch = ags_add_bulk_member_launch;
}

void
ags_add_bulk_member_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_add_bulk_member_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_add_bulk_member_connect;
  connectable->disconnect = ags_add_bulk_member_disconnect;
}

void
ags_add_bulk_member_init(AgsAddBulkMember *add_bulk_member)
{
  add_bulk_member->effect_bulk = NULL;
  add_bulk_member->bulk_member = NULL;
  add_bulk_member->x = 0;
  add_bulk_member->y = 0;
  add_bulk_member->width = 0;
  add_bulk_member->height = 0;
}

void
ags_add_bulk_member_connect(AgsConnectable *connectable)
{
  ags_add_bulk_member_parent_connectable_interface->connect(connectable);

  /* empty */
}

void
ags_add_bulk_member_disconnect(AgsConnectable *connectable)
{
  ags_add_bulk_member_parent_connectable_interface->disconnect(connectable);

  /* empty */
}

void
ags_add_bulk_member_finalize(GObject *gobject)
{
  G_OBJECT_CLASS(ags_add_bulk_member_parent_class)->finalize(gobject);

  /* empty */
}

void
ags_add_bulk_member_launch(AgsTask *task)
{
  AgsWindow *window;

  AgsEffectBridge *effect_bridge;
  AgsAddBulkMember *add_bulk_member;

  AgsGuiThread *gui_thread;

  AgsMutexManager *mutex_manager;
  AgsThread *main_loop;

  AgsApplicationContext *application_context;

  AgsChannel *channel;

  gboolean found_ports;
  
  pthread_mutex_t *application_mutex;
    
  add_bulk_member = AGS_ADD_BULK_MEMBER(task);

  window = (AgsWindow *) gtk_widget_get_toplevel((GtkWidget *) add_bulk_member->effect_bulk);

  application_context = (AgsApplicationContext *) window->application_context;

  mutex_manager = ags_mutex_manager_get_instance();
  application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);

  /* get audio loop */
  pthread_mutex_lock(application_mutex);

  main_loop = (AgsThread *) application_context->main_loop;
  
  pthread_mutex_unlock(application_mutex);

  /* get task thread */
  gui_thread = (AgsGuiThread *) ags_thread_find_type((AgsThread *) main_loop,
						      AGS_TYPE_GUI_THREAD);
  /*  */
  gtk_table_attach(AGS_EFFECT_BULK(add_bulk_member->effect_bulk)->table,
		   (GtkWidget *) add_bulk_member->bulk_member,
		   add_bulk_member->x, add_bulk_member->x + add_bulk_member->width,
		   add_bulk_member->y, add_bulk_member->y + add_bulk_member->height,
		   GTK_FILL, GTK_FILL,
		   0, 0);
  ags_connectable_connect(AGS_CONNECTABLE(add_bulk_member->bulk_member));
  gtk_widget_show_all((GtkWidget *) AGS_EFFECT_BULK(add_bulk_member->effect_bulk)->table);

  /* find ports */
  ags_bulk_member_find_port(add_bulk_member->bulk_member);
}

/**
 * ags_add_bulk_member_new:
 * @effect_bulk: the #AgsEffectBulk
 * @bulk_member: the #AgsBulkMember to add
 * @x: pack start x
 * @y: pack start y
 * @width: pack width
 * @height: pack height
 *
 * Creates an #AgsAddBulkMember.
 *
 * Returns: an new #AgsAddBulkMember.
 *
 * Since: 1.0.0
 */
AgsAddBulkMember*
ags_add_bulk_member_new(GtkWidget *effect_bulk,
			AgsBulkMember *bulk_member,
			guint x, guint y,
			guint width, guint height)
{
  AgsAddBulkMember *add_bulk_member;

  add_bulk_member = (AgsAddBulkMember *) g_object_new(AGS_TYPE_ADD_BULK_MEMBER,
						      NULL);

  add_bulk_member->effect_bulk = effect_bulk;
  add_bulk_member->bulk_member = bulk_member;
  add_bulk_member->x = x;
  add_bulk_member->y = y;
  add_bulk_member->width = width;
  add_bulk_member->height = height;

  return(add_bulk_member);
}
