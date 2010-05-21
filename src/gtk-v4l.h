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


#ifndef _GTK_V4L2_
#define _GTK_V4L2_

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/types.h>
#include <linux/videodev.h>
#include <gtk/gtk.h>
#include <libv4l2.h>
#include <gudev/gudev.h>

#define MAX_CONTROLS_ON_MAIN_PAGE 8
#define MIN_CONTROLS_ON_MAIN_PAGE 6
#define ICON_LOC "/usr/share/icons/gnome/24x24/devices/camera-web.png"

struct v4l2_property {
	__u32 id;
	__s32 value;
	__s32 def;
	GtkWidget *w;
	int ctrl_type;
};

struct v4l2_device {
        char *device_file;
        char *product_name;
        char *vendor;
        char *product;
        char *dev_path;
        int vendor_id;
        int product_id;
};

/* Functions to manage lists */
void v4l2_list_add (__u32 id, __s32 value,__s32 def,GtkWidget *w, int ctrl_type);
gboolean v4l2_list_edit_element (__u32 id, __s32 value);
void v4l2_list_print(void); /*Used only for debugging */
void v4l2_list_destroy(void);
void v4l2_reset_list_to_default();
void v4l2_combo_list_print(void);



/* Functions to talk to the driver */
int v4l2_count_controls(void);
int v4l2_write_to_driver(void);
void v4l2_load_from_driver (int fd, gboolean advanced);
int v4l2_write_to_driver_one(__u32 id,__u32 value, gboolean toggle);
void v4l2_switch_to_new_device(const char *device);


/* Callbacks */
void close_cb(GtkWidget *widget, gpointer user_data);
void destroy(GtkWidget *widget, gpointer user_data);
void bool_control_changed_cb (GtkButton *button, gpointer user_data);
void advanced_cb (GtkWidget *widget, gpointer user_data);
void int_control_changed_cb (GtkRange *range, gpointer user_data);


/* Functions used to display the window */
void v4l2_add_int_control (struct v4l2_queryctrl ctrl,struct v4l2_control c,gboolean advanced);
void v4l2_add_bool_control (struct v4l2_queryctrl ctrl,struct v4l2_control c,gboolean advanced);
void v4l2_add_control (struct v4l2_queryctrl ctrl, struct v4l2_control c,gboolean advanced);
void v4l2_add_header (void);
void v4l2_control_panel_create (void);
void v4l2_add_footer (gboolean advanced);
void v4l2_add_dialog_buttons(void);



void v4l2_add_footer (gboolean advanced);



#endif /* _GTK_V4L2_ */
