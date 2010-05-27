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

#include <linux/videodev.h>
#include <libv4l2.h>
#include "gtk-v4l-control.h"
#include "gtk-v4l-widget.h"

#define GTK_V4L_WIDGET_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GTK_V4L_TYPE_WIDGET, Gtkv4lWidgetPrivate))

enum
{
  PROP_0,
  PROP_DEVICE,
};

struct _Gtkv4lWidgetPrivate {
  gint placeholder;
};

/* will create gtk_v4l_widget_get_type and set gtk_v4l_widget_parent_class */
G_DEFINE_TYPE (Gtkv4lWidget, gtk_v4l_widget, GTK_TYPE_TABLE);

static void
gtk_v4l_widget_set_property (GObject      *object,
                             guint         property_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  Gtkv4lWidget *self = GTK_V4L_WIDGET (object);

  switch (property_id) {
  case PROP_DEVICE:
    self->device = g_value_dup_object (value);
    break;
  default:
    /* We don't have any other property... */
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}

static void
gtk_v4l_widget_get_property (GObject    *object,
                             guint       property_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  Gtkv4lWidget *self = GTK_V4L_WIDGET (object);

  switch (property_id) {
  case PROP_DEVICE:
    g_value_set_object (value, self->device);
    break;
  default:
    /* We don't have any other property... */
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}

void int_control_changed_cb (GtkWidget *range, gpointer user_data)
{
  static int in_handler = 0;
  Gtkv4lControl *control = GTK_V4L_CONTROL (user_data);
  gdouble value;

  /* We set the widget to what the hardware reports it can actually
     achieve this may result in recursive calls */
  if (in_handler)
    return;

  value = gtk_range_get_value (GTK_RANGE (range));
  gtk_v4l_control_set (control, value);

  /* Set the widget to what the hardware reports as the actual result */
  value = gtk_v4l_control_get (control);
  in_handler++;
  gtk_range_set_value (GTK_RANGE (range), value);
  in_handler--;
}

gchar *
int_control_format_cb (GtkWidget *scale, gdouble value, gpointer user_data)
{
  Gtkv4lControl *control = GTK_V4L_CONTROL (user_data);
  gint div = control->maximum - control->minimum;

  value -= control->minimum;

  return g_strdup_printf("%5.2f %%", ((value / div) * 100) );
}

GtkWidget *v4l2_create_int_widget (Gtkv4lControl *control)
{  
  GtkWidget *HScale;
  gint min = control->minimum, max = control->maximum, step = control->step;

  HScale = gtk_hscale_new_with_range (min,max,step);
  gtk_scale_set_value_pos (GTK_SCALE(HScale), GTK_POS_RIGHT);
  gtk_scale_set_digits (GTK_SCALE(HScale), 0);
  gtk_range_set_increments (GTK_RANGE(HScale), step, step);
  gtk_range_set_value (GTK_RANGE(HScale),
                       (gdouble) gtk_v4l_control_get(control));
  g_signal_connect (G_OBJECT (HScale), "value-changed", G_CALLBACK (int_control_changed_cb), control);
  g_signal_connect (G_OBJECT (HScale), "format-value", G_CALLBACK (int_control_format_cb), control);

  return HScale;
}

void bool_control_changed_cb (GtkWidget *button, gpointer user_data)
{
  static int in_handler = 0;
  Gtkv4lControl *control = GTK_V4L_CONTROL (user_data);
  gboolean state;

  /* We set the widget to what the hardware reports it can actually
     achieve this may result in recursive calls */
  if (in_handler)
    return;

  state = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button));
  gtk_v4l_control_set (control, state);

  /* Set the widget to what the hardware reports as the actual result */
  state = gtk_v4l_control_get (control);
  in_handler++;
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), state);
  in_handler--;
}

GtkWidget *v4l2_create_bool_widget (Gtkv4lControl *control)
{
  GtkWidget *check;
  gboolean state;

  state = (gboolean) gtk_v4l_control_get (control);
  check = gtk_check_button_new();
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(check), state);
  g_signal_connect ( G_OBJECT(check), "clicked", G_CALLBACK (bool_control_changed_cb), control);

  return check;
}

