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


#include "gtk-v4l.h"
#include "gtk-v4l-device-list.h"

Gtkv4lDeviceList *devlist;
Gtkv4lDevice *curr_dev;
GtkWidget *window,*advanced_window,*dev_combo;
GtkTable *main_table,*table,*table2=NULL;
GtkWidget *content_area,*content_area2;

GtkWidget  *label_driver, *label_card, *label_bus;

gchar *device = NULL;

gboolean expanded=FALSE;

GOptionEntry entries[] =
  {
    { "device", 'd',0,G_OPTION_ARG_STRING, &device, "V4L2 device", NULL},
    {NULL}
  };


GList *list=NULL;

struct v4l2_capability cap;

int rownum,rownum_advanced,controls=0;
int curr_controls=0;
gboolean started_cb=FALSE;

void close_cb(GtkWidget *widget, gpointer user_data)
{
	gtk_main_quit();
}
void destroy(GtkWidget *widget, gpointer user_data)
{
	gtk_main_quit();
}

void bool_control_changed_cb (GtkButton *button, gpointer user_data)
{
	int id;
	gboolean state;
	id = GPOINTER_TO_INT (user_data);
	state = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(button));
	if (started_cb) {
	        if (!v4l2_list_edit_element(id,state))
			g_warning ("Error inserting value in the list: %d, %d", id, state);
		
	       if (v4l2_write_to_driver_one(id,state,FALSE) == EXIT_FAILURE) 
			g_warning ("Error writing values to the driver");
	}
}

void button_control_changed_cb (GtkButton *button, gpointer user_data)
{
	int id;
	id = GPOINTER_TO_INT(user_data);
	if (v4l2_write_to_driver_one(id,0,TRUE) == EXIT_FAILURE)
		g_warning("Error writting button value to the driver");
}

void v4l2_list_add (__u32 id, __s32 value, __s32 def, GtkWidget *w, int ctrl_type)
{
	struct v4l2_property* temp;
	temp = g_malloc (sizeof (struct v4l2_property));
	temp->id = id; temp->value = value; temp->def = def;
	temp->w = w;
	temp->ctrl_type = ctrl_type;
	list = g_list_append (list, (gpointer) temp);
}

gboolean v4l2_list_edit_element (__u32 id, __s32 value)
{
	gboolean output=FALSE;
	GList *l;
	struct v4l2_property *s;
	for (l=list; l; l=l->next) {
		s = l->data;
		if (s->id == id) {
			s->value = value;
			output = TRUE;
		}
	}
	return output;
}
void v4l2_list_print(void)
{
	GList *l;
	struct v4l2_property *s;
	for (l=list; l;l=l->next) {
		s = l->data;
		fprintf(stderr, "\nid=%d, value=%d, default=%d",s->id, s->value,s->def);
	}
	
}

void v4l2_list_reset(void)
{
	GList *l;
	struct v4l2_property *s;
	for (l=list; l; l=l->next) {
		s=l->data;
		g_free (s);
		g_list_free(l);
	}
	list = NULL;
}

void v4l2_reset_list_to_default(void)
{
	GList *l;
	struct v4l2_property *s;
	for (l=list;l;l=l->next) {
		s=l->data;
		s->value = s->def;
		if (v4l2_write_to_driver_one(s->id,s->value,FALSE) == EXIT_SUCCESS) {
			switch(s->ctrl_type) {
				case V4L2_CTRL_TYPE_INTEGER:
					gtk_range_set_value(GTK_RANGE(s->w),s->value);
					break;
				case V4L2_CTRL_TYPE_BOOLEAN:
					gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(s->w),s->value);
					break;
				case V4L2_CTRL_TYPE_MENU:
					gtk_combo_box_set_active(GTK_COMBO_BOX(s->w),s->value);
					break;
				case V4L2_CTRL_TYPE_BUTTON:
					// do nothing
					break;
			}
		}
			
	}
}

