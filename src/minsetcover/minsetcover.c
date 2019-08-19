#include "minsetcover.h"
#include "servicestable.h"
#include "targetstable2.h"
#include "candidatetargetmappingtable.h"
#include "targetmapping.h"
#include <stdlib.h>

int minsetcover(gchar *services, gchar *infrastructure, gchar *distribution, gchar *target_property, const unsigned int flags)
{
    int automapped;
    int xml = flags & DYDISNIX_FLAG_XML;
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
        GHashTable *covered_services_table = g_hash_table_new(g_str_hash, g_str_equal);
        GPtrArray *target_mapping_array = create_target_mapping_array(candidate_target_table);
        GHashTable *result_table = g_hash_table_new(g_str_hash, g_str_equal);
        GHashTableIter iter;
        gpointer key, value;

        /* Create a result array with the same services as in the input distribution model and empty candidate hosts */

        g_hash_table_iter_init(&iter, candidate_target_table);
        while(g_hash_table_iter_next(&iter, &key, &value))
        {
            gchar *service = (gchar*)key;
            GPtrArray *result_targets = g_ptr_array_new();
            g_hash_table_insert(result_table, service, result_targets);
        }

        /* Execute minimum set cover approximation */

	while(g_hash_table_size(covered_services_table) < g_hash_table_size(service_table))
	{
	    unsigned int i;
	    double min_cost = -1;
	    int min_cost_index = -1;
	    TargetMappingItem *min_cost_target_mapping;

	    for(i = 0; i < target_mapping_array->len; i++)
	    {
		TargetMappingItem *target_mapping = g_ptr_array_index(target_mapping_array, i);
		Target *target = g_hash_table_lookup(targets_table, target_mapping->target);
		gchar *target_value = find_target_property(target, target_property);

		int count = 0;
		double cost;
		unsigned int j;
	    
		if(target_value == NULL)
		{
		    g_printerr("Value for target property: %s not found!\n", target_property);
		    exit_status = 1;
		    break;
		}
	    
		for(j = 0; j < target_mapping->services->len; j++)
		{
		    gchar *service_name = g_ptr_array_index(target_mapping->services, j);
		
		    if(g_hash_table_lookup(covered_services_table, service_name) == NULL)
			count++;
		}
		
		cost = atoi(target_value) / (double)count;
	    
		if(min_cost == -1 || cost < min_cost)
		{
		    min_cost = cost;
		    min_cost_index = i;
		}
	    }
	
	    min_cost_target_mapping = g_ptr_array_index(target_mapping_array, min_cost_index);
	
	    for(i = 0; i < min_cost_target_mapping->services->len; i++)
	    {
		gchar *service = g_ptr_array_index(min_cost_target_mapping->services, i);
		GPtrArray *targets = g_hash_table_lookup(result_table, service);

		if(g_hash_table_lookup(covered_services_table, service) == NULL)
		{
		    CandidateTargetMapping *target_mapping = (CandidateTargetMapping*)g_malloc(sizeof(CandidateTargetMapping));
		    target_mapping->target = (xmlChar*)min_cost_target_mapping->target;
		    target_mapping->container = NULL;

		    g_ptr_array_add(targets, target_mapping);
		    g_hash_table_insert(covered_services_table, service, service);
		}
	    }
	}

        /* Print resulting expression to stdout */
        if(flags & DYDISNIX_FLAG_OUTPUT_XML)
            print_candidate_target_table_xml(stdout, result_table, 0, NULL, NULL);
        else
            print_candidate_target_table_nix(stdout, result_table, 0, &automapped);

        /* Cleanup */

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

        delete_target_mapping_array(target_mapping_array);
        g_hash_table_destroy(covered_services_table);
    }

    /* Cleanup */
    delete_candidate_target_table(candidate_target_table);
    delete_targets_table(targets_table);
    delete_service_table(service_table);

    return exit_status;
}
