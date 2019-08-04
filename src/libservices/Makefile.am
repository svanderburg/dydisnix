AM_CPPFLAGS = -DDATADIR="\"$(prefix)/share/dydisnix\""

pkglib_LTLIBRARIES = libservices.la
pkginclude_HEADERS = service.h servicestable.h servicegroup.h

libservices_la_SOURCES = service.c servicestable.c servicegroup.c
libservices_la_CFLAGS = $(LIBXML2_CFLAGS) $(DISNIX_CFLAGS) $(GLIB_CFLAGS)
libservices_la_LIBADD = $(DISNIX_LIBS) $(LIBXSLT_LIBS) $(GLIB_LIBS) -lnixxml -lnixxml-glib -lprocreact
