#ifndef __DYDISNIX_SERVICE_MAPPING_H
#define __DYDISNIX_SERVICE_MAPPING_H
#include <glib.h>

typedef struct
{
    gchar *target;
    
    GArray *services;
}
TargetMappingItem;

GArray *create_target_mapping_array(GArray *candidate_target_array);

void delete_target_mapping_array(GArray *target_mapping_array);

void print_target_mapping_array(GArray *target_mapping_array);

int service_name_index(TargetMappingItem *item, gchar *service);

gint compare_target_mapping_item(const TargetMappingItem **l, const TargetMappingItem **r);

#endif
