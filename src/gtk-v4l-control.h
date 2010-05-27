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
  struct _Gtkv4lDevice *device;
  guint32 id;
  /* Control properties from v4l2_queryctrl, treat as read only */
  gint type;
  gchar *name;
  gint32 minimum;
  gint32 maximum;
  gint32 step;
  gint32 default_value;
  guint32 flags;
  /* Menu entries for menu type controls */
  GList *menu_entries;
  /* For users of the control to associate their own data with the control */
  gpointer user_data;
  Gtkv4lControlPrivate *priv;
};

struct _Gtkv4lControlClass {
  GObjectClass parent;
  /* signals */
  /* Called when an io error happens */
  void (*io_error) (Gtkv4lControl *control, const gchar *error_msg);
  /* Called when a control with the V4L2_CTRL_FLAG_UPDATE flag set gets set */
  void (*controls_need_update) (Gtkv4lControl *control);
};

GType gtk_v4l_control_get_type (void);
void gtk_v4l_control_set (Gtkv4lControl *control, gint value);
gint gtk_v4l_control_get (Gtkv4lControl *control);
gboolean gtk_v4l_control_is_advanced (Gtkv4lControl *self);

/* The below functions are meant for use by Gtkv4lDevice, there is
   never a need to call these from other sources (so please don't). */

/* Force re-reading of control properties and value from the driver / hw */
void gtk_v4l_control_update (Gtkv4lControl *self);

/* This adds certain fixed flags to controls based on their CID, as some
   drivers (esp in older kernels) don't properly report these flags. */
void gtk_v4l_control_fixup_flags (Gtkv4lControl *self);

#endif
