/* gtk-v4l - GTK tool for control v4l camera properties
 *
 * Copyright (C) 2010 - Huzaifa Sidhpurwala <huzaifas@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 *
 */



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
