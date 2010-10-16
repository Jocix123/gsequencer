#include "ags_property_collection_editor_callbacks.h"

#include "../object/ags_connectable.h"

void
ags_property_collection_editor_add_collection_callback(GtkButton *button,
						       AgsPropertyCollectionEditor *property_collection_editor)
{
  GtkTable *table;
  GtkWidget *collection;
  GtkButton *remove_collection;
  GtkAlignment *alignment;

  table = (GtkTable *) gtk_table_new(2, 2, FALSE);
  gtk_box_pack_start(GTK_BOX(property_collection_editor->child),
		     GTK_WIDGET(table),
		     FALSE, FALSE,
		     0);
  
  collection = (GtkWidget *) g_object_newv(property_collection_editor->child_type,
					   property_collection_editor->child_parameter_count,
					   property_collection_editor->child_parameter);
  g_object_set_data(G_OBJECT(table), "AgsChild\0", collection);
  gtk_table_attach(table,
		   GTK_WIDGET(collection),
		   0, 2,
		   0, 1,
		   GTK_FILL, GTK_FILL,
		   0, 0);

  alignment = (GtkAlignment *) gtk_alignment_new(1.0, 0.0,
						 0.0, 0.0);
  gtk_table_attach(table,
		   GTK_WIDGET(alignment),
		   1, 2,
		   1, 2,
		   GTK_FILL, 0,
		   0, 0);

  remove_collection = (GtkButton *) gtk_button_new_from_stock(GTK_STOCK_REMOVE);
  gtk_container_add(GTK_CONTAINER(alignment),
		    GTK_WIDGET(remove_collection));
  g_signal_connect_after(G_OBJECT(remove_collection), "clicked\0",
			 G_CALLBACK(ags_property_collection_editor_remove_collection_callback), table);

  gtk_table_set_row_spacing(table, 0, 8);

  ags_connectable_connect(AGS_CONNECTABLE(collection));

  gtk_widget_show_all(GTK_WIDGET(table));
}

void
ags_property_collection_editor_remove_collection_callback(GtkButton *button,
							  GtkTable *table)
{
  gtk_widget_destroy(GTK_WIDGET(table));
}
