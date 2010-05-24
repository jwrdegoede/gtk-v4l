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


#include "gtk-v4l.h"
#include "gtk-v4l-control.h"
#include "gtk-v4l-device-list.h"

Gtkv4lDeviceList *devlist;
Gtkv4lDevice *curr_dev;
GtkWidget *window,*advanced_window,*dev_combo;
GtkTable *main_table,*table,*table2=NULL;
GtkWidget *content_area,*content_area2;

GtkWidget  *label_driver, *label_card, *label_bus;

gchar *device = NULL;

GOptionEntry entries[] =
  {
    { "device", 'd',0,G_OPTION_ARG_STRING, &device, "V4L2 device", NULL},
    {NULL}
  };


int rownum,rownum_advanced;

void destroy(GtkWidget *widget, gpointer user_data)
{
	gtk_main_quit();
}

void bool_control_changed_cb (GtkButton *button, gpointer user_data)
{
	Gtkv4lControl *ctrl = GTK_V4L_CONTROL (user_data);
	gboolean state;

	state = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(button));
	gtk_v4l_control_set (ctrl, state);
	/* FIXME read back actual resulting value and update widget */
}

void button_control_changed_cb (GtkButton *button, gpointer user_data)
{
	Gtkv4lControl *ctrl = GTK_V4L_CONTROL (user_data);

	gtk_v4l_control_set (ctrl, 0xffffffff);
}

void int_control_changed_cb (GtkRange *range, gpointer user_data)
{
	Gtkv4lControl *ctrl = GTK_V4L_CONTROL (user_data);
	gdouble value;

	value = gtk_range_get_value (GTK_RANGE (range));
	gtk_v4l_control_set (ctrl, value);
	/* FIXME read back actual resulting value and update widget */
}

gchar *
int_control_format_cb (GtkScale *scale, gdouble value, gpointer user_data)
{
	int max = GPOINTER_TO_INT(user_data);
	return g_strdup_printf("%5.2f %%",  ((value/max)*100) );
}

void menu_control_changed_cb (GtkWidget *wid, gpointer user_data)
{
	Gtkv4lControl *ctrl = GTK_V4L_CONTROL (user_data);
	gint value;

	value = gtk_combo_box_get_active(GTK_COMBO_BOX(wid));
	gtk_v4l_control_set (ctrl, value);
	/* FIXME read back actual resulting value and update widget */
}

GtkWidget *v4l2_create_menu_widget (Gtkv4lControl *control)
{
	GtkWidget *combo;
	int k;

	combo = gtk_combo_box_new_text();
	for (k=control->minimum; k<=control->maximum; k++) {
		struct v4l2_querymenu qm;
	        qm.id = control->id;
	        qm.index = k;
	        /* FIXME */
		if(ioctl(curr_dev->fd, VIDIOC_QUERYMENU, &qm) == 0) 
			gtk_combo_box_append_text (GTK_COMBO_BOX(combo), (const gchar *) qm.name);
		else
			g_warning ("Unable to use menu item for :%d", qm.index);
	}

	gtk_combo_box_set_active (GTK_COMBO_BOX(combo),
	                          gtk_v4l_control_get(control));
	g_signal_connect (G_OBJECT(combo), "changed", G_CALLBACK(menu_control_changed_cb), control);

	return combo;
}

GtkWidget *v4l2_create_int_widget (Gtkv4lControl *control)
{	
	GtkWidget *HScale;
	gint min, max, step,def;
	min = control->minimum; max = control->maximum; step = control->step;

	HScale = gtk_hscale_new_with_range (min,max,step);
	gtk_scale_set_value_pos (GTK_SCALE(HScale), GTK_POS_RIGHT);
	gtk_scale_set_digits (GTK_SCALE(HScale), 0);
	gtk_range_set_increments (GTK_RANGE(HScale), step, step);
	gtk_range_set_value (GTK_RANGE(HScale),
	                     (gdouble) gtk_v4l_control_get(control));
	g_signal_connect (G_OBJECT (HScale), "value-changed", G_CALLBACK (int_control_changed_cb), control);
	g_signal_connect (G_OBJECT (HScale), "format-value", G_CALLBACK (int_control_format_cb), GINT_TO_POINTER(max));

	return HScale;
}

GtkWidget *v4l2_create_button_widget (Gtkv4lControl *control)
{	
	GtkWidget *button;
	
	button = gtk_button_new_with_label("Reset");
	g_signal_connect(G_OBJECT(button),"clicked",G_CALLBACK(button_control_changed_cb), control);

	return button;
}

GtkWidget *v4l2_create_bool_widget (Gtkv4lControl *control)
{
	GtkWidget *check;
	gboolean state;

	state = (gboolean) gtk_v4l_control_get (control);
	check = gtk_check_button_new();
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(check), state);
	g_signal_connect ( G_OBJECT(check), "clicked", G_CALLBACK (bool_control_changed_cb), control);

	return check;
}

