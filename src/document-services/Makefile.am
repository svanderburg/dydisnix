AM_CPPFLAGS = -DDATADIR="\"$(prefix)/share/dydisnix\""

bin_PROGRAMS = dydisnix-document-services
noinst_HEADERS = document-services.h docs-config.h

dydisnix_document_services_SOURCES = document-services.c main.c docs-config.c
dydisnix_document_services_LDADD = ../libservices/libservices.la ../libdymain/libdymain.la -lnixxml -lnixxml-glib $(LIBXML2_LIBS)
dydisnix_document_services_CFLAGS = -I../libservices -I../libdymain $(GLIB_CFLAGS) $(DISNIX_CFLAGS) $(LIBXML2_CFLAGS)
