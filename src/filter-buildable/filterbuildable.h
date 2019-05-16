#ifndef __DYDISNIX_FILTERBUILDABLE_H
#define __DYDISNIX_FILTERBUILDABLE_H
#include <checkoptions.h>

int filter_buildable(char *services_expr, char *infrastructure_expr, char *distribution_expr, const unsigned int flags, char *interface, char *target_property);

#endif
