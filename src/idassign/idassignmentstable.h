#ifndef __DYDISNIX_IDASSIGNMENTSTABLE_H
#define __DYDISNIX_IDASSIGNMENTSTABLE_H
#include <stdio.h>
#include <libxml/parser.h>
#include <glib.h>
#include <nixxml-types.h>
#include "idresourcetype.h"

GHashTable *create_empty_id_assignments_table(void);

void *parse_id_assignments_table(xmlNodePtr element, void *userdata);

void delete_id_assignments_table(GHashTable *id_assignments_table);

void print_id_assignments_table_nix(FILE *file, GHashTable *id_assignments_table, const int indent_level, void *userdata);

void print_id_assignments_table_xml(FILE *file, GHashTable *id_assignments_table, const int indent_level, const char *type_property_name, void *userdata);

void remove_invalid_service_id_assignments(GHashTable *id_assignments_table, gchar *resource_name, GHashTable *services_table, IdResourceType *type, gchar *service_property);

void remove_invalid_distribution_id_assignments(GHashTable *id_assignments_table, gchar *resource_name, GHashTable *services_table, GHashTable *distribution_table, IdResourceType *type, gchar *service_property);

NixXML_bool create_id_assignments_for_services(GHashTable *id_assignments_table, GPtrArray *service_names, gchar *resource_name, IdResourceType *type, GHashTable *services_table, gchar *service_property);

#endif
