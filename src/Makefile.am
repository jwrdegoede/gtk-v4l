bin_PROGRAMS = gtk-v4l

gtk_v4l_SOURCES = gtk-v4l.c

gtk_v4l_LDADD = $(top_builddir)/lib/libgtkv4l.la \
	        $(GTK_LIBS) \
	        $(GUDEV_LIBS)\
		$(V4L_LIBS)

gtk_v4l_CFLAGS = -I$(top_srcdir)/lib \
		  $(GTK_CFLAGS) \
		  $(GUDEV_CFLAGS) \
		  $(V4L_CFLAGS)
