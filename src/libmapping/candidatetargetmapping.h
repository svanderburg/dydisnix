#ifndef __CANDIDATETARGETMAPPING_H
#define __CANDIDATETARGETMAPPING_H
#include <glib.h>

/**
 * Contains a mapping of a service to an array of canidate targets
 */
typedef struct
{
    /** Name of a service */
    gchar *service;
    /** Array of target hosts */
    GArray *targets;
}
DistributionItem;

GArray *create_candidate_target_array(char *candidate_mapping_file);

void delete_candidate_target_array(GArray *candidate_target_array);

void print_candidate_target_array(GArray *candidate_target_array);

void print_expr_of_candidate_target_array(GArray *candidate_target_array);

gint distribution_item_index(GArray *candidate_target_array, gchar *service);

DistributionItem *lookup_distribution_item(GArray *candidate_target_array, gchar *service);

#endif
