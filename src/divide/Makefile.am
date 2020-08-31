bin_PROGRAMS = dydisnix-divide
noinst_HEADERS = divide.h

dydisnix_divide_SOURCES = divide.c main.c
dydisnix_divide_LDADD = ../libservices/libservices.la ../libinfrastructure2/libinfrastructure2.la ../libdistribution/libdistribution.la ../libdymain/libdymain.la
dydisnix_divide_CFLAGS = -I../libservices -I../libinfrastructure2 -I../libdistribution -I../libdymain $(GLIB_CFLAGS) $(DISNIX_CFLAGS) $(LIBXML2_CFLAGS)
