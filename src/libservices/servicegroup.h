#ifndef __DYDISNIX_SERVICEGROUP_H
#define __DYDISNIX_SERVICEGROUP_H
#include <glib.h>
#include <stdio.h>
#include <libxml/parser.h>

int is_subgroup_of(xmlChar *current_group, gchar *group);

GHashTable *query_services_in_group(GHashTable *service_table, gchar *group);

GHashTable *group_services(GHashTable *queried_services_table, gchar *group);

GPtrArray *query_unique_groups(GHashTable *service_table);

void mkdirp(const char *dir);

int generate_group_artifacts(GHashTable *table, gchar *group, gchar *output_dir, gchar *filename, gchar *image_format, void *data, int (*generate_artifact) (gchar *filepath, gchar *image_format, gchar *group, void *data, GHashTable *service_table) );

#endif