void int_control_changed_cb (GtkRange *range, gpointer user_data)
{
	int id;
	gdouble value;

	id = GPOINTER_TO_INT (user_data);
	value = gtk_range_get_value (GTK_RANGE (range));
	if (started_cb) {
		if (!v4l2_list_edit_element(id,value))
			g_warning ("Error inserting value in the list: %d, %d", (int)id, (int)value);
       		 if (v4l2_write_to_driver_one(id,value,FALSE) == EXIT_FAILURE) 
			g_warning ("Error writing values to the driver");			
	}
}

gchar * int_control_format_cb (GtkScale *scale, gdouble value, gpointer user_data)
{
	int max = GPOINTER_TO_INT(user_data);
	return  g_strdup_printf("%5.2f %%",  ((value/max)*100) );
	

}

void menu_control_changed_cb (GtkWidget *wid, gpointer user_data)
{
	int id;
	gdouble value;

        id = GPOINTER_TO_INT (user_data);
	value = gtk_combo_box_get_active(GTK_COMBO_BOX(wid));
	
        if (started_cb) {
		if (!v4l2_list_edit_element(id,value))
			g_warning ("Error inserting value in the list: %d, %d", (int)id, (int)value);
		if (v4l2_write_to_driver_one(id,value,FALSE) == EXIT_FAILURE)
			g_warning ("Error writing values to the driver");
        }

}

void v4l2_add_menu_control (struct v4l2_queryctrl ctrl, struct v4l2_control c, gboolean advanced)
{
	GtkWidget *label,*combo,*align;
	int k;	
	curr_controls++;
	combo = gtk_combo_box_new_text();
	for (k=ctrl.minimum; k<=ctrl.maximum; k++) {
		struct v4l2_querymenu qm;
	        qm.id = ctrl.id;
	        qm.index = k;
		if(ioctl(curr_dev->fd, VIDIOC_QUERYMENU, &qm) == 0) 
			gtk_combo_box_append_text (GTK_COMBO_BOX(combo), (const gchar *) qm.name);
		else
			g_warning ("Unable to use menu item for :%d", qm.index);
	}

        gtk_widget_set_size_request (GTK_WIDGET(combo),250, -1);
	
	gtk_combo_box_set_active (GTK_COMBO_BOX(combo), c.value);
	g_signal_connect (G_OBJECT(combo), "changed", G_CALLBACK(menu_control_changed_cb),GINT_TO_POINTER(ctrl.id));

	label = gtk_label_new ((gchar *)ctrl.name);
	gtk_misc_set_alignment (GTK_MISC(label),0.0,0.5);

	align = gtk_alignment_new (0.0,0.5,0.0,0.0);
	gtk_container_add (GTK_CONTAINER(align), combo);

        if ((advanced == TRUE) && curr_controls > MIN_CONTROLS_ON_MAIN_PAGE) {
                rownum_advanced++;
                gtk_table_resize (table2, rownum_advanced, 2);
                gtk_table_attach (GTK_TABLE(table2), label, 0,1,rownum_advanced, rownum_advanced+1, GTK_FILL, GTK_FILL, 5, 5);
                gtk_table_attach (GTK_TABLE(table2), align, 1,2,rownum_advanced, rownum_advanced+1, GTK_FILL, GTK_FILL, 5, 5);
        } else {
                rownum++;
                gtk_table_resize (table, rownum, 2);
                gtk_table_attach (GTK_TABLE(table), label, 0,1,rownum, rownum+1, GTK_FILL, GTK_FILL, 5, 5);
                gtk_table_attach (GTK_TABLE(table), align, 1,2,rownum, rownum+1, GTK_FILL, GTK_FILL, 5, 5);
        }

        v4l2_list_add (ctrl.id, c.value,ctrl.default_value,GTK_WIDGET(combo),V4L2_CTRL_TYPE_MENU);

}

