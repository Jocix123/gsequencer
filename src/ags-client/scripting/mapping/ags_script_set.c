/* AGS Client - Advanced GTK Sequencer Client
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

#include <ags-client/scripting/mapping/ags_script_set.h>

#include <ags-lib/object/ags_connectable.h>

void ags_script_set_class_init(AgsScriptSetClass *script_set);
void ags_script_set_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_script_set_init(AgsScriptSet *script_set);
void ags_script_set_connect(AgsConnectable *connectable);
void ags_script_set_disconnect(AgsConnectable *connectable);
void ags_script_set_finalize(GObject *gobject);

AgsScriptObject* ags_script_set_launch(AgsScriptObject *script_object);

xmlNode* ags_script_set_matrix_find_index(AgsScriptSet *script_set,
					  AgsScriptArray *matrix,
					  guint y);

void ags_script_set_matrix_get(AgsScriptSet *script_set,
			       AgsScriptArray *matrix,
			       AgsScriptVar *lvalue,
			       guint offset,
			       guint *x, guint *y);
void ags_script_set_matrix_put(AgsScriptSet *script_set,
			       AgsScriptArray *matrix,
			       AgsScriptVar *value,
			       guint offset,
			       guint *x, guint *y);

void ags_script_set_value(AgsScriptSet *script_set,
			  AgsScriptVar *value,
			  AgsScriptArray *lvalue);

void ags_script_set_default_index(AgsScriptSet *script_set,
				  AgsScriptArray *index);
void ags_script_set_matrix_move_index(AgsScriptSet *script_set,
				      AgsScriptArray *matrix,
				      AgsScriptArray *index,
				      guint dest_x, guint dest_y,
				      guint src_x, guint src_y);

void ags_script_set_matrix_sort(AgsScriptSet *script_set,
				AgsScriptArray *matrix,
				gboolean sort_column, gboolean sort_row,
				AgsScriptArray *index, guint depth,
				guint *x, guint *y);
void ags_script_set_matrix_sort_by_operands(AgsScriptSet *script_set,
				       AgsScriptArray *matrix,
				       AgsScriptArray *operands,
				       AgsScriptArray *index, guint depth,
				       guint *x, guint *y);

void ags_script_set_matrix_vector_mirror(AgsScriptSet *script_set,
					 AgsScriptArray *matrix,
					 AgsScriptArray *vector);

void ags_script_set_equation(AgsScriptSet *script_set,
			     AgsScriptArray *factorized,
			     AgsScriptArray *operands);

#define AGS_SCRIPT_SET_BASE64_UNIT (4)

static gpointer ags_script_set_parent_class = NULL;

GType
ags_script_set_get_type()
{
  static GType ags_type_script_set = 0;

  if(!ags_type_script_set){
    static const GTypeInfo ags_script_set_info = {
      sizeof (AgsScriptSetClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_script_set_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsScriptSet),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_script_set_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_script_set_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_script_set = g_type_register_static(AGS_TYPE_SCRIPT_CONTROLLER,
						 "AgsScriptSet\0",
						 &ags_script_set_info,
						 0);
    
    g_type_add_interface_static(ags_type_script_set,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
  }

  return (ags_type_script_set);
}

void
ags_script_set_class_init(AgsScriptSetClass *script_set)
{
  AgsScriptObjectClass *script_object;
  GObjectClass *gobject;

  ags_script_set_parent_class = g_type_class_peek_parent(script_set);

  /* GObjectClass */
  gobject = (GObjectClass *) script_set;

  gobject->finalize = ags_script_set_finalize;

  /* AgsScriptObjectClass */
  script_object = (AgsScriptObjectClass *) script_set;

  script_object->launch = ags_script_set_launch;
}

void
ags_script_set_connectable_interface_init(AgsConnectableInterface *connectable)
{
  connectable->connect = ags_script_set_connect;
  connectable->disconnect = ags_script_set_disconnect;
}

void
ags_script_set_init(AgsScriptSet *script_set)
{
  script_set->flags = 0;

  script_set->start =
    script_set->current = NULL;
}

void
ags_script_set_connect(AgsConnectable *connectable)
{
  /* empty */
}

void
ags_script_set_disconnect(AgsConnectable *connectable)
{
  /* empty */
}

void
ags_script_set_finalize(GObject *gobject)
{
  AgsScriptSet *script_set;

  script_set = AGS_SCRIPT_SET(gobject);

  if(script_set->start != NULL){
    free(script_set->start);
  }

  G_OBJECT_CLASS(ags_script_set_parent_class)->finalize(gobject);
}

AgsScriptObject*
ags_script_set_launch(AgsScriptObject *script_object)
{
  //TODO:JK: implement me

  return(script_object);
}

void
ags_script_set_from_string(AgsScriptSet *script_set, gchar *string)
{
  //TODO:JK: implement me
}

