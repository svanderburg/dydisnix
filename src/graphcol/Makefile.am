bin_PROGRAMS = dydisnix-graphcol
noinst_HEADERS = graphcol.h partialcoloredgraph.h partialcoloredgraph-transform.h graphcol-approximate.h

dydisnix_graphcol_SOURCES = graphcol.c partialcoloredgraph.c partialcoloredgraph-transform.c graphcol-approximate.c main.c
dydisnix_graphcol_LDADD = ../libservices/libservices.la ../libinfproperties/libinfproperties.la ../libdistribution/libdistribution.la ../libdymain/libdymain.la ../libgraph/libgraph.la $(LIBXML2_LIBS)
dydisnix_graphcol_CFLAGS = -I../libservices -I../libinfproperties -I../libdistribution -I../libdymain -I../libgraph $(GLIB_CFLAGS) $(DISNIX_CFLAGS) $(LIBXML2_CFLAGS)
