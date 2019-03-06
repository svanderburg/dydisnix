bin_PROGRAMS = dydisnix-multiwaycut
noinst_HEADERS = multiwaycut.h

dydisnix_multiwaycut_SOURCES = multiwaycut.c main.c
dydisnix_multiwaycut_LDADD = ../libmapping/libmapping.la ../libinvmapping/libinvmapping.la ../libdymain/libdymain.la
dydisnix_multiwaycut_CFLAGS = -I../libmapping -I../libinvmapping -I../libdymain $(GLIB_CFLAGS)
