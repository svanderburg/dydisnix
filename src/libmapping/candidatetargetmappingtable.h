#ifndef __DYDISNIX_CANDIDATETARGETMAPPINGTABLE_H
#define __DYDISNIX_CANDIDATETARGETMAPPINGTABLE_H
#include <stdio.h>
#include <glib.h>
#include <nixxml-types.h>
#include "candidatetargetsarray.h"

char *generate_distribution_xml_from_expr(char *distribution_expr, char *infrastructure_expr);

GHashTable *create_candidate_target_table_from_xml(const char *candidate_mapping_file, NixXML_bool *automapped);

GHashTable *create_candidate_target_table_from_nix(gchar *distribution_expr, gchar *infrastructure_expr, NixXML_bool *automapped);

GHashTable *create_candidate_target_table(gchar *distribution_expr, gchar *infrastructure_expr, NixXML_bool xml, NixXML_bool *automapped);

void delete_candidate_target_table(GHashTable *candidate_target_array);

void print_candidate_target_table_nix(FILE *file, GHashTable *candidate_target_table, const int indent_level, NixXML_bool *automapped);

void print_candidate_target_table_xml(FILE *file, GHashTable *candidate_target_table, const int indent_level, const char *type_property_name, void *userdata);

#endif
