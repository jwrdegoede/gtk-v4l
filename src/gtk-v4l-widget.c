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

#include <linux/videodev2.h>
#include "gtk-v4l-control.h"
#include "gtk-v4l-widget.h"

#define GTK_V4L_WIDGET_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GTK_V4L_TYPE_WIDGET, Gtkv4lWidgetPrivate))

enum
{
  PROP_0,
  PROP_DEVICE,
};

enum
{
  IO_ERROR_SIGNAL,
  LAST_SIGNAL,
};

typedef struct _Gtkv4lWidgetControlData Gtkv4lWidgetControlData;

struct _Gtkv4lWidgetPrivate {
  gint placeholder;
};

struct _Gtkv4lWidgetControlData {
  GtkWidget *widget;
  gulong widget_handler;
  gulong io_error_handler;
  gulong controls_need_update_handler;
};

static guint signals[LAST_SIGNAL] = { 0, };

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

static void
gtk_v4l_widget_set_widget_value (Gtkv4lControl *control, gint value)
{
  Gtkv4lWidgetControlData *control_data = control->user_data;

  switch (control->type) {
  case V4L2_CTRL_TYPE_INTEGER:
    if (!(control->flags & V4L2_CTRL_FLAG_WRITE_ONLY))
      gtk_range_set_value (GTK_RANGE (control_data->widget), value);
    break;
  case V4L2_CTRL_TYPE_BOOLEAN:
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (control_data->widget),
                                  value);
    break;
  case V4L2_CTRL_TYPE_MENU:
    gtk_combo_box_set_active (GTK_COMBO_BOX (control_data->widget), value);
    break;
  case V4L2_CTRL_TYPE_BUTTON:
    /* do nothing */
    break;
  }
}

static void
gtk_v4l_widget_update_control (gpointer data, gpointer user_data)
{
  Gtkv4lControl *control = GTK_V4L_CONTROL (data);
  Gtkv4lWidgetControlData *control_data = control->user_data;

  if (!control_data)
    return; /* This control has no associated widget */

  if (!(control->flags & V4L2_CTRL_FLAG_WRITE_ONLY)) {
    gint value = gtk_v4l_control_get(control);
    g_signal_handler_block (control_data->widget, control_data->widget_handler);
    gtk_v4l_widget_set_widget_value (control, value);
    g_signal_handler_unblock (control_data->widget, control_data->widget_handler);
  }

  if (control->flags &
      (V4L2_CTRL_FLAG_GRABBED | V4L2_CTRL_FLAG_READ_ONLY | V4L2_CTRL_FLAG_INACTIVE))
    gtk_widget_set_sensitive (control_data->widget, FALSE);
  else
    gtk_widget_set_sensitive (control_data->widget, TRUE);
}

void int_control_changed_cb (GtkWidget *range, gpointer user_data)
{
  Gtkv4lControl *control = GTK_V4L_CONTROL (user_data);
  gdouble value;

  value = gtk_range_get_value (GTK_RANGE (range));
  gtk_v4l_control_set (control, value);
  /* Set the widget to what the hardware reports as the actual result */
  gtk_v4l_widget_update_control (control, NULL);
}

gchar *
int_control_format_cb (GtkWidget *scale, gdouble value, gpointer user_data)
{
  Gtkv4lControl *control = GTK_V4L_CONTROL (user_data);
  gint div = control->maximum - control->minimum;

  value -= control->minimum;

  return g_strdup_printf("%5.2f %%", ((value / div) * 100) );
}

static GtkWidget *
gtk_v4l_widget_create_int_widget (Gtkv4lControl *control)
{  
  GtkWidget *HScale;
  Gtkv4lWidgetControlData *control_data = control->user_data;
  gint min = control->minimum, max = control->maximum, step = control->step;

  HScale = gtk_hscale_new_with_range (min,max,step);
  gtk_scale_set_value_pos (GTK_SCALE(HScale), GTK_POS_RIGHT);
  gtk_scale_set_digits (GTK_SCALE(HScale), 0);
  gtk_range_set_increments (GTK_RANGE(HScale), step, step);
  control_data->widget_handler =
    g_signal_connect (G_OBJECT (HScale), "value-changed",
                      G_CALLBACK (int_control_changed_cb), control);
  g_signal_connect (G_OBJECT (HScale), "format-value",
                    G_CALLBACK (int_control_format_cb), control);

  return HScale;
}

