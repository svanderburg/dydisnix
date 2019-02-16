AM_CPPFLAGS = -DDATADIR=\"$(datadir)\"

bin_PROGRAMS = dydisnix-filter-buildable
noinst_HEADERS = filterbuildable.h

dydisnix_filter_buildable_SOURCES = filterbuildable.c main.c
dydisnix_filter_buildable_LDADD = ../libmapping/libmapping.la $(DISNIX_LIBS) -lprocreact
dydisnix_filter_buildable_CFLAGS = -I../libmapping $(GLIB_CFLAGS) $(DISNIX_CFLAGS)
