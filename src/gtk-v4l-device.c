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



#include <gudev/gudev.h>

v4l_device_uevent_cb (GUdevClient               *client,
                                        const gchar               *action,
                                        GUdevDevice               *udevice)
{
  if (g_str_equal (action, "remove"))
        g_warning("Removed cam");
  else if (g_str_equal (action, "add"))
        {
                g_warning ("Added cam");
                v4l_device_added (udevice);
        }
}

v4l_device_added (GUdevDevice *udevice)
{
        const char         *device_file;
        const char         *product_name;
        const char         *vendor;
        const char         *product;
        const char         *bus;
        gint                vendor_id   = 0;
        gint                product_id  = 0;
        gint                v4l_version = 0;

        g_warning ("Device added, properties follow:");
        const gchar *devpath = g_udev_device_get_property (udevice, "DEVPATH");
        bus = g_udev_device_get_property (udevice, "ID_BUS");
        if (g_strcmp0 (bus, "usb") == 0)
                {
                vendor = g_udev_device_get_property (udevice, "ID_VENDOR_ID");

                if (vendor != NULL)
                        vendor_id = g_ascii_strtoll (vendor, NULL, 16);

                product = g_udev_device_get_property (udevice, "ID_MODEL_ID");

                if (product != NULL)
                        product_id = g_ascii_strtoll (product, NULL, 16);

                if (vendor_id == 0  || product_id == 0 )
                        g_error ("Error getting product and vendor id");
                else
                        g_warning ("Found device 04x:%04x, getting capabilities...", vendor_id, product_id);

                }
        else
                {
                        g_warning ("Not a usb device, skipping product/vendor retrieval");
                }


        device_file = g_udev_device_get_device_file (udevice);
        if (device_file == NULL)
        {
                g_error ("Error getting V4L device");
                return;
        }
        if (strstr (device_file, "vbi"))
        {
                g_error ("Skipping vbi device: %s", device_file);
                return;
        }
        product_name  = g_udev_device_get_property (udevice, "ID_V4L_PRODUCT");
        g_warning ("name=%s, device file=%s",product_name, device_file);
}

GUdevClient *
v4l2_device_init(void)
{
        const gchar *const                subsystems[] = {"video4linux", NULL};
        GUdevClient *camera;

        camera = g_udev_client_new (subsystems);
        g_signal_connect (G_OBJECT(camera), "uevent",
                          G_CALLBACK (v4l_device_uevent_cb),NULL);
	return (camera);
}

void
v4l2_device_destroy (GUdevClient *camera)
{
	if (camera)
		g_object_unref (camera);
}
