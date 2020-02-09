#ifndef __SUPPORT_H__
#define __SUPPORT_H__
 
#include <gtk/gtk.h>
 
/* Convenience macros for obtaining objects from UI file */
#define CH_GET_OBJECT( builder, name, type, data ) \
    data->name = type( gtk_builder_get_object( builder, #name ) )
#define CH_GET_WIDGET( builder, name, data ) \
    CH_GET_OBJECT( builder, name, GTK_WIDGET, data )
 
/* Main data structure definition */
typedef struct _ChData ChData;
struct _ChData
{
    /* Widgets */
    GtkWidget *main_window;  /* Main application window */
};
 
#endif /* __SUPPORT_H__ */