gchar*
ags_script_set_matrix_to_string(AgsScriptSet *script_set,
				AgsScriptArray *array)
{
  gchar *string;

  auto gchar* ags_script_set_fill_column(xmlNode *column);
  auto gchar* ags_script_set_fill_row(xmlNode *row);

  gchar* ags_script_set_fill_column(xmlNode *column){
    gchar *column, *tmp;
    xmlChar *content, *current;
    guchar *value;
    gsize retlength;
    guint i, i_stop;

    content = xmlNodeGetContent(column);

    value = g_base64_decode(content, &retlength);

    current = value;
    column = NULL;

    switch(script_array->mode){
    case AGS_SCRIPT_ARRAY_INT16:
      {
	i_stop = retlength / sizeof(gint16);
      }
      break;
    case AGS_SCRIPT_ARRAY_UINT16:
      {
	i_stop = retlength / sizeof(guint16);
      }
      break;
    case AGS_SCRIPT_ARRAY_INT32:
      {
	i_stop = retlength / sizeof(gint32);
      }
      break;
    case AGS_SCRIPT_ARRAY_UINT32:
      {
	i_stop = retlength / sizeof(guint32);
      }
      break;
    case AGS_SCRIPT_ARRAY_INT64:
      {
	i_stop = retlength / sizeof(gint64);
      }
      break;
    case AGS_SCRIPT_ARRAY_UINT64:
      {
	i_stop = retlength / sizeof(guint64);
      }
      break;
    case AGS_SCRIPT_ARRAY_DOUBLE:
      {
	i_stop = retlength / sizeof(gdouble);
      }
      break;
    case AGS_SCRIPT_ARRAY_CHAR:
      {
	g_message("unsupported data type: gchar\0");
      }
      break;
    case AGS_SCRIPT_ARRAY_POINTER:
      {
	g_message("unsupported data type: gpointer\0");
      }
      break;
    }

    for(i = 0; i < i_stop; i++){
      switch(script_array->mode){
      case AGS_SCRIPT_ARRAY_INT16:
	{
	  gint16 k;

	  memcpy(&k, current, sizeof(gint16));
	  current += sizeof(gint16);

	  tmp = column;

	  column = g_strdup_printf("%s %d:\0", column, k);
	  g_free(tmp);
	}
	break;
      case AGS_SCRIPT_ARRAY_UINT16:
	{
	  guint16 k;

	  memcpy(&k, current, sizeof(guint16));
	  current += sizeof(guint16);

	  tmp = column;

	  column = g_strdup_printf("%s %d:\0", column, k);
	  g_free(tmp);
	}
	break;
      case AGS_SCRIPT_ARRAY_INT32:
	{
	  gint32 k;

	  memcpy(&k, current, sizeof(gint32));
	  current += sizeof(gint32);

	  tmp = column;

	  column = g_strdup_printf("%s %d:\0", column, k);
	  g_free(tmp);
	}
	break;
      case AGS_SCRIPT_ARRAY_UINT32:
	{
	  guint32 k;

	  memcpy(&k, current, sizeof(guint32));
	  current += sizeof(guint32);

	  tmp = column;

	  column = g_strdup_printf("%s %d:\0", column, k);
	  g_free(tmp);
	}
	break;
      case AGS_SCRIPT_ARRAY_INT64:
	{
	  gint64 k;

	  memcpy(&k, current, sizeof(gint64));
	  current += sizeof(gint64);

	  tmp = column;

	  column = g_strdup_printf("%s %d:\0", column, k);
	  g_free(tmp);
	}
	break;
      case AGS_SCRIPT_ARRAY_UINT64:
	{
	  guint64 k;

	  memcpy(&k, current, sizeof(guint64));
	  current += sizeof(guint64);

	  tmp = column;

	  column = g_strdup_printf("%s %d:\0", column, k);
	  g_free(tmp);
	}
	break;
      case AGS_SCRIPT_ARRAY_DOUBLE:
	{
	  gdouble k;

	  memcpy(&k, current, sizeof(gdouble));
	  current += sizeof(gdouble);

	  tmp = column;

	  column = g_strdup_printf("%s %f:\0", column, k);
	  g_free(tmp);
	}
	break;
      case AGS_SCRIPT_ARRAY_CHAR:
	{
	  g_message("unsupported data type: gchar\0");
	}
	break;
      case AGS_SCRIPT_ARRAY_POINTER:
	{
	  g_message("unsupported data type: gpointer\0");
	}
	break;
      }
    }
  }
  gchar* ags_script_set_fill_row(xmlNode *row){
    xmlNode *current;
    gchar *matrix, *column, *tmp;

    current = row;
    matrix = g_strdup("{\0");
    
    while(current != NULL){
      if(current->type == XML_ELEMENT_NODE){
	column = ags_script_set_fill_column();
	tmp = matrix;

	matrix = g_strconcat(matrix, column, ";\0");
	g_free(tmp);
      }

      current = current->next;
    }

    tmp = matrix;

    matrix = g_strconcat(matrix, "}\0");
    g_free(tmp);

    return(matrix);
  }

  string = ags_script_set_fill_row(script_object->node);

  return(string);
}

