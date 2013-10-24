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

#include <ags/file/ags_file_util.h>

#include <ags/util/ags_id_generator.h>

#include <ags/file/ags_file_stock.h>
#include <ags/file/ags_file_id_ref.h>
#include <ags/file/ags_file_lookup.h>

void ags_file_util_read_value_resolve(AgsFileLookup *file_lookup,
				      GValue *value);
void ags_file_util_write_value_resolve(AgsFileLookup *file_lookup,
				       GValue *value);

void
ags_file_util_read_value(AgsFile *file,
			 xmlNode *node, gchar **id,
			 GValue *value, xmlChar **xpath)
{
  AgsFileLookup *file_lookup;
  xmlChar *type_str;
  xmlChar *value_str;
  xmlChar *content;
  GValue a = G_VALUE_INIT;

  if(id != NULL)
    *id = xmlGetProp(node, AGS_FILE_ID_PROP);

  type_str = xmlGetProp(node, AGS_FILE_TYPE_PROP);

  content = xmlNodeGetContent(node);

  if(!xmlStrcmp(type_str, "gboolean\0")){
    g_value_init(&a, G_TYPE_BOOLEAN);
    g_value_set_boolean(&a, g_ascii_strtoll(content, NULL, 10));

    if(value != NULL)
      g_value_copy(&a, value);
    else
      g_value_unset(&a);

    if(xpath != NULL)
      *xpath = NULL;
  }else if(!xmlStrcmp(type_str, "gchar\0")){
    g_value_init(&a, G_TYPE_CHAR);
    g_value_set_schar(&a, content[0]);

    if(value != NULL)
      g_value_copy(&a, value);
    else
      g_value_unset(&a);

    if(xpath != NULL)
      *xpath = NULL;
  }else if(!xmlStrcmp(type_str, "gint64\0")){
    g_value_init(&a, G_TYPE_INT64);
    g_value_set_int64(&a, g_ascii_strtoll(content, NULL, 10));

    if(value != NULL)
      g_value_copy(&a, value);
    else
      g_value_unset(&a);

    if(xpath != NULL)
      *xpath = NULL;
  }else if(!xmlStrcmp(type_str, "guint64\0")){
    g_value_init(&a, G_TYPE_UINT64);
    g_value_set_uint64(&a, g_ascii_strtoull(content, NULL, 10));

    if(value != NULL)
      g_value_copy(&a, value);
    else
      g_value_unset(&a);

    if(xpath != NULL)
      *xpath = NULL;
  }else if(!xmlStrcmp(type_str, "gdouble\0")){
    g_value_init(&a, G_TYPE_DOUBLE);
    g_value_set_double(&a, g_ascii_strtod(content, NULL));

    if(value != NULL)
      g_value_copy(&a, value);
    else
      g_value_unset(&a);

    if(xpath != NULL)
      *xpath = NULL;
  }else if(!xmlStrcmp(type_str, "gchar-pointer\0")){
    g_value_init(&a, G_TYPE_STRING);
    g_value_set_static_string(&a, content);

    if(value != NULL)
      g_value_copy(&a, value);
    else
      g_value_unset(&a);

    if(xpath != NULL)
      *xpath = NULL;
  }else if(!xmlStrcmp(type_str, "gboolean-pointer\0")){
    gchar **str_arr, **str_iter;
    gint64 *arr;
    guint i;

    str_arr = g_strsplit(content, " \0", -1);
    
    g_value_init(&a, G_TYPE_POINTER);

    arr = NULL;
    str_iter = str_arr;
    i = 0;

    while(*str_iter != NULL){
      if(arr == NULL){
	arr = (gboolean *) malloc(sizeof(gboolean));
      }else{
	arr = (gboolean *) realloc(arr, (i + 1) * sizeof(gboolean));
      }

      if(!xmlStrcmp(*str_iter, AGS_FILE_TRUE)){
	arr[i] = TRUE;
      }else{
	arr[i] = FALSE;
      }

      str_iter++;
      i++;
    }

    g_value_set_pointer(&a, (gpointer) arr);
    g_free(str_arr);

    if(value != NULL)
      g_value_copy(&a, value);
    else
      g_value_unset(&a);
    
    if(xpath != NULL)
      *xpath = NULL;
  }else if(!xmlStrcmp(type_str, "gint64-pointer\0")){
    gchar **str_arr, **str_iter;
    gint64 *arr;
    guint i;

    str_arr = g_strsplit(content, " \0", -1);
    
    g_value_init(&a, G_TYPE_POINTER);

    arr = NULL;
    str_iter = str_arr;
    i = 0;

    while(*str_iter != NULL){
      if(arr == NULL){
	arr = (gint64 *) malloc(sizeof(gint64));
      }else{
	arr = (gint64 *) realloc(arr, (i + 1) * sizeof(gint64));
      }

      arr[i] = g_ascii_strtoll(*str_iter, NULL, 10);

      str_iter++;
      i++;
    }
    
    g_value_set_pointer(&a, (gpointer) arr);
    g_free(str_arr);

    if(value != NULL)
      g_value_copy(&a, value);
    else
      g_value_unset(&a);
    
    if(xpath != NULL)
      *xpath = NULL;
  }else if(!xmlStrcmp(type_str, "guint64-pointer\0")){
    gchar **str_arr, **str_iter;
    guint64 *arr;
    guint i;

    str_arr = g_strsplit(content, " \0", -1);

    g_value_init(&a, G_TYPE_POINTER);

    arr = NULL;
    str_iter = str_arr;
    i = 0;

    while(*str_iter != NULL){
      if(arr == NULL){
	arr = (guint64 *) malloc(sizeof(guint64));
      }else{
	arr = (guint64 *) realloc(arr, (i + 1) * sizeof(guint64));
      }

      arr[i] = g_ascii_strtoll(*str_iter, NULL, 10);

      str_iter++;
      i++;
    }
    
    g_value_set_pointer(&a, (gpointer) arr);
    g_free(str_arr);

    if(value != NULL)
      g_value_copy(&a, value);
    else
      g_value_unset(&a);
    
    if(xpath != NULL)
      *xpath = NULL;
  }else if(!xmlStrcmp(type_str, "gpointer\0")){
    g_value_init(&a, G_TYPE_POINTER);

    g_value_set_pointer(&a, NULL);

    if(value != NULL)
      g_value_copy(&a, value);
    else
      g_value_unset(&a);

    /* gpointer */
    file_lookup = (AgsFileLookup *) g_object_new(AGS_TYPE_FILE_LOOKUP,
						 "file\0", file,
						 "node\0", node,
						 "reference\0", value,
						 NULL);
    ags_file_add_lookup(file, (GObject *) file_lookup);
    g_signal_connect(G_OBJECT(file_lookup), "resolve\0",
		     G_CALLBACK(ags_file_util_read_value_resolve), value);

    /* xpath */
    if(xpath != NULL)
      *xpath = content;
  }else if(!xmlStrcmp(type_str, "gobject\0")){
    g_value_init(&a, G_TYPE_OBJECT);

    g_value_set_object(&a, NULL);
    
    if(value != NULL)
      g_value_copy(&a, value);
    else
      g_value_unset(&a);

    /* GObject */
    file_lookup = (AgsFileLookup *) g_object_new(AGS_TYPE_FILE_LOOKUP,
						 "file\0", file,
						 "node\0", node,
						 "reference\0", value,
						 NULL);
    ags_file_add_lookup(file, (GObject *) file_lookup);
    g_signal_connect(G_OBJECT(file_lookup), "resolve\0",
		     G_CALLBACK(ags_file_util_read_value_resolve), value);

    /* xpath */
    if(xpath != NULL)
      *xpath = content;
  }else{
    g_warning("ags_file_util_read_value: unsupported type\0");
  }
}

