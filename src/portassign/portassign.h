#ifndef __DYDISNIX_PORTASSIGN_H
#define __DYDISNIX_PORTASSIGN_H
#include <glib.h>

int portassign(gchar *service_xml, gchar *infrastructure_xml, gchar *distribution_xml, gchar *ports_xml, gchar *service_property);

#endif
