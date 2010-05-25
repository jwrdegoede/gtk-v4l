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

#ifndef __GTK_V4L_DEVICE_LIST_H__
#define __GTK_V4L_DEVICE_LIST_H__

#include <glib-object.h>
#include "gtk-v4l-device.h"

#define GTK_V4L_TYPE_DEVICE_LIST            (gtk_v4l_device_list_get_type ())
#define GTK_V4L_DEVICE_LIST(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_V4L_TYPE_DEVICE_LIST, Gtkv4lDeviceList))
#define GTK_V4L_DEVICE_LIST_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_V4L_TYPE_DEVICE_LIST, Gtkv4lDeviceListClass))
#define GTK_V4L_IS_DEVICE_LIST(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_V4L_TYPE_DEVICE_LIST))
#define GTK_V4L_IS_DEVICE_LIST_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_V4L_TYPE_DEVICE_LIST))
#define GTK_V4L_DEVICE_LIST_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_V4L_TYPE_DEVICE_LIST, Gtkv4lDeviceListClass))

typedef struct _Gtkv4lDeviceList Gtkv4lDeviceList;
typedef struct _Gtkv4lDeviceListClass Gtkv4lDeviceListClass;
typedef struct _Gtkv4lDeviceListPrivate Gtkv4lDeviceListPrivate;

struct _Gtkv4lDeviceList {
  GObject parent;
  /* FIXME convert these 2 into signals */
  /* Called after a device is added to the list */
  void (*device_added)(Gtkv4lDevice *device, int idx);
  /* Called after a device is removed from the list, the idx argument
     is the idx in the list the device used to have. */
  void (*device_removed)(Gtkv4lDevice *device, int idx);
  /* instance members */
  GList *list;
  Gtkv4lDeviceListPrivate *priv;
};

struct _Gtkv4lDeviceListClass {
  GObjectClass parent;
  /* class members */
};

GType gtk_v4l_device_list_get_type (void);
void gtk_v4l_device_list_coldplug (Gtkv4lDeviceList *self);
Gtkv4lDevice *gtk_v4l_device_list_get_dev_by_device_file (
  Gtkv4lDeviceList *self, const gchar *device_file);

#endif
