#include <gtk/gtk.h>

void show_error_dialog(gchar *error)
{
	GtkDialog *dialog;
	dialog = gtk_message_dialog_new( NULL,
					 GTK_DIALOG_DESTROY_WITH_PARENT,
					 GTK_MESSAGE_ERROR,
					 GTK_BUTTONS_CLOSE,
					 (const gchar *)error );
	gtk_dialog_run (dialog);
	gtk_widget_destroy (GTK_WIDGET( dialog));
}
