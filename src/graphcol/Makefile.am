bin_PROGRAMS = dydisnix-graphcol
noinst_HEADERS = graphcol.h

dydisnix_graphcol_SOURCES = graphcol.c main.c
dydisnix_graphcol_LDADD = ../libservices/libservices.la ../libinfproperties/libinfproperties.la ../libdymain/libdymain.la
dydisnix_graphcol_CFLAGS = -I../libservices -I../libinfproperties -I../libdymain $(GLIB_CFLAGS) $(DISNIX_CFLAGS) $(LIBXML2_CFLAGS)
