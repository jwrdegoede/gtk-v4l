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
#include "gtk-v4l-control.h"
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
  GList *controls;
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
  Gtkv4lDevice *self;
  struct v4l2_capability cap;

  {
    /* Always chain up to the parent constructor */
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
  
  self->driver  = g_strdup((gchar *)cap.driver);
  self->card    = g_strdup((gchar *)cap.card);
  self->bus_info = g_strdup((gchar *)cap.bus_info);
  self->version = g_strdup_printf("%d.%d.%d",
                                  (cap.version >> 16) & 0xff, 
                                  (cap.version >>  8) & 0xff, 
                                  (cap.version >>  0) & 0xff);

  return obj;
}

static void
gtk_v4l_device_free_control (gpointer data, gpointer user_data)
{
  Gtkv4lControl *control = GTK_V4L_CONTROL (data);
  
  /* We are going to close the fd, so set it to -1 in case others still hold
     a reference to the control. */
  control->fd = -1;
  g_object_unref (control);
}

static void
gtk_v4l_device_finalize (GObject *object)
{
  Gtkv4lDevice *self = GTK_V4L_DEVICE (object);

  g_list_foreach (self->priv->controls, gtk_v4l_device_free_control, NULL);
  g_list_free (self->priv->controls);

  if (self->fd != -1)
    close (self->fd);

  g_free (self->device_file);
  g_free (self->driver);
  g_free (self->card);
  g_free (self->bus_info);
  g_free (self->version);
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
  Gtkv4lDevicePrivate *priv;

  /* initialize the object */
  self->fd = -1;
  self->priv = priv = GTK_V4L_DEVICE_GET_PRIVATE(self);
}

GList *
gtk_v4l_device_get_controls (Gtkv4lDevice *self)
{
  Gtkv4lControl *control;
  struct v4l2_queryctrl query;
  int i;
  
  if (self->priv->controls)
    return self->priv->controls;

  /* Check if the driver supports V4L2_CTRL_FLAG_NEXT_CTRL, if not do some
     "brute force" control enumeration. */
  query.id = V4L2_CTRL_FLAG_NEXT_CTRL;
  if (v4l2_ioctl (self->fd, VIDIOC_QUERYCTRL, &query) != -1) {
    do {
      if (query.flags & V4L2_CTRL_FLAG_DISABLED) {
        query.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
        continue;
      }

      control = g_object_new (GTK_V4L_TYPE_CONTROL,
                              "fd", self->fd,
                              "query_result", &query,
                              NULL);
      self->priv->controls = g_list_append (self->priv->controls, control);
      query.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
    } while(v4l2_ioctl (self->fd, VIDIOC_QUERYCTRL, &query) != -1);
  } else {
    for (i = V4L2_CID_BASE; i < V4L2_CID_LASTP1; i++) {
      query.id = i;
      if (v4l2_ioctl (self->fd, VIDIOC_QUERYCTRL, &query) == -1)
        continue;

      if (query.flags & V4L2_CTRL_FLAG_DISABLED)
        continue;

      control = g_object_new (GTK_V4L_TYPE_CONTROL,
                              "fd", self->fd,
                              "query_result", &query,
                              NULL);
      self->priv->controls = g_list_append (self->priv->controls, control);
    }

    for (i = V4L2_CID_PRIVATE_BASE; ; i++) {
      query.id = i;
      if (v4l2_ioctl (self->fd, VIDIOC_QUERYCTRL, &query) == -1)
        break;

      if (query.flags & V4L2_CTRL_FLAG_DISABLED)
        continue;

      control = g_object_new (GTK_V4L_TYPE_CONTROL,
                              "fd", self->fd,
                              "query_result", &query,
                              NULL);
      self->priv->controls = g_list_append (self->priv->controls, control);
    }
  }

  return self->priv->controls;
}
