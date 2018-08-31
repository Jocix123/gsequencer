/* GSequencer - Advanced GTK Sequencer
 * Copyright (C) 2005-2018 Joël Krähemann
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

#include <ags/audio/file/ags_ipatch_dls2_reader.h>

#include <ags/libags.h>

#include <stdlib.h>

#include <ags/i18n.h>

void ags_ipatch_dls2_reader_class_init(AgsIpatchDLS2ReaderClass *ipatch_dls2_reader);
void ags_ipatch_dls2_reader_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_ipatch_dls2_reader_init(AgsIpatchDLS2Reader *ipatch_dls2_reader);
void ags_ipatch_dls2_reader_set_property(GObject *gobject,
					 guint prop_id,
					 const GValue *value,
					 GParamSpec *param_spec);
void ags_ipatch_dls2_reader_get_property(GObject *gobject,
					 guint prop_id,
					 GValue *value,
					 GParamSpec *param_spec);
void ags_ipatch_dls2_reader_dispose(GObject *gobject);
void ags_ipatch_dls2_reader_finalize(GObject *gobject);

/**
 * SECTION:ags_ipatch_dls2_reader
 * @short_description: interfacing Soundfont2 related API of libinstpatch
 * @title: AgsIpatchDLS2Reader
 * @section_id:
 * @include: ags/audio/file/ags_ipatch_dls2_reader.h
 *
 * #AgsIpatchDLS2Reader is the base object to ineract with Soundfont2 related API.
 */

static gpointer ags_ipatch_dls2_reader_parent_class = NULL;
static AgsConnectableInterface *ags_ipatch_dls2_reader_parent_connectable_interface;

enum{
  PROP_0,
  PROP_IPATCH,
};

