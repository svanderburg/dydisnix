bin_PROGRAMS = dydisnix-multiwaycut
noinst_HEADERS = multiwaycut.h applicationhostgraph.h applicationhostgraph-transform.h

dydisnix_multiwaycut_SOURCES = multiwaycut.c main.c applicationhostgraph.c applicationhostgraph-transform.c
dydisnix_multiwaycut_LDADD = ../libservices/libservices.la ../libmapping/libmapping.la ../libinvmapping/libinvmapping.la ../libdymain/libdymain.la ../libgraph/libgraph.la
dydisnix_multiwaycut_CFLAGS = -I../libservices -I../libmapping -I../libinvmapping -I../libdymain -I../libgraph $(GLIB_CFLAGS) $(LIBXML2_CFLAGS) $(DISNIX_CFLAGS)
