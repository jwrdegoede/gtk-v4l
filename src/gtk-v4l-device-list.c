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

#include <gudev/gudev.h>
#include "gtk-v4l-device-list.h"

#define GTK_V4L_DEVICE_LIST_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GTK_V4L_TYPE_DEVICE_LIST, Gtkv4lDeviceListPrivate))

struct _Gtkv4lDeviceListPrivate {
  GUdevClient *udev;
};

static void gtk_v4l_device_list_uevent_cb (GUdevClient            *client,
                                           const gchar            *action,
                                           GUdevDevice            *udevice,
                                           gpointer                user_data);

/* will create gtk_v4l_device_list_get_type and set gtk_v4l_device_list_parent_class */
G_DEFINE_TYPE (Gtkv4lDeviceList, gtk_v4l_device_list, G_TYPE_OBJECT);
    

static void
gtk_v4l_device_list_free_device (gpointer data, gpointer user_data)
{
  g_object_unref (data);
}

static void
gtk_v4l_device_list_finalize (GObject *object)
{
  Gtkv4lDeviceList *self = GTK_V4L_DEVICE_LIST (object);

  g_list_foreach (self->list, gtk_v4l_device_list_free_device, NULL);
  g_list_free (self->list);

  g_object_unref (self->priv->udev);
}

static void
gtk_v4l_device_list_class_init (Gtkv4lDeviceListClass *klass)
{
  GObjectClass *gobject_class = (GObjectClass *) klass;

  g_type_class_add_private (klass, sizeof (Gtkv4lDeviceListPrivate));

  gobject_class->finalize = gtk_v4l_device_list_finalize;
}

static void
gtk_v4l_device_list_init (Gtkv4lDeviceList *self)
{
  const gchar *const subsystems[] = {"video4linux", NULL};
  Gtkv4lDeviceListPrivate *priv;

  /* initialize the object */
  self->priv = priv = GTK_V4L_DEVICE_LIST_GET_PRIVATE(self);
  priv->udev = g_udev_client_new (subsystems);
  g_signal_connect (G_OBJECT (priv->udev), "uevent",
                    G_CALLBACK (gtk_v4l_device_list_uevent_cb), self);
}

static void
gtk_v4l_device_list_add_dev (Gtkv4lDeviceList *self,
                             GUdevDevice      *udevice)
{
  Gtkv4lDevice       *device;
  const gchar        *device_file;
  const gchar        *devpath;
  const gchar        *product_name;

  device_file = g_udev_device_get_device_file (udevice);
  if (device_file == NULL)
  {
    g_warning ("%s: Error getting V4L device", G_STRLOC);
    return;
  }

  /* Skip vbi devices */
  if (!strncmp (device_file, "/dev/vbi", 8))
    return;

  device = g_object_new (GTK_V4L_TYPE_DEVICE, "device_file", device_file, NULL);
  /* If fd == -1, then for some reason the device could not be opened, or it
     is not a v4l2 device, skip it */
  if (device->fd == -1) {
    g_object_unref (device);
    return;
  }

  self->list = g_list_append (self->list, device);

  if (self->device_added)
    self->device_added(device, g_list_index (self->list, device));
}

static void
gtk_v4l_device_list_remove_dev (Gtkv4lDeviceList *self,
                                GUdevDevice      *udevice)
{
  GList *elem;
  Gtkv4lDevice *device = NULL;
  const gchar *device_file = g_udev_device_get_device_file (udevice);
  int idx = 0;

  for (elem = g_list_first (self->list); elem; elem = g_list_next (elem)) {
    device = elem->data;
    if (g_str_equal (device->device_file, device_file))
      break;
    idx++;
  }
  if (!elem) {
    g_error ("Error device_file: %s not found in device list", device_file);
    return;
  }

  self->list = g_list_remove (self->list, device);

  if (self->device_removed)
    self->device_removed (device, idx);

  g_object_unref (device);
}

static void
gtk_v4l_device_list_uevent_cb (GUdevClient            *client,
                               const gchar            *action,
                               GUdevDevice            *udevice,
                               gpointer                user_data)
{
  Gtkv4lDeviceList *self =  GTK_V4L_DEVICE_LIST (user_data);

  if (g_str_equal (action, "add"))
    gtk_v4l_device_list_add_dev (self, udevice);
  else if (g_str_equal (action, "remove"))
    gtk_v4l_device_list_remove_dev (self, udevice);
}

void
gtk_v4l_device_list_coldplug (Gtkv4lDeviceList *self) 
{
  GList *devices, *elem;

  devices = g_udev_client_query_by_subsystem (self->priv->udev, "video4linux");
  for (elem = g_list_first (devices); elem; elem = g_list_next (elem)) {
    gtk_v4l_device_list_add_dev (self, elem->data);
    g_object_unref (elem->data);
  }
  g_list_free (devices);
}

Gtkv4lDevice *
gtk_v4l_device_list_get_dev_by_device_file (Gtkv4lDeviceList *self,
                                            const gchar *device_file)
{
  GList *elem;
  Gtkv4lDevice *device = NULL;

  for (elem = g_list_first (self->list); elem; elem = g_list_next (elem)) {
    device = elem->data;
    if (g_str_equal (device->device_file, device_file))
      break;
  }
  if (!elem)
    return NULL;

  return device;
}
