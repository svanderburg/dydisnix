#ifndef __DYDISNIX_TARGET_MAPPING_TABLE_H
#define __DYDISNIX_TARGET_MAPPING_TABLE_H
#include <glib.h>

GHashTable *create_target_mapping_table(GHashTable *candidate_target_table);

void delete_target_mapping_table(GHashTable *candidate_target_table);

#endif
