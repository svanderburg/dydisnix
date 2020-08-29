pkglib_LTLIBRARIES = libdistribution.la
pkginclude_HEADERS = distributionmapping.h distributionmappingarray.h distributiontable.h

libdistribution_la_SOURCES = distributionmapping.c distributionmappingarray.c distributiontable.c
libdistribution_la_CFLAGS = $(LIBXML2_CFLAGS) $(GLIB_CFLAGS) $(DISNIX_CFLAGS)
libdistribution_la_LIBADD = $(GLIB_LIBS) $(DISNIX_LIBS) -lnixxml -lnixxml-glib
