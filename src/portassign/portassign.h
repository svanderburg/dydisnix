#ifndef __DYDISNIX_PORTASSIGN_H
#define __DYDISNIX_PORTASSIGN_H
#include <glib.h>
#include <checkoptions.h>

int portassign(gchar *service, gchar *infrastructure, gchar *distribution, gchar *ports, gchar *service_property, const unsigned int flags);

#endif
