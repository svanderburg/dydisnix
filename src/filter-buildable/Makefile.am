AM_CPPFLAGS = -DDISNIX_PREFIX=\"$(DISNIX_PREFIX)\" -DDATADIR=\"$(datadir)\"

bin_PROGRAMS = dydisnix-filter-buildable
noinst_HEADERS = filterbuildable.h

dydisnix_filter_buildable_SOURCES = filterbuildable.c main.c
dydisnix_filter_buildable_LDADD = ../libdistribution/libdistribution.la ../libdymain/libdymain.la $(LIBXML2_LIBS) $(DISNIX_LIBS) -lprocreact -lmain
dydisnix_filter_buildable_CFLAGS = -I../libdistribution -I../libdymain $(LIBXML2_CFLAGS) $(GLIB_CFLAGS) $(DISNIX_CFLAGS)
