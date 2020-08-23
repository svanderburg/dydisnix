bin_PROGRAMS = dydisnix-multiwaycut
noinst_HEADERS = multiwaycut.h node.h applicationhostgraph.h

dydisnix_multiwaycut_SOURCES = multiwaycut.c node.c main.c applicationhostgraph.c
dydisnix_multiwaycut_LDADD = ../libservices/libservices.la ../libmapping/libmapping.la ../libinvmapping/libinvmapping.la ../libdymain/libdymain.la
dydisnix_multiwaycut_CFLAGS = -I../libservices -I../libmapping -I../libinvmapping -I../libdymain $(GLIB_CFLAGS) $(LIBXML2_CFLAGS) $(DISNIX_CFLAGS)