void
ags_file_util_read_value_resolve(AgsFileLookup *file_lookup,
				 GValue *value)
{
  AgsFileIdRef *id_ref;
  gchar *xpath;

  xpath = (gchar *) xmlGetProp(file_lookup->node,
			       "link\0");

  id_ref = (AgsFileIdRef *) ags_file_find_id_ref_by_xpath(file_lookup->file, xpath);

  if(G_VALUE_HOLDS(value, G_TYPE_POINTER)){
    g_value_set_pointer(&value, (gpointer) id_ref->ref);
  }else if(G_VALUE_HOLDS(value, G_TYPE_OBJECT)){
    g_value_set_object(&value, (GObject *) id_ref->ref);
  }else{
    g_warning("ags_file_util_read_parameter_resolve: unknown type of GValue\0");
  }
}

xmlNode*
ags_file_util_write_value(AgsFile *file,
			  xmlNode *parent, gchar *id,
			  GValue *value, GType pointer_type)
{
  xmlNode *node;
  xmlChar *type;
  xmlChar *content;

  if(value == NULL){
    return;
  }

  node = xmlNewNode(AGS_FILE_DEFAULT_NS,
		    "ags-value\0");  
  xmlNewProp(node,
	     AGS_FILE_ID_PROP,
	     id);

  switch(G_VALUE_TYPE(value)){
  case G_TYPE_CHAR:
    {
      content = BAD_CAST g_strdup_printf("%c\0", g_value_get_schar(value));
    }
    break;
  case G_TYPE_INT64:
    {
      content = BAD_CAST g_strdup_printf("%lld\0", g_value_get_int64(value));
    }
    break;
  case G_TYPE_UINT64:
    {
      content = BAD_CAST g_strdup_printf("%lld\0", g_value_get_uint64(value));
    }
    break;
  case G_TYPE_DOUBLE:
    {
      content = BAD_CAST g_strdup_printf("%lf\0", g_value_get_double(value));
    }
    break;
  case G_TYPE_STRING:
    {
      content = BAD_CAST g_strdup_printf("%s\0", g_value_get_string(value));
    }
    break;
  case G_TYPE_POINTER:
    {
      //TODO:JK: implement me
    }
    break;
  case G_TYPE_OBJECT:
    {
      //TODO:JK: implement me
    }
    break;
  default:
    g_warning("ags_file_util_write_value\0");
  }

  xmlNodeSetContent(node, content);
  xmlAddChild(parent,
	      node);

  return(node);
}

