/* gtk-v4l - GTK tool for control v4l camera properties
 *
 * Copyright (C) 2010 - Huzaifa Sidhpurwala <huzaifas@redhat.com>
 * Copyright (C) 2010 - Hans de Goede <hdegoede@redhat.com>
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
 */

#include <gtk-v4l-widget.h>
#include <gtk-v4l-device-list.h>

#define ICON_LOC "/usr/share/icons/gnome/24x24/devices/camera-web.png"

GtkTable *main_table = NULL;
GtkWidget *controls = NULL;
Gtkv4lDeviceList *devlist = NULL;
GtkWidget *default_button;


static void show_error_dialog (const gchar *error)
{
  GtkWidget *dialog;
  dialog = gtk_message_dialog_new (NULL,
                                   GTK_DIALOG_DESTROY_WITH_PARENT,
                                   GTK_MESSAGE_ERROR,
                                   GTK_BUTTONS_OK, 
                                   "%s", error);

  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);
}

void close_cb (GtkWidget *w, gpointer user_data)
{
  gtk_main_quit();
}

void reset_cb (GtkButton *button, gpointer user_data)
{
  gpointer *widgetp = user_data;
  Gtkv4lWidget *widget = GTK_V4L_WIDGET (*widgetp);
  
  gtk_v4l_widget_reset_to_defaults (widget);
}

static void
io_error_cb (Gtkv4lWidget *widget,
             const gchar *error_msg,
             gpointer user_data)
{
  show_error_dialog (error_msg);
}

void
v4l2_combo_add_device(Gtkv4lDeviceList *devlist,
                      guint idx,
                      Gtkv4lDevice *device,
                      gpointer user_data)
{
  gint active;
  GtkComboBox *combo = GTK_COMBO_BOX(user_data);

  gtk_combo_box_text_insert_text (combo, idx, device->card);
  active = gtk_combo_box_get_active (combo);
  if (active == -1)
  {
    gtk_combo_box_set_active (combo, idx);
    if ( gtk_widget_get_sensitive (GTK_WIDGET(default_button)) == FALSE)
      gtk_widget_set_sensitive (default_button, TRUE);
  }
}

void
v4l2_combo_remove_device(Gtkv4lDeviceList *devlist,
                         guint idx,
                         Gtkv4lDevice *device,
                         gpointer user_data)
{
  GtkComboBox *combo = GTK_COMBO_BOX (user_data);
  /* If this removes the current device
     v4l2_combo_change_device_cb() will get called and that will
     handle selecting a new device. */
  gtk_combo_box_text_remove (combo, idx);
  if ((gtk_tree_model_iter_n_children( gtk_combo_box_get_model (combo),NULL)) == 0)
    gtk_widget_set_sensitive (default_button, FALSE);
}

void v4l2_combo_change_device_cb (GtkWidget *combo, gpointer user_data)
{
  gint active;

  if (controls) {
    gtk_widget_destroy(controls);
    controls = NULL;
  }

  active = gtk_combo_box_get_active (GTK_COMBO_BOX (combo));
  /* This happens when our current device gets unplugged */
  if (active == -1) {
    /* Just select the first one in the list */
    if (g_list_length (devlist->list) > 0) 
      gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 0);
    return;
  }

  controls = gtk_v4l_widget_new (g_list_nth_data (devlist->list, active));
  g_signal_connect (controls, "io_error", G_CALLBACK (io_error_cb), NULL);
  
  gtk_table_attach (GTK_TABLE (main_table), controls, 0, 2, 2, 3, GTK_FILL, GTK_FILL, 0, 5);
  gtk_widget_show_all (controls);
}

