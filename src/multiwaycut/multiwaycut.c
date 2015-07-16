#include "targetmapping.h"
#include "candidatetargetmapping.h"

static GPtrArray *filter_cuts(GPtrArray *target_mapping_array)
{
    unsigned int i;
    GPtrArray *filtered = g_ptr_array_new();
    
    for(i = 0; i < target_mapping_array->len; i++)
    {
	unsigned int j;
	TargetMappingItem *item = g_ptr_array_index(target_mapping_array, i);
	
	TargetMappingItem *cut_item = (TargetMappingItem*)g_malloc(sizeof(TargetMappingItem));
	cut_item->target = item->target;
	cut_item->services = g_ptr_array_new();
	
	for(j = 0; j < item->services->len; j++)
	{
	    gchar *service = g_ptr_array_index(item->services, j);
	    unsigned int k;
	    
	    for(k = i + 1; k < target_mapping_array->len; k++)
	    {
		TargetMappingItem *target_item = g_ptr_array_index(target_mapping_array, k);
		    
		if(find_service_name(target_item, service) != NULL)
		{
		    g_ptr_array_add(cut_item->services, service);
		    break;
		}
	    }
	}
	
	g_ptr_array_add(filtered, cut_item);
    }
    
    return filtered;
}

static void discard_heaviest_cut(GPtrArray *target_mapping_array)
{
    unsigned int i;
    int max_len = 0;
    int max_index = -1;
    
    /* For each cut determine the length */
    for(i = 0; i < target_mapping_array->len; i++)
    {
	TargetMappingItem *item = g_ptr_array_index(target_mapping_array, i);
	
	if(item->services->len > max_len)
	{
	    max_index = i;
	    max_len = item->services->len;
	}
    }
    
    /* Discard the heaviest cut */
    if(max_index >= 0)
    {
	TargetMappingItem *item = g_ptr_array_index(target_mapping_array, max_index);
	
	g_ptr_array_free(item->services, TRUE);
	g_free(item);
	g_ptr_array_remove_index(target_mapping_array, max_index);
    }
}

static GPtrArray *create_candidate_target_mapping_from(GPtrArray *target_mapping_array)
{
    unsigned int i;
    GPtrArray *result = g_ptr_array_new();
    
    for(i = 0; i < target_mapping_array->len; i++)
    {
	TargetMappingItem *target_mapping_item = g_ptr_array_index(target_mapping_array, i);
	unsigned int j;
	
	for(j = 0; j < target_mapping_item->services->len; j++)
	{
	    gchar *service = g_ptr_array_index(target_mapping_item->services, j);
	    DistributionItem *distribution_item = find_distribution_item(result, service);
	    
	    if(distribution_item == NULL)
	    {
		distribution_item = (DistributionItem*)g_malloc(sizeof(DistributionItem));
		distribution_item->service = service;
		distribution_item->targets = g_ptr_array_new();
		
		g_ptr_array_add(result, distribution_item);
	    }
	
	    g_ptr_array_add(distribution_item->targets, target_mapping_item->target);
	}
    }
    
    return result;
}

static void fix_unmapped_services(GPtrArray *initial_candidate_target_array, GPtrArray *candidate_target_array)
{
    unsigned int i;
    
    for(i = 0; i < initial_candidate_target_array->len; i++)
    {
	DistributionItem *item = g_ptr_array_index(initial_candidate_target_array, i);
	
	if(find_distribution_item(candidate_target_array, item->service) == NULL)
	{
	    DistributionItem *candidate_item = (DistributionItem*)g_malloc(sizeof(DistributionItem));
	    candidate_item->service = item->service;
	    candidate_item->targets = g_ptr_array_new();
	    
	    if(item->targets->len > 0)
		g_ptr_array_add(candidate_item->targets, g_ptr_array_index(item->targets, 0));
	
	    g_ptr_array_add(candidate_target_array, candidate_item);
	    g_ptr_array_sort(candidate_target_array, (GCompareFunc)compare_target_mapping_item);
	}
    }
}

int multiwaycut(gchar *distribution_xml)
{
    GPtrArray *candidate_target_array = create_candidate_target_array(distribution_xml);
    
    if(candidate_target_array == NULL)
    {
	g_printerr("Error opening candidate target host mapping!\n");
	return 1;
    }
    else
    {
	GPtrArray *target_mapping_array = create_target_mapping_array(candidate_target_array);
	GPtrArray *filtered;
	GPtrArray *distmapping;
	unsigned int i;
    
	filtered = filter_cuts(target_mapping_array);
    
	discard_heaviest_cut(filtered);
    
	distmapping = create_candidate_target_mapping_from(filtered);
    
	fix_unmapped_services(candidate_target_array, distmapping);
    
	print_expr_of_candidate_target_array(distmapping);
    
	/* Cleanup */
    
	for(i = 0; i < distmapping->len; i++)
	{
	    DistributionItem *item = g_ptr_array_index(distmapping, i);
	    g_ptr_array_free(item->targets, TRUE);
	    g_free(item);
	}
    
	g_ptr_array_free(distmapping, TRUE);
    
	delete_target_mapping_array(filtered);
	delete_target_mapping_array(target_mapping_array);
	delete_candidate_target_array(candidate_target_array);
	
	return 0;
    }
}
