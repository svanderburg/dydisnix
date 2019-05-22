bin_PROGRAMS = dydisnix-visualize-services
noinst_HEADERS = visualize-services.h

dydisnix_visualize_services_SOURCES = visualize-services.c main.c
dydisnix_visualize_services_LDADD = ../libservices/libservices.la ../libdymain/libdymain.la $(LIBXML2_LIBS)
dydisnix_visualize_services_CFLAGS = -I../libservices -I../libdymain $(GLIB_CFLAGS) $(DISNIX_CFLAGS) $(LIBXML2_CFLAGS)