void v4l2_add_int_control (struct v4l2_queryctrl ctrl,struct v4l2_control c,gboolean advanced)
{	
	GtkWidget *HScale,*label;
	curr_controls++;
	gint min, max, step,def;
	min = ctrl.minimum; max = ctrl.maximum; step = ctrl.step; def = c.value;

	HScale = gtk_hscale_new_with_range (min,max,step);
	gtk_scale_set_value_pos (GTK_SCALE(HScale), GTK_POS_RIGHT);
	gtk_scale_set_digits (GTK_SCALE(HScale), 0);
	gtk_widget_set_size_request (GTK_WIDGET(HScale),250,-1);
	g_signal_connect (G_OBJECT (HScale), "value-changed", G_CALLBACK (int_control_changed_cb), GINT_TO_POINTER(ctrl.id));
	g_signal_connect (G_OBJECT (HScale), "format-value", G_CALLBACK (int_control_format_cb), GINT_TO_POINTER(max));
	gtk_range_set_increments (GTK_RANGE(HScale), step, step);
	gtk_range_set_value (GTK_RANGE(HScale), (gdouble) def);
//	gtk_scale_set_draw_value(GTK_SCALE(HScale), FALSE);

	label = gtk_label_new ((const gchar *)ctrl.name);
	gtk_misc_set_alignment (GTK_MISC(label),0.0,0.5);	
	gtk_widget_set_size_request (label, 250, -1);

//	tag = gtk_label_new (g_strdup_printf("%d", def));
//	gtk_misc_set_alignment (GTK_MISC(tag), 0.0,0.5);
//	gtk_widget_set_size_request (label, 50,-1);

	if ((advanced == TRUE) && curr_controls > MIN_CONTROLS_ON_MAIN_PAGE) {
		rownum_advanced++;
		gtk_table_resize (table2, rownum_advanced, 2);
		gtk_table_attach (GTK_TABLE(table2), label, 0,1,rownum_advanced, rownum_advanced+1, GTK_FILL, GTK_FILL, 5, 5);
		gtk_table_attach (GTK_TABLE(table2), HScale, 1,2,rownum_advanced, rownum_advanced+1, GTK_FILL, GTK_FILL, 5, 5);
//		gtk_table_attach (GTK_TABLE(table2), tag, 1,2, rownum_advanced, rownum_advanced+1, GTK_FILL, GTK_FILL, 5, 5);
	} else {
		rownum++;
		gtk_table_resize (table, rownum, 2);
		gtk_table_attach (GTK_TABLE(table), label, 0,1,rownum, rownum+1, GTK_FILL, GTK_FILL, 5, 5);
		gtk_table_attach (GTK_TABLE(table), HScale, 1,2,rownum, rownum+1, GTK_FILL, GTK_FILL, 5, 5);
 //              gtk_table_attach (GTK_TABLE(table), tag, 1,2,rownum, rownum+1, GTK_FILL, GTK_FILL, 5, 5);

	}

	v4l2_list_add (ctrl.id, def,ctrl.default_value,GTK_WIDGET(HScale),V4L2_CTRL_TYPE_INTEGER);
}

void v4l2_add_button_control (struct v4l2_queryctrl ctrl,struct v4l2_control c,gboolean advanced)
{	
	GtkWidget *button,*label;
	curr_controls++;
	
	button = gtk_button_new_with_label("Reset");

	label = gtk_label_new ((const gchar *)ctrl.name);
	gtk_misc_set_alignment (GTK_MISC(label),0.0,0.5);	
	gtk_widget_set_size_request (label, 250, -1);

	g_signal_connect(G_OBJECT(button),"clicked",G_CALLBACK(button_control_changed_cb),GINT_TO_POINTER(ctrl.id));

	if ((advanced == TRUE) && curr_controls > MIN_CONTROLS_ON_MAIN_PAGE) {
		rownum_advanced++;
		gtk_table_resize (table2, rownum_advanced, 2);
		gtk_table_attach (GTK_TABLE(table2), label, 0,1,rownum_advanced, rownum_advanced+1, GTK_FILL, GTK_FILL, 5, 5);
		gtk_table_attach (GTK_TABLE(table2), button, 1,2,rownum_advanced, rownum_advanced+1, GTK_FILL, GTK_FILL, 5, 5);
	} else {
		rownum++;
		gtk_table_resize (table, rownum, 2);
		gtk_table_attach (GTK_TABLE(table), label, 0,1,rownum, rownum+1, GTK_FILL, GTK_FILL, 5, 5);
		gtk_table_attach (GTK_TABLE(table), button, 1,2,rownum, rownum+1, GTK_FILL, GTK_FILL, 5, 5);
	}
}

