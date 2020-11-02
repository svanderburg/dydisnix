#ifndef __DYDISNIX_DISTRIBUTIONTABLE_H
#define __DYDISNIX_DISTRIBUTIONTABLE_H
#include <stdio.h>
#include <glib.h>
#include <nixxml-types.h>
#include "distributionmappingarray.h"

char *generate_distribution_xml_from_expr(char *distribution_expr, char *infrastructure_expr, char *extra_params);

GHashTable *create_distribution_table_from_xml(const char *distribution_file, NixXML_bool *automapped);

GHashTable *create_distribution_table_from_nix(gchar *distribution_expr, gchar *infrastructure_expr, gchar *extra_params, NixXML_bool *automapped);

GHashTable *create_distribution_table(gchar *distribution_expr, gchar *infrastructure_expr, gchar *extra_params, NixXML_bool xml, NixXML_bool *automapped);

void delete_distribution_table(GHashTable *distribution_table);

void print_distribution_table_nix(FILE *file, GHashTable *distribution_table, const int indent_level, NixXML_bool *automapped);

void print_distribution_table_xml(FILE *file, GHashTable *distribution_table, const int indent_level, const char *type_property_name, void *userdata);

#endif
