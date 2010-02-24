#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT(gtk-v4l, 0.1, sidhpurwala.huzaifa@gmail.com)
AM_INIT_AUTOMAKE([-Wall -Werror])
AC_CONFIG_SRCDIR([src/gtk-v4l.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
#AC_PROG_CC
AM_PROG_CC_C_O
# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h sys/ioctl.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
GTK_REQUIRED=2.14
PKG_CHECK_MODULES([GTK],[gtk+-2.0 >= $GTK_REQUIRED glib-2.0])
PKG_CHECK_MODULES([V4L], [libv4l2 >= 0.6])

AC_SUBST([GTK_CFLAGS])
AC_SUBST([GTK_LIBS])

AC_SUBST([V4L_CFLAGS])
AC_SUBST([V4L_LIBS])

AC_SUBST([AM_CPPFLAGS])
AC_SUBST([AM_CFLAGS])
AC_SUBST([AM_LDFLAGS])



# Checks for library functions.
AC_CHECK_FUNCS([strerror])

AC_CONFIG_FILES([Makefile
		src/Makefile
		gtk-v4l.desktop])
AC_OUTPUT
