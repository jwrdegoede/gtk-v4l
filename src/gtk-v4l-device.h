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

#ifndef __GTK_V4L_DEVICE_H__
#define __GTK_V4L_DEVICE_H__

#include <glib-object.h>

#define GTK_V4L_TYPE_DEVICE            (gtk_v4l_device_get_type ())
#define GTK_V4L_DEVICE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_V4L_TYPE_DEVICE, Gtkv4lDevice))
#define GTK_V4L_DEVICE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_V4L_TYPE_DEVICE, Gtkv4lDeviceClass))
#define GTK_V4L_IS_DEVICE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_V4L_TYPE_DEVICE))
#define GTK_V4L_IS_DEVICE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_V4L_TYPE_DEVICE))
#define GTK_V4L_DEVICE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_V4L_TYPE_DEVICE, Gtkv4lDeviceClass))

typedef struct _Gtkv4lDevice Gtkv4lDevice;
typedef struct _Gtkv4lDeviceClass Gtkv4lDeviceClass;
typedef struct _Gtkv4lDevicePrivate Gtkv4lDevicePrivate;

struct _Gtkv4lDevice {
  GObject parent;
  /* instance members */
  int fd;
  gchar *device_file;
  gchar *driver;
  gchar *card;
  gchar *bus_info;
  gchar *version;
  Gtkv4lDevicePrivate *priv;
};

struct _Gtkv4lDeviceClass {
  GObjectClass parent;
  /* class members */
};

GType gtk_v4l_device_get_type (void);
GList *gtk_v4l_device_get_controls (Gtkv4lDevice *self);
struct _Gtkv4lControl *gtk_v4l_device_get_control_by_id (Gtkv4lDevice *self, guint32 id);
void gtk_v4l_device_update_controls (Gtkv4lDevice *self);

#endif