void v4l2_add_bool_control (struct v4l2_queryctrl ctrl,struct v4l2_control c,gboolean advanced)
{
	GtkWidget *check,*label,*align;
	curr_controls++;
	gboolean state;

	state = (gboolean) c.value;
	check = gtk_check_button_new();
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(check), state);
	g_signal_connect ( G_OBJECT(check), "clicked", G_CALLBACK (bool_control_changed_cb), GINT_TO_POINTER(ctrl.id));	

	label = gtk_label_new ((const gchar *)ctrl.name);
	gtk_misc_set_alignment (GTK_MISC(label),0.0,0.5);

        align = gtk_alignment_new (0.0,0.5,0.0,0.0);
        gtk_container_add (GTK_CONTAINER(align), check);

       if ((advanced == TRUE) && curr_controls > MIN_CONTROLS_ON_MAIN_PAGE) {
                rownum_advanced++;
		gtk_table_resize (table2, rownum_advanced, 2);
		gtk_table_attach (GTK_TABLE(table2), label,0,1,rownum_advanced, rownum_advanced+1, GTK_FILL, GTK_FILL, 5,5);
		gtk_table_attach (GTK_TABLE(table2), align,1,2, rownum_advanced, rownum_advanced+1, GTK_FILL, GTK_FILL, 5,5);
		} else {
		rownum++;
	        gtk_table_resize (table, rownum, 2);
	        gtk_table_attach (GTK_TABLE(table), label, 0,1,rownum, rownum+1, GTK_FILL, GTK_FILL, 5, 5);
	        gtk_table_attach (GTK_TABLE(table), align, 1,2,rownum, rownum+1, GTK_FILL, GTK_FILL, 5, 5);
	}
	v4l2_list_add (ctrl.id, state,ctrl.default_value,GTK_WIDGET(check),V4L2_CTRL_TYPE_BOOLEAN);
}

void v4l2_add_control (struct v4l2_queryctrl ctrl, struct v4l2_control c,gboolean advanced)
{
	switch(ctrl.type) {
		case V4L2_CTRL_TYPE_INTEGER:
			v4l2_add_int_control (ctrl,c,advanced);
			break;
		case V4L2_CTRL_TYPE_BOOLEAN:
			v4l2_add_bool_control (ctrl,c,advanced);
			break;
		case V4L2_CTRL_TYPE_MENU:
			v4l2_add_menu_control (ctrl,c,advanced);
			break;
		case V4L2_CTRL_TYPE_BUTTON:
			v4l2_add_button_control (ctrl,c,advanced);
			break;
		default:
			g_message("something else");
			break;
	}
}


void v4l2_add_header_init ()
{
//	const char *driver, *card, *bus_info;
	GtkWidget *sep,*label,*align;

//	driver = (const char *)cap.driver;
//	card = (const char *)cap.card;
//	bus_info = (const char *)cap.bus_info;

	label = gtk_label_new ("Device");
        align = gtk_alignment_new (0.0,0.5,0.0,0.0);
        gtk_container_add (GTK_CONTAINER(align), label);
        gtk_table_attach (GTK_TABLE(main_table), align, 0,1,0,1,GTK_FILL,GTK_FILL,5,5);//, GTK_EXPAND, GTK_SHRINK, 5, 5);    


        align = gtk_alignment_new (0.0,0.5,0.0,0.0);
        gtk_container_add (GTK_CONTAINER(align), dev_combo);
        gtk_table_attach (GTK_TABLE(main_table), align, 1,2,0,1,GTK_FILL,GTK_FILL,5,5);//, GTK_EXPAND, GTK_SHRINK, 5, 5);    

	label = gtk_label_new ("Driver");
	align = gtk_alignment_new (0.0,0.5,0.0,0.0);
	gtk_container_add (GTK_CONTAINER(align), label);
	gtk_table_attach (GTK_TABLE(main_table), align, 0,1,1,2,GTK_FILL,GTK_FILL,5,5);//, GTK_EXPAND, GTK_SHRINK, 5, 5);	
       
	 
	label_driver = gtk_label_new ("");
	align = gtk_alignment_new (0.0,0.5,0.0,0.0);
        gtk_container_add (GTK_CONTAINER(align), label_driver);
        gtk_table_attach (GTK_TABLE(main_table), align, 1,2,1,2, GTK_FILL, GTK_FILL, 5, 5);

        label = gtk_label_new ("Card");
        align = gtk_alignment_new (0.0,0.5,0.0,0.0);
        gtk_container_add (GTK_CONTAINER(align), label);
        gtk_table_attach (GTK_TABLE(main_table), align, 0,1,2,3, GTK_FILL, GTK_FILL, 5, 5);

        label_card = gtk_label_new ("");
        align = gtk_alignment_new (0.0,0.5,0.0,0.0);
        gtk_container_add (GTK_CONTAINER(align), label_card);
        gtk_table_attach (GTK_TABLE(main_table), align, 1,2,2,3, GTK_FILL, GTK_FILL, 5, 5);

        label = gtk_label_new ("Bus Information");
        align = gtk_alignment_new (0.0,0.5,0.0,0.0);
        gtk_container_add (GTK_CONTAINER(align), label);
        gtk_table_attach (GTK_TABLE(main_table), align, 0,1,3,4, GTK_FILL, GTK_FILL, 5, 5);

        label_bus = gtk_label_new ("");
        align = gtk_alignment_new (0.0,0.5,0.0,0.0);
        gtk_container_add (GTK_CONTAINER(align), label_bus);
        gtk_table_attach (GTK_TABLE(main_table), align, 1,2,3,4, GTK_FILL, GTK_FILL, 5, 5);
	
	sep = gtk_hseparator_new();
	gtk_table_attach (GTK_TABLE(main_table), sep, 0,4,4,5, GTK_FILL, GTK_FILL, 0, 0);
	
}