void menu_control_changed_cb (GtkWidget *combo, gpointer user_data)
{
  static int in_handler = 0;
  Gtkv4lControl *control = GTK_V4L_CONTROL (user_data);
  gint value;

  /* We set the widget to what the hardware reports it can actually
     achieve this may result in recursive calls */
  if (in_handler)
    return;

  value = gtk_combo_box_get_active(GTK_COMBO_BOX (combo));
  gtk_v4l_control_set (control, value);

  /* Set the widget to what the hardware reports as the actual result */
  value = gtk_v4l_control_get (control);
  in_handler++;
  gtk_combo_box_set_active (GTK_COMBO_BOX (combo), value);
  in_handler--;
}

GtkWidget *v4l2_create_menu_widget (Gtkv4lControl *control)
{
  GtkWidget *combo;
  GList *elem;

  combo = gtk_combo_box_new_text();

  for (elem = g_list_first (control->menu_entries);
       elem; elem = g_list_next (elem))
    gtk_combo_box_append_text (GTK_COMBO_BOX(combo), (gchar *) elem->data);

  gtk_combo_box_set_active (GTK_COMBO_BOX(combo),
                            gtk_v4l_control_get(control));
  g_signal_connect (G_OBJECT(combo), "changed", G_CALLBACK(menu_control_changed_cb), control);

  return combo;
}

void button_control_changed_cb (GtkWidget *button, gpointer user_data)
{
  Gtkv4lControl *control = GTK_V4L_CONTROL (user_data);

  gtk_v4l_control_set (control, 0xffffffff);
}

GtkWidget *v4l2_create_button_widget (Gtkv4lControl *control)
{  
  GtkWidget *button;
  
  button = gtk_button_new_with_label("Reset");
  g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(button_control_changed_cb), control);

  return button;
}

static GObject *
gtk_v4l_widget_constructor (GType                  gtype,
                            guint                  n_properties,
                            GObjectConstructParam *properties)
{
  GObject *obj;
  GList *elem;
  Gtkv4lWidget *self;
  GtkWidget *control_widget, *label, *align, *advanced_table = NULL;
  int rownum = 0, rownum_advanced = 0;

  {
    /* Always chain up to the parent constructor */
    GObjectClass *parent_class;
    parent_class = G_OBJECT_CLASS (gtk_v4l_widget_parent_class);
    obj = parent_class->constructor (gtype, n_properties, properties);
  }

  /* update the object state depending on constructor properties */
  self = GTK_V4L_WIDGET (obj);

  for (elem = g_list_first (gtk_v4l_device_get_controls (self->device));
       elem; elem = g_list_next (elem)) {
    Gtkv4lControl *control = GTK_V4L_CONTROL (elem->data);

    switch(control->type) {
    case V4L2_CTRL_TYPE_INTEGER:
      control_widget = v4l2_create_int_widget (control);
      break;
    case V4L2_CTRL_TYPE_BOOLEAN:
      control_widget = v4l2_create_bool_widget (control);
      break;
    case V4L2_CTRL_TYPE_MENU:
      control_widget = v4l2_create_menu_widget (control);
      break;
    case V4L2_CTRL_TYPE_BUTTON:
      control_widget = v4l2_create_button_widget (control);
      break;
    default:
      g_warning("Skipping control %s with unknown type %d",
                control->name, control->type);
      continue;
    }

    control->user_data = control_widget;

    gtk_widget_set_size_request (control_widget, 250, -1);

    label = gtk_label_new ((gchar *)control->name);
    gtk_misc_set_alignment (GTK_MISC(label),0.0,0.5);

    align = gtk_alignment_new (0.0,0.5,0.0,0.0);
    gtk_container_add (GTK_CONTAINER(align), control_widget);

    if (gtk_v4l_control_is_advanced (control)) {
       rownum_advanced++;
       if (!advanced_table)
         advanced_table = gtk_table_new(rownum_advanced, 2, FALSE);
       else
         gtk_table_resize (GTK_TABLE (advanced_table), rownum_advanced, 2);
       gtk_table_attach (GTK_TABLE (advanced_table), label, 0,1, rownum_advanced, rownum_advanced + 1, GTK_FILL, GTK_FILL, 5, 5);
       gtk_table_attach (GTK_TABLE (advanced_table), align, 1,2, rownum_advanced, rownum_advanced + 1, GTK_FILL, GTK_FILL, 5, 5);
    } else {
       rownum++;
       gtk_table_resize (GTK_TABLE (self), rownum, 2);
       gtk_table_attach (GTK_TABLE (self), label, 0,1, rownum, rownum + 1, GTK_FILL, GTK_FILL, 5, 5);
       gtk_table_attach (GTK_TABLE (self), align, 1,2, rownum, rownum + 1, GTK_FILL, GTK_FILL, 5, 5);
    }
  }

  if (advanced_table) {
    GtkWidget *expander = gtk_expander_new ("More properties");

    gtk_expander_set_expanded (GTK_EXPANDER (expander), FALSE);
    gtk_container_add (GTK_CONTAINER (expander), advanced_table);
    rownum++;
    gtk_table_resize (GTK_TABLE(self), rownum, 2);
    gtk_table_attach (GTK_TABLE(self), expander, 0, 2, rownum, rownum + 1, GTK_FILL, GTK_FILL, 0, 5);
  }

  return obj;
}

