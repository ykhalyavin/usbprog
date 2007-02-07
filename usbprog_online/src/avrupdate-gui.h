#include <gtk/gtk.h>
#include <glib.h>
#include <glib/gprintf.h>

#include "../lib/avrupdate.h"

/* root widget */
GtkWidget *root;

void InitAVRUpdate(GtkWidget *parent);

char * LogBuffer;
void Log(char *msg);
void ClearLog();

int GetHex(unsigned char *s, int n);
