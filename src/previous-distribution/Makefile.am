bin_PROGRAMS = dydisnix-previous-distribution
noinst_HEADERS = generate-previous-distribution.h

dydisnix_previous_distribution_SOURCES = generate-previous-distribution.c main.c
dydisnix_previous_distribution_LDADD = ../libmapping/libmapping.la ../libdymain/libdymain.la -lmanifest $(LIBXML2_LIBS)
dydisnix_previous_distribution_CFLAGS = -I../libmapping -I../libdymain $(GLIB_CFLAGS) $(DISNIX_CFLAGS) $(LIBXML2_CFLAGS)
