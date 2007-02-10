#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"

#include "../lib/avrupdate.h"

int selectedVersion=0;

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
on_buttonFLash_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
	
	GtkWidget *treeviewVersions;
	treeviewVersions = lookup_widget(GTK_WIDGET(button),  "treeviewVersions");
	
	GtkWidget *entryFile;
	entryFile    = lookup_widget(GTK_WIDGET(button),  "entryFile");

	gchar *url = gtk_entry_get_text(GTK_ENTRY(entryFile));

	GtkWidget *entryPID, *entryVID;
	entryPID    = lookup_widget(GTK_WIDGET(button),  "entryPID");
	entryVID    = lookup_widget(GTK_WIDGET(button),  "entryVID");

	int pid,vid;
	pid = GetHex(gtk_entry_get_text(GTK_ENTRY(entryPID)),4);
	vid = GetHex(gtk_entry_get_text(GTK_ENTRY(entryVID)),4);
	
	// change to update mode 		
	avrupdate_start_with_vendor_request(vid, pid);
	wait(2);

	GtkWidget *entryPIDa, *entryVIDa;
	entryPIDa    = lookup_widget(GTK_WIDGET(button),  "entryPIDa");
	entryVIDa    = lookup_widget(GTK_WIDGET(button),  "entryVIDa");

	pid = GetHex(gtk_entry_get_text(GTK_ENTRY(entryPIDa)),4);
	vid = GetHex(gtk_entry_get_text(GTK_ENTRY(entryVIDa)),4);


	avrupdate_net_flash_version(url,selectedVersion,vid,pid);

}


void
on_buttonStart_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
	struct usb_dev_handle* usb_handle;
	
	GtkWidget *entryPIDa, *entryVIDa;
	entryPIDa    = lookup_widget(GTK_WIDGET(button),  "entryPIDa");
	entryVIDa    = lookup_widget(GTK_WIDGET(button),  "entryVIDa");

	int pid,vid;
	pid = GetHex(gtk_entry_get_text(GTK_ENTRY(entryPIDa)),4);
	vid = GetHex(gtk_entry_get_text(GTK_ENTRY(entryVIDa)),4);



    usb_handle = avrupdate_open(vid,pid);
	avrupdate_startapp(usb_handle);
	avrupdate_close(usb_handle);
}


void
on_buttonStartAU_clicked               (GtkButton       *button,
                                        gpointer         user_data)
{
	struct usb_dev_handle* usb_handle;
	
	GtkWidget *entryPID, *entryVID;
	entryPID    = lookup_widget(GTK_WIDGET(button),  "entryPID");
	entryVID    = lookup_widget(GTK_WIDGET(button),  "entryVID");

	int pid,vid;
	pid = GetHex(gtk_entry_get_text(GTK_ENTRY(entryPID)),4);
	vid = GetHex(gtk_entry_get_text(GTK_ENTRY(entryVID)),4);


	//g_print("dvid %i pid %s\n",dvid,pid);


	avrupdate_start_with_vendor_request(vid, pid);
}


void
on_treeviewVersions_cursor_changed     (GtkTreeView     *treeview,
                                        gpointer         user_data)
{
	GtkTreeSelection *selection;
	selection = gtk_tree_view_get_selection(treeview);

	//gtk_tree_selection_get_user_data(selection);

		GtkTreeIter iter;
        GtkTreeModel *model;
		int * version;

if (gtk_tree_selection_get_selected (selection, &model, &iter))
{
	gtk_tree_model_get (model, &iter, 0, &version, -1);
	selectedVersion = version; 
	selectedVersion--; 
}
	
}


void
on_treeviewVersions_row_activated      (GtkTreeView     *treeview,
                                        GtkTreePath     *path,
                                        GtkTreeViewColumn *column,
                                        gpointer         user_data)
{
	//g_print("flash file %i",selectedVersion);
	on_buttonFLash_clicked((GtkButton*)treeview,user_data);

}


void
on_download_clicked                    (GtkButton       *button,
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
	list = gtk_list_store_new(4, G_TYPE_INT,G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
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
		0, i+1,
		1, tmp->title,
		2, tmp->version,
		3, tmp->description,
		-1);
	}

	gtk_tree_view_set_model(treeviewVersions, list);

}


void
on_find_usbprog_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{

	GtkWidget *entryVID,*entryPID, *version;
	entryVID = lookup_widget(GTK_WIDGET(button),  "entryVID");
	entryPID = lookup_widget(GTK_WIDGET(button),  "entryPID");
	
	version = lookup_widget(GTK_WIDGET(button),  "version");
	
	gchar vid[4];
	gchar pid[4];

  int device = avrupdate_find_usbdevice();

	switch(device)
	{
		case AVRISPMKII:
			gtk_entry_set_text(GTK_ENTRY(entryPID), "2104");
			gtk_entry_set_text(GTK_ENTRY(entryVID), "03eb");
			gtk_label_set_text(GTK_LABEL(version), "AVRISP mkII Klon");
		break;

		case USBPROG:
			gtk_entry_set_text(GTK_ENTRY(entryPID), "0c62");
			gtk_entry_set_text(GTK_ENTRY(entryVID), "1781");
			gtk_label_set_text(GTK_LABEL(version), "usbprog (Benes ISP + RS232 Interface)");
		break;
			
		case AVRUPDATE:
			gtk_entry_set_text(GTK_ENTRY(entryPID), "0c62");
			gtk_entry_set_text(GTK_ENTRY(entryVID), "1781");
			gtk_label_set_text(GTK_LABEL(version), "usprog Adapter with no firmware");
		break;
			
		case BLINKDEMO:
			gtk_entry_set_text(GTK_ENTRY(entryPID), "0c62");
			gtk_entry_set_text(GTK_ENTRY(entryVID), "1781");
			gtk_label_set_text(GTK_LABEL(version), "Blink Demo");
		break;

		default:
			gtk_entry_set_text(GTK_ENTRY(entryPID), "?");
			gtk_entry_set_text(GTK_ENTRY(entryVID), "?");
			gtk_label_set_text(GTK_LABEL(version), "Version: ???");
	}
		
}


