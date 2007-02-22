#include <gtk/gtk.h>


void
on_buttonVersion_clicked               (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonQuick_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonDownload_clicked              (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonFLash_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonStart_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonStartAU_clicked               (GtkButton       *button,
                                        gpointer         user_data);

void
on_treeviewVersions_row_activated      (GtkTreeView     *treeview,
                                        GtkTreePath     *path,
                                        GtkTreeViewColumn *column,
                                        gpointer         user_data);

gboolean
on_treeviewVersions_select_cursor_row  (GtkTreeView     *treeview,
                                        gboolean         start_editing,
                                        gpointer         user_data);

void
on_treeviewVersions_cursor_changed     (GtkTreeView     *treeview,
                                        gpointer         user_data);

void
on_treeviewVersions_row_activated      (GtkTreeView     *treeview,
                                        GtkTreePath     *path,
                                        GtkTreeViewColumn *column,
                                        gpointer         user_data);

void
on_download_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_find_usbprog_clicked                (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonFLash_clicked                 (GtkButton       *button,
                                        gpointer         user_data);
