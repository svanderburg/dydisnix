#ifndef __DYDISNIX_SERVICE_MAPPING_H
#define __DYDISNIX_SERVICE_MAPPING_H
#include <glib.h>

typedef struct
{
    gchar *target;
    
    GPtrArray *services;
}
TargetMappingItem;

GPtrArray *create_target_mapping_array(GHashTable *candidate_target_table);

void delete_target_mapping_array(GPtrArray *target_mapping_array);

void print_target_mapping_array(GPtrArray *target_mapping_array);

gchar *find_service_name(TargetMappingItem *item, gchar *service);

gint compare_target_mapping_item(const TargetMappingItem **l, const TargetMappingItem **r);

TargetMappingItem *find_target_mapping_item(GPtrArray *target_mapping_array, gchar *target);

#endif