void v4l2_add_control (Gtkv4lControl *control)
{
	GtkWidget *widget, *label, *align;

	switch(control->type) {
		case V4L2_CTRL_TYPE_INTEGER:
			widget = v4l2_create_int_widget (control);
			break;
		case V4L2_CTRL_TYPE_BOOLEAN:
			widget = v4l2_create_bool_widget (control);
			break;
		case V4L2_CTRL_TYPE_MENU:
			widget = v4l2_create_menu_widget (control);
			break;
		case V4L2_CTRL_TYPE_BUTTON:
			widget = v4l2_create_button_widget (control);
			break;
		default:
			g_warning("Skipping control %s with unknown type %d",
			          control->name, control->type);
			return;
	}

	control->user_data = widget;

        gtk_widget_set_size_request (widget, 250, -1);

	label = gtk_label_new ((gchar *)control->name);
	gtk_misc_set_alignment (GTK_MISC(label),0.0,0.5);

	align = gtk_alignment_new (0.0,0.5,0.0,0.0);
	gtk_container_add (GTK_CONTAINER(align), widget);

        /* FIXME */
        if (FALSE) {
                rownum_advanced++;
                gtk_table_resize (table2, rownum_advanced, 2);
                gtk_table_attach (GTK_TABLE(table2), label, 0,1,rownum_advanced, rownum_advanced+1, GTK_FILL, GTK_FILL, 5, 5);
                gtk_table_attach (GTK_TABLE(table2), align, 1,2,rownum_advanced, rownum_advanced+1, GTK_FILL, GTK_FILL, 5, 5);
        } else {
                rownum++;
                gtk_table_resize (table, rownum, 2);
                gtk_table_attach (GTK_TABLE(table), label, 0,1,rownum, rownum+1, GTK_FILL, GTK_FILL, 5, 5);
                gtk_table_attach (GTK_TABLE(table), align, 1,2,rownum, rownum+1, GTK_FILL, GTK_FILL, 5, 5);
        }
}


void v4l2_add_header_init ()
{
//	const char *driver, *card, *bus_info;
	GtkWidget *sep,*label,*align;

//	driver = (const char *)cap.driver;
//	card = (const char *)cap.card;
//	bus_info = (const char *)cap.bus_info;

	label = gtk_label_new ("Device");
        align = gtk_alignment_new (0.0,0.5,0.0,0.0);
        gtk_container_add (GTK_CONTAINER(align), label);
        gtk_table_attach (GTK_TABLE(main_table), align, 0,1,0,1,GTK_FILL,GTK_FILL,5,5);//, GTK_EXPAND, GTK_SHRINK, 5, 5);    


        align = gtk_alignment_new (0.0,0.5,0.0,0.0);
        gtk_container_add (GTK_CONTAINER(align), dev_combo);
        gtk_table_attach (GTK_TABLE(main_table), align, 1,2,0,1,GTK_FILL,GTK_FILL,5,5);//, GTK_EXPAND, GTK_SHRINK, 5, 5);    

	label = gtk_label_new ("Driver");
	align = gtk_alignment_new (0.0,0.5,0.0,0.0);
	gtk_container_add (GTK_CONTAINER(align), label);
	gtk_table_attach (GTK_TABLE(main_table), align, 0,1,1,2,GTK_FILL,GTK_FILL,5,5);//, GTK_EXPAND, GTK_SHRINK, 5, 5);	
       
	 
	label_driver = gtk_label_new ("");
	align = gtk_alignment_new (0.0,0.5,0.0,0.0);
        gtk_container_add (GTK_CONTAINER(align), label_driver);
        gtk_table_attach (GTK_TABLE(main_table), align, 1,2,1,2, GTK_FILL, GTK_FILL, 5, 5);

        label = gtk_label_new ("Card");
        align = gtk_alignment_new (0.0,0.5,0.0,0.0);
        gtk_container_add (GTK_CONTAINER(align), label);
        gtk_table_attach (GTK_TABLE(main_table), align, 0,1,2,3, GTK_FILL, GTK_FILL, 5, 5);

        label_card = gtk_label_new ("");
        align = gtk_alignment_new (0.0,0.5,0.0,0.0);
        gtk_container_add (GTK_CONTAINER(align), label_card);
        gtk_table_attach (GTK_TABLE(main_table), align, 1,2,2,3, GTK_FILL, GTK_FILL, 5, 5);

        label = gtk_label_new ("Bus Information");
        align = gtk_alignment_new (0.0,0.5,0.0,0.0);
        gtk_container_add (GTK_CONTAINER(align), label);
        gtk_table_attach (GTK_TABLE(main_table), align, 0,1,3,4, GTK_FILL, GTK_FILL, 5, 5);

        label_bus = gtk_label_new ("");
        align = gtk_alignment_new (0.0,0.5,0.0,0.0);
        gtk_container_add (GTK_CONTAINER(align), label_bus);
        gtk_table_attach (GTK_TABLE(main_table), align, 1,2,3,4, GTK_FILL, GTK_FILL, 5, 5);
	
	sep = gtk_hseparator_new();
	gtk_table_attach (GTK_TABLE(main_table), sep, 0,4,4,5, GTK_FILL, GTK_FILL, 0, 0);
	
}

