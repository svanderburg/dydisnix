bin_PROGRAMS = dydisnix-multiwaycut
noinst_HEADERS = multiwaycut.h applicationhostgraph.h applicationhostgraph-transform.h multiwaycut-approximate.h

dydisnix_multiwaycut_SOURCES = multiwaycut.c main.c applicationhostgraph.c applicationhostgraph-transform.c multiwaycut-approximate.c
dydisnix_multiwaycut_LDADD = ../libservices/libservices.la ../libdistribution/libdistribution.la ../libinvmapping/libinvmapping.la ../libdymain/libdymain.la ../libgraph/libgraph.la
dydisnix_multiwaycut_CFLAGS = -I../libservices -I../libdistribution -I../libinvmapping -I../libdymain -I../libgraph $(GLIB_CFLAGS) $(LIBXML2_CFLAGS) $(DISNIX_CFLAGS)
