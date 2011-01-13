#include "serviceproperties.h"
#include "infrastructureproperties.h"
#include "candidatetargetmapping.h"
#include "targetmapping.h"
#include <stdlib.h>

int minsetcover(gchar *services_xml, gchar *infrastructure_xml, gchar *distribution_xml, gchar *target_property)
{
    GArray *service_property_array = create_service_property_array(services_xml);
    GArray *infrastructure_property_array = create_infrastructure_property_array(infrastructure_xml);
    GArray *candidate_target_array = create_candidate_target_array(distribution_xml);
    int exit_status = 0;
    
    if(service_property_array == NULL || infrastructure_property_array == NULL || candidate_target_array == NULL)
    {
	g_printerr("Error with opening one of the models!\n");
	exit_status = 1;
    }
    else
    {
	GHashTable *covered_services_table = g_hash_table_new(g_str_hash, g_str_equal);
	GArray *target_mapping_array = create_target_mapping_array(candidate_target_array);
	GArray *result = g_array_new(FALSE, FALSE, sizeof(DistributionItem*));
	unsigned int i;
    
	for(i = 0; i < candidate_target_array->len; i++)
	{
	    DistributionItem *item = g_array_index(candidate_target_array, DistributionItem*, i);
	
	    DistributionItem *result_item = (DistributionItem*)g_malloc(sizeof(DistributionItem));
	    result_item->service = item->service;
	    result_item->targets = g_array_new(FALSE, FALSE, sizeof(gchar*));
	
	    g_array_append_val(result, result_item);
	}
    
	while(g_hash_table_size(covered_services_table) < service_property_array->len)
	{
	    unsigned int i;
	    double min_cost = -1;
	    int min_cost_index = -1;
	    TargetMappingItem *min_cost_target_mapping;
	
	    for(i = 0; i < target_mapping_array->len; i++)
	    {
		TargetMappingItem *target_mapping = g_array_index(target_mapping_array, TargetMappingItem*, i);
		int target_index = infrastructure_index(infrastructure_property_array, target_mapping->target);
		Target *target = g_array_index(infrastructure_property_array, Target*, target_index);
		int infrastructure_prop_index = infrastructure_property_index(target, target_property);
		InfrastructureProperty *infrastructure_prop = g_array_index(target->property, InfrastructureProperty*, infrastructure_prop_index);
		int count = 0;
		double cost;
		unsigned int j;
	    
		for(j = 0; j < target_mapping->services->len; j++)
		{
		    gchar *service_name = g_array_index(target_mapping->services, gchar*, j);
		
		    if(g_hash_table_lookup(covered_services_table, service_name) == NULL)
			count++;
		}
	    
		cost = atoi(infrastructure_prop->value) / (double)count;
	    
		if(min_cost == -1 || cost < min_cost)
		{
		    min_cost = cost;
		    min_cost_index = i;
		}
	    }
	
	    min_cost_target_mapping = g_array_index(target_mapping_array, TargetMappingItem*, min_cost_index);
	
	    for(i = 0; i < min_cost_target_mapping->services->len; i++)
	    {
		gchar *service = g_array_index(min_cost_target_mapping->services, gchar*, i);
		int index = distribution_item_index(result, service);
		DistributionItem *item = g_array_index(result, DistributionItem*, index);
	    
		g_hash_table_insert(covered_services_table, service, service);

		g_array_append_val(item->targets, min_cost_target_mapping->target);
	    }
	}
    
	print_expr_of_candidate_target_array(result);
    
	/* Cleanup */
    
	for(i = 0; i < result->len; i++)
	{
	    DistributionItem *item = g_array_index(result, DistributionItem*, i);
	    g_array_free(item->targets, TRUE);
	    g_free(item);
	}
    
	g_array_free(result, TRUE);
    
	delete_target_mapping_array(target_mapping_array);
	g_hash_table_destroy(covered_services_table);
    }
    
    /* Cleanup */
    delete_candidate_target_array(candidate_target_array);
    delete_infrastructure_property_array(infrastructure_property_array);
    delete_service_property_array(service_property_array);
    
    return exit_status;
}