AgsScriptArray*
ags_script_set_matrix_from_string(AgsScriptSet *script_set,
				  gchar *string)
{
  AgsXmlScriptFactory *xml_script_factory;
  AgsScriptArray *array;
  xmlNode *matrix;
  GError *error;

  auto void ags_script_set_matrix_from_string_read_col(xmlNode *node,
						       gchar *string);
  auto void ags_script_set_matrix_from_string_read_row(xmlNode *node,
						       gchar *string);
  auto void ags_script_set_matrix_from_string_validate(xmlNode *node);

  void ags_script_set_matrix_from_string_read_col(xmlNode *node,
						  gchar *string){
    guchar *col;
    gchar *current, *prev;
    guint cols;

    col = NULL;

    current =
      prev = string;
    cols = 0;

    while((current = strchr(current, ':')) != NULL){

      switch(script_array->mode){
      case AGS_SCRIPT_ARRAY_INT16:
	{
	  gint16 k;

	  sscanf(prev, "%d:\0", &k);

	  if(col == NULL){
	    col = (gint16 *) malloc(sizeof(gint16));
	  }else{
	    col = (gint16 *) realloc((cols + 1) * sizeof(gint16));
	  }

	  memcpy(col, &k, sizeof(gint16));
	}
	break;
      case AGS_SCRIPT_ARRAY_UINT16:
	{
	  guint16 k;

	  sscanf(prev, "%d:\0", &k);

	  if(col == NULL){
	    col = (guint16 *) malloc(sizeof(guint16));
	  }else{
	    col = (guint16 *) realloc((cols + 1) * sizeof(guint16));
	  }

	  memcpy(col, &k, sizeof(guint16));
	}
	break;
      case AGS_SCRIPT_ARRAY_INT32:
	{
	  gint32 k;

	  sscanf(prev, "%d:\0", &k);

	  if(col == NULL){
	    col = (gint32 *) malloc(sizeof(gint32));
	  }else{
	    col = (gint32 *) realloc((cols + 1) * sizeof(gint32));
	  }

	  memcpy(col, &k, sizeof(gint32));
	}
	break;
      case AGS_SCRIPT_ARRAY_UINT32:
	{
	  guint32 k;

	  sscanf(prev, "%d:\0", &k);

	  if(col == NULL){
	    col = (guint32 *) malloc(sizeof(guint32));
	  }else{
	    col = (guint32 *) realloc((cols + 1) * sizeof(guint32));
	  }

	  memcpy(col, &k, sizeof(guint32));
	}
	break;
      case AGS_SCRIPT_ARRAY_INT64:
	{
	  gint64 k;

	  sscanf(prev, "%d:\0", &k);

	  if(col == NULL){
	    col = (gint64 *) malloc(sizeof(gint64));
	  }else{
	    col = (gint64 *) realloc((cols + 1) * sizeof(gint64));
	  }

	  memcpy(col, &k, sizeof(gint64));
	}
	break;
      case AGS_SCRIPT_ARRAY_UINT64:
	{
	  guint64 k;

	  sscanf(prev, "%d:\0", &k);

	  if(col == NULL){
	    col = (guint64 *) malloc(sizeof(guint64));
	  }else{
	    col = (guint64 *) realloc((cols + 1) * sizeof(guint64));
	  }

	  memcpy(col, &k, sizeof(guint64));
	}
	break;
      case AGS_SCRIPT_ARRAY_DOUBLE:
	{
	  gdouble k;

	  sscanf(prev, "%f:\0", &k);

	  if(col == NULL){
	    col = (gdouble *) malloc(sizeof(gdouble));
	  }else{
	    col = (gdouble *) realloc((cols + 1) * sizeof(gdouble));
	  }

	  memcpy(col, &k, sizeof(gdouble));
	}
	break;
      case AGS_SCRIPT_ARRAY_CHAR:
	{
	  g_message("unsupported data type: gchar\0");
	}
	break;
      case AGS_SCRIPT_ARRAY_POINTER:
	{
	  g_message("unsupported data type: gpointer\0");
	}
	break;
      }

      current++;
      prev = current;
      cols++;
    }

    xmlNodeSetContent(node, (xmlChar *) col);
    xmlSetProp(node, "length\0", cols);
  }
  void ags_script_set_matrix_from_string_read_row(xmlNode *node,
						  gchar *string){
    xmlNode *row;
    gchar *current, *prev;
    guint rows;

    current =
      prev = string;
    rows = 0;

    while((current = strchr(current, ';')) != NULL){
      row = ags_xml_script_factory_map(xml_script_factory,
				       "ags-array\0",
				       &error);
      ags_script_set_matrix_from_string_read_col(row,
						 prev);
      xmlAddChild(node, row);

      current++;
      prev = current;
      rows++;
    }

    xmlSetProp(node, "length\0", rows);
  }
  void ags_script_set_matrix_from_string_validate(xmlNode *node){
    xmlNode *current;
    guchar *col;
    guint *cols;
    guint n_cols, n_rows;
    guint i, j, j_start;
    gsize retlength;

    n_rows = strtoul(xmlGetProp(node, "length\0"), NULL, 10);
    cols = (guint *) malloc(n_rows * sizeof(guint));

    current = node->children;
    n_cols = 0;

    /* read cols */
    for(i = 0; i < n_rows; i++){
      cols[i] = strtoul(xmlGetProp(current, "length\0"), NULL, 10);

      /* find longest row */
      if(n_cols < cols[i]){
	n_cols = cols[i];
      }

      current = current->next;
    }

    /* fill invalid rows */
    current = node->children;

    for(i = 0; i < n_rows; i++){
      col = g_base64_decode((guchar *) xmlNodeGetContent(node),
			    &retlength);
      j_start = strtoul(xmlGetProp(node, "length\0"), NULL, 10);

      for(j = j_start; j < n_cols; j++){

	switch(script_array->mode){
	case AGS_SCRIPT_ARRAY_INT16:
	  {
	    gint16 k;

	    k = 0;
	    
	    if(col == NULL){
	      col = (gint16 *) malloc(sizeof(gint16));
	    }else{
	      col = (gint16 *) realloc((cols + 1) * sizeof(gint16));
	    }

	    memcpy(col, &k, sizeof(gint16));
	  }
	  break;
	case AGS_SCRIPT_ARRAY_UINT16:
	  {
	    guint16 k;

	    k = 0;
	    
	    if(col == NULL){
	      col = (guint16 *) malloc(sizeof(guint16));
	    }else{
	      col = (guint16 *) realloc((cols + 1) * sizeof(guint16));
	    }

	    memcpy(col, &k, sizeof(guint16));
	  }
	  break;
	case AGS_SCRIPT_ARRAY_INT32:
	  {
	    gint32 k;

	    k = 0;
	    
	    if(col == NULL){
	      col = (gint32 *) malloc(sizeof(gint32));
	    }else{
	      col = (gint32 *) realloc((cols + 1) * sizeof(gint32));
	    }

	    memcpy(col, &k, sizeof(gint32));
	  }
	  break;
	case AGS_SCRIPT_ARRAY_UINT32:
	  {
	    guint32 k;

	    k = 0;
	    
	    if(col == NULL){
	      col = (guint32 *) malloc(sizeof(guint32));
	    }else{
	      col = (guint32 *) realloc((cols + 1) * sizeof(guint32));
	    }

	    memcpy(col, &k, sizeof(guint32));
	  }
	  break;
	case AGS_SCRIPT_ARRAY_INT64:
	  {
	    gint64 k;

	    k = 0;
	    
	    if(col == NULL){
	      col = (gint64 *) malloc(sizeof(gint64));
	    }else{
	      col = (gint64 *) realloc((cols + 1) * sizeof(gint64));
	    }

	    memcpy(col, &k, sizeof(gint64));
	  }
	  break;
	case AGS_SCRIPT_ARRAY_UINT64:
	  {
	    guint64 k;

	    k = 0;
	    
	    if(col == NULL){
	      col = (guint64 *) malloc(sizeof(guint64));
	    }else{
	      col = (guint64 *) realloc((cols + 1) * sizeof(guint64));
	    }

	    memcpy(col, &k, sizeof(guint64));
	  }
	  break;
	case AGS_SCRIPT_ARRAY_DOUBLE:
	  {
	    gdouble k;

	    k = 0.0;
	    
	    if(col == NULL){
	      col = (gdouble *) malloc(sizeof(gdouble));
	    }else{
	      col = (gdouble *) realloc((cols + 1) * sizeof(gdouble));
	    }

	    memcpy(col, &k, sizeof(gdouble));
	  }
	  break;
	case AGS_SCRIPT_ARRAY_CHAR:
	  {
	    g_message("unsupported data type: gchar\0");
	  }
	  break;
	case AGS_SCRIPT_ARRAY_POINTER:
	  {
	    g_message("unsupported data type: gpointer\0");
	  }
	  break;
	}

      }

      current = current->next;
    }
  }

  xml_script_factory = AGS_SCRIPT(AGS_SCRIPT_OBJECT(script_set)->script)->xml_script_factory;

  array = ags_script_array_new();

  error = NULL;

  matrix = AGS_SCRIPT_OBJECT(array)->node;

  if(error != NULL){
    g_message("failed to instantiate xmlNode: ags-array\0");

    g_object_unref(array);

    return(NULL);
  }

  ags_script_set_matrix_from_string_read_row(matrix,
					     string);
  ags_script_set_matrix_from_string_validate(matrix);

  return(array);
}

