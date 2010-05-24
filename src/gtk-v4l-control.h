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

#ifndef __GTK_V4L_CONTROL_H__
#define __GTK_V4L_CONTROL_H__

#include <glib-object.h>

#define GTK_V4L_TYPE_CONTROL            (gtk_v4l_control_get_type ())
#define GTK_V4L_CONTROL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_V4L_TYPE_CONTROL, Gtkv4lControl))
#define GTK_V4L_CONTROL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_V4L_TYPE_CONTROL, Gtkv4lControlClass))
#define GTK_V4L_IS_CONTROL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_V4L_TYPE_CONTROL))
#define GTK_V4L_IS_CONTROL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_V4L_TYPE_CONTROL))
#define GTK_V4L_CONTROL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_V4L_TYPE_CONTROL, Gtkv4lControlClass))

typedef struct _Gtkv4lControl Gtkv4lControl;
typedef struct _Gtkv4lControlClass Gtkv4lControlClass;
typedef struct _Gtkv4lControlPrivate Gtkv4lControlPrivate;

struct _Gtkv4lControl {
  GObject parent;
  /* instance members */
  int fd;
  guint32 id;
  /* Control properties from v4l2_queryctrl, treat as read only */
  gint type;
  gchar *name;
  gint32 minimum;
  gint32 maximum;
  gint32 step;
  gint32 default_value;
  guint32 flags;
  /* For users of the control to associate their own data with the control */
  gpointer user_data;
  Gtkv4lControlPrivate *priv;
};

struct _Gtkv4lControlClass {
  GObjectClass parent;
  /* class members */
};

void gtk_v4l_control_set (Gtkv4lControl *control, gint value);
gint gtk_v4l_control_get (Gtkv4lControl *control);
/* Force re-reading of control properties and value from the driver / hw */
void gtk_v4l_control_update (Gtkv4lControl *self);

#endif
