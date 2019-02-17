#ifndef __DYDISNIX_SERVICEGROUP_H
#define __DYDISNIX_SERVICEGROUP_H
#include <glib.h>
#include <stdio.h>

int is_subgroup_of(gchar *current_group, gchar *group);

GHashTable *query_services_in_group(GPtrArray *service_property_array, gchar *group);

GPtrArray *create_service_property_array_from_table(GHashTable *table);

void delete_services_table(GHashTable *services_table);

GHashTable *group_services(GHashTable *queried_services_table, gchar *group);

GPtrArray *query_unique_groups(GPtrArray *service_property_array);

void mkdirp(const char *dir);

void generate_group_artifacts(GHashTable *table, gchar *group, gchar *output_dir, gchar *filename, void (*generate_artifact) (FILE *fd, const GPtrArray *service_property_array) );

#endif
