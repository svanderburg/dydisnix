#ifndef __DYDISNIX_IDSCONFIG_H
#define __DYDISNIX_IDSCONFIG_H
#include <stdio.h>
#include <glib.h>
#include <nixxml-types.h>
#include "idresourcetype.h"

typedef struct
{
    GHashTable *id_assignments_per_resource_table;

    GHashTable *last_assignments_table;

    GHashTable *last_assignments_per_target_table;
}
IdsConfig;

IdsConfig *create_empty_ids_config(void);

char *generate_ids_xml_from_expr(char *ids_expr);

IdsConfig *create_ids_config_from_xml(const gchar *ids_xml_file);

IdsConfig *create_ids_config_from_nix(gchar *ids_nix);

IdsConfig *create_ids_config(gchar *ids, const NixXML_bool xml);

void delete_ids_config(IdsConfig *ids_config);

void print_ids_config_nix(FILE *file, const IdsConfig *ids_config, const int indent_level, void *userdata);

void print_ids_config_xml(FILE *file, const IdsConfig *ids_config, const int indent_level, const char *type_property_name, void *userdata);

void remove_invalid_service_ids(IdsConfig *ids_config, GHashTable *services_table, GHashTable *id_resources_table, gchar *service_property);

void remove_invalid_distribution_ids(IdsConfig *ids_config, GHashTable *services_table, GHashTable *distribution_table, GHashTable *target_to_service_table, GHashTable *id_resources_table, gchar *service_property);

NixXML_bool create_id_assignments_for_services(GHashTable *id_assignments_table, GHashTable *last_assignments_table, GPtrArray *service_names, gchar *resource_name, IdResourceType *type, GHashTable *services_table, gchar *service_property);

#endif
