#ifndef __DYDISNIX_MINSETCOVER_H
#define __DYDISNIX_MINSETCOVER_H
#include <glib.h>
#include <checkoptions.h>

int minsetcover(gchar *services, gchar *infrastructure, gchar *distribution, gchar *target_property, gchar *extra_params, const unsigned int flags);

#endif
