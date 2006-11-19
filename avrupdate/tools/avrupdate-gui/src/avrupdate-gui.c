#include "avrupdate-gui.h"



void InitAVRUpdate(GtkWidget *parent)
{
	root = parent;	
	
	//default values
	GtkWidget *entryFile;
	entryFile = lookup_widget(GTK_WIDGET(root),  "entryFile");
	char * url = "http://www.ixbat.de/versions.conf";
	gtk_entry_set_text(GTK_ENTRY(entryFile), url);





	/* setup treeview table */

	
	GtkWidget *treeviewVersions;
	treeviewVersions = lookup_widget(GTK_WIDGET(root),  "treeviewVersions");


	GtkCellRenderer *text_renderer;
	GtkTreeViewColumn *title_column, *version_column, *description_column;

	/* create and initialize text renderer for cells */
	text_renderer = gtk_cell_renderer_text_new();

	/* create column views */
	title_column = gtk_tree_view_column_new_with_attributes("Application",
		text_renderer,
		"text",
		0,
		NULL);


	g_object_set(title_column,
		"resizable", TRUE,
		"clickable", TRUE,
		"reorderable", TRUE,
		NULL);

	version_column = gtk_tree_view_column_new_with_attributes("Vers.",
		text_renderer,
		"text",
		1,
		NULL);


	g_object_set(version_column,
		"resizable", TRUE,
		"clickable", TRUE,
		"reorderable", TRUE,
		NULL);

	description_column = gtk_tree_view_column_new_with_attributes("Description",
		text_renderer,
		"text",
		2,
		NULL);


	g_object_set(description_column,
		"resizable", TRUE,
		"clickable", TRUE,
		"reorderable", TRUE,
		NULL);





	/* insert columns into the view */
	gtk_tree_view_append_column(treeviewVersions, title_column);
	gtk_tree_view_append_column(treeviewVersions, version_column);
	gtk_tree_view_append_column(treeviewVersions, description_column);

 	GtkListStore *list;

	list = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	gtk_tree_view_set_model(treeviewVersions, list);
}


void Log(char *msg)
{
	GtkWidget *textviewLog;
	textviewLog    = lookup_widget(GTK_WIDGET(root),  "textviewLog");

	if(LogBuffer==NULL){
		LogBuffer=(char *) malloc(strlen(msg)+1);     
		g_stpcpy(LogBuffer,msg);
	}
	
	int size  = strlen(LogBuffer)+strlen(msg)+1;
	g_print("size %i\n",size);

	/* get new bigger memory */
	char * tmp = (char *) malloc(size);

	/* copy old to new memory */
	g_stpcpy(tmp,LogBuffer);
	g_strlcat(tmp,msg,size);

	/* free old memory */
	free(LogBuffer);	
	/* link buffer to new memory */
	LogBuffer = tmp;

	gtk_text_buffer_set_text(gtk_text_view_get_buffer (GTK_TEXT_VIEW (textviewLog)),
		LogBuffer, -1);

}


void ClearLog(){

}


