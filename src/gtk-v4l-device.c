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

#include <fcntl.h>
#include <libv4l2.h>
#include <linux/videodev2.h>
#include "gtk-v4l-device.h"

#define GTK_V4L_DEVICE_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GTK_V4L_TYPE_DEVICE, Gtkv4lDevicePrivate))

enum
{
  PROP_0,
  PROP_DEVICE_FILE,
  PROP_DRIVER,
  PROP_CARD,
  PROP_BUS_INFO,
  PROP_VERSION,
};

struct _Gtkv4lDevicePrivate {
  gint placeholder;
};

/* will create gtk_v4l_device_get_type and set gtk_v4l_device_parent_class */
G_DEFINE_TYPE (Gtkv4lDevice, gtk_v4l_device, G_TYPE_OBJECT);

static void
gtk_v4l_device_set_property (GObject      *object,
                             guint         property_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  Gtkv4lDevice *self = GTK_V4L_DEVICE (object);

  switch (property_id) {
  case PROP_DEVICE_FILE:
    g_free (self->device_file);
    self->device_file = g_value_dup_string (value);
    break;
  case PROP_DRIVER:
  case PROP_CARD:
  case PROP_BUS_INFO:
  case PROP_VERSION:
    G_OBJECT_WARN_INVALID_PSPEC (object, "read-only property",
                                 property_id, pspec);
    break;
  default:
    /* We don't have any other property... */
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}

static void
gtk_v4l_device_get_property (GObject    *object,
                             guint       property_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  Gtkv4lDevice *self = GTK_V4L_DEVICE (object);

  switch (property_id) {
  case PROP_DEVICE_FILE:
    g_value_set_string (value, self->device_file);
    break;
  case PROP_DRIVER:
    g_value_set_string (value, self->driver);
    break;
  case PROP_CARD:
    g_value_set_string (value, self->card);
    break;
  case PROP_BUS_INFO:
    g_value_set_string (value, self->bus_info);
    break;
  case PROP_VERSION:
    g_value_set_string (value, self->version);
    break;
  default:
    /* We don't have any other property... */
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}

static GObject *
gtk_v4l_device_constructor (GType                  gtype,
                            guint                  n_properties,
                            GObjectConstructParam *properties)
{
  GObject *obj;
  struct v4l2_capability cap;
  Gtkv4lDevice *self;

  {
    /* Always chain up to the parent constructor */
    Gtkv4lDeviceClass *klass;
    GObjectClass *parent_class;  
    parent_class = G_OBJECT_CLASS (gtk_v4l_device_parent_class);
    obj = parent_class->constructor (gtype, n_properties, properties);
  }

  /* update the object state depending on constructor properties */
  self = GTK_V4L_DEVICE (obj);
  self->fd = v4l2_open(self->device_file, O_RDWR);
  if (self->fd == -1) {
    g_warning ("Could not open device file: '%s'", self->device_file);
    return obj;
  }

  if (v4l2_ioctl (self->fd, VIDIOC_QUERYCAP, &cap) == -1) {
    g_warning ("Device '%s' is not a v4l2 device", self->device_file);
    v4l2_close (self->fd);
    self->fd = -1;
    return obj;
  }
  
  self->driver  = g_strdup(cap.driver);
  self->card    = g_strdup(cap.card);
  self->bus_info = g_strdup(cap.bus_info);
  self->version = g_strdup_printf("%d.%d.%d",
                                  (cap.version >> 16) & 0xff, 
                                  (cap.version >>  8) & 0xff, 
                                  (cap.version >>  0) & 0xff);

  printf("constructed device: %s\n", self->card);

  return obj;
}

static void
gtk_v4l_device_finalize (GObject *object)
{
  Gtkv4lDevice *self = GTK_V4L_DEVICE (object);

  if (self->fd != -1)
    close (self->fd);
}

static void
gtk_v4l_device_class_init (Gtkv4lDeviceClass *klass)
{
  GObjectClass *gobject_class = (GObjectClass *) klass;
  GParamSpec *pspec;

  g_type_class_add_private (klass, sizeof (Gtkv4lDevicePrivate));

  gobject_class->constructor  = gtk_v4l_device_constructor;
  gobject_class->finalize     = gtk_v4l_device_finalize;
  gobject_class->set_property = gtk_v4l_device_set_property;
  gobject_class->get_property = gtk_v4l_device_get_property;

  pspec = g_param_spec_string ("device_file",
                               "Gtkv4lDevice construct prop",
                               "Set the device file to use",
                               "/dev/video0" /* default value */,
                               G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class,
                                   PROP_DEVICE_FILE,
                                   pspec);
  pspec = g_param_spec_string ("driver",
                               "Gtkv4lDevice read only prop",
                               "Get the v4l2 driver",
                               NULL,
                               G_PARAM_READABLE);
  g_object_class_install_property (gobject_class,
                                   PROP_DRIVER,
                                   pspec);
  pspec = g_param_spec_string ("card",
                               "Gtkv4lDevice read only prop",
                               "Get the v4l2 card",
                               NULL,
                               G_PARAM_READABLE);
  g_object_class_install_property (gobject_class,
                                   PROP_CARD,
                                   pspec);
  pspec = g_param_spec_string ("bus_info",
                               "Gtkv4lDevice read only prop",
                               "Get the v4l2 bus_info",
                               NULL,
                               G_PARAM_READABLE);
  g_object_class_install_property (gobject_class,
                                   PROP_BUS_INFO,
                                   pspec);
  pspec = g_param_spec_string ("version",
                               "Gtkv4lDevice read only prop",
                               "Get the v4l2 driver version",
                               NULL,
                               G_PARAM_READABLE);
  g_object_class_install_property (gobject_class,
                                   PROP_VERSION,
                                   pspec);
}

static void
gtk_v4l_device_init (Gtkv4lDevice *self)
{
  const gchar *const subsystems[] = {"video4linux", NULL};
  Gtkv4lDevicePrivate *priv;

  /* initialize the object */
  self->fd = -1;
  self->priv = priv = GTK_V4L_DEVICE_GET_PRIVATE(self);
}
