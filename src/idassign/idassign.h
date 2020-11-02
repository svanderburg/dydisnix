#ifndef __DYDISNIX_IDASSIGN_H
#define __DYDISNIX_IDASSIGN_H
#include <glib.h>
#include <checkoptions.h>

int idassign(gchar *services, gchar *infrastructure, gchar *distribution, gchar *resources, gchar *ids, gchar *service_property, const char *output_file, gchar *extra_params, const unsigned int flags);

#endif
