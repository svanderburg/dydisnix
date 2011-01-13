#include "divide.h"
#include "serviceproperties.h"
#include "infrastructureproperties.h"
#include "candidatetargetmapping.h"

static void delete_result_array(GArray *result_array)
{
    unsigned int i;
    
    for(i = 0; i < result_array->len; i++)
    {
	DistributionItem *item = g_array_index(result_array, DistributionItem*, i);
	g_array_free(item->targets, TRUE);
	g_free(item);
    }
    
    g_array_free(result_array, TRUE);
}

int divide(Strategy strategy, gchar *service_xml, gchar *infrastructure_xml, gchar *distribution_xml, gchar *service_property, gchar *infrastructure_property)
{
    unsigned int i;
    int exit_status = 0;
    GArray *service_property_array = create_service_property_array(service_xml);
    GArray *infrastructure_property_array = create_infrastructure_property_array(infrastructure_xml);
    GArray *candidate_target_array = create_candidate_target_array(distribution_xml);
    
    if(service_property_array == NULL || infrastructure_property_array == NULL || candidate_target_array == NULL)
    {
	g_printerr("Error with opening one of the models!\n");
	exit_status = 1;
    }
    else
    {
	GArray *result_array = g_array_new(FALSE, FALSE, sizeof(DistributionItem*));
    
	/* Iterate over each service */
	for(i = 0; i < candidate_target_array->len; i++)
	{
	    DistributionItem *item = g_array_index(candidate_target_array, DistributionItem*, i);
	    gint s_index = service_index(service_property_array, item->service);
	    Service *service = g_array_index(service_property_array, Service*, s_index);

	    gint service_prop_index = service_property_index(service, service_property);
    	    ServiceProperty *service_prop = g_array_index(service->property, ServiceProperty*, service_prop_index);
	
	    GArray *targets = item->targets;
	    unsigned int j;
	
	    DistributionItem *result_item = (DistributionItem*)g_malloc(sizeof(DistributionItem));
	    result_item->service = item->service;
	    result_item->targets = g_array_new(FALSE, FALSE, sizeof(gchar*));
	
	    /* Iterate over targets for the current service */
	
	    Target *select_target = NULL;
	
	    for(j = 0; j < targets->len; j++)
	    {
		gchar *target_name = g_array_index(targets, gchar*, j);
		gint i_index = infrastructure_index(infrastructure_property_array, target_name);
		Target *target = g_array_index(infrastructure_property_array, Target*, i_index);
	    
		gint infrastructure_prop_index = infrastructure_property_index(target, infrastructure_property);
		InfrastructureProperty *infrastructure_prop = g_array_index(target->property, InfrastructureProperty*, infrastructure_prop_index);
	    
		if(strategy == STRATEGY_GREEDY)
		{
		    if(atoi(service_prop->value) <= atoi(infrastructure_prop->value))
		    {
			substract_target_value(target, infrastructure_property, atoi(service_prop->value));
			g_array_append_val(result_item->targets, target_name);
			break;
		    }
		}
		else if(strategy == STRATEGY_HIGHEST_BIDDER)
		{
		    if(select_target == NULL)
		    {
			if(atoi(service_prop->value) <= atoi(infrastructure_prop->value))
			    select_target = target;
		    }
		    else
		    {
			gint select_infrastructure_prop_index = infrastructure_property_index(select_target, infrastructure_property);
			InfrastructureProperty *select_infrastructure_prop = g_array_index(select_target->property, InfrastructureProperty*, select_infrastructure_prop_index);
		    
			if(atoi(infrastructure_prop->value) > atoi(select_infrastructure_prop->value))
			    select_target = target;
		    }
		}
		else if(strategy == STRATEGY_LOWEST_BIDDER)
		{
	    	    if(select_target == NULL)
		    {
			if(atoi(service_prop->value) <= atoi(infrastructure_prop->value))
			    select_target = target;
		    }
		    else
		    {
			gint select_infrastructure_prop_index = infrastructure_property_index(select_target, infrastructure_property);
			InfrastructureProperty *select_infrastructure_prop = g_array_index(select_target->property, InfrastructureProperty*, select_infrastructure_prop_index);
		    
			if(atoi(infrastructure_prop->value) < atoi(select_infrastructure_prop->value))
			    select_target = target;
		    }
		}
	    }
	
	    if(strategy == STRATEGY_HIGHEST_BIDDER || strategy == STRATEGY_LOWEST_BIDDER)
	    {
		if(select_target != NULL)
		{
	    	    substract_target_value(select_target, infrastructure_property, atoi(service_prop->value));
		    g_array_append_val(result_item->targets, select_target->name);
		}
	    }
	
	    g_array_append_val(result_array, result_item);
	}
    
	/* Print Nix expression of the result */
	print_expr_of_candidate_target_array(result_array);
    
	/* Cleanup */
	delete_result_array(result_array);
    }
    
    /* Cleanup */
    
    if(service_property_array != NULL)
	delete_service_property_array(service_property_array);
	
    if(infrastructure_property_array != NULL)
	delete_infrastructure_property_array(infrastructure_property_array);

    if(candidate_target_array != NULL)
	delete_candidate_target_array(candidate_target_array);

    /* Return exit status */
    return exit_status;
}
