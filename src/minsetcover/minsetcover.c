#include "minsetcover.h"
#include "servicestable.h"
#include "targetstable2.h"
#include "candidatetargetmappingtable.h"
#include "targetmapping.h"
#include <stdlib.h>

static GHashTable *generate_empty_mappings_table(GHashTable *candidate_target_table)
{
    GHashTableIter iter;
    gpointer key, value;

    GHashTable *result_table = g_hash_table_new(g_str_hash, g_str_equal);

    g_hash_table_iter_init(&iter, candidate_target_table);

    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        gchar *service = (gchar*)key;
        GPtrArray *result_targets = g_ptr_array_new();
        g_hash_table_insert(result_table, service, result_targets);
    }

    return result_table;
}

static void delete_result_table(GHashTable *result_table)
{
    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, result_table);

    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        GPtrArray *targets = (GPtrArray*)targets;

        if(targets != NULL)
        {
            unsigned int i;

            for(i = 0; i < targets->len; i++)
            {
                CandidateTargetMapping *target_mapping = (CandidateTargetMapping*)g_ptr_array_index(targets, i);
                g_free(target_mapping);
            }

            g_ptr_array_free(targets, TRUE);
        }
    }

    g_hash_table_destroy(result_table);
}

static int count_number_of_uncovered_services_for_target_mapping(TargetMappingItem *target_mapping, GHashTable *covered_services_table)
{
    int count = 0;
    unsigned int i;

    for(i = 0; i < target_mapping->services->len; i++)
    {
        gchar *service_name = g_ptr_array_index(target_mapping->services, i);

        if(g_hash_table_lookup(covered_services_table, service_name) == NULL)
            count++;
    }

    return count;
}

static double compute_cost_fraction_for_target_mapping(TargetMappingItem *target_mapping, GHashTable *targets_table, gchar *target_property, GHashTable *covered_services_table)
{
    Target *target = g_hash_table_lookup(targets_table, target_mapping->target);
    gchar *target_value = find_target_property(target, target_property);

    if(target_value == NULL)
        target_value = "0";

    int count = count_number_of_uncovered_services_for_target_mapping(target_mapping, covered_services_table);

    return atoi(target_value) / (double)count;
}

static TargetMappingItem *find_target_mapping_with_smallest_cost_fraction(GPtrArray *target_mapping_array, GHashTable *targets_table, gchar *target_property, GHashTable *covered_services_table)
{
    unsigned int i;
    double min_cost_fraction = -1;
    int min_cost_index = -1;

    for(i = 0; i < target_mapping_array->len; i++)
    {
        TargetMappingItem *target_mapping = g_ptr_array_index(target_mapping_array, i);
        double cost_fraction = compute_cost_fraction_for_target_mapping(target_mapping, targets_table, target_property, covered_services_table);

        if(min_cost_fraction == -1 || cost_fraction < min_cost_fraction)
        {
            min_cost_fraction = cost_fraction;
            min_cost_index = i;
        }
    }

    return g_ptr_array_index(target_mapping_array, min_cost_index);
}

static void process_min_cost_target_mapping(TargetMappingItem *min_cost_target_mapping, GHashTable *result_table, GHashTable *covered_services_table)
{
    unsigned int i;

    for(i = 0; i < min_cost_target_mapping->services->len; i++)
    {
        gchar *service = g_ptr_array_index(min_cost_target_mapping->services, i);
        GPtrArray *targets = g_hash_table_lookup(result_table, service);

        if(g_hash_table_lookup(covered_services_table, service) == NULL)
        {
            CandidateTargetMapping *target_mapping = (CandidateTargetMapping*)g_malloc(sizeof(CandidateTargetMapping));
            target_mapping->target = (xmlChar*)min_cost_target_mapping->target;
            target_mapping->container = NULL;

            g_hash_table_insert(covered_services_table, service, service); // Mark the service as covered
            g_ptr_array_add(targets, target_mapping); // Add the targets to the result table
        }
    }
}

static GHashTable *approximate_minset_cover_greedy(GHashTable *service_table, GHashTable *targets_table, GHashTable *candidate_target_table, GPtrArray *target_mapping_array, gchar *target_property)
{
    /* Create a result table with the same services as in the input distribution model and empty candidate hosts */
    GHashTable *result_table = generate_empty_mappings_table(candidate_target_table);

    GHashTable *covered_services_table = g_hash_table_new(g_str_hash, g_str_equal);

    while(g_hash_table_size(covered_services_table) < g_hash_table_size(service_table))
    {
        TargetMappingItem *min_cost_target_mapping = find_target_mapping_with_smallest_cost_fraction(target_mapping_array, targets_table, target_property, covered_services_table);
        process_min_cost_target_mapping(min_cost_target_mapping, result_table, covered_services_table);
    }

    g_hash_table_destroy(covered_services_table);
    return result_table;
}

int minsetcover(gchar *services, gchar *infrastructure, gchar *distribution, gchar *target_property, const unsigned int flags)
{
    NixXML_bool automapped;
    NixXML_bool xml = flags & DYDISNIX_FLAG_XML;
    GHashTable *service_table = create_service_table(services, xml);
    GHashTable *targets_table = create_targets_table2(infrastructure, xml);
    GHashTable *candidate_target_table = create_candidate_target_table(distribution, infrastructure, xml, &automapped);
    int exit_status = 0;

    if(service_table == NULL || targets_table == NULL || candidate_target_table == NULL)
    {
        g_printerr("Error with opening one of the models!\n");
        exit_status = 1;
    }
    else
    {
        GPtrArray *target_mapping_array = create_target_mapping_array(candidate_target_table);

        /* Execute minimum set cover approximation */
        GHashTable *result_table = approximate_minset_cover_greedy(service_table, targets_table, candidate_target_table, target_mapping_array, target_property);

        /* Print resulting expression to stdout */
        if(flags & DYDISNIX_FLAG_OUTPUT_XML)
            print_candidate_target_table_xml(stdout, result_table, 0, NULL, NULL);
        else
            print_candidate_target_table_nix(stdout, result_table, 0, &automapped);

        /* Cleanup */
        delete_result_table(result_table);
        delete_target_mapping_array(target_mapping_array);
    }

    /* Cleanup */
    delete_candidate_target_table(candidate_target_table);
    delete_targets_table(targets_table);
    delete_service_table(service_table);

    return exit_status;
}
