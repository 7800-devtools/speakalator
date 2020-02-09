#include "support.h"
#include "serial.h"
#include "dictionary.h"
#include <stdio.h>
#include <stdlib.h>

#include <gtk/gtk.h>
 
#define UI_FILE "speakalator.glade"


// Uncomment the line below, if you want the program to load the glade
// resource file at runtime. It will need to be in the working-directory
// of the programe, so for convenience we embed it by default.
//#define LOADGLADEATRUNTIME 1 

#ifndef LOADGLADEATRUNTIME
#include "gladeui.h" // an embedded version of the glade file
#define UI_STRING_LEN 500000
#endif

GtkWidget *textview_saydata;
GtkWidget *entry_delay;
GtkWidget *entry_volume;
GtkWidget *entry_speed;
GtkWidget *entry_pitch;
GtkWidget *entry_bend;
GtkWidget *entry_repeat;
GtkWidget *entry_dictionary;
GtkDialog *dialog_about;
GtkDialog *dialog_viewdata;
GtkDialog *dialog_settings;
GtkWidget *label_datatype;
GtkWidget *textview_speakdata;
GtkWidget *entry_serialdev;
GtkWidget *radiobutton_19200;
GtkWidget *radiobutton_9600;
GtkWidget *radiobutton_4800;
GtkWidget *radiobutton_2400;
 
int main( int argc, char **argv )
{
	ChData     *data;
	GtkBuilder *builder;
	GError     *error = NULL;

	open_port();
	LoadDictionary();
 
	// Init GTK+
	gtk_init( &argc, &argv );
 
	// Create new GtkBuilder object
	builder = gtk_builder_new();
#ifdef LOADGLADEATRUNTIME
	if( ! gtk_builder_add_from_file( builder, UI_FILE, &error ) )
#else
	if( ! gtk_builder_add_from_string( builder, (const gchar *)gladeui, -1 , &error ) )
#endif
	{
		g_warning( "%s", error->message );
		g_free( error );
		return( 1 );
	}

 
	// Allocate data structure
	data = g_slice_new( ChData );
 
	CH_GET_WIDGET( builder, main_window, data );

	// Grab some widget handles while the builder is still alive... 
	textview_saydata = GTK_WIDGET (gtk_builder_get_object (builder,"textview_saydata")); 
	entry_delay = GTK_WIDGET (gtk_builder_get_object (builder,"entry_delay")); 
	entry_volume = GTK_WIDGET (gtk_builder_get_object (builder,"entry_volume")); 
	entry_speed = GTK_WIDGET (gtk_builder_get_object (builder,"entry_speed")); 
	entry_pitch = GTK_WIDGET (gtk_builder_get_object (builder,"entry_pitch")); 
	entry_bend = GTK_WIDGET (gtk_builder_get_object (builder,"entry_bend")); 
	entry_repeat = GTK_WIDGET (gtk_builder_get_object (builder,"entry_repeat")); 
	entry_dictionary = GTK_WIDGET (gtk_builder_get_object (builder,"entry_dictionary")); 
	dialog_about = GTK_DIALOG (gtk_builder_get_object (builder,"dialog_about")); 
	dialog_viewdata = GTK_DIALOG (gtk_builder_get_object (builder,"dialog_viewdata")); 
	dialog_settings = GTK_DIALOG (gtk_builder_get_object (builder,"dialog_settings")); 
	label_datatype = GTK_WIDGET (gtk_builder_get_object (builder,"label_datatype")); 
	textview_speakdata = GTK_WIDGET (gtk_builder_get_object (builder,"textview_speakdata")); 
	entry_serialdev = GTK_WIDGET (gtk_builder_get_object (builder,"entry_serialdev")); 
	radiobutton_19200 = GTK_WIDGET (gtk_builder_get_object (builder,"radiobutton_19200")); 
	radiobutton_9600 = GTK_WIDGET (gtk_builder_get_object (builder,"radiobutton_9600")); 
	radiobutton_4800 = GTK_WIDGET (gtk_builder_get_object (builder,"radiobutton_4800")); 
	radiobutton_2400 = GTK_WIDGET (gtk_builder_get_object (builder,"radiobutton_2400")); 

	// Connect signals
	gtk_builder_connect_signals( builder, data );
 
	// Destroy builder, since we don't need it anymore
	g_object_unref( G_OBJECT( builder ) );

	// Show window. All other widgets are automatically shown by GtkBuilder
	gtk_widget_show( data->main_window );

	// Start main loop
	gtk_main();
 
	// Free any allocated data
	g_slice_free( ChData, data );

	// Free the dictionary memory
	UnloadDictionary();
 
	return( 0 );
}