void
v4l2_add_header_populate(Gtkv4lDevice *device)
{
	gtk_label_set_text(GTK_LABEL(label_driver), device->driver);
	gtk_label_set_text(GTK_LABEL(label_card), device->card);
	gtk_label_set_text(GTK_LABEL(label_bus), device->bus_info);
}

int v4l2_count_controls(void)
{
	int j=0,k=0,m=0;
	struct v4l2_queryctrl ctrl;
	struct v4l2_control c;

	ctrl.id = V4L2_CTRL_FLAG_NEXT_CTRL;
	if(0 == ioctl (curr_dev->fd, VIDIOC_QUERYCTRL, &ctrl)) {
		do {
			c.id = ctrl.id;
			ctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
			if(ctrl.flags & V4L2_CTRL_FLAG_DISABLED) 
		                continue;

			if(v4l2_ioctl(curr_dev->fd, VIDIOC_G_CTRL, &c)==0) {
				if ((ctrl.type == V4L2_CTRL_TYPE_INTEGER) || 
				     (ctrl.type == V4L2_CTRL_TYPE_BOOLEAN) ||
				     (ctrl.type == V4L2_CTRL_TYPE_MENU))
					j++;
				}
     		
		} while(0 == v4l2_ioctl (curr_dev->fd, VIDIOC_QUERYCTRL, &ctrl));
	} else {
		for(k=V4L2_CID_BASE; k<V4L2_CID_LASTP1; k++) {
			ctrl.id = k;
			if(v4l2_ioctl(curr_dev->fd, VIDIOC_QUERYCTRL, &ctrl) == 0) 
				if(ctrl.flags & V4L2_CTRL_FLAG_DISABLED)
					continue;

			c.id = k;
			if(v4l2_ioctl(curr_dev->fd, VIDIOC_G_CTRL, &c)==0) {
				if ((ctrl.type == V4L2_CTRL_TYPE_INTEGER) || 
				     (ctrl.type == V4L2_CTRL_TYPE_BOOLEAN))
					j++;
			} 
	
		}
	}
        /* Check any custom controls */
	for(m=V4L2_CID_PRIVATE_BASE; ; m++) {
		ctrl.id = m;
		if(v4l2_ioctl(curr_dev->fd, VIDIOC_QUERYCTRL, &ctrl) == 0) {
			if(ctrl.flags & V4L2_CTRL_FLAG_DISABLED) 
				continue;

		c.id = m;
		if(v4l2_ioctl(curr_dev->fd, VIDIOC_G_CTRL, &c)==0) {
			if ((ctrl.type == V4L2_CTRL_TYPE_INTEGER) || 
			     (ctrl.type == V4L2_CTRL_TYPE_BOOLEAN))
				j++;
			}

		} else {
			break;
		}
	}
	return j;
}

