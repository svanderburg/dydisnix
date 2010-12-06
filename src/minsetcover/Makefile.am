AM_CPPFLAGS = -ggdb

bin_PROGRAMS = dydisnix-minsetcover

dydisnix_minsetcover_SOURCES = minsetcover.c main.c
dydisnix_minsetcover_LDADD = ../libservices/libservices.la ../libinfproperties/libinfproperties.la ../libmapping/libmapping.la ../libinvmapping/libinvmapping.la
dydisnix_minsetcover_CFLAGS = -I../libservices -I../libinfproperties -I../libmapping -I../libinvmapping $(GLIB_CFLAGS)