xmlNode*
ags_script_set_matrix_find_index(AgsScriptSet *script_set,
				 AgsScriptArray *matrix,
				 guint y)
{
  xmlNode *current;

  current = matrix->node->children;

  while(current != NULL){
    if(strtoul(xmlGetProp(current, "index\0") NULL, 10) == y){
      return(current);
    }

    current = current->next;
  }

  return(NULL);
}

void
ags_script_set_matrix_get(AgsScriptSet *script_set,
			  AgsScriptArray *matrix,
			  AgsScriptVar *lvalue,
			  guint offset,
			  guint *x, guint *y)
{
  xmlNode *current, *row;
  guchar *data;
  guint n_cols;
  guint ret_x, ret_y;
  gsize retlength;
  
  row = AGS_SCRIPT_OBJECT(value)->node->children;

  if(xmlNodeGetContent(AGS_SCRIPT_OBJECT(lvalue)->node) != NULL){
    xmlNodeSetContent(AGS_SCRIPT_OBJECT(lvalue)->node,
		      NULL);
  }

  if(row != NULL){
    n_cols = strtoul(xmlGetProp(row, "length\0"), NULL, 10);
    
    ret_x = (guint) floor(offset / n_cols);
    ret_y = offset % n_cols;

    current = ags_script_set_matrix_find_index(matrix,
					       ret_y);

    data = g_base64_decode(xmlNodeGetContent(current),
			   &retlength);

    switch(script_array->mode){
    case AGS_SCRIPT_ARRAY_INT16:
      {
	gint16 *k;

	k = (gint16 *) data;
	xmlNodeSetContent(AGS_SCRIPT_OBJECT(lvalue)->node, g_base64_encode((guchar *) &(k[ret_x]),
									   sizeof(gint16)));
      }
      break;
    case AGS_SCRIPT_ARRAY_UINT16:
      {
	guint16 *k;

	k = (guint16 *) data;
	xmlNodeSetContent(AGS_SCRIPT_OBJECT(lvalue)->node, g_base64_encode((guchar *) &(k[ret_x]),
									   sizeof(guint16)));
      }
      break;
    case AGS_SCRIPT_ARRAY_INT32:
      {
	gint32 *k;

	k = (gint32 *) data;
	xmlNodeSetContent(AGS_SCRIPT_OBJECT(lvalue)->node, g_base64_encode((guchar *) &(k[ret_x]),
									   sizeof(gint32)));
      }
      break;
    case AGS_SCRIPT_ARRAY_UINT32:
      {
	guint32 *k;

	k = (guint32 *) data;
	xmlNodeSetContent(AGS_SCRIPT_OBJECT(lvalue)->node, g_base64_encode((guchar *) &(k[ret_x]),
									   sizeof(guint32)));
      }
      break;
    case AGS_SCRIPT_ARRAY_INT64:
      {
	gint64 *k;

	k = (gint64 *) data;
	xmlNodeSetContent(AGS_SCRIPT_OBJECT(lvalue)->node, g_base64_encode((guchar *) &(k[ret_x]),
									   sizeof(gint64)));
      }
      break;
    case AGS_SCRIPT_ARRAY_UINT64:
      {
	guint64 *k;

	k = (guint64 *) data;
	xmlNodeSetContent(AGS_SCRIPT_OBJECT(lvalue)->node, g_base64_encode((guchar *) &(k[ret_x]),
									   sizeof(guint64)));
      }
      break;
    case AGS_SCRIPT_ARRAY_DOUBLE:
      {
	gdouble *k;

	k = (gdouble *) data;
	xmlNodeSetContent(AGS_SCRIPT_OBJECT(lvalue)->node, g_base64_encode((guchar *) &(k[ret_x]),
									   sizeof(gdouble)));
      }
      break;
    case AGS_SCRIPT_ARRAY_CHAR:
      {
	char *k;

	k = (char *) data;
	xmlNodeSetContent(AGS_SCRIPT_OBJECT(lvalue)->node, g_base64_encode((guchar *) &(k[ret_x]),
									   sizeof(char)));
      }
      break;
    case AGS_SCRIPT_ARRAY_POINTER:
      {
	g_message("unsupported data type: gpointer\0");
      }
      break;
    }

    free(data);
  }else{
    ret_x = G_MAXUINT;
    ret_y = G_MAXUINT;
  }
  
  *x = ret_x;
  *y = ret_y;
}

