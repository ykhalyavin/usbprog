#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"

#include "../../../lib/avrupdate.h"


void
on_buttonVersion_clicked               (GtkButton       *button,
                                        gpointer         user_data)
{

	GtkWidget *entryPID, *textviewLog;
	entryPID    = lookup_widget(GTK_WIDGET(button),  "entryPID");
	textviewLog    = lookup_widget(GTK_WIDGET(button),  "textviewLog");

	gchar *name;
	name = gtk_entry_get_text(GTK_ENTRY(entryPID));
	g_print("Name: %s\n",    name);

	char      *txtBuffer  = name;
	gtk_text_buffer_set_text(gtk_text_view_get_buffer (GTK_TEXT_VIEW (textviewLog)),
	                             txtBuffer, -1);

	Log("hissa\n");
	Log("cool\n");
}


void
on_buttonQuick_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_buttonDownload_clicked              (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget *entryFile;
	entryFile    = lookup_widget(GTK_WIDGET(button),  "entryFile");

	gchar *url = gtk_entry_get_text(GTK_ENTRY(entryFile));
	int versions = avrupdate_net_versions(url);

	
	/* update List */
	GtkWidget *treeviewVersions;
	GtkListStore *list;
	GtkTreeIter iter;

	treeviewVersions = lookup_widget(GTK_WIDGET(button),  "treeviewVersions");

	/* create a two-column list */
	list = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	gtk_list_store_clear   (list);

	int i;

	struct avrupdate_info *tmp;
	/* put some data into the list */
	for (i = 0; i < versions; i++)
	{
		tmp = avrupdate_net_get_version_info(url,i);
		/* remove linebreak */
		g_strdelimit  (tmp->description,"\n",0x00);

		gtk_list_store_append(list, &iter);
		gtk_list_store_set(list, &iter,
		0, tmp->title,
		1, tmp->version,
		2, tmp->description,
		-1);
	}

	gtk_tree_view_set_model(treeviewVersions, list);
}


void
on_buttonFLash_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_buttonStart_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{

}

