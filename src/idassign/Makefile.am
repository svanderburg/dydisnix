bin_PROGRAMS = dydisnix-id-assign
noinst_HEADERS = idassignmentsperresourcetable.h idassignmentstable.h idresourcestable.h idresourcetype.h idtoservicetable.h lastassignmentstable.h lastassignmentspertargettable.h idsconfig.h assignservices.h assigndistribution.h idassign.h

dydisnix_id_assign_SOURCES = idassignmentsperresourcetable.c idassignmentstable.c idresourcestable.c idresourcetype.c idtoservicetable.c lastassignmentstable.c lastassignmentspertargettable.c idsconfig.c assignservices.c assigndistribution.c idassign.c main.c
dydisnix_id_assign_LDADD = ../libservices/libservices.la ../libinfrastructure2/libinfrastructure2.la ../libdistribution/libdistribution.la ../libdymain/libdymain.la $(DISNIX_LIBS) $(LIBXML2_LIBS) -lmodel -lprocreact
dydisnix_id_assign_CFLAGS = -I../libservices -I../libinfrastructure2 -I../libdistribution -I../libdymain $(GLIB_CFLAGS) $(DISNIX_CFLAGS) $(LIBXML2_CFLAGS)
