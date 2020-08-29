#ifndef __DYDISNIX_TARGET_MAPPING_TABLE_H
#define __DYDISNIX_TARGET_MAPPING_TABLE_H
#include <glib.h>

GHashTable *create_target_mapping_table(GHashTable *distribution_table);

void delete_target_mapping_table(GHashTable *target_mapping_table);

#endif