static void 
gtk_v4l_widget_relative_int_dec_clicked (GtkWidget *button, gpointer user_data)
{
  Gtkv4lControl *control = GTK_V4L_CONTROL (user_data);
  gint step, range = control->maximum - control->minimum;

  /* Allow the user to walk over the entire range in 50 clicks */
  step = range / 50;
  if (step < control->step)
    step = control->step;

  /* Assume range / 2 is the no change value */
  gtk_v4l_control_set (control, control->minimum + range / 2 - step);
}

static void 
gtk_v4l_widget_relative_int_inc_clicked (GtkWidget *button, gpointer user_data)
{
  Gtkv4lControl *control = GTK_V4L_CONTROL (user_data);
  gint step, range = control->maximum - control->minimum;

  /* Allow the user to walk over the entire range in 50 clicks */
  step = range / 50;
  if (step < control->step)
    step = control->step;

  /* Assume range / 2 is the no change value */
  gtk_v4l_control_set (control, control->minimum + range / 2 + step);
}

static GtkWidget *
gtk_v4l_widget_create_relative_int_widget (Gtkv4lControl *control)
{  
  GtkWidget *table, *button;

  table = gtk_table_new(1, 2, FALSE);

  button = gtk_button_new_with_label("<");
  gtk_widget_set_size_request (button, 125, -1);
  g_signal_connect(G_OBJECT (button), "clicked",
                     G_CALLBACK (gtk_v4l_widget_relative_int_dec_clicked),
                     control);
  gtk_table_attach (GTK_TABLE (table), button, 0, 1, 0, 1, GTK_FILL, GTK_FILL, 0, 0);

  button = gtk_button_new_with_label(">");
  gtk_widget_set_size_request (button, 125, -1);
  g_signal_connect(G_OBJECT (button), "clicked",
                     G_CALLBACK (gtk_v4l_widget_relative_int_inc_clicked),
                     control);
  gtk_table_attach (GTK_TABLE (table), button, 1, 2, 0, 1, GTK_FILL, GTK_FILL, 0, 0);

  return table;
}

void bool_control_changed_cb (GtkWidget *button, gpointer user_data)
{
  Gtkv4lControl *control = GTK_V4L_CONTROL (user_data);
  gboolean state;

  state = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button));
  gtk_v4l_control_set (control, state);
  /* Set the widget to what the hardware reports as the actual result */
  gtk_v4l_widget_update_control (control, NULL);
}

static GtkWidget *
gtk_v4l_widget_create_bool_widget (Gtkv4lControl *control)
{
  Gtkv4lWidgetControlData *control_data = control->user_data;
  GtkWidget *check;

  check = gtk_check_button_new();
  control_data->widget_handler =
    g_signal_connect (G_OBJECT(check), "clicked",
                      G_CALLBACK (bool_control_changed_cb), control);

  return check;
}

void menu_control_changed_cb (GtkWidget *combo, gpointer user_data)
{
  Gtkv4lControl *control = GTK_V4L_CONTROL (user_data);
  gint value;

  value = gtk_combo_box_get_active(GTK_COMBO_BOX (combo));
  gtk_v4l_control_set (control, value);
  /* Set the widget to what the hardware reports as the actual result */
  gtk_v4l_widget_update_control (control, NULL);
}

static GtkWidget *
gtk_v4l_widget_create_menu_widget (Gtkv4lControl *control)
{
  Gtkv4lWidgetControlData *control_data = control->user_data;
  GtkWidget *combo;
  GList *elem;

  combo = gtk_combo_box_new_text();

  for (elem = g_list_first (control->menu_entries);
       elem; elem = g_list_next (elem))
    gtk_combo_box_append_text (GTK_COMBO_BOX(combo), (gchar *) elem->data);

  control_data->widget_handler =
    g_signal_connect (G_OBJECT(combo), "changed",
                      G_CALLBACK(menu_control_changed_cb), control);

  return combo;
}

void button_control_changed_cb (GtkWidget *button, gpointer user_data)
{
  Gtkv4lControl *control = GTK_V4L_CONTROL (user_data);

  gtk_v4l_control_set (control, 0xffffffff);
}