void
ags_script_set_matrix_put(AgsScriptSet *script_set,
			  AgsScriptArray *matrix,
			  AgsScriptVar *value,
			  guint offset,
			  guint *x, guint *y)
{
  xmlNode *current, *row;
  xmlChar *data;
  guchar *tmp_col;
  guint n_cols;
  guint ret_x, ret_y;
  gsize putlength;
  
  row = AGS_SCRIPT_OBJECT(value)->node->children;

  if(row != NULL){
    n_cols = strtoul(xmlGetProp(row, "length\0"), NULL, 10);
    
    ret_x = (guint) floor(offset / n_cols);
    ret_y = offset % n_cols;

    current = ags_script_set_matrix_find_index(matrix,
					       ret_y);

    data = xmlNodeGetContent(current);

    switch(script_array->mode){
    case AGS_SCRIPT_ARRAY_INT16:
      {
	guchar *value_data;
	gint16 *k_ptr;
	gint16 k;
	gint state;
	guint save;
	gsize retlength;

	value_data = g_base64_decode(xmlNodeGetContent(AGS_SCRIPT_OBJECT(value)->node), retlength);
	memcpy(&k, value_data, sizeof(gint16));

	putlength = AGS_SCRIPT_SET_BASE64_UNIT / sizeof(gint16);
	k_ptr = (gint16 *) malloc(putlength);

	state = 0;
	save = 0;

	g_base64_decode_step(data,
			     putlength,
			     k_ptr,
			     &state,
			     &save);

	k_ptr[x % putlength] = k;

	tmp_col = g_base64_encode((guchar *) k_ptr,
				  putlength);
	free(k_ptr);
      }
      break;
    case AGS_SCRIPT_ARRAY_UINT16:
      {
	guchar *value_data;
	guint16 *k_ptr;
	guint16 k;
	gint state;
	guint save;
	gsize retlength;

	value_data = g_base64_decode(xmlNodeGetContent(AGS_SCRIPT_OBJECT(value)->node), retlength);
	memcpy(&k, value_data, sizeof(guint16));

	putlength = AGS_SCRIPT_SET_BASE64_UNIT / sizeof(guint16);
	k_ptr = (guint16 *) malloc(putlength);

	state = 0;
	save = 0;

	g_base64_decode_step(data,
			     putlength,
			     k_ptr,
			     &state,
			     &save);

	k_ptr[x % putlength] = k;

	tmp_col = g_base64_encode((guchar *) k_ptr,
				  putlength);
	free(k_ptr);
      }
      break;
    case AGS_SCRIPT_ARRAY_INT32:
      {
	guchar *value_data;
	gint32 *k_ptr;
	gint32 k;
	gint state;
	guint save;
	gsize retlength;

	value_data = g_base64_decode(xmlNodeGetContent(AGS_SCRIPT_OBJECT(value)->node), retlength);
	memcpy(&k, value_data, sizeof(gint32));

	putlength = AGS_SCRIPT_SET_BASE64_UNIT / sizeof(gint32);
	k_ptr = (gint32 *) malloc(putlength);

	state = 0;
	save = 0;

	g_base64_decode_step(data,
			     putlength,
			     k_ptr,
			     &state,
			     &save);

	k_ptr[x % putlength] = k;

	tmp_col = g_base64_encode((guchar *) k_ptr,
				  putlength);
	free(k_ptr);
      }
      break;
    case AGS_SCRIPT_ARRAY_UINT32:
      {
	guchar *value_data;
	guint32 *k_ptr;
	guint32 k;
	gint state;
	guint save;
	gsize retlength;

	value_data = g_base64_decode(xmlNodeGetContent(AGS_SCRIPT_OBJECT(value)->node), retlength);
	memcpy(&k, value_data, sizeof(guint32));

	putlength = AGS_SCRIPT_SET_BASE64_UNIT / sizeof(guint32);
	k_ptr = (guint32 *) malloc(putlength);

	state = 0;
	save = 0;

	g_base64_decode_step(data,
			     putlength,
			     k_ptr,
			     &state,
			     &save);

	k_ptr[x % putlength] = k;

	tmp_col = g_base64_encode((guchar *) k_ptr,
				  putlength);
	free(k_ptr);
      }
      break;
    case AGS_SCRIPT_ARRAY_INT64:
      {
	guchar *value_data;
	gint64 *k_ptr;
	gint64 k;
	gint state;
	guint save;
	gsize retlength;

	value_data = g_base64_decode(xmlNodeGetContent(AGS_SCRIPT_OBJECT(value)->node), retlength);
	memcpy(&k, value_data, sizeof(gint64));

	putlength = sizeof(gint64);
	k_ptr = (gint64 *) malloc(putlength);

	state = 0;
	save = 0;

	g_base64_decode_step(data,
			     putlength,
			     k_ptr,
			     &state,
			     &save);

	k_ptr[x % putlength] = k;

	tmp_col = g_base64_encode((guchar *) k_ptr,
				  putlength);
	free(k_ptr);
      }
      break;
    case AGS_SCRIPT_ARRAY_UINT64:
      {
	guchar *value_data;
	guint64 *k_ptr;
	guint64 k;
	gint state;
	guint save;
	gsize retlength;

	value_data = g_base64_decode(xmlNodeGetContent(AGS_SCRIPT_OBJECT(value)->node), retlength);
	memcpy(&k, value_data, sizeof(guint64));

	putlength = sizeof(guint64);
	k_ptr = (guint64 *) malloc(putlength);

	state = 0;
	save = 0;

	g_base64_decode_step(data,
			     putlength,
			     k_ptr,
			     &state,
			     &save);

	k_ptr[x % putlength] = k;

	tmp_col = g_base64_encode((guchar *) k_ptr,
				  putlength);
	free(k_ptr);
      }
      break;
    case AGS_SCRIPT_ARRAY_DOUBLE:
      {
	guchar *value_data;
	gdouble *k_ptr;
	gdouble k;
	gint state;
	guint save;
	gsize retlength;

	value_data = g_base64_decode(xmlNodeGetContent(AGS_SCRIPT_OBJECT(value)->node), retlength);
	memcpy(&k, value_data, sizeof(gdouble));

	putlength = sizeof(gdouble);
	k_ptr = (gdouble *) malloc(putlength);

	state = 0;
	save = 0;

	g_base64_decode_step(data,
			     putlength,
			     k_ptr,
			     &state,
			     &save);

	k_ptr[x % putlength] = k;

	tmp_col = g_base64_encode((guchar *) k_ptr,
				  putlength);
	free(k_ptr);
      }
      break;
    case AGS_SCRIPT_ARRAY_CHAR:
      {
	guchar *value_data;
	char *k_ptr;
	char k;
	gint state;
	guint save;
	gsize retlength;

	value_data = g_base64_decode(xmlNodeGetContent(AGS_SCRIPT_OBJECT(value)->node), retlength);
	memcpy(&k, value_data, sizeof(char));

	putlength = AGS_SCRIPT_SET_BASE64_UNIT / sizeof(char);
	k_ptr = (char *) malloc(putlength);

	state = 0;
	save = 0;

	g_base64_decode_step(data,
			     putlength,
			     k_ptr,
			     &state,
			     &save);

	k_ptr[x % putlength] = k;

	tmp_col = g_base64_encode((guchar *) k_ptr,
				  putlength);
	free(k_ptr);
      }
      break;
    case AGS_SCRIPT_ARRAY_POINTER:
      {
	g_message("unsupported data type: gpointer\0");
      }
      break;
    }
  }else{
    ret_x = G_MAXUINT;
    ret_y = G_MAXUINT;
  }

  memcpy(data, tmp_col, strlen(tmp_col));
  g_free(tmp_col);

  *x = ret_x;
  *y = ret_y;
}

