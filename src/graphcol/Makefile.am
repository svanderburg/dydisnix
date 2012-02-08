AM_CPPFLAGS = -ggdb

bin_PROGRAMS = dydisnix-graphcol
noinst_HEADERS = graphcol.h

dydisnix_graphcol_SOURCES = graphcol.c main.c
dydisnix_graphcol_LDADD = ../libservices/libservices.la ../libinfproperties/libinfproperties.la
dydisnix_graphcol_CFLAGS = -I../libservices -I../libinfproperties $(GLIB_CFLAGS)
