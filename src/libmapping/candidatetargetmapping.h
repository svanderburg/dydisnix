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
    GArray *targets;
}
DistributionItem;

GArray *create_candidate_target_array(const char *candidate_mapping_file);

void delete_candidate_target_array(GArray *candidate_target_array);

void print_candidate_target_array(const GArray *candidate_target_array);

void print_expr_of_candidate_target_array(const GArray *candidate_target_array);

gint distribution_item_index(GArray *candidate_target_array, gchar *service);

DistributionItem *lookup_distribution_item(GArray *candidate_target_array, gchar *service);

#endif