void
ags_script_set_value(AgsScriptSet *script_set,
		     AgsScriptVar *value,
		     AgsScriptArray *lvalue)
{
  xmlNode *current;
  guchar *data;
  guint n_cols;
  gsize retlength;

  current = AGS_SCRIPT_OBJECT(script_set)->node->children;

  while(current != NULL){
    n_cols = strtoul(xmlGetProp(current, "length\0"), NULL, 10);

    switch(script_var->mode){
    case AGS_SCRIPT_ARRAY_INT16:
      {
	guchar *value;
	gint16 k;
	int c, mask;
	guint i, shift;
	
	data = (guchar *) malloc(n_cols * sizeof(gint16));

	value = g_base64_decode(xmlNodeGetContent(current), &retlength);
	memcpy(&k, value, sizeof(gint16));

	mask = (int) (gint16) 0xffff;

	for(i = 0; i < sizeof(int) / sizeof(gint16) && i < n_cols; i++){
	  shift = i * sizeof(gint16);
	  mask = mask << shift;
	  c = mask & (k << shift);
	}

	memset(data, c, (size_t) ceil((n_cols * sizeof(gint16)) / sizeof(int)));
      }
      break;
    case AGS_SCRIPT_ARRAY_UINT16:
      {
	guchar *value;
	guint16 k;
	int c, mask;
	guint i, shift;

	data = (guchar *) malloc(n_cols * sizeof(guint16));

	value = g_base64_decode(xmlNodeGetContent(current), &retlength);
	memcpy(&k, value, sizeof(guint16));

	mask = (int) (guint16) 0xffff;

	for(i = 0; i < sizeof(int) / sizeof(guint16) && i < n_cols; i++){
	  shift = i * sizeof(guint16);
	  mask = mask << shift;
	  c = mask & (k << shift);
	}

	memset(data, c, (size_t) ceil((n_cols * sizeof(guint16)) / sizeof(int)));
      }
      break;
    case AGS_SCRIPT_ARRAY_INT32:
      {
	guchar *value;
	gint32 k;
	int c, mask;
	guint i, shift;

	data = (guchar *) malloc(n_cols * sizeof(gint32));

	value = g_base64_decode(xmlNodeGetContent(current), &retlength);
	memcpy(&k, value, sizeof(gint32));

	mask = (int) (gint32) 0xffff;

	for(i = 0; i < sizeof(int) / sizeof(gint32) && i < n_cols; i++){
	  shift = i * sizeof(gint32);
	  mask = mask << shift;
	  c = mask & (k << shift);
	}

	memset(data, c, (size_t) ceil((n_cols * sizeof(gint32)) / sizeof(int)));
      }
      break;
    case AGS_SCRIPT_ARRAY_UINT32:
      {
	guchar *value;
	guint32 k;
	int c, mask;
	guint i, shift;

	data = (guchar *) malloc(n_cols * sizeof(guint32));

	value = g_base64_decode(xmlNodeGetContent(current), &retlength);
	memcpy(&k, value, sizeof(guint32));

	mask = (int) (guint32) 0xffff;

	for(i = 0; i < sizeof(int) / sizeof(guint32) && i < n_cols; i++){
	  shift = i * sizeof(guint32);
	  mask = mask << shift;
	  c = mask & (k << shift);
	}

	memset(data, c, (size_t) ceil((n_cols * sizeof(guint32)) / sizeof(int)));
      }
      break;
    case AGS_SCRIPT_ARRAY_INT64:
      {
	char *ptr;
	gint64 k, mask;
	char c;
	guint i, j, j_stop, shift;

	ptr =
	  data = (char *) malloc(n_cols * sizeof(gint64) * sizeof(char));

	value = g_base64_decode(xmlNodeGetContent(current), &retlength);
	memcpy(&k, value, sizeof(gint64));

	j_stop = sizeof(gint64) / sizeof(char);

	mask = (gint64) 0xff;
	
	for(i = 0; i < n_cols; i++){
	  mask = (gint64) 0xff;

	  for(j = 0; j < j_stop; j++){
	    c = (mask & k) >> j; 

	    ptr[i * j_stop + j] = c;

	    if(j + 1 < j_stop){
	      mask = mask << sizeof(char);
	    }
	  }
	}
      }
      break;
    case AGS_SCRIPT_ARRAY_UINT64:
      {
	char *ptr;
	guint offset;
	guint64 k, mask;
	char c;
	guint i, j, j_stop, shift;

	ptr =
	  data = (char *) malloc(n_cols * sizeof(guint64) * sizeof(char));

	value = g_base64_decode(xmlNodeGetContent(current), &retlength);
	memcpy(&k, value, sizeof(guint64));

	j_stop = sizeof(guint64) / sizeof(char);

	mask = (guint64) 0xff;
	
	for(i = 0; i < n_cols; i++){
	  mask = (gint64) 0xff;

	  for(j = 0; j < j_stop; j++){
	    c = (mask & k) >> j; 

	    ptr[i * j_stop + j] = c;

	    if(j + 1 < j_stop){
	      mask = mask << sizeof(char);
	    }
	  }
	}
      }
      break;
    case AGS_SCRIPT_ARRAY_DOUBLE:
      {
	gdouble *ptr;
	gdouble k;
	guint i;

	data = (char *) malloc(n_cols * sizeof(gdouble) * sizeof(guchar));
	ptr = (gdouble *) data;
	
	for(i = 0; i < n_cols; i++){
	  ptr[i] = k;
	}
      }
      break;
    case AGS_SCRIPT_ARRAY_CHAR:
      {
	guchar *value;
	char k;
	int c, mask;
	guint i, shift;
	
	data = (guchar *) malloc(n_cols * sizeof(char));

	value = g_base64_decode(xmlNodeGetContent(current), &retlength);
	memcpy(&k, value, sizeof(char));

	mask = (int) (char) 0xffff;

	for(i = 0; i < sizeof(int) / sizeof(char) && i < n_cols; i++){
	  shift = i * sizeof(char);
	  mask = mask << shift;
	  c = mask & (k << shift);
	}

	memset(data, c, (size_t) ceil((n_cols * sizeof(char)) / sizeof(int)));	
      }
      break;
    case AGS_SCRIPT_ARRAY_POINTER:
      {
	g_message("unsupported data type: gpointer\0");
      }
      break;
    }

    xmlNodeSetContent(current, g_base64_encode(data));
    free(data);

    current = current->next;
  }
}

