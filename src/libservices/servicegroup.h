#ifndef __DYDISNIX_SERVICEGROUP_H
#define __DYDISNIX_SERVICEGROUP_H
#include <glib.h>

int is_subgroup_of(gchar *current_group, gchar *group);

GHashTable *query_services_in_group(GPtrArray *service_property_array, gchar *group);

GPtrArray *create_service_property_array_from_table(GHashTable *table);

void delete_services_table(GHashTable *services_table);

GHashTable *group_services(GHashTable *queried_services_table, gchar *group);

#endif
