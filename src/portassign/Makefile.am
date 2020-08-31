bin_PROGRAMS = dydisnix-port-assign
noinst_HEADERS = portassign.h portconfiguration.h portdistribution.h targetconfig.h

dydisnix_port_assign_SOURCES = portassign.c portconfiguration.c portdistribution.c targetconfig.c main.c
dydisnix_port_assign_LDADD = ../libservices/libservices.la ../libinfrastructure2/libinfrastructure2.la ../libdistribution/libdistribution.la ../libdymain/libdymain.la $(DISNIX_LIBS) $(LIBXML2_LIBS) -lmodel -lprocreact
dydisnix_port_assign_CFLAGS = -I../libservices -I../libinfrastructure2 -I../libdistribution -I../libdymain $(GLIB_CFLAGS) $(DISNIX_CFLAGS) $(LIBXML2_CFLAGS)