static void
gtk_v4l_widget_finalize (GObject *object)
{
  Gtkv4lWidget *self = GTK_V4L_WIDGET (object);

  g_object_unref (self->device);
}

static void
gtk_v4l_widget_class_init (Gtkv4lWidgetClass *klass)
{
  GObjectClass *gobject_class = (GObjectClass *) klass;
  GParamSpec *pspec;

  g_type_class_add_private (klass, sizeof (Gtkv4lWidgetPrivate));

  gobject_class->constructor  = gtk_v4l_widget_constructor;
  gobject_class->finalize     = gtk_v4l_widget_finalize;
  gobject_class->set_property = gtk_v4l_widget_set_property;
  gobject_class->get_property = gtk_v4l_widget_get_property;

  pspec = g_param_spec_object ("device",
                               "Gtkv4lWidget construct prop",
                               "Set the Gtkv4lDevice",
                               GTK_V4L_TYPE_DEVICE,
                               G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class,
                                   PROP_DEVICE,
                                   pspec);
}

static void
gtk_v4l_widget_init (Gtkv4lWidget *self)
{
  Gtkv4lWidgetPrivate *priv;

  /* initialize the object */
  self->priv = priv = GTK_V4L_WIDGET_GET_PRIVATE(self);
}

GtkWidget *
gtk_v4l_widget_new (Gtkv4lDevice *device)
{
  return g_object_new (GTK_V4L_TYPE_WIDGET, "device", device, NULL);
}

void gtk_v4l_widget_reset_to_defaults (Gtkv4lWidget *self)
{
  GList *elem;

  for (elem = g_list_first (gtk_v4l_device_get_controls (self->device));
       elem; elem = g_list_next (elem)) {
    Gtkv4lControl *control = GTK_V4L_CONTROL (elem->data);

    switch (control->type) {
    case V4L2_CTRL_TYPE_INTEGER:
      gtk_range_set_value (GTK_RANGE (control->user_data),
                           control->default_value);
      break;
    case V4L2_CTRL_TYPE_BOOLEAN:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (control->user_data),
                                    control->default_value);
      break;
    case V4L2_CTRL_TYPE_MENU:
      gtk_combo_box_set_active (GTK_COMBO_BOX (control->user_data),
                                control->default_value);
      break;
    case V4L2_CTRL_TYPE_BUTTON:
      /* do nothing */
      break;
    }
  }
}