void v4l2_reset_defaults(void)
{

	v4l2_reset_list_to_default();
	v4l2_list_print();	
	g_warning("defaults");

//	expanded = gtk_expander_get_expanded(GTK_EXPANDER(advanced_window));

//	gtk_widget_destroy(GTK_WIDGET(table));
//	curr_controls=0;
//	v4l2_control_panel_create();
//	gtk_widget_show_all(GTK_WIDGET(table));

}


int v4l2_write_to_driver_one(__u32 id,__u32 value, gboolean toggle)
{
	struct v4l2_queryctrl ctrl;
	struct v4l2_control c;

//	v4l2_list_print();
	ctrl.id=id;
	if(v4l2_ioctl(curr_dev->fd, VIDIOC_QUERYCTRL, &ctrl) == 0) {
		if(ctrl.flags & (V4L2_CTRL_FLAG_READ_ONLY |
			         V4L2_CTRL_FLAG_DISABLED |
	                         V4L2_CTRL_FLAG_GRABBED)) {
					g_warning("Unable to reset property id: %d",(int)id);
					return EXIT_FAILURE;
				}
		c.id = id;
		if (toggle)
			c.value = 0xffffffff;
		else
			c.value = value;

		if(v4l2_ioctl(curr_dev->fd, VIDIOC_S_CTRL, &c) != 0) {
			fprintf(stderr, "\nFailed to set control \"%s\": %s\n Setting id=%d and value=%d",
                        ctrl.name, strerror(errno),c.id, c.value);
		return EXIT_FAILURE;
		}
	} else {
		g_message( "Error querying control %s\n", strerror(errno));
		return EXIT_FAILURE;
		
		}
	
	return EXIT_SUCCESS;
}

int v4l2_write_to_driver(void)
{
	GList *l;
	struct v4l2_queryctrl ctrl;
	struct v4l2_control c;
	struct v4l2_property *s;	

//	v4l2_list_print();

	for (l=list; l; l=l->next) {
		s=l->data;
		ctrl.id=s->id;
		if(v4l2_ioctl(curr_dev->fd, VIDIOC_QUERYCTRL, &ctrl) == 0) {
		        if(ctrl.flags & (V4L2_CTRL_FLAG_READ_ONLY |
                        V4L2_CTRL_FLAG_DISABLED |
                        V4L2_CTRL_FLAG_GRABBED)) {
		                continue;
			}
			if(ctrl.type != V4L2_CTRL_TYPE_INTEGER &&
			ctrl.type != V4L2_CTRL_TYPE_BOOLEAN &&
			ctrl.type != V4L2_CTRL_TYPE_MENU) {
		                continue;
			}
			c.id = s->id;
			c.value = s->value;
			if(v4l2_ioctl(curr_dev->fd, VIDIOC_S_CTRL, &c) != 0) {
				fprintf(stderr, "\nFailed to set control \"%s\": %s\n Setting id=%d and value=%d",
				ctrl.name, strerror(errno),c.id, c.value);
				continue;
				}
			}		
		else {
			g_message( "Error querying control %s\n", strerror(errno));
			return EXIT_FAILURE;
			}
		}				
	return EXIT_SUCCESS;
}

