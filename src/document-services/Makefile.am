bin_PROGRAMS = dydisnix-document-services
noinst_HEADERS = document-services.h

dydisnix_document_services_SOURCES = document-services.c main.c
dydisnix_document_services_LDADD = ../libservices/libservices.la
dydisnix_document_services_CFLAGS = -I../libservices $(GLIB_CFLAGS) $(DISNIX_CFLAGS) $(LIBXML2_CFLAGS)
