dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(paquery, 1.0)


AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE()

AC_PROG_CC
AM_PROG_LIBTOOL

PKG_CHECK_MODULES(GLIB, glib-2.0 >= 2.18 gio-2.0 >= 2.18)
PKG_CHECK_MODULES(PULSEAUDIO, libpulse >= 0.9.19 libpulse-mainloop-glib >= 0.9.19)

AC_CONFIG_FILES(Makefile src/Makefile)
AC_OUTPUT

