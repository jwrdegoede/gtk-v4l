/* gtk-v4l - GTK tool for control v4l camera properties
 *
 * Copyright (C) 2010 - Huzaifa Sidhpurwala <huzaifas@redhat.com>
 * Copyright (C) 2010 - Hans de Goede <hdegoede@redhat.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 *
 */

#ifndef __GTK_V4L_WIDGET_H__
#define __GTK_V4L_WIDGET_H__

#include <gtk/gtk.h>
#include "gtk-v4l-device.h"

#define GTK_V4L_TYPE_WIDGET            (gtk_v4l_widget_get_type ())
#define GTK_V4L_WIDGET(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_V4L_TYPE_WIDGET, Gtkv4lWidget))
#define GTK_V4L_WIDGET_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_V4L_TYPE_WIDGET, Gtkv4lWidgetClass))
#define GTK_V4L_IS_WIDGET(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_V4L_TYPE_WIDGET))
#define GTK_V4L_IS_WIDGET_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_V4L_TYPE_WIDGET))
#define GTK_V4L_WIDGET_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_V4L_TYPE_WIDGET, Gtkv4lWidgetClass))

typedef struct _Gtkv4lWidget Gtkv4lWidget;
typedef struct _Gtkv4lWidgetClass Gtkv4lWidgetClass;
typedef struct _Gtkv4lWidgetPrivate Gtkv4lWidgetPrivate;

struct _Gtkv4lWidget {
  GtkTable parent;
  /* instance members */
  Gtkv4lDevice *device;
  Gtkv4lWidgetPrivate *priv;
};

struct _Gtkv4lWidgetClass {
  GtkTableClass parent;
  /* signals */
  /* Called when an io error happens */
  void (*io_error) (Gtkv4lWidget *widget, const gchar *error_msg);
};

GType gtk_v4l_widget_get_type (void);
GtkWidget *gtk_v4l_widget_new (Gtkv4lDevice *device);
void gtk_v4l_widget_reset_to_defaults (Gtkv4lWidget *self);

#endif
