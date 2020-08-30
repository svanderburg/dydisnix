#ifndef __DYDISNIX_TARGETSTOSERVICESTABLE_H
#define __DYDISNIX_TARGETSTOSERVICESTABLE_H
#include <glib.h>

GHashTable *create_target_to_services_table(GHashTable *distribution_table);

void delete_target_to_services_table(GHashTable *target_to_services_table);

#endif
