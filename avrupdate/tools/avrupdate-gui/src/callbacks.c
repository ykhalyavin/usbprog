#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"


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
	char * url = "http://www.ixbat.de/versions.conf";
	avrupdate_net_versions(url);
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

