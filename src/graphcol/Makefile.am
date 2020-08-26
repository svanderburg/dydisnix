bin_PROGRAMS = dydisnix-graphcol
noinst_HEADERS = graphcol.h

dydisnix_graphcol_SOURCES = graphcol.c main.c
dydisnix_graphcol_LDADD = ../libservices/libservices.la ../libinfproperties/libinfproperties.la ../libmapping/libmapping.la ../libdymain/libdymain.la ../libgraph/libgraph.la $(LIBXML2_LIBS)
dydisnix_graphcol_CFLAGS = -I../libservices -I../libinfproperties -I../libmapping -I../libdymain -I../libgraph $(GLIB_CFLAGS) $(DISNIX_CFLAGS) $(LIBXML2_CFLAGS)
