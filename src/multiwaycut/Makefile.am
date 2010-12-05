AM_CPPFLAGS = -ggdb

bin_PROGRAMS = dydisnix-multiwaycut

dydisnix_multiwaycut_SOURCES = targetmapping.c multiwaycut.c main.c
dydisnix_multiwaycut_LDADD = ../libmapping/libmapping.la
dydisnix_multiwaycut_CFLAGS = -I../libmapping $(GLIB_CFLAGS)
