AM_CPPFLAGS = -DDATADIR="\"$(prefix)/share/dydisnix\""

pkglib_LTLIBRARIES = libinfrastructure2.la
pkginclude_HEADERS = targetstable2.h

libinfrastructure2_la_SOURCES = targetstable2.c
libinfrastructure2_la_CFLAGS = $(LIBXML2_CFLAGS) $(DISNIX_CFLAGS) $(GLIB_CFLAGS)
libinfrastructure2_la_LIBADD = $(DISNIX_LIBS) $(LIBXSLT_LIBS) $(GLIB_LIBS) -linfrastructure -lprocreact
