bin_PROGRAMS = dydisnix-minsetcover
noinst_HEADERS = minsetcover.h

dydisnix_minsetcover_SOURCES = minsetcover.c main.c
dydisnix_minsetcover_LDADD = ../libservices/libservices.la ../libinfrastructure2/libinfrastructure2.la ../libdistribution/libdistribution.la ../libdymain/libdymain.la
dydisnix_minsetcover_CFLAGS = -I../libservices -I../libinfrastructure2 -I../libdistribution -I../libdymain $(GLIB_CFLAGS) $(DISNIX_CFLAGS) $(LIBXML2_CFLAGS)
