bin_PROGRAMS = dydisnix-port-assign
noinst_HEADERS = portassign.h portconfiguration.h

dydisnix_port_assign_SOURCES = portassign.c portconfiguration.c main.c
dydisnix_port_assign_LDADD = ../libservices/libservices.la ../libinfproperties/libinfproperties.la ../libmapping/libmapping.la ../libinvmapping/libinvmapping.la $(DISNIX_LIBS) -lmodel -lprocreact
dydisnix_port_assign_CFLAGS = -I../libservices -I../libinfproperties -I../libmapping -I../libinvmapping $(GLIB_CFLAGS) $(DISNIX_CFLAGS) $(LIBXML2_CFLAGS)
