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
#include "gtk-v4l-control.h"

#define GTK_V4L_CONTROL_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GTK_V4L_TYPE_CONTROL, Gtkv4lControlPrivate))

enum
{
  PROP_0,
  PROP_FD,
  PROP_ID,
  PROP_QUERY_RESULT,
  PROP_TYPE,
  PROP_NAME,
  PROP_MINIMUM,
  PROP_MAXIMUM,
  PROP_STEP,
  PROP_DEFAULT_VALUE,
  PROP_FLAGS,
};

struct _Gtkv4lControlPrivate {
  gint value;
  gboolean value_valid;
  /* This is used at construction time and can be set when creating an
     instance to avoid the need for doing a query_ctrl ioctl for the 2nd
     time when enumerating all controls on a device. */
  struct v4l2_queryctrl *query_result;
};

static void
gtk_v4l_control_handle_query_result (Gtkv4lControl *self,
                                     const struct v4l2_queryctrl *query);
static void
gtk_v4l_control_query (Gtkv4lControl *self);

/* will create gtk_v4l_control_get_type and set gtk_v4l_control_parent_class */
G_DEFINE_TYPE (Gtkv4lControl, gtk_v4l_control, G_TYPE_OBJECT);

static void
gtk_v4l_control_set_property (GObject      *object,
                              guint         property_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  Gtkv4lControl *self = GTK_V4L_CONTROL (object);

  switch (property_id) {
  case PROP_FD:
    self->fd = g_value_get_int(value);
    break;
  case PROP_ID:
    self->id = g_value_get_uint(value);
    break;
  case PROP_QUERY_RESULT:
    self->priv->query_result = g_value_get_pointer(value);
    break;
  case PROP_TYPE:
  case PROP_NAME:
  case PROP_MINIMUM:
  case PROP_MAXIMUM:
  case PROP_STEP:
  case PROP_DEFAULT_VALUE:
  case PROP_FLAGS:
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
gtk_v4l_control_get_property (GObject    *object,
                              guint       property_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  Gtkv4lControl *self = GTK_V4L_CONTROL (object);

  switch (property_id) {
  case PROP_FD:
    g_value_set_int (value, self->fd);
    break;
  case PROP_ID:
    g_value_set_uint (value, self->id);
    break;
  case PROP_QUERY_RESULT:
    G_OBJECT_WARN_INVALID_PSPEC (object, "write-only property",
                                 property_id, pspec);
    break;
  case PROP_TYPE:
    g_value_set_int (value, self->type);
    break;
  case PROP_NAME:
    g_value_set_string (value, self->name);
    break;
  case PROP_MINIMUM:
    g_value_set_int (value, self->minimum);
    break;
  case PROP_MAXIMUM:
    g_value_set_int (value, self->maximum);
    break;
  case PROP_STEP:
    g_value_set_int (value, self->step);
    break;
  case PROP_DEFAULT_VALUE:
    g_value_set_int (value, self->default_value);
    break;
  case PROP_FLAGS:
    g_value_set_uint (value, self->flags);
    break;
  default:
    /* We don't have any other property... */
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}

static GObject *
gtk_v4l_control_constructor (GType                  gtype,
                             guint                  n_properties,
                             GObjectConstructParam *properties)
{
  GObject *obj;
  Gtkv4lControl *self;

  {
    /* Always chain up to the parent constructor */
    GObjectClass *parent_class;  
    parent_class = G_OBJECT_CLASS (gtk_v4l_control_parent_class);
    obj = parent_class->constructor (gtype, n_properties, properties);
  }

  /* update the object state depending on constructor properties */
  self = GTK_V4L_CONTROL (obj);
  if (self->fd == -1) {
    g_error ("%s: No fd passed in to GtkV4lControl constructor", G_STRLOC);
    return obj;
  }
  if (self->id == 0 && self->priv->query_result == NULL) {
    g_error ("%s: No control id passed in to GtkV4lControl constructor",
             G_STRLOC);
    return obj;
  }

  if (self->priv->query_result) {
    gtk_v4l_control_handle_query_result(self, self->priv->query_result);
    self->priv->query_result = NULL;
  } else
    gtk_v4l_control_query(self);

  return obj;
}

static void
gtk_v4l_control_finalize (GObject *object)
{
  Gtkv4lControl *self = GTK_V4L_CONTROL (object);

  g_free (self->name);
}

static void
gtk_v4l_control_class_init (Gtkv4lControlClass *klass)
{
  GObjectClass *gobject_class = (GObjectClass *) klass;
  GParamSpec *pspec;

  g_type_class_add_private (klass, sizeof (Gtkv4lControlPrivate));

  gobject_class->constructor  = gtk_v4l_control_constructor;
  gobject_class->finalize     = gtk_v4l_control_finalize;
  gobject_class->set_property = gtk_v4l_control_set_property;
  gobject_class->get_property = gtk_v4l_control_get_property;

  pspec = g_param_spec_int ("fd",
                            "Gtkv4lControl construct prop",
                            "Set the fd",
                            G_MININT32,
                            G_MAXINT32,
                            -1,
                            G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class,
                                   PROP_FD,
                                   pspec);

  pspec = g_param_spec_uint ("id",
                             "Gtkv4lControl construct prop",
                             "Set the id",
                             0,
                             G_MAXUINT32,
                             0,
                             G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class,
                                   PROP_ID,
                                   pspec);

  pspec = g_param_spec_pointer ("query_result",
                                "Gtkv4lControl construct prop",
                                "Set the query_result",
                                G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE);
  g_object_class_install_property (gobject_class,
                                   PROP_QUERY_RESULT,
                                   pspec);

  pspec = g_param_spec_int ("type",
                            "Gtkv4lDevice read only prop",
                            "Get the type",
                            0,
                            G_MAXINT32,
                            0,
                            G_PARAM_READABLE);
  g_object_class_install_property (gobject_class,
                                   PROP_TYPE,
                                   pspec);

  pspec = g_param_spec_string ("name",
                               "Gtkv4lControl read only prop",
                               "Get the name",
                               NULL,
                               G_PARAM_READABLE);
  g_object_class_install_property (gobject_class,
                                   PROP_NAME,
                                   pspec);

  pspec = g_param_spec_int ("minimum",
                            "Gtkv4lControl read only prop",
                            "Get the minimum",
                            G_MININT32,
                            G_MAXINT32,
                            0,
                            G_PARAM_READABLE);
  g_object_class_install_property (gobject_class,
                                   PROP_MINIMUM,
                                   pspec);

  pspec = g_param_spec_int ("maximum",
                            "Gtkv4lControl read only prop",
                            "Get the maximum",
                            G_MININT32,
                            G_MAXINT32,
                            0,
                            G_PARAM_READABLE);
  g_object_class_install_property (gobject_class,
                                   PROP_MAXIMUM,
                                   pspec);

  pspec = g_param_spec_int ("step",
                            "Gtkv4lControl read only prop",
                            "Get the step",
                            G_MININT32,
                            G_MAXINT32,
                            0,
                            G_PARAM_READABLE);
  g_object_class_install_property (gobject_class,
                                   PROP_STEP,
                                   pspec);

  pspec = g_param_spec_int ("default_value",
                            "Gtkv4lControl read only prop",
                            "Get the default value",
                            G_MININT32,
                            G_MAXINT32,
                            0,
                            G_PARAM_READABLE);
  g_object_class_install_property (gobject_class,
                                   PROP_DEFAULT_VALUE,
                                   pspec);

  pspec = g_param_spec_uint ("flags",
                             "Gtkv4lControl read only prop",
                             "get the flags",
                             0,
                             G_MAXUINT32,
                             0,
                             G_PARAM_READABLE);
  g_object_class_install_property (gobject_class,
                                   PROP_FLAGS,
                                   pspec);
}

static void
gtk_v4l_control_init (Gtkv4lControl *self)
{
  Gtkv4lControlPrivate *priv;

  /* initialize the object */
  self->fd = -1;
  self->priv = priv = GTK_V4L_CONTROL_GET_PRIVATE(self);
}

static void
gtk_v4l_control_handle_query_result (Gtkv4lControl *self,
                                     const struct v4l2_queryctrl *query)
{
  g_free (self->name);

  self->id            = query->id;
  self->type          = query->type;
  self->name          = g_strdup((gchar *)query->name);
  self->minimum       = query->minimum;
  self->maximum       = query->maximum;
  self->step          = query->step;
  self->default_value = query->default_value;
  self->flags         = query->flags;
}

static void
gtk_v4l_control_query (Gtkv4lControl *self)
{
  struct v4l2_queryctrl query = { .id = self->id };
  
  if (v4l2_ioctl (self->fd, VIDIOC_QUERYCTRL, &query) == -1) {
    g_warning("query ctrl for id: %u failed", self->id);
    return;
  }

  gtk_v4l_control_handle_query_result (self, &query);
}

void
gtk_v4l_control_set (Gtkv4lControl *self, gint value)
{
  struct v4l2_control ctrl = { .id = self->id, .value = value };

  if (self->priv->value_valid && self->priv->value == value)
    return;

  if (v4l2_ioctl (self->fd, VIDIOC_S_CTRL, &ctrl) == -1) {
    g_warning("set ctrl for id: %u failed", self->id);
    return;
  }
  
  /* From the v4l2 api: "VIDIOC_S_CTRL is a write-only ioctl,
     it does not return the actual new value" */
  self->priv->value_valid = FALSE;
}

gint
gtk_v4l_control_get (Gtkv4lControl *self)
{
  if (self->flags & V4L2_CTRL_FLAG_WRITE_ONLY)
    return 0;

  if (!self->priv->value_valid) {
    struct v4l2_control ctrl = { .id = self->id };

    if (v4l2_ioctl (self->fd, VIDIOC_G_CTRL, &ctrl) == -1) {
      g_warning("get ctrl for id: %u failed", self->id);
      return 0;
    }
    self->priv->value = ctrl.value;
    self->priv->value_valid = TRUE;
  }

  return self->priv->value;
}

void
gtk_v4l_control_update (Gtkv4lControl *self)
{
  gtk_v4l_control_query (self);
  self->priv->value_valid = FALSE;  
}

gboolean
gtk_v4l_control_is_advanced (Gtkv4lControl *self)
{
  switch (self->id) {
  case V4L2_CID_BRIGHTNESS:
  case V4L2_CID_CONTRAST:
  case V4L2_CID_SATURATION:
  case V4L2_CID_HUE:
  case V4L2_CID_GAMMA:
  case V4L2_CID_HFLIP:
  case V4L2_CID_VFLIP:
  case V4L2_CID_POWER_LINE_FREQUENCY:

  case V4L2_CID_PAN_RELATIVE:
  case V4L2_CID_TILT_RELATIVE:
  case V4L2_CID_PAN_RESET:
  case V4L2_CID_TILT_RESET:
  case V4L2_CID_PAN_ABSOLUTE:
  case V4L2_CID_TILT_ABSOLUTE:

  case V4L2_CID_FOCUS_ABSOLUTE:
  case V4L2_CID_FOCUS_RELATIVE:
  case V4L2_CID_FOCUS_AUTO:

  case V4L2_CID_ZOOM_ABSOLUTE:
  case V4L2_CID_ZOOM_RELATIVE:
  case V4L2_CID_ZOOM_CONTINUOUS:
    return FALSE;
  }
  /* Everything else is considered advanced */
  return TRUE;
}
