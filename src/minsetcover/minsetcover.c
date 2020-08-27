#include "minsetcover.h"
#include <stdlib.h>
#include <nixxml-ghashtable-iter.h>
#include "servicestable.h"
#include "targetstable2.h"
#include "candidatetargetmappingtable.h"
#include "targetmappingtable.h"

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
        GPtrArray *targets = (GPtrArray*)value;

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

static int count_number_of_uncovered_services(GPtrArray *services, GHashTable *covered_services_table)
{
    int count = 0;
    unsigned int i;

    for(i = 0; i < services->len; i++)
    {
        gchar *service_name = g_ptr_array_index(services, i);

        if(!g_hash_table_contains(covered_services_table, service_name))
            count++;
    }

    return count;
}

static double compute_cost_fraction(gchar *host_name, GPtrArray *services, GHashTable *targets_table, gchar *target_property, GHashTable *covered_services_table)
{
    Target *target = g_hash_table_lookup(targets_table, host_name);
    gchar *target_value = find_target_property(target, target_property);

    if(target_value == NULL)
        target_value = "0";

    int count = count_number_of_uncovered_services(services, covered_services_table);

    return atoi(target_value) / (double)count;
}

static gchar *find_target_with_smallest_cost_fraction(GHashTable *target_mapping_table, GHashTable *targets_table, gchar *target_property, GHashTable *covered_services_table)
{
    double min_cost_fraction = -1;
    gchar *min_cost_host_name = NULL;

    NixXML_GHashTableOrderedIter iter;
    gchar *host_name;
    gpointer value;

    NixXML_g_hash_table_ordered_iter_init(&iter, target_mapping_table);

    while(NixXML_g_hash_table_ordered_iter_next(&iter, &host_name, &value))
    {
        GPtrArray *services = (GPtrArray*)value;
        double cost_fraction = compute_cost_fraction(host_name, services, targets_table, target_property, covered_services_table);

        if(min_cost_fraction == -1 || cost_fraction < min_cost_fraction)
        {
            min_cost_fraction = cost_fraction;
            min_cost_host_name = host_name;
        }
    }

    NixXML_g_hash_table_ordered_iter_destroy(&iter);

    return min_cost_host_name;
}

static void process_min_cost_target_mapping(gchar *host_name, GPtrArray *services, GHashTable *result_table, GHashTable *covered_services_table)
{
    unsigned int i;

    for(i = 0; i < services->len; i++)
    {
        gchar *service = g_ptr_array_index(services, i);
        GPtrArray *targets = g_hash_table_lookup(result_table, service);

        if(!g_hash_table_contains(covered_services_table, service))
        {
            CandidateTargetMapping *target_mapping = create_candidate_target_auto_mapping((xmlChar*)host_name);

            g_hash_table_insert(covered_services_table, service, service); // Mark the service as covered
            g_ptr_array_add(targets, target_mapping); // Add the targets to the result table
        }
    }
}

static GHashTable *approximate_minset_cover_greedy(GHashTable *service_table, GHashTable *targets_table, GHashTable *candidate_target_table, GHashTable *target_mapping_table, gchar *target_property)
{
    /* Create a result table with the same services as in the input distribution model and empty candidate hosts */
    GHashTable *result_table = generate_empty_mappings_table(candidate_target_table);

    GHashTable *covered_services_table = g_hash_table_new(g_str_hash, g_str_equal);

    while(g_hash_table_size(covered_services_table) < g_hash_table_size(service_table))
    {
        gchar *host_name = find_target_with_smallest_cost_fraction(target_mapping_table, targets_table, target_property, covered_services_table);
        process_min_cost_target_mapping(host_name, g_hash_table_lookup(target_mapping_table, host_name), result_table, covered_services_table);
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
        GHashTable *target_mapping_table = create_target_mapping_table(candidate_target_table);

        /* Execute minimum set cover approximation */
        GHashTable *result_table = approximate_minset_cover_greedy(service_table, targets_table, candidate_target_table, target_mapping_table, target_property);

        /* Print resulting expression to stdout */
        if(flags & DYDISNIX_FLAG_OUTPUT_XML)
            print_candidate_target_table_xml(stdout, result_table, 0, NULL, NULL);
        else
            print_candidate_target_table_nix(stdout, result_table, 0, &automapped);

        /* Cleanup */
        delete_result_table(result_table);
        delete_target_mapping_table(target_mapping_table);
    }

    /* Cleanup */
    delete_candidate_target_table(candidate_target_table);
    delete_targets_table(targets_table);
    delete_service_table(service_table);

    return exit_status;
}
