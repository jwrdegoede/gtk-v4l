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

struct v4l2_device {
        const char *device_file;
        const char *product_name;
        const char *vendor;
        const char *product;
        const char *dev_path;
        int vendor_id;
        int product_id;
};

GList *dev_list=NULL;

void v4l2_device_removed (const char *devpath);
void v4l2_device_list_print(void);
void v4l2_device_free_node(struct v4l2_device *node);




v4l_device_uevent_cb (GUdevClient               *client,
                                        const gchar               *action,
                                        GUdevDevice               *udevice)
{
  if (g_str_equal (action, "remove"))
  {
	g_warning("Removed cam");
	v4l2_device_removed (g_udev_device_get_property (udevice, "DEVPATH"));
//	v4l2_device_list_print();
  }
  else if (g_str_equal (action, "add"))
        {
                g_warning ("Added cam");
                v4l2_device_added (udevice,FALSE);
//		v4l2_device_list_print();
        }
}

void
v4l2_device_removed (const char *devpath)
{
/*	GList *l;
	struct v4l2_device *temp;

	for (l=dev_list;l;l=l->next) {
		temp = l->data;
		if (!g_strcmp0(devpath,temp->dev_path)) 
		{
			g_warning("We can remove : %s now", temp->device_file);
			g_list_remove(dev_list, temp);
			return;
		}
	}*/
	v4l2_combo_remove_device(devpath);
}

void
v4l2_device_free_all(void) 
{
	GList *l;
        struct v4l2_device *temp;

        for (l=dev_list;l;l=l->next) {
                temp = l->data;
		v4l2_device_free_node(temp);
	}
	g_list_free (dev_list);
}

void 
v4l2_device_free_node(struct v4l2_device *node)
{
	g_free (node);	
}

/* Use the below function for debugging only*/
void
v4l2_device_list_print(void)
{
	GList *l;
	struct v4l2_device *temp;
	
	g_message ("printing device list now");

	for (l=dev_list;l;l=l->next) {
		temp = l->data;
		g_message ("Device file: %s", temp->device_file);
	}
}

v4l2_device_added (GUdevDevice *udevice,gboolean iscoldplug)
{
        const char         *device_file;
        const char         *product_name;
        const char         *vendor;
        const char         *product;
        const char         *bus;
        gint                vendor_id   = 0;
        gint                product_id  = 0;
        gint                v4l_version = 0;

	struct v4l2_device* temp;

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
        if (g_strstr_len (device_file,3, "vbi"))
        {
                g_error ("Skipping vbi device: %s", device_file);
                return;
        }
        product_name  = g_udev_device_get_property (udevice, "ID_V4L_PRODUCT");
        g_warning ("name=%s, device file=%s, devpath=%s",product_name, device_file,devpath);

	/* The probing went well, so go ahead and add the device */
	temp = g_malloc (sizeof (struct v4l2_device));

	temp->device_file = g_strdup(device_file);
	temp->product_name = g_strdup(product_name);
	temp->vendor = g_strdup(vendor);
	temp->product = g_strdup (product);
	temp->vendor_id = vendor_id;
	temp->product_id = product_id;
	temp->dev_path = g_strdup(devpath);
	if (iscoldplug)
		dev_list = g_list_append (dev_list, (gpointer) temp);
	else
		v4l2_combo_add_device(temp);

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

GList * 
v4l2_device_coldplug(GUdevClient *camera)
{
	GList *devices, *l;
	gint i = 0;

	g_message("Probing for devices...");

	devices = g_udev_client_query_by_subsystem (camera, "video4linux");
	
	for (l = devices; l != NULL; l = l->next)
	{
		v4l2_device_added (l->data,TRUE);
		g_object_unref (l->data);
		i++;
	}
	g_list_free (devices);

	if (i==0) 
	{
		g_warning ("No devices found");
		return (NULL);
	}


	v4l2_device_list_print();
	return (dev_list);
}

char *
v4l2_device_default(void)
{
	struct v4l2_device *temp;
	if (!dev_list)
	{	
		g_warning ("null");
		return (NULL);
	}

	temp = dev_list->data;

	/* Otherwise return the first member of the list for use as default device*/
	g_message("Using %s as the default device",temp->device_file);
	return((char *)temp->device_file);
}

GList *
v4l2_device_get_dev_list()
{
	return(dev_list);
}

/* DEVPATH is going to be the unique identifier for each device */
char *
v4l2_device_get_dev_path(char *device)
{
        GList *l;
        struct v4l2_device *temp;

        for (l=dev_list;l;l=l->next) {
                temp = l->data;
		if (!g_strcmp0(temp->device_file, device))
			return (temp->dev_path);
        }
	return (NULL);	
}