void
ags_script_set_default_index(AgsScriptSet *script_set,
			     AgsScriptArray *index)
{
  //TODO:JK: implement me
}

void
ags_script_set_matrix_move_index(AgsScriptSet *script_set,
				 AgsScriptArray *matrix,
				 AgsScriptArray *index,
				 guint dest_x, guint dest_y,
				 guint src_x, guint src_y)
{
  //TODO:JK: implement me
}

void
ags_script_set_matrix_sort(AgsScriptSet *script_set,
			   AgsScriptArray *matrix,
			   gboolean sort_column, gboolean sort_row,
			   AgsScriptArray *index, guint depth,
			   guint *x, guint *y)
{
  xmlNode current_matrix, current_index;
  guchar *matrix_data, *index_data;
  guint *index_ptr;
  guint n_cols;
  guint offset, stop;
  guint i, j, k;

  current_matrix = ags_script_set_matrix_find_index(script_set,
						    matrix,
						    y);
  current_index = ags_script_set_matrix_find_index(script_set,
						   index,
						   y);

  offset = 0;

  /* depth^2 / 2.0 - 1 */
  //TODO:JK: verify
  stop = (guint) exp(log((double) depth) / log((double) 2));

  for(i = 0; i < depth && current != NULL;){
    matrix_data = g_base64_decode(xmlNodeGetContent(current));
    index_data = g_base64_decode(xmlNodeGetContent(current));

    index_ptr = (guint) index_data;

    /*  */
    //TODO:JK: verify
    n_cols = strtoul(xmlGetProp(current, "length\0"), NULL, 10);

    for(k = 0; k < depth - i && offset < stop; k++){
      for(j = 0; j < (guint) ceil((double) depth / (double) k) && offset < stop; j++, i++, offset++){
	if(j + x >= n_cols){
	  break;
	}

	switch(script_array->mode){
	case AGS_SCRIPT_ARRAY_INT16:
	  {
	    gint16 *matrix_ptr;

	    matrix_ptr = (gint16 *) matrix_data;

	    if(matrix_ptr > ){
	    }

	  }
	  break;
	case AGS_SCRIPT_ARRAY_UINT16:
	  {
	  }
	  break;
	case AGS_SCRIPT_ARRAY_INT32:
	  {
	  }
	  break;
	case AGS_SCRIPT_ARRAY_UINT32:
	  {
	  }
	  break;
	case AGS_SCRIPT_ARRAY_INT64:
	  {
	  }
	  break;
	case AGS_SCRIPT_ARRAY_UINT64:
	  {
	  }
	  break;
	case AGS_SCRIPT_ARRAY_DOUBLE:
	  {
	  }
	  break;
	case AGS_SCRIPT_ARRAY_CHAR:
	  {
	    g_message("unsupported data type: gchar\0");
	  }
	  break;
	case AGS_SCRIPT_ARRAY_POINTER:
	  {
	    g_message("unsupported data type: gpointer\0");
	  }
	  break;
	}
      }
    }
    
    xmlNodeSetContent(current_index, g_base64_encode(index_data));
    free(index_data);

    xmlNodeSetContent(current_matrix, g_base64_encode(matrix_data));
    free(matrix_data);

    current_index = current_index->next;
    current_matrix = current_matrix->next;
  }
}