int main(int argc, char *argv[])
{
  gchar *device_file = NULL;
  GOptionEntry entries[] = {
    { "device", 'd', 0, G_OPTION_ARG_STRING, &device_file, "V4L2 device", NULL },
    { NULL }
  };
  GError *error = NULL;
  GdkPixbuf *icon_pixbuf = NULL;
  GtkWidget *window, *content_area, *label, *align, *sep,*button, *dev_combo;

  GOptionContext* context = g_option_context_new("- Gtk V4l app");
  g_option_context_add_main_entries (context,entries, NULL);
  g_option_context_add_group (context, gtk_get_option_group (TRUE));
  g_option_context_parse (context, &argc, &argv, &error);

  gtk_init (&argc, &argv);

  window = gtk_dialog_new();
  gtk_window_set_title (GTK_WINDOW (window), "GTK V4L Device properties");
  gtk_window_set_resizable (GTK_WINDOW (window), FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (window), 7);
  g_signal_connect(G_OBJECT (window), "destroy", G_CALLBACK (close_cb), NULL);

  if(g_file_test(ICON_LOC,G_FILE_TEST_EXISTS)) {
    icon_pixbuf = gdk_pixbuf_new_from_file(ICON_LOC, NULL);
    if(icon_pixbuf)
      gtk_window_set_icon(GTK_WINDOW(window),icon_pixbuf);
  } else {
    g_warning ("Window icon not found");
  }
  content_area = gtk_dialog_get_content_area (GTK_DIALOG (window));

  main_table = GTK_TABLE(gtk_table_new (3, 2, FALSE));

  label = gtk_label_new ("Device");
  align = gtk_alignment_new (0.0, 0.5, 0.0, 0.0);
  gtk_container_add (GTK_CONTAINER (align), label);
  gtk_table_attach (GTK_TABLE (main_table), align, 0, 1, 0, 1, GTK_FILL, GTK_FILL, 5, 5);

  dev_combo = gtk_combo_box_text_new ();
  g_signal_connect (G_OBJECT (dev_combo), "changed", G_CALLBACK (v4l2_combo_change_device_cb), NULL);
  align = gtk_alignment_new (0.0, 0.5, 0.0, 0.0);
  gtk_container_add (GTK_CONTAINER (align), dev_combo);
  gtk_table_attach (GTK_TABLE (main_table), align, 1, 2, 0, 1, GTK_FILL, GTK_FILL, 5, 5);

  sep = gtk_hseparator_new();
  gtk_table_attach (GTK_TABLE (main_table), sep, 0, 2, 1, 2, GTK_FILL, GTK_FILL, 0, 0);

  gtk_container_add (GTK_CONTAINER (content_area), GTK_WIDGET (main_table));


  default_button = gtk_dialog_add_button (GTK_DIALOG (window), "_Defaults", GTK_RESPONSE_APPLY);
  g_signal_connect(G_OBJECT (default_button), "clicked", G_CALLBACK (reset_cb), &controls);

  button = gtk_dialog_add_button (GTK_DIALOG (window), GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE);
  g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (close_cb), NULL);

  devlist = g_object_new (GTK_V4L_TYPE_DEVICE_LIST, NULL);
  g_signal_connect (G_OBJECT (devlist), "device_added", G_CALLBACK (v4l2_combo_add_device), dev_combo);
  g_signal_connect (G_OBJECT (devlist), "device_removed", G_CALLBACK (v4l2_combo_remove_device), dev_combo);
  gtk_v4l_device_list_coldplug (devlist);

  if (device_file)
  {
    Gtkv4lDevice *device;
    device = gtk_v4l_device_list_get_dev_by_device_file (devlist, device_file);
    if (device)
      gtk_combo_box_set_active (GTK_COMBO_BOX (dev_combo),
                                g_list_index (devlist->list, device));
    else
      show_error_dialog ("Specified video device not found");
  } else if (g_list_length (devlist->list) == 0)
  {
    show_error_dialog ("No video devices found");
    gtk_widget_set_sensitive (default_button, FALSE);    
  }
  gtk_widget_show_all (window);

  gtk_main();

  return 0;
}
