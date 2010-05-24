bin_PROGRAMS = gtk-v4l
gtk_v4l_SOURCES = gtk-v4l.c \
		  gtk-v4l.h \
		  error_dialog.c \
		  gtk-v4l-control.c \
		  gtk-v4l-control.h \
		  gtk-v4l-device.c \
		  gtk-v4l-device.h \
		  gtk-v4l-device-list.c \
		  gtk-v4l-device-list.h
gtk_v4l_CFLAGS = \
        $(GTK_CFLAGS)        \
        $(V4L_CFLAGS)     \
	$(GUDEV_CFLAGS)	  \
        $(WARN_CFLAGS)          \
        $(AM_CFLAGS)

gtk_v4l_LDFLAGS = \
        $(AM_LDFLAGS)

gtk_v4l_LDADD = \
        $(GTK_LIBS)          \
        $(V4L_LIBS)       \
	$(GUDEV_LIBS) 