void
v4l2_add_header_populate(Gtkv4lDevice *device)
{
	gtk_label_set_text(GTK_LABEL(label_driver), device->driver);
	gtk_label_set_text(GTK_LABEL(label_card), device->card);
	gtk_label_set_text(GTK_LABEL(label_bus), device->bus_info);
}

void v4l2_reset_defaults(void)
{
  GList *elem;

  for (elem = g_list_first (gtk_v4l_device_get_controls (curr_dev));
       elem; elem = g_list_next (elem)) {
    Gtkv4lControl *control = GTK_V4L_CONTROL (elem->data);

    switch (control->type) {
    case V4L2_CTRL_TYPE_INTEGER:
      gtk_range_set_value (GTK_RANGE (control->user_data),
                           control->default_value);
      break;
    case V4L2_CTRL_TYPE_BOOLEAN:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (control->user_data),
                                    control->default_value);
      break;
    case V4L2_CTRL_TYPE_MENU:
      gtk_combo_box_set_active (GTK_COMBO_BOX (control->user_data),
                                control->default_value);
      break;
    case V4L2_CTRL_TYPE_BUTTON:
      /* do nothing */
      break;
    }
  }
}

void v4l2_load_from_driver (void)
{
  GList *elem;

  for (elem = g_list_first (gtk_v4l_device_get_controls (curr_dev));
       elem; elem = g_list_next (elem)) {
    Gtkv4lControl *control = GTK_V4L_CONTROL (elem->data);

    v4l2_add_control (control);
  }
}

void v4l2_control_panel_create_properties(void)
{
	advanced_window = gtk_expander_new("More properties");

	gtk_expander_set_expanded(GTK_EXPANDER(advanced_window), FALSE);

	table = GTK_TABLE(gtk_table_new(1,3,FALSE));

        /* FIXME */
	if (FALSE) {
		table2 = GTK_TABLE(gtk_table_new(rownum_advanced, 3, FALSE));
		gtk_container_add (GTK_CONTAINER(advanced_window), GTK_WIDGET(table2));

		v4l2_load_from_driver ();
		v4l2_add_footer (TRUE);

	} else {
		v4l2_load_from_driver ();
		v4l2_add_footer (FALSE);			
		}

        gtk_table_attach (GTK_TABLE(main_table),GTK_WIDGET(table), 0,2,rownum, rownum+1,GTK_FILL, GTK_FILL, 0, 5);
}

void v4l2_control_panel_create (Gtkv4lDevice *device)
{
	rownum=5;
	main_table = GTK_TABLE(gtk_table_new (rownum,3,FALSE));
	v4l2_add_header_init();
	v4l2_add_header_populate(device);

	v4l2_control_panel_create_properties();
	
	gtk_container_add (GTK_CONTAINER(content_area),GTK_WIDGET(main_table));

}

void v4l2_add_footer (gboolean advanced)
{
        rownum++;
	gtk_table_resize (table, rownum, 2);

        gtk_table_attach (GTK_TABLE(table),advanced_window, 0,2,rownum, rownum+1,GTK_FILL, GTK_FILL, 0, 5);
	gtk_widget_set_sensitive(GTK_WIDGET(advanced_window),advanced);


	rownum++;
        gtk_table_resize (table, rownum, 2);

}

void v4l2_add_dialog_buttons(void)
{
	GtkWidget *button;

        button = gtk_dialog_add_button ( GTK_DIALOG (window), "_Defaults", GTK_RESPONSE_APPLY);
        g_signal_connect(G_OBJECT(button),"clicked", G_CALLBACK(v4l2_reset_defaults),NULL);

        button = gtk_dialog_add_button ( GTK_DIALOG (window), GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE);
        g_signal_connect(G_OBJECT(button),"clicked", G_CALLBACK(destroy), NULL);

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
	v4l2_add_header_populate(device);
	gtk_widget_destroy(GTK_WIDGET(table));
	v4l2_control_panel_create_properties();
	gtk_widget_show_all(GTK_WIDGET(table));
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


  if (device == NULL) // Assume default device
  {
	curr_dev = g_list_nth_data(devlist->list, 0);
  }
  else
        curr_dev = gtk_v4l_device_list_get_dev_by_device_file(devlist, device);

  if (!curr_dev) {
          show_error_dialog ("No V4L2 devices found");
          return EXIT_FAILURE;
  }

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
  g_signal_connect(G_OBJECT(window),"destroy", G_CALLBACK(destroy), NULL);

  gtk_widget_show_all (window);

  gtk_main();
  return 0;
	

}
