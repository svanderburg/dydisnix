#ifndef __DYDISNIX_PORTCONFIGURATION_H
#define __DYDISNIX_PORTCONFIGURATION_H
#include <stdio.h>
#include <glib.h>
#include <nixxml-types.h>
#include "targetconfig.h"

typedef struct
{
    TargetConfig *global_config;

    GHashTable *target_configs;
}
PortConfiguration;

char *generate_ports_xml_from_expr(char *ports_expr);

PortConfiguration *create_empty_port_configuration(void);

PortConfiguration *open_port_configuration_from_xml(const gchar *port_configuration_file);

PortConfiguration *open_port_configuration_from_nix(gchar *port_configuration_file);

PortConfiguration *open_port_configuration(gchar *port_configuration_file, NixXML_bool xml);

void delete_port_configuration(PortConfiguration *port_configuration);

/**
 * If a port reservation has been made use that port number.
 * Otherwise assign a new one
 *
 * @param port_configuration Port configuration
 * @param taget Target machine to which a service is deployed, or NULL to use global port space
 * @param service Name of the service to assign a port to
 */
gint assign_or_reuse_port(PortConfiguration *port_configuration, gchar *target, gchar *service);

void print_port_configuration_nix(FILE *file, const void *value, const int indent_level, void *userdata);

void print_port_configuration_xml(FILE *file, const void *value, const int indent_level, const char *type_property_name, void *userdata);

void clean_obsolete_reservations(PortConfiguration *port_configuration, GHashTable *candidate_target_table, GHashTable *service_table, gchar *service_property);

#endif
