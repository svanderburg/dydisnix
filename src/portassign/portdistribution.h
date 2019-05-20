#ifndef __DYDISNIX_PORTDISTRIBUTION_H
#define __DYDISNIX_PORTDISTRIBUTION_H
#include <glib.h>
#include <stdio.h>
#include "portconfiguration.h"

GHashTable *create_port_distribution_table(PortConfiguration *port_configuration, GHashTable *service_table, GHashTable *candidate_target_table, gchar *service_property);

void delete_port_distribution_table(GHashTable *port_distribution_table);

void print_port_distribution_table_nix(FILE *file, const void *value, const int indent_level, void *userdata);

void print_port_distribution_table_xml(FILE *file, const void *value, const int indent_level, const char *type_property_name, void *userdata);

#endif