void
ags_file_util_write_value_resolve(AgsFileLookup *file_lookup,
				  GValue *value)
{
  //TODO:JK: implement me
}

void
ags_file_util_read_parameter(AgsFile *file,
			     xmlNode *node, gchar **id,
			     GParameter **parameter, gint *n_params, xmlChar ***xpath)
{
  xmlNode *child;
  GParameter *parameter_arr;
  gchar **name_arr, **name_iter;
  gchar **xpath_arr;
  guint i;

  name_arr = g_strsplit(xmlGetProp(node, AGS_FILE_NAME_PROP), " \0", -1);

  parameter_arr = NULL;
  name_iter = name_arr;
  xpath_arr = NULL;
  i = 0;
  
  child = node->children;

  while(name_iter != NULL){
    if(parameter_arr == NULL){
      parameter_arr = (GParameter *) g_new(GParameter, 1);
      xpath_arr = (xmlChar **) malloc(sizeof(xmlChar *));
    }else{
      parameter_arr = (GParameter *) g_renew(GParameter, parameter_arr, i + 1);
      xpath_arr = (xmlChar **) realloc(xpath_arr, (i + 1) * sizeof(xmlChar *));
    }

    parameter_arr[i].name = name_arr[i];
    ags_file_util_read_value(file,
			     child, NULL,
			     &(parameter_arr[i].value), &(xpath_arr[i]));

    child = child->next;
    name_iter++;
    i++;
  }

  g_free(name_arr);
  
  if(id != NULL)
    *id = (gchar *) xmlGetProp(node, AGS_FILE_ID_PROP);

  if(parameter != NULL)
    *parameter = parameter_arr;
  else
    g_free(parameter);

  if(n_params != NULL)
    *n_params = i;

  if(xpath != NULL)
    *xpath = xpath_arr;
  else
    free(xpath_arr);
}

