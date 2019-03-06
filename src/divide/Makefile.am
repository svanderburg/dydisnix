bin_PROGRAMS = dydisnix-divide
noinst_HEADERS = divide.h

dydisnix_divide_SOURCES = divide.c main.c
dydisnix_divide_LDADD = ../libservices/libservices.la ../libinfproperties/libinfproperties.la ../libmapping/libmapping.la ../libdymain/libdymain.la
dydisnix_divide_CFLAGS = -I../libservices -I../libinfproperties -I../libmapping -I../libdymain $(GLIB_CFLAGS) $(DISNIX_CFLAGS) $(LIBXML2_CFLAGS)