void v4l2_load_from_driver (int fd, gboolean advanced)
{
	struct v4l2_queryctrl ctrl;
	struct v4l2_control c;
	int j,m;

	ctrl.id = V4L2_CTRL_FLAG_NEXT_CTRL;
	if(0 == ioctl (fd, VIDIOC_QUERYCTRL, &ctrl)) {
		do {
			c.id = ctrl.id;
			if(ctrl.flags & V4L2_CTRL_FLAG_DISABLED) 
		                continue;
			
			if (ctrl.type == V4L2_CTRL_TYPE_BUTTON)
				v4l2_add_control(ctrl,c,advanced);
			else {
				if(v4l2_ioctl(fd, VIDIOC_G_CTRL, &c)==0)	
					v4l2_add_control(ctrl,c,advanced);
				}
				
			ctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;

		} while(0 == v4l2_ioctl (fd, VIDIOC_QUERYCTRL, &ctrl));
	} else {
		for(j=V4L2_CID_BASE; j<V4L2_CID_LASTP1; j++) {
			ctrl.id = j;
			if(v4l2_ioctl(fd, VIDIOC_QUERYCTRL, &ctrl) == 0) 
				if(ctrl.flags & V4L2_CTRL_FLAG_DISABLED)
					continue;

			c.id = j;
                        if (ctrl.type == V4L2_CTRL_TYPE_BUTTON)
                                v4l2_add_control(ctrl,c,advanced);
                        else {
                                if(v4l2_ioctl(fd, VIDIOC_G_CTRL, &c)==0)
                                        v4l2_add_control(ctrl,c,advanced);
                                }
	
		}
	}
        /* Check any custom controls */
	for(m=V4L2_CID_PRIVATE_BASE; ; m++) {
		ctrl.id = m;
		if(v4l2_ioctl(fd, VIDIOC_QUERYCTRL, &ctrl) == 0) {
			if(ctrl.flags & V4L2_CTRL_FLAG_DISABLED) 
				continue;

		c.id = m;
                       if (ctrl.type == V4L2_CTRL_TYPE_BUTTON)
                                v4l2_add_control(ctrl,c,advanced);
                        else {
                                if(v4l2_ioctl(fd, VIDIOC_G_CTRL, &c)==0)
                                        v4l2_add_control(ctrl,c,advanced);
                                }

		} else {
			break;
		}
	}
}

void v4l2_control_panel_create_properties(void)
{
	controls = v4l2_count_controls();
	advanced_window = gtk_expander_new("More properties");

	gtk_expander_set_expanded(GTK_EXPANDER(advanced_window),expanded);

	table = GTK_TABLE(gtk_table_new(1,3,FALSE));

	fprintf(stderr, "controls = %d", controls);
	if (controls > MAX_CONTROLS_ON_MAIN_PAGE) {
		table2 = GTK_TABLE(gtk_table_new(rownum_advanced, 3, FALSE));
		gtk_container_add (GTK_CONTAINER(advanced_window), GTK_WIDGET(table2));

		v4l2_load_from_driver (curr_dev->fd,TRUE);
		v4l2_add_footer (TRUE);

	} else {
		v4l2_load_from_driver (curr_dev->fd, FALSE);
		v4l2_add_footer (FALSE);			
		}

	

        gtk_table_attach (GTK_TABLE(main_table),GTK_WIDGET(table), 0,2,rownum, rownum+1,GTK_FILL, GTK_FILL, 0, 5);
	
}

void v4l2_control_panel_create (Gtkv4lDevice *device)
{
	rownum=5;
	main_table = GTK_TABLE(gtk_table_new (rownum,3,FALSE));
	v4l2_add_header_init();
	v4l2_add_header_populate(device);

	v4l2_control_panel_create_properties();
	
	gtk_container_add (GTK_CONTAINER(content_area),GTK_WIDGET(main_table));

}

void v4l2_add_footer (gboolean advanced)
{
        rownum++;
	gtk_table_resize (table, rownum, 2);

        gtk_table_attach (GTK_TABLE(table),advanced_window, 0,2,rownum, rownum+1,GTK_FILL, GTK_FILL, 0, 5);
	gtk_widget_set_sensitive(GTK_WIDGET(advanced_window),advanced);


	rownum++;
        gtk_table_resize (table, rownum, 2);

}

void v4l2_add_dialog_buttons(void)
{
	GtkWidget *button;

        button = gtk_dialog_add_button ( GTK_DIALOG (window), "_Defaults", GTK_RESPONSE_APPLY);
        g_signal_connect(G_OBJECT(button),"clicked", G_CALLBACK(v4l2_reset_defaults),NULL);

        button = gtk_dialog_add_button ( GTK_DIALOG (window), GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE);
        g_signal_connect(G_OBJECT(button),"clicked", G_CALLBACK(destroy), NULL);

}

