lib_LTLIBRARIES = libgtkv4l.la

include_HEADERS = gtk-v4l-widget.h \
		  gtk-v4l-device-list.h

libgtkv4l_la_CFLAGS = \
        $(GTK_CFLAGS)        \
        $(V4L_CFLAGS)     \
	$(GUDEV_CFLAGS)	  \
        $(WARN_CFLAGS)          \
        $(AM_CFLAGS)

libgtkv4l_la_LDFLAGS = \
        $(AM_LDFLAGS)\
	$(GTK_LIBS) \
	$(V4L_LIBS) \
	$(GUDEV_LIBS)

libgtkv4l_la_LIBDADD = \
        $(GTK_LIBS)          \
        $(V4L_LIBS)       \
	$(GUDEV_LIBS) 

libgtkv4l_la_SOURCES = gtk-v4l-control.c \
                  gtk-v4l-control.h \
                  gtk-v4l-device.c \
                  gtk-v4l-device.h \
                  gtk-v4l-device-list.c \
                  gtk-v4l-device-list.h \
                  gtk-v4l-widget.c \
                  gtk-v4l-widget.h

