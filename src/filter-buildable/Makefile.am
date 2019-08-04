AM_CPPFLAGS = -DDATADIR=\"$(datadir)\"

bin_PROGRAMS = dydisnix-filter-buildable
noinst_HEADERS = filterbuildable.h

dydisnix_filter_buildable_SOURCES = filterbuildable.c main.c
dydisnix_filter_buildable_LDADD = ../libmapping/libmapping.la ../libdymain/libdymain.la $(LIBXML2_LIBS) $(DISNIX_LIBS) -lprocreact -lmain
dydisnix_filter_buildable_CFLAGS = -I../libmapping -I../libdymain $(LIBXML2_CFLAGS) $(GLIB_CFLAGS) $(DISNIX_CFLAGS)
