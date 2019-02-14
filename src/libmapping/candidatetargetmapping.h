#ifndef __DYDISNIX_CANDIDATETARGETMAPPING_H
#define __DYDISNIX_CANDIDATETARGETMAPPING_H
#include <glib.h>

/**
 * @brief Contains a mapping of a service to an array of candidate targets
 */
typedef struct
{
    /** Name of a service */
    gchar *service;
    /** Array of target hosts */
    GPtrArray *targets;
}
DistributionItem;

char *generate_distribution_xml_from_expr(char *distribution_expr, char *infrastructure_expr);

GPtrArray *create_candidate_target_array_from_xml(const char *candidate_mapping_file);

GPtrArray *create_candidate_target_array_from_nix(gchar *distribution_expr, gchar *infrastructure_expr);

void delete_candidate_target_array(GPtrArray *candidate_target_array);

void print_candidate_target_array(const GPtrArray *candidate_target_array);

void print_expr_of_candidate_target_array(const GPtrArray *candidate_target_array);

DistributionItem *find_distribution_item(GPtrArray *candidate_target_array, gchar *service);

#endif
