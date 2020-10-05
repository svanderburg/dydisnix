bin_PROGRAMS = dydisnix-visualize-infra
noinst_HEADERS = visualize-infra.h

dydisnix_visualize_infra_SOURCES = visualize-infra.c main.c
dydisnix_visualize_infra_LDADD = ../libinfrastructure2/libinfrastructure2.la ../libdymain/libdymain.la -lnixxml-glib $(LIBXML2_LIBS)
dydisnix_visualize_infra_CFLAGS = -I../libinfrastructure2 -I../libdymain $(GLIB_CFLAGS) $(DISNIX_CFLAGS) $(LIBXML2_CFLAGS)