void
v4l2_combo_add_device(Gtkv4lDevice *device, int idx)
{
  gtk_combo_box_insert_text (GTK_COMBO_BOX(dev_combo), idx, device->card);
}

void
v4l2_combo_remove_device(Gtkv4lDevice *device, int idx)
{
  /* If this removes the current device
     v4l2_combo_change_device_cb() will get called and that will
     handle selecting a new device. */
  gtk_combo_box_remove_text (GTK_COMBO_BOX(dev_combo), idx);
}

void v4l2_switch_to_new_device(Gtkv4lDevice *device)
{
	v4l2_list_reset();
	started_cb = FALSE;

	curr_dev = device;
	v4l2_add_header_populate(device);
	gtk_widget_destroy(GTK_WIDGET(table));
	curr_controls=0;
	v4l2_control_panel_create_properties();
	started_cb = TRUE;
	gtk_widget_show_all(GTK_WIDGET(table));

}

void v4l2_combo_change_device_cb (GtkWidget *wid, gpointer user_data)
{
  gint active;

  active = gtk_combo_box_get_active (GTK_COMBO_BOX(dev_combo));
  /* This happens when our current device gets unplugged, just select the
     first one in the list instead. */
  if (active == -1) {
    gtk_combo_box_set_active (GTK_COMBO_BOX(dev_combo), 0);
    return;
  }

  curr_dev = g_list_nth_data (devlist->list, active);
  v4l2_switch_to_new_device (g_list_nth_data (devlist->list, active));
}


int main(int argc, char *argv[])
{
  GError *error = NULL;
  GdkPixbuf *icon_pixbuf = NULL;
  gchar *error_string;

  GOptionContext* context = g_option_context_new("- Gtk V4l app");
  g_option_context_add_main_entries (context,entries, NULL);
  g_option_context_add_group (context, gtk_get_option_group (TRUE));
  g_option_context_parse (context, &argc, &argv, &error);

  devlist = g_object_new (GTK_V4L_TYPE_DEVICE_LIST, NULL);
  devlist->device_added = v4l2_combo_add_device;
  devlist->device_removed = v4l2_combo_remove_device;

  dev_combo = gtk_combo_box_new_text();
  g_signal_connect (G_OBJECT(dev_combo), "changed", G_CALLBACK(v4l2_combo_change_device_cb),NULL);

  gtk_v4l_device_list_coldplug (devlist);


  gtk_init (&argc, &argv);


  if (device == NULL) // Assume default device
  {
	curr_dev = g_list_nth_data(devlist->list, 0);
  }
  else
        curr_dev = gtk_v4l_device_list_get_dev_by_device_file(devlist, device);

  if (!curr_dev) {
          show_error_dialog ("No V4L2 devices found");
          return EXIT_FAILURE;
  }

  printf("curr_dev: %s\n", curr_dev->card);

  window = gtk_dialog_new();
  gtk_window_set_title(GTK_WINDOW(window), "GTK V4L Device properties");
  gtk_window_set_resizable (GTK_WINDOW(window), FALSE);

  if(g_file_test(ICON_LOC,G_FILE_TEST_EXISTS)) {
    icon_pixbuf = gdk_pixbuf_new_from_file(ICON_LOC, NULL);
    if(icon_pixbuf)
      gtk_window_set_icon(GTK_WINDOW(window),icon_pixbuf);
  }  else {
    g_warning("Window icon not found");
  }
  content_area = gtk_dialog_get_content_area(GTK_DIALOG(window));
  
  gtk_container_set_border_width (GTK_CONTAINER(window),7);


  v4l2_control_panel_create(curr_dev);

  v4l2_add_dialog_buttons();
  g_signal_connect(G_OBJECT(window),"destroy", G_CALLBACK(destroy), NULL);

  gtk_widget_show_all (window);

 v4l2_list_print();
  started_cb = TRUE;
//  dialog_result = gtk_dialog_run(GTK_DIALOG(window));
/*  
  switch(dialog_result)	{
	case GTK_RESPONSE_APPLY:
		v4l2_driver_load_defaults();
		break;

	case GTK_RESPONSE_CLOSE:
		v4l2_close(fd);
		gtk_widget_destroy(window);
		break;		
	}
*/

	gtk_main();
  return 0;
	

}