void
ags_script_set_matrix_sort_by_operands(AgsScriptSet *script_set,
				       AgsScriptArray *matrix,
				       AgsScriptArray *operands,
				       AgsScriptArray *index, guint depth,
				       guint *x, guint *y)
{

}


void
ags_script_set_matrix_vector_mirror(AgsScriptSet *script_set,
				    AgsScriptArray *matrix,
				    AgsScriptArray *vector)
{

}

void
ags_script_set_equation(AgsScriptSet *script_set,
			AgsScriptArray *factorized,
			AgsScriptArray *operands)
{

}

gboolean
ags_script_set_boolean_term(AgsScriptSet *script_set)
{
  gboolean term;

  term = FALSE;

  //TODO:JK: implement me

  return(term);
}

gint
ags_script_set_int_term(AgsScriptSet *script_set)
{
  gint term;

  term = -1;

  //TODO:JK: implement me

  return(term);
}

guint
ags_script_set_uint_term(AgsScriptSet *script_set)
{
  guint term;

  term = 0;

  //TODO:JK: implement me

  return(term);
}

gdouble
ags_script_set_double_term(AgsScriptSet *script_set)
{
  gdouble term;

  term = 1.0;

  //TODO:JK: implement me

  return(term);
}


AgsScriptSet*
ags_script_set_new()
{
  AgsScriptSet *script_set;

  script_set = (AgsScriptSet *) g_object_new(AGS_TYPE_SCRIPT_SET,
					     NULL);

  return(script_set);
}
