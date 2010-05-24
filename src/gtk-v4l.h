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

#include "gtk-v4l-device.h"

#define ICON_LOC "/usr/share/icons/gnome/24x24/devices/camera-web.png"

/* Functions to talk to the driver */
void v4l2_switch_to_new_device(Gtkv4lDevice *device);

/* Callbacks */
void destroy(GtkWidget *widget, gpointer user_data);
void bool_control_changed_cb (GtkButton *button, gpointer user_data);
void advanced_cb (GtkWidget *widget, gpointer user_data);
void int_control_changed_cb (GtkRange *range, gpointer user_data);
void v4l2_combo_change_device_cb (GtkWidget *wid, gpointer user_data);

/* Functions used to display the window */
void v4l2_add_header (Gtkv4lDevice *device);
void v4l2_control_panel_create (Gtkv4lDevice *device);
void v4l2_add_footer (gboolean advanced);
void v4l2_add_dialog_buttons(void);

#endif /* _GTK_V4L2_ */