static GtkWidget *
gtk_v4l_widget_create_button_widget (Gtkv4lControl *control)
{  
  GtkWidget *button;
  Gtkv4lWidgetControlData *control_data = control->user_data;

  button = gtk_button_new_with_label(control->name);
  control_data->widget_handler =
    g_signal_connect(G_OBJECT(button), "clicked",
                     G_CALLBACK(button_control_changed_cb), control);

  return button;
}

static void
gtk_v4l_widget_io_error_cb (Gtkv4lControl *control,
                            const gchar *error_msg,
                            gpointer user_data)
{
  Gtkv4lWidget *self = GTK_V4L_WIDGET (user_data);

  g_signal_emit (self, signals[IO_ERROR_SIGNAL], 0, error_msg);
}

static void
gtk_v4l_widget_controls_need_update_cb (Gtkv4lControl *control,
                                        gpointer user_data)
{
  Gtkv4lWidget *self = GTK_V4L_WIDGET (user_data);

  gtk_v4l_device_update_controls (self->device);
  g_list_foreach (gtk_v4l_device_get_controls (self->device),
                  gtk_v4l_widget_update_control, NULL);
}

static GObject *
gtk_v4l_widget_constructor (GType                  gtype,
                            guint                  n_properties,
                            GObjectConstructParam *properties)
{
  GObject *obj;
  GList *elem;
  Gtkv4lWidget *self;
  Gtkv4lWidgetControlData *control_data;
  GtkWidget *control_widget, *label, *align, *advanced_table = NULL;
  int rownum = 0, rownum_advanced = 0;
  gulong handler_id;

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

    control_data = control->user_data = g_malloc (sizeof (*control_data));

    switch(control->type) {
    case V4L2_CTRL_TYPE_INTEGER:
      if (control->flags & V4L2_CTRL_FLAG_WRITE_ONLY)
        control_widget = gtk_v4l_widget_create_relative_int_widget (control);
      else
        control_widget = gtk_v4l_widget_create_int_widget (control);
      break;
    case V4L2_CTRL_TYPE_BOOLEAN:
      control_widget = gtk_v4l_widget_create_bool_widget (control);
      break;
    case V4L2_CTRL_TYPE_MENU:
      control_widget = gtk_v4l_widget_create_menu_widget (control);
      break;
    case V4L2_CTRL_TYPE_BUTTON:
      control_widget = gtk_v4l_widget_create_button_widget (control);
      break;
    default:
      g_warning("Skipping control %s with unknown type %d",
                control->name, control->type);
    case V4L2_CTRL_TYPE_CTRL_CLASS:
      g_free (control_data);
      control->user_data = NULL;
      continue;
    }

    control_data->widget = control_widget;
    handler_id = g_signal_connect (control,
                                   "io_error",
                                   G_CALLBACK (gtk_v4l_widget_io_error_cb),
                                   self);
    control_data->io_error_handler = handler_id;
    handler_id = g_signal_connect (control,
                                   "controls_need_update",
                                   G_CALLBACK (gtk_v4l_widget_controls_need_update_cb),
                                   self);
    control_data->controls_need_update_handler = handler_id;

    gtk_widget_set_size_request (control_widget, 250, -1);
    gtk_v4l_widget_update_control (control, NULL);

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
  GList *elem;

  for (elem = g_list_first (gtk_v4l_device_get_controls (self->device));
       elem; elem = g_list_next (elem)) {
    Gtkv4lControl *control = GTK_V4L_CONTROL (elem->data);
    Gtkv4lWidgetControlData *control_data = control->user_data;

    if (!control_data)
      continue;

    g_signal_handler_disconnect (control, control_data->io_error_handler);
    g_signal_handler_disconnect (control, control_data->controls_need_update_handler);
    g_free (control_data);
    control->user_data = NULL;
  }

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

  signals[IO_ERROR_SIGNAL] = g_signal_new ("io_error",
                                         G_TYPE_FROM_CLASS (klass),
                                         G_SIGNAL_RUN_LAST,
                                         G_STRUCT_OFFSET (Gtkv4lWidgetClass, io_error),
                                         NULL,
                                         NULL,
                                         g_cclosure_marshal_VOID__STRING,
                                         G_TYPE_NONE,
                                         1,
                                         G_TYPE_STRING);
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

    if (control->flags & (V4L2_CTRL_FLAG_GRABBED | V4L2_CTRL_FLAG_READ_ONLY |
                          V4L2_CTRL_FLAG_INACTIVE))
      continue;

    gtk_v4l_widget_set_widget_value (control, control->default_value);
  }
}
