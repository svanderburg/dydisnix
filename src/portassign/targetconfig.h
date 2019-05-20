#ifndef __DYDISNIX_TARGETCONFIG_H
#define __DYDISNIX_TARGETCONFIG_H
#include <glib.h>
#include <nixxml-parse.h>

typedef struct
{
    GHashTable *services_to_ports;
    GHashTable *ports_to_services;
    gint last_port;
    gint min_port;
    gint max_port;
}
TargetConfig;

TargetConfig *create_target_config(gint last_port, gint min_port, gint max_port);

void *parse_target_config(xmlNodePtr element, void *userdata);

void delete_target_config(TargetConfig *target_config);

void print_target_config_nix(FILE *file, const void *value, const int indent_level, void *userdata);

void print_target_config_xml(FILE *file, const void *value, const int indent_level, const char *type_property_name, void *userdata);

void clean_obsolete_services_to_ports(TargetConfig *target_config, GHashTable *candidate_target_table, GHashTable *service_table, gchar *service_property, gchar *service_property_value);

#endif
