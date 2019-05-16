#ifndef __DYDISNIX_CANDIDATETARGETMAPPING_H
#define __DYDISNIX_CANDIDATETARGETMAPPING_H
#include <glib.h>

char *generate_distribution_xml_from_expr(char *distribution_expr, char *infrastructure_expr);

GHashTable *create_candidate_target_table_from_xml(const char *candidate_mapping_file);

GHashTable *create_candidate_target_table_from_nix(gchar *distribution_expr, gchar *infrastructure_expr);

GHashTable *create_candidate_target_table(gchar *distribution_expr, gchar *infrastructure_expr, int xml);

void delete_candidate_target_table(GHashTable *candidate_target_array);

void print_candidate_target_table_nix(GHashTable *candidate_target_table);

void print_candidate_target_table_xml(GHashTable *candidate_target_table);

#endif
