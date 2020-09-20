#ifndef __DYDISNIX_IDASSIGNMENTSPERRESOURCETABLE_H
#define __DYDISNIX_IDASSIGNMENTSPERRESOURCETABLE_H
#include <stdio.h>
#include <libxml/parser.h>
#include "idassignmentstable.h"

GHashTable *create_empty_id_assignments_per_resource_table(void);

GHashTable *retrieve_or_add_empty_id_assignments_table_for_resource(GHashTable *id_assignments_per_resource_table, const gchar *resource_name);

void *parse_id_assignments_per_resource_table(xmlNodePtr element, void *userdata);

void delete_id_assignments_per_resource_table(GHashTable *id_assignments_per_resource_table);

void print_id_assignments_per_resource_table_nix(FILE *file, GHashTable *id_assignments_per_resource_table, const int indent_level, void *userdata);

void print_id_assignments_per_resource_table_xml(FILE *file, GHashTable *id_assignments_per_resource_table, const int indent_level, const char *type_property_name, void *userdata);

void remove_invalid_service_id_assignments_per_resource(GHashTable *id_assignments_per_resource_table, GHashTable *services_table, GHashTable *id_resources_table, gchar *service_property);

void remove_invalid_distribution_id_assignments_per_resource(GHashTable *id_assignments_per_resource_table, GHashTable *services_table, GHashTable *distribution_table, GHashTable *id_resources_table, gchar *service_property);

#endif
