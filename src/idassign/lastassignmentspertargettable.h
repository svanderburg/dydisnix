#ifndef __DYDISNIX_LASTASSIGNMENTSPERTARGETSTABLE_H
#define __DYDISNIX_LASTASSIGNMENTSPERTARGETSTABLE_H
#include <stdio.h>
#include <libxml/parser.h>
#include <glib.h>

GHashTable *create_empty_last_assignments_per_target_table(void);

GHashTable *retrieve_or_add_empty_last_assignments_table_for_target(GHashTable *last_assignments_per_target_table, const gchar *target_name);

void *parse_last_assignments_per_target_table(xmlNodePtr element, void *userdata);

void delete_last_assignments_per_target_table(GHashTable *last_assignments_per_target_table);

void print_last_assignments_per_target_table_nix(FILE *file, GHashTable *last_assignments_per_target_table, const int indent_level, void *userdata);

void print_last_assignments_per_target_table_xml(FILE *file, GHashTable *last_assignments_per_target_table, const int indent_level, const char *type_property_name, void *userdata);

void remove_invalid_last_assignments_per_target(GHashTable *last_assignments_per_target_table, GHashTable *id_resources_table, GHashTable *target_to_service_table);

#endif
