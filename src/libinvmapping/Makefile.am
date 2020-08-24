pkglib_LTLIBRARIES = libinvmapping.la
pkginclude_HEADERS = targetmappingtable.h

libinvmapping_la_SOURCES = targetmappingtable.c
libinvmapping_la_CFLAGS = $(LIBXML2_CFLAGS) $(GLIB_CFLAGS) -I../libmapping $(DISNIX_CFLAGS)
libinvmapping_la_LIBADD = $(DISNIX_LIBS) ../libmapping/libmapping.la -lmodel
