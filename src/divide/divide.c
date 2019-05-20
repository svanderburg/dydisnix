#include "divide.h"
#include <stdlib.h>
#include "serviceproperties.h"
#include "infrastructureproperties.h"
#include "candidatetargetmapping.h"

static void delete_result_table(GHashTable *result_table)
{
    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, result_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        GPtrArray *targets = (GPtrArray*)value;
        g_ptr_array_free(targets, TRUE);
    }

    g_hash_table_destroy(result_table);
}

int divide(Strategy strategy, gchar *services, gchar *infrastructure, gchar *distribution, gchar *service_property, gchar *target_property, const unsigned int flags)
{
    int exit_status = 0;
    int xml = flags & DYDISNIX_FLAG_XML;
    GHashTable *service_table = create_service_table(services, xml);
    GPtrArray *targets_array = create_target_property_array(infrastructure, xml);
    GHashTable *candidate_target_table = create_candidate_target_table(distribution, infrastructure, xml);

    if(service_table == NULL || targets_array == NULL || candidate_target_table == NULL)
    {
	g_printerr("Error with opening one of the models!\n");
	exit_status = 1;
    }
    else
    {
        GHashTable *result_table = g_hash_table_new(g_str_hash, g_str_equal);
        GHashTableIter iter;
        gpointer key, value;

        /* Iterate over each service */

        g_hash_table_iter_init(&iter, candidate_target_table);
        while(g_hash_table_iter_next(&iter, &key, &value))
        {
            gchar *service_name = (gchar*)key;
            GPtrArray *targets = (GPtrArray*)value;

            Service *service = g_hash_table_lookup(service_table, service_name);
            gchar *service_value = g_hash_table_lookup(service->properties, service_property);
            GPtrArray *result_targets = g_ptr_array_new();

	    unsigned int j;
	
	    if(service_value == NULL)
	    {
		g_printerr("Value for service property: %s not found!\n", service_property);
		exit_status = 1;
		break;
	    }

	    /* Iterate over targets for the current service */
	
	    Target *select_target = NULL;
	
	    for(j = 0; j < targets->len; j++)
	    {
		gchar *target_name = g_ptr_array_index(targets, j);
		Target *target = find_target_by_name(targets_array, target_name);
		gchar *target_value = find_target_property(target, target_property);
		
		if(target_value == NULL)
		{
		    g_printerr("Value for target property: %s not found!\n", target_property);
		    exit_status = 1;
		    break;
		}

		if(strategy == STRATEGY_GREEDY)
		{
		    if(atoi(service_value) <= atoi(target_value))
		    {
			substract_target_value(target, target_property, atoi(service_value));
			select_target = target;
			g_ptr_array_add(result_targets, target_name);
			break;
		    }
		}
		else if(strategy == STRATEGY_HIGHEST_BIDDER)
		{
		    if(select_target == NULL)
		    {
			if(atoi(service_value) <= atoi(target_value))
			    select_target = target;
		    }
		    else
		    {
			gchar *select_target_value = find_target_property(select_target, target_property);

			if(atoi(target_value) > atoi(select_target_value))
			    select_target = target;
		    }
		}
		else if(strategy == STRATEGY_LOWEST_BIDDER)
		{
	    	    if(select_target == NULL)
		    {
			if(atoi(service_value) <= atoi(target_value))
			    select_target = target;
		    }
		    else
		    {
			gchar *select_target_value = find_target_property(select_target, target_property);

			if(atoi(target_value) < atoi(select_target_value) && atoi(service_value) <= atoi(select_target_value))
			    select_target = target;
		    }
		}
	    }
	    
	    if(select_target == NULL)
	    {
	        g_printerr("Unable to select a target machine for service: %s, because none of them has sufficient resources left!\n", service_name);
	        exit_status = 1;
	    }
	
	    if(strategy == STRATEGY_HIGHEST_BIDDER || strategy == STRATEGY_LOWEST_BIDDER)
	    {
		if(select_target != NULL)
		{
		    substract_target_value(select_target, target_property, atoi(service_value));
		    g_ptr_array_add(result_targets, select_target->name);
		}
	    }

            g_hash_table_insert(result_table, service_name, result_targets);
	}

        /* Print Nix expression of the result */
        if(flags & DYDISNIX_FLAG_OUTPUT_XML)
            print_candidate_target_table_xml(result_table);
        else
            print_candidate_target_table_nix(result_table);

        /* Cleanup */
        delete_result_table(result_table);
    }

    /* Cleanup */

    delete_service_table(service_table);
    delete_target_array(targets_array);
    delete_candidate_target_table(candidate_target_table);

    /* Return exit status */
    return exit_status;
}
