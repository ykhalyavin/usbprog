#include "avrupdate-gui.h"



void InitAVRUpdate(GtkWidget *parent)
{
	root = parent;	

	//default values
	GtkWidget *entryFile;
	entryFile = lookup_widget(GTK_WIDGET(root),  "entryFile");
	char * url = "http://www.ixbat.de/usbprog/versions.conf";
	gtk_entry_set_text(GTK_ENTRY(entryFile), url);

	GtkWidget *entryVID,*entryPID,*entryVIDa,*entryPIDa;
	entryPID = lookup_widget(GTK_WIDGET(root),  "entryPID");
	entryPIDa = lookup_widget(GTK_WIDGET(root),  "entryPIDa");
	entryVID = lookup_widget(GTK_WIDGET(root),  "entryVID");
	entryVIDa = lookup_widget(GTK_WIDGET(root),  "entryVIDa");

	char *pid = "c35d";
	char *pida = "9876";
	char *vid = "0400";
	char *vida = "0400";

	
	gtk_entry_set_text(GTK_ENTRY(entryPID), pid);
	gtk_entry_set_text(GTK_ENTRY(entryPIDa), pida);
	gtk_entry_set_text(GTK_ENTRY(entryVID), vid);
	gtk_entry_set_text(GTK_ENTRY(entryVIDa), vida);


	/* setup treeview table */

	
	GtkWidget *treeviewVersions;
	treeviewVersions = lookup_widget(GTK_WIDGET(root),  "treeviewVersions");


	GtkCellRenderer *text_renderer;
	GtkTreeViewColumn *title_column, *version_column, *description_column, *nr_column;

	/* create and initialize text renderer for cells */
	text_renderer = gtk_cell_renderer_text_new();

	/* create column views */
	nr_column = gtk_tree_view_column_new_with_attributes("Nr.",
		text_renderer,
		"text",
		0,
		NULL);


	g_object_set(nr_column,
		"resizable", TRUE,
		"clickable", TRUE,
		"reorderable", TRUE,
		NULL);


	/* create column views */
	title_column = gtk_tree_view_column_new_with_attributes("Application",
		text_renderer,
		"text",
		1,
		NULL);


	g_object_set(title_column,
		"resizable", TRUE,
		"clickable", TRUE,
		"reorderable", TRUE,
		NULL);

	version_column = gtk_tree_view_column_new_with_attributes("Vers.",
		text_renderer,
		"text",
		2,
		NULL);


	g_object_set(version_column,
		"resizable", TRUE,
		"clickable", TRUE,
		"reorderable", TRUE,
		NULL);

	description_column = gtk_tree_view_column_new_with_attributes("Description",
		text_renderer,
		"text",
		3,
		NULL);


	g_object_set(description_column,
		"resizable", TRUE,
		"clickable", TRUE,
		"reorderable", TRUE,
		NULL);





	/* insert columns into the view */
	gtk_tree_view_append_column(treeviewVersions, nr_column);
	gtk_tree_view_append_column(treeviewVersions, title_column);
	gtk_tree_view_append_column(treeviewVersions, version_column);
	gtk_tree_view_append_column(treeviewVersions, description_column);

 	GtkListStore *list;

	list = gtk_list_store_new(4, G_TYPE_INT,G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
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


int GetHex(unsigned char *s, int n) {

	int i;
	int l = strlen( s );
	int j, k;
	unsigned char 
		hexit[]={'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
	unsigned char c;
	unsigned int x = 0;
	k = -1;
	for ( i = 0; ( i < n ) && (i < l); i++ ) {
		c = s[i];
		for ( j = 0; ( k == -1 ) && ( j < 10 ); j++ )
			if ( c == hexit[j] ) k = j;
		c |= 32; // lower case
		for ( j = 10; ( k == -1 ) && ( j < 16 ); j++ )
			if ( c == hexit[j] ) k = j;
		if ( k == -1 ) {
			printf("HEX-Error in MAC!\n");
			return -1;
		}
		x <<= 4;
		x |= k;
		k = -1;
	}

 	return x;
}