GType
ags_ipatch_dls2_reader_get_type()
{
  static volatile gsize g_define_type_id__volatile = 0;

  if(g_once_init_enter (&g_define_type_id__volatile)){
    GType ags_type_ipatch_dls2_reader;

    static const GTypeInfo ags_ipatch_dls2_reader_info = {
      sizeof(AgsIpatchDLS2ReaderClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_ipatch_dls2_reader_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof(AgsIpatchDLS2Reader),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_ipatch_dls2_reader_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_ipatch_dls2_reader_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_ipatch_dls2_reader = g_type_register_static(G_TYPE_OBJECT,
							 "AgsIpatchDLS2Reader",
							 &ags_ipatch_dls2_reader_info,
							 0);

    g_type_add_interface_static(ags_type_ipatch_dls2_reader,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);

    g_once_init_leave (&g_define_type_id__volatile, ags_type_ipatch_dls2_reader);
  }

  return g_define_type_id__volatile;
}

void
ags_ipatch_dls2_reader_class_init(AgsIpatchDLS2ReaderClass *ipatch_dls2_reader)
{
  GObjectClass *gobject;
  GParamSpec *param_spec;
  
  ags_ipatch_dls2_reader_parent_class = g_type_class_peek_parent(ipatch_dls2_reader);

  gobject = (GObjectClass *) ipatch_dls2_reader;

  gobject->set_property = ags_ipatch_dls2_reader_set_property;
  gobject->get_property = ags_ipatch_dls2_reader_get_property;

  gobject->dispose = ags_ipatch_dls2_reader_dispose;
  gobject->finalize = ags_ipatch_dls2_reader_finalize;

  /* properties */
  /**
   * AgsIpatchDLS2Reader:ipatch:
   *
   * The assigned #AgsIpatch
   * 
   * Since: 2.0.0
   */
  param_spec = g_param_spec_object("ipatch",
				   i18n_pspec("the ipatch"),
				   i18n_pspec("The assigned ipatch"),
				   AGS_TYPE_IPATCH,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_IPATCH,
				  param_spec);
}

void
ags_ipatch_dls2_reader_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_ipatch_dls2_reader_parent_connectable_interface = g_type_interface_peek_parent(connectable);
}

void
ags_ipatch_dls2_reader_init(AgsIpatchDLS2Reader *ipatch_dls2_reader)
{
  guint i;

  /* reader */
  ipatch_dls2_reader->reader = NULL;
  ipatch_dls2_reader->dls2 = NULL;
  
  /* selected */
  ipatch_dls2_reader->index_selected = (guint *) malloc(3 * sizeof(guint));
  memset(ipatch_dls2_reader->index_selected, 0, 3 * sizeof(guint));
  
  ipatch_dls2_reader->name_selected = (gchar **) malloc(4 * sizeof(gchar *));

  for(i = 0; i < 4; i++){
    ipatch_dls2_reader->name_selected[i] = NULL;
  }
  
  ipatch_dls2_reader->instrument = NULL;
  ipatch_dls2_reader->sample = NULL;

  ipatch_dls2_reader->error = NULL;
}

void
ags_ipatch_dls2_reader_set_property(GObject *gobject,
				    guint prop_id,
				    const GValue *value,
				    GParamSpec *param_spec)
{
  AgsIpatchDLS2Reader *ipatch_dls2_reader;

  ipatch_dls2_reader = AGS_IPATCH_DLS2_READER(gobject);

  switch(prop_id){
  case PROP_IPATCH:
    {
      AgsIpatch *ipatch;

      ipatch = (AgsIpatch *) g_value_get_object(value);

      if(ipatch_dls2_reader->ipatch == ipatch){
	return;
      }
      
      if(ipatch_dls2_reader->ipatch != NULL){
	g_object_unref(ipatch_dls2_reader->ipatch);
      }

      if(ipatch != NULL){
	g_object_ref(ipatch);
      }

      ipatch_dls2_reader->ipatch = ipatch;
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_ipatch_dls2_reader_get_property(GObject *gobject,
				    guint prop_id,
				    GValue *value,
				    GParamSpec *param_spec)
{
  AgsIpatchDLS2Reader *ipatch_dls2_reader;

  ipatch_dls2_reader = AGS_IPATCH_DLS2_READER(gobject);

  switch(prop_id){
  case PROP_IPATCH:
    {
      g_value_set_object(value, ipatch_dls2_reader->ipatch);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_ipatch_dls2_reader_dispose(GObject *gobject)
{
  AgsIpatchDLS2Reader *ipatch_dls2_reader;

  ipatch_dls2_reader = AGS_IPATCH_DLS2_READER(gobject);

  if(ipatch_dls2_reader->ipatch != NULL){
    g_object_unref(ipatch_dls2_reader->ipatch);

    ipatch_dls2_reader->ipatch = NULL;
  }

  /* call parent */  
  G_OBJECT_CLASS(ags_ipatch_dls2_reader_parent_class)->dispose(gobject);
}

void
ags_ipatch_dls2_reader_finalize(GObject *gobject)
{
  AgsIpatchDLS2Reader *ipatch_dls2_reader;

  ipatch_dls2_reader = AGS_IPATCH_DLS2_READER(gobject);

  if(ipatch_dls2_reader->ipatch != NULL){
    g_object_unref(ipatch_dls2_reader->ipatch);
  }

  /* call parent */  
  G_OBJECT_CLASS(ags_ipatch_dls2_reader_parent_class)->finalize(gobject);
}

gboolean
ags_ipatch_dls2_reader_load(AgsIpatchDLS2Reader *ipatch_dls2_reader,
			    IpatchFileHandle *handle)
{
  //TODO:JK: implement me
}

gboolean
ags_ipatch_dls2_reader_select_instrument(AgsIpatchDLS2Reader *ipatch_dls2_reader,
					 guint instrument_index)
{
  //TODO:JK: implement me
}

gboolean
ags_ipatch_dls2_reader_select_sample(AgsIpatchDLS2Reader *ipatch_dls2_reader,
				     guint sample_index)
{
  //TODO:JK: implement me
}

gchar**
ags_ipatch_dls2_reader_get_instrument_all(AgsIpatchDLS2Reader *ipatch_dls2_reader)
{
  //TODO:JK: implement me
}

gchar**
ags_ipatch_dls2_reader_get_sample_all(AgsIpatchDLS2Reader *ipatch_dls2_reader)
{
  //TODO:JK: implement me
}

gchar**
ags_ipatch_dls2_reader_get_sample_by_instrument_index(AgsIpatchDLS2Reader *ipatch_dls2_reader,
						      guint instrument_index)
{
  //TODO:JK: implement me
}

/**
 * ags_ipatch_dls2_reader_new:
 * @ipatch: the #AgsIpatch
 *
 * Creates an #AgsIpatchDLS2Reader.
 *
 * Returns: an empty #AgsIpatchDLS2Reader.
 *
 * Since: 2.0.0
 */
AgsIpatchDLS2Reader*
ags_ipatch_dls2_reader_new(AgsIpatch *ipatch)
{
  AgsIpatchDLS2Reader *ipatch_dls2_reader;

  ipatch_dls2_reader = (AgsIpatchDLS2Reader *) g_object_new(AGS_TYPE_IPATCH_DLS2_READER,
							    "ipatch", ipatch,
							    NULL);

  return(ipatch_dls2_reader);
}