xmlNode*
ags_file_util_write_parameter(AgsFile *file,
			      xmlNode *parent, gchar *id,
			      GParameter *parameter, gint n_params)
{
  xmlNode *node;
  xmlChar *name, *old_name;
  gchar *child_id;
  guint i;

  if(parameter == NULL){
    return;
  }

  node = xmlNewNode(AGS_FILE_DEFAULT_NS,
		    "ags-parameter\0");  
  xmlNewProp(node,
	     AGS_FILE_ID_PROP,
	     id);

  name = NULL;

  for(i = 0; i < n_params; i++){
    if(name == NULL){
      name = g_strdup(parameter[i].name);
    }else{
      old_name = name;
      name = (xmlChar *) g_strdup_printf("%s %s\0", name, parameter[i].name);
      free(old_name);
    }

    child_id = ags_id_generator_create_uuid();

    ags_file_util_write_value(file,
			      node, child_id,
			      &(parameter[i].value), G_TYPE_NONE);
  }

  xmlNewProp(node,
	     AGS_FILE_NAME_PROP,
	     name);

  xmlAddChild(parent,
	      node);

  return(node);
}

void
ags_file_util_read_dependency(AgsFile *file,
			      xmlNode *node, gchar **id,
			      gchar **name, xmlChar **xpath)
{
  *id = xmlGetProp(node, AGS_FILE_ID_PROP);
  *name = xmlGetProp(node, AGS_FILE_NAME_PROP);
  *xpath = xmlGetProp(node, AGS_FILE_XPATH_PROP);
}

xmlNode*
ags_file_util_write_dependency(AgsFile *file,
			       xmlNode *parent, gchar *id,
			       gchar *name, xmlChar *xpath)
{
  xmlNode *node;

  node = xmlNewNode(AGS_FILE_DEFAULT_NS,
		    "ags-dependency\0");
  xmlNewProp(node,
	     AGS_FILE_ID_PROP,
	     id);

  xmlNewProp(node,
	     AGS_FILE_NAME_PROP,
	     name);

  xmlNewProp(node,
	     AGS_FILE_XPATH_PROP,
	     xpath);

  xmlAddChild(parent,
	      node);
  
  return(node);
}

void
ags_file_read_history(AgsFile *file, xmlNode *node, AgsHistory **history)
{
  //TODO:JK: implement me
}

xmlNode*
ags_file_write_history(AgsFile *file, xmlNode *parent, AgsHistory *history)
{
  //TODO:JK: implement me
}

void
ags_file_read_embedded_audio(AgsFile *file, xmlNode *node, AgsEmbeddedAudio **embedded_audio)
{
  //TODO:JK: implement me
}

xmlNode*
ags_file_write_embedded_audio(AgsFile *file, xmlNode *parent, AgsEmbeddedAudio *embedded_audio)
{
  //TODO:JK: implement me
}

void
ags_file_read_embedded_audio_list(AgsFile *file, xmlNode *node, GList **embedded_audio)
{
  //TODO:JK: implement me
}

xmlNode*
ags_file_write_embedded_audio_list(AgsFile *file, xmlNode *parent, GList *embedded_audio)
{
  //TODO:JK: implement me
}

void
ags_file_read_file_link(AgsFile *file, xmlNode *node, AgsFileLink **file_link)
{
  //TODO:JK: implement me
}

xmlNode*
ags_file_write_file_link(AgsFile *file, xmlNode *parent, AgsFileLink *file_link)
{
  //TODO:JK: implement me
}

void
ags_file_read_file_link_list(AgsFile *file, xmlNode *node, GList **file_link)
{
  //TODO:JK: implement me
}

xmlNode*
ags_file_write_file_link_list(AgsFile *file, xmlNode *parent, GList *file_link)
{
  //TODO:JK: implement me
}
