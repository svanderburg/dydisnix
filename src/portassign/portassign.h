#ifndef __DYDISNIX_PORTASSIGN_H
#define __DYDISNIX_PORTASSIGN_H
#include <glib.h>

int portassign(gchar *service, gchar *infrastructure, gchar *distribution, gchar *ports, gchar *service_property, int xml);

#endif
