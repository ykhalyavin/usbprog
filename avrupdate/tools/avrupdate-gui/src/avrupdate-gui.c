#include "avrupdate-gui.h"



void InitAVRUpdate(GtkWidget *parent)
{
	root = parent;	
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


