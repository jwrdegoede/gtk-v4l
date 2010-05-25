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
 *
 *
 */


#include "gtk-v4l-widget.h"
#include "gtk-v4l-device-list.h"

#define ICON_LOC "/usr/share/icons/gnome/24x24/devices/camera-web.png"

Gtkv4lDeviceList *devlist;
Gtkv4lDevice *curr_dev = NULL;
GtkWidget *window,*dev_combo;
GtkTable *main_table;
GtkWidget *content_area;
GtkWidget *controls;
gchar *device = NULL;

GOptionEntry entries[] =
  {
    { "device", 'd',0,G_OPTION_ARG_STRING, &device, "V4L2 device", NULL},
    {NULL}
  };


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

void v4l2_add_header_init ()
{
	GtkWidget *sep, *label, *align;

	label = gtk_label_new ("Device");
        align = gtk_alignment_new (0.0,0.5,0.0,0.0);
        gtk_container_add (GTK_CONTAINER(align), label);
        gtk_table_attach (GTK_TABLE(main_table), align, 0,1,0,1,GTK_FILL,GTK_FILL,5,5);//, GTK_EXPAND, GTK_SHRINK, 5, 5);    

        align = gtk_alignment_new (0.0,0.5,0.0,0.0);
        gtk_container_add (GTK_CONTAINER(align), dev_combo);
        gtk_table_attach (GTK_TABLE(main_table), align, 1,2,0,1,GTK_FILL,GTK_FILL,5,5);

	sep = gtk_hseparator_new();
	gtk_table_attach (GTK_TABLE(main_table), sep, 0,2,1,2, GTK_FILL, GTK_FILL, 0, 0);
	
}

void v4l2_control_panel_create (Gtkv4lDevice *device)
{
	main_table = GTK_TABLE(gtk_table_new (3, 2, FALSE));
	v4l2_add_header_init();

        controls = gtk_v4l_widget_new (device);
        gtk_table_attach (GTK_TABLE(main_table), controls, 0, 2, 2, 3, GTK_FILL, GTK_FILL, 0, 5);
	
	gtk_container_add (GTK_CONTAINER(content_area),GTK_WIDGET(main_table));
}

void v4l2_add_dialog_buttons(void)
{
	GtkWidget *button;

        button = gtk_dialog_add_button ( GTK_DIALOG (window), "_Defaults", GTK_RESPONSE_APPLY);
        g_signal_connect(G_OBJECT(button),"clicked", G_CALLBACK(reset_cb), &controls);

        button = gtk_dialog_add_button ( GTK_DIALOG (window), GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE);
        g_signal_connect(G_OBJECT(button),"clicked", G_CALLBACK(close_cb), NULL);
}

void
v4l2_combo_add_device(Gtkv4lDevice *device, int idx)
{
  gtk_combo_box_insert_text (GTK_COMBO_BOX(dev_combo), idx, device->card);
}

void
v4l2_combo_remove_device(Gtkv4lDevice *device, int idx)
{
  /* If this removes the current device
     v4l2_combo_change_device_cb() will get called and that will
     handle selecting a new device. */
  gtk_combo_box_remove_text (GTK_COMBO_BOX(dev_combo), idx);
}

void v4l2_switch_to_new_device(Gtkv4lDevice *device)
{
	curr_dev = device;
	gtk_widget_destroy(controls);
        controls = gtk_v4l_widget_new (device);
        gtk_table_attach (GTK_TABLE(main_table), controls, 0, 2, 5, 6, GTK_FILL, GTK_FILL, 0, 5);
	gtk_widget_show_all(controls);
}

void v4l2_combo_change_device_cb (GtkWidget *wid, gpointer user_data)
{
  gint active;

  active = gtk_combo_box_get_active (GTK_COMBO_BOX(dev_combo));
  /* This happens when our current device gets unplugged, just select the
     first one in the list instead. */
  if (active == -1) {
    gtk_combo_box_set_active (GTK_COMBO_BOX(dev_combo), 0);
    return;
  }

  curr_dev = g_list_nth_data (devlist->list, active);
  v4l2_switch_to_new_device (g_list_nth_data (devlist->list, active));
}


int main(int argc, char *argv[])
{
  GError *error = NULL;
  GdkPixbuf *icon_pixbuf = NULL;
  gchar *error_string;

  GOptionContext* context = g_option_context_new("- Gtk V4l app");
  g_option_context_add_main_entries (context,entries, NULL);
  g_option_context_add_group (context, gtk_get_option_group (TRUE));
  g_option_context_parse (context, &argc, &argv, &error);

  devlist = g_object_new (GTK_V4L_TYPE_DEVICE_LIST, NULL);
  devlist->device_added = v4l2_combo_add_device;
  devlist->device_removed = v4l2_combo_remove_device;

  dev_combo = gtk_combo_box_new_text();
  g_signal_connect (G_OBJECT(dev_combo), "changed", G_CALLBACK(v4l2_combo_change_device_cb),NULL);

  gtk_v4l_device_list_coldplug (devlist);


  gtk_init (&argc, &argv);

  if (device)
  {
    curr_dev = gtk_v4l_device_list_get_dev_by_device_file (devlist, device);
    if (!curr_dev)
      show_error_dialog ("Specified V4L2 device not found");
  }

  curr_dev = g_list_nth_data(devlist->list, 0);

  window = gtk_dialog_new();
  gtk_window_set_title(GTK_WINDOW(window), "GTK V4L Device properties");
  gtk_window_set_resizable (GTK_WINDOW(window), FALSE);

  if(g_file_test(ICON_LOC,G_FILE_TEST_EXISTS)) {
    icon_pixbuf = gdk_pixbuf_new_from_file(ICON_LOC, NULL);
    if(icon_pixbuf)
      gtk_window_set_icon(GTK_WINDOW(window),icon_pixbuf);
  }  else {
    g_warning("Window icon not found");
  }
  content_area = gtk_dialog_get_content_area(GTK_DIALOG(window));
  
  gtk_container_set_border_width (GTK_CONTAINER(window),7);

  v4l2_control_panel_create(curr_dev);

  v4l2_add_dialog_buttons();
  g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(close_cb), NULL);

  gtk_widget_show_all (window);

  gtk_main();
  return 0;
	

}
