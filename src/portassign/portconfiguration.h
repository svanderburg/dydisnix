#ifndef __DYDISNIX_PORTCONFIGURATION_H
#define __DYDISNIX_PORTCONFIGURATION_H
#include <glib.h>

typedef struct
{
    GHashTable *services_to_ports;
    GHashTable *ports_to_services;
    gint last_port;
    gint min_port;
    gint max_port;
}
TargetConfig;

typedef struct
{
    TargetConfig *global_config;
    
    GHashTable *target_configs;
}
PortConfiguration;

char *generate_ports_xml_from_expr(char *ports_expr);

void init_port_configuration(PortConfiguration *port_configuration);

int open_port_configuration_from_xml(PortConfiguration *port_configuration, const gchar *port_configuration_file);

int open_port_configuration_from_nix(PortConfiguration *port_configuration, gchar *port_configuration_file);

int open_port_configuration(PortConfiguration *port_configuration, gchar *port_configuration_file, int xml);

void destroy_port_configuration(PortConfiguration *port_configuration);

/**
 * If a port reservation has been made use that port number.
 * Otherwise assign a new one
 *
 * @param port_configuration Port configuration
 * @param taget Target machine to which a service is deployed, or NULL to use global port space
 * @param service Name of the service to assign a port to
 */
gint assign_or_reuse_port(PortConfiguration *port_configuration, gchar *target, gchar *service);

void print_port_configuration(PortConfiguration *port_configuration);

void clean_obsolete_reservations(PortConfiguration *port_configuration, GPtrArray *candidate_target_array, GPtrArray *service_property_array, gchar *service_property);

#endif
