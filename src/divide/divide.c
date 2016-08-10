#include "divide.h"
#include <stdlib.h>
#include "serviceproperties.h"
#include "infrastructureproperties.h"
#include "candidatetargetmapping.h"

static void delete_result_array(GPtrArray *result_array)
{
    unsigned int i;
    
    for(i = 0; i < result_array->len; i++)
    {
	DistributionItem *item = g_ptr_array_index(result_array, i);
	g_ptr_array_free(item->targets, TRUE);
	g_free(item);
    }
    
    g_ptr_array_free(result_array, TRUE);
}

int divide(Strategy strategy, gchar *service_xml, gchar *infrastructure_xml, gchar *distribution_xml, gchar *service_property, gchar *target_property)
{
    unsigned int i;
    int exit_status = 0;
    GPtrArray *service_property_array = create_service_property_array(service_xml);
    GPtrArray *targets_array = create_target_array_from_xml(infrastructure_xml);
    GPtrArray *candidate_target_array = create_candidate_target_array(distribution_xml);
    
    if(service_property_array == NULL || targets_array == NULL || candidate_target_array == NULL)
    {
	g_printerr("Error with opening one of the models!\n");
	exit_status = 1;
    }
    else
    {
	GPtrArray *result_array = g_ptr_array_new();
    
	/* Iterate over each service */
	for(i = 0; i < candidate_target_array->len; i++)
	{
	    DistributionItem *item = g_ptr_array_index(candidate_target_array, i);
	    Service *service = find_service(service_property_array, item->service);
	    ServiceProperty *service_prop = find_service_property(service, service_property);
	    
	    GPtrArray *targets = item->targets;
	    unsigned int j;
	
	    DistributionItem *result_item;
	    
	    if(service_prop == NULL)
	    {
		g_printerr("Service property: %s not found!\n", service_property);
		exit_status = 1;
		break;
	    }
	    
	    result_item = (DistributionItem*)g_malloc(sizeof(DistributionItem));
	    result_item->service = item->service;
	    result_item->targets = g_ptr_array_new();
	
	    /* Iterate over targets for the current service */
	
	    Target *select_target = NULL;
	
	    for(j = 0; j < targets->len; j++)
	    {
		gchar *target_name = g_ptr_array_index(targets, j);
		Target *target = find_target_by_name(targets_array, target_name);
		TargetProperty *target_prop = find_target_property(target, target_property);
		
		if(target_prop == NULL)
		{
		    g_printerr("Target property: %s not found!\n", target_property);
		    exit_status = 1;
		    break;
		}
	    
		if(strategy == STRATEGY_GREEDY)
		{
		    if(atoi(service_prop->value) <= atoi(target_prop->value))
		    {
			substract_target_value(target, target_property, atoi(service_prop->value));
			select_target = target;
			g_ptr_array_add(result_item->targets, target_name);
			break;
		    }
		}
		else if(strategy == STRATEGY_HIGHEST_BIDDER)
		{
		    if(select_target == NULL)
		    {
			if(atoi(service_prop->value) <= atoi(target_prop->value))
			    select_target = target;
		    }
		    else
		    {
			TargetProperty *select_target_prop = find_target_property(select_target, target_property);
		    
			if(atoi(target_prop->value) > atoi(select_target_prop->value))
			    select_target = target;
		    }
		}
		else if(strategy == STRATEGY_LOWEST_BIDDER)
		{
	    	    if(select_target == NULL)
		    {
			if(atoi(service_prop->value) <= atoi(target_prop->value))
			    select_target = target;
		    }
		    else
		    {
			TargetProperty *select_target_prop = find_target_property(select_target, target_property);
			
			if(atoi(target_prop->value) < atoi(select_target_prop->value) && atoi(service_prop->value) <= atoi(select_target_prop->value))
			    select_target = target;
		    }
		}
	    }
	    
	    if(select_target == NULL)
	    {
	        g_printerr("Unable to select a target machine for service: %s, because none of them has sufficient resources left!\n", item->service);
	        exit_status = 1;
	    }
	
	    if(strategy == STRATEGY_HIGHEST_BIDDER || strategy == STRATEGY_LOWEST_BIDDER)
	    {
		if(select_target != NULL)
		{
	    	    substract_target_value(select_target, target_property, atoi(service_prop->value));
		    g_ptr_array_add(result_item->targets, select_target->name);
		}
	    }
	
	    g_ptr_array_add(result_array, result_item);
	}
    
	/* Print Nix expression of the result */
	print_expr_of_candidate_target_array(result_array);
    
	/* Cleanup */
	delete_result_array(result_array);
    }
    
    /* Cleanup */
    
    delete_service_property_array(service_property_array);
    delete_target_array(targets_array);
    delete_candidate_target_array(candidate_target_array);

    /* Return exit status */
    return exit_status;
}
