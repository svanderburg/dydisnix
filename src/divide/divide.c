#include "divide.h"
#include <stdlib.h>
#include <nixxml-generate-env-generic.h>
#include <nixxml-ghashtable-iter.h>
#include "servicestable.h"
#include "targetstable2.h"
#include "distributiontable.h"

static void delete_result_table(GHashTable *result_table)
{
    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, result_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        GPtrArray *mappings = (GPtrArray*)value;
        g_ptr_array_free(mappings, TRUE);
    }

    g_hash_table_destroy(result_table);
}

typedef enum
{
    SELECT_STATUS_NO,
    SELECT_STATUS_YES,
    SELECT_STATUS_MAYBE
}
SelectStatus;

static NixXML_bool target_has_sufficient_resources(const char *service_property_value, const char *target_property_value)
{
    return (atoi(service_property_value) <= atoi(target_property_value));
}

typedef SelectStatus (*select_target_function) (Target *target, const char *service_property_value, const char *target_property_value, Target *candidate_target, const char *target_property);

static SelectStatus select_target_with_sufficient_resources(Target *target, const char *service_property_value, const char *target_property_value, Target *candidate_target, const char *target_property)
{
    if(target_has_sufficient_resources(service_property_value, target_property_value))
        return SELECT_STATUS_YES;
    else
        return SELECT_STATUS_NO;
}

static SelectStatus select_higher_bidder(Target *target, const char *service_property_value, const char *target_property_value, Target *candidate_target, const char *target_property)
{
    if(candidate_target == NULL)
    {
        if(target_has_sufficient_resources(service_property_value, target_property_value))
            return SELECT_STATUS_MAYBE;
        else
            return SELECT_STATUS_NO;
    }
    else
    {
        gchar *candidate_target_property_value = find_target_property(candidate_target, target_property);

        if(atoi((char*)target_property_value) > atoi(candidate_target_property_value))
            return SELECT_STATUS_MAYBE;
        else
            return SELECT_STATUS_NO;
    }
}

static SelectStatus select_lower_bidder(Target *target, const char *service_property_value, const char *target_property_value, Target *candidate_target, const char *target_property)
{
    if(candidate_target == NULL)
    {
        if(target_has_sufficient_resources(service_property_value, target_property_value))
            return SELECT_STATUS_MAYBE;
        else
            return SELECT_STATUS_NO;
    }
    else
    {
        gchar *candidate_target_property_value = find_target_property(candidate_target, target_property);

        if(target_has_sufficient_resources(service_property_value, target_property_value) && atoi((char*)target_property_value) < atoi(candidate_target_property_value))
            return SELECT_STATUS_MAYBE;
        else
            return SELECT_STATUS_NO;
    }
}

static DistributionMapping *select_mapping(const GPtrArray *targets, GHashTable *targets_table, const char *service_property_value, const char *target_property, select_target_function select_target)
{
    DistributionMapping *candidate_distribution_mapping = NULL;
    Target *candidate_target = NULL;
    unsigned int i;

    for(i = 0; i < targets->len; i++)
    {
        DistributionMapping *mapping = g_ptr_array_index(targets, i);
        Target *target = g_hash_table_lookup(targets_table, mapping->target);
        gchar *target_property_value = find_target_property(target, target_property);

        if(target_property_value != NULL) // Only consider a target a candidate if it provides the desired target property
        {
            SelectStatus status = select_target(target, service_property_value, target_property_value, candidate_target, target_property);

            if(status != SELECT_STATUS_NO)
            {
                candidate_distribution_mapping = mapping;
                candidate_target = target;
            }

            if(status == SELECT_STATUS_YES) // If we know for sure that we have selected the right target, then we don't need to expect the rest
                 break;
        }
    }

    if(candidate_target != NULL) // If a candidate target was selected, substract the service property from it
        substract_target_value(candidate_target, target_property, atoi(service_property_value));

    return candidate_distribution_mapping;
}

static NixXML_bool select_target_for_each_mapping(GHashTable *service_table, GHashTable *targets_table, GHashTable *distribution_table, GHashTable *result_table, const char *service_property, const char *target_property, select_target_function select_target)
{
    NixXML_GHashTableOrderedIter iter;
    gchar *service_name;
    gpointer value;

    NixXML_g_hash_table_ordered_iter_init(&iter, distribution_table);

    while(NixXML_g_hash_table_ordered_iter_next(&iter, &service_name, &value))
    {
        GPtrArray *targets = (GPtrArray*)value;
        Service *service = g_hash_table_lookup(service_table, service_name);
        NixXML_Node *service_property_value = g_hash_table_lookup(service->properties, service_property);

        if(service_property_value == NULL)
        {
            g_printerr("Service: %s has no property named: %s\n", service_name, service_property);
            return FALSE;
        }
        else
        {
            DistributionMapping *selected_mapping = select_mapping(targets, targets_table, service_property_value->value, target_property, select_target);

            if(selected_mapping == NULL)
            {
                g_printerr("Unable to select a target machine for service: %s, because none of them has sufficient resources left!\n", service_name);
                return FALSE;
            }
            else
            {
                GPtrArray *result_targets = g_ptr_array_new();
                g_ptr_array_add(result_targets, selected_mapping);
                g_hash_table_insert(result_table, service_name, result_targets);
            }
        }
    }

    NixXML_g_hash_table_ordered_iter_destroy(&iter);
    return TRUE;
}

int divide(Strategy strategy, gchar *services, gchar *infrastructure, gchar *distribution, gchar *service_property, gchar *target_property, const unsigned int flags)
{
    int exit_status = 0;
    NixXML_bool xml = flags & DYDISNIX_FLAG_XML;
    NixXML_bool automapped;

    GHashTable *service_table = create_service_table(services, xml);
    GHashTable *targets_table = create_targets_table2(infrastructure, xml);
    GHashTable *distribution_table = create_distribution_table(distribution, infrastructure, xml, &automapped);

    if(service_table == NULL || targets_table == NULL || distribution_table == NULL)
    {
        g_printerr("Error with opening one of the models!\n");
        exit_status = 1;
    }
    else
    {
        GHashTable *result_table = g_hash_table_new(g_str_hash, g_str_equal);
        select_target_function select_target;

        /* Select function that checks for the appropriate target, based on the selected strategy */
        switch(strategy)
        {
            case STRATEGY_GREEDY:
                select_target = select_target_with_sufficient_resources;
                break;
            case STRATEGY_HIGHEST_BIDDER:
                select_target = select_higher_bidder;
                break;
            case STRATEGY_LOWEST_BIDDER:
                select_target = select_lower_bidder;
                break;
            default:
                g_printerr("Unknown strategy: %d\n", strategy);
                select_target = NULL;
        }

        /* Execute division strategy */
        exit_status = !select_target_for_each_mapping(service_table, targets_table, distribution_table, result_table, service_property, target_property, select_target);

        /* Print Nix expression of the result */
        if(flags & DYDISNIX_FLAG_OUTPUT_XML)
            print_distribution_table_xml(stdout, result_table, 0, NULL, NULL);
        else
            print_distribution_table_nix(stdout, result_table, 0, &automapped);

        /* Cleanup */
        delete_result_table(result_table);
    }

    /* Cleanup */
    delete_service_table(service_table);
    delete_targets_table(targets_table);
    delete_distribution_table(distribution_table);

    /* Return exit status */
    return exit_status;
}
