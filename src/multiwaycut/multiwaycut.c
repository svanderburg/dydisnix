#include "targetmapping.h"
#include "candidatetargetmapping.h"

static GArray *filter_cuts(GArray *target_mapping_array)
{
    unsigned int i;
    GArray *filtered = g_array_new(FALSE, FALSE, sizeof(TargetMappingItem*));
    
    for(i = 0; i < target_mapping_array->len; i++)
    {
	unsigned int j;
	TargetMappingItem *item = g_array_index(target_mapping_array, TargetMappingItem*, i);
	
	TargetMappingItem *cut_item = (TargetMappingItem*)g_malloc(sizeof(TargetMappingItem));
	cut_item->target = item->target;
	cut_item->services = g_array_new(FALSE, FALSE, sizeof(TargetMappingItem*));
	
	for(j = 0; j < item->services->len; j++)
	{
	    gchar *service = g_array_index(item->services, gchar*, j);
	    unsigned int k;
	    
	    for(k = i + 1; k < target_mapping_array->len; k++)
	    {
		TargetMappingItem *target_item = g_array_index(target_mapping_array, TargetMappingItem*, k);
		    
		if(service_name_index(target_item, service) != -1)
		{
		    g_array_append_val(cut_item->services, service);
		    break;		    
		}
	    }
	}
	
	g_array_append_val(filtered, cut_item);
    }
    
    return filtered;
}

static void discard_heaviest_cut(GArray *target_mapping_array)
{
    unsigned int i;
    int max_len = 0;
    int max_index = -1;
    
    /* For each cut determine the length */
    for(i = 0; i < target_mapping_array->len; i++)
    {
	TargetMappingItem *item = g_array_index(target_mapping_array, TargetMappingItem*, i);
	
	if(item->services->len > max_len)
	{
	    max_index = i;
	    max_len = item->services->len;
	}
    }
    
    /* Discard the heaviest cut */
    if(max_index >= 0)
    {
	TargetMappingItem *item = g_array_index(target_mapping_array, TargetMappingItem*, max_index);
	
	g_array_free(item->services, TRUE);
	g_free(item);
	g_array_remove_index(target_mapping_array, max_index);
    }
}

static GArray *create_candidate_target_mapping_from(GArray *target_mapping_array)
{
    unsigned int i;
    GArray *result = g_array_new(FALSE, FALSE, sizeof(DistributionItem*));
    
    for(i = 0; i < target_mapping_array->len; i++)
    {
	TargetMappingItem *target_mapping_item = g_array_index(target_mapping_array, TargetMappingItem*, i);
	unsigned int j;
	
	for(j = 0; j < target_mapping_item->services->len; j++)
	{
	    gchar *service = g_array_index(target_mapping_item->services, gchar*, j);
	    int index = distribution_item_index(result, service);
	    DistributionItem *distribution_item;
	    
	    if(index == -1)
	    {
		distribution_item = (DistributionItem*)g_malloc(sizeof(DistributionItem));
		distribution_item->service = service;
		distribution_item->targets = g_array_new(FALSE, FALSE, sizeof(gchar*));
		
		g_array_append_val(result, distribution_item);
	    }
	    else
		distribution_item = g_array_index(target_mapping_array, DistributionItem*, index);
	
	    g_array_append_val(distribution_item->targets, target_mapping_item->target);
	}
    }
    
    return result;
}

static void fix_unmapped_services(GArray *initial_candidate_target_array, GArray *candidate_target_array)
{
    unsigned int i;
    
    for(i = 0; i < initial_candidate_target_array->len; i++)
    {
	DistributionItem *item = g_array_index(initial_candidate_target_array, DistributionItem*, i);
	
	if(distribution_item_index(candidate_target_array, item->service) == -1)
	{
	    DistributionItem *candidate_item = (DistributionItem*)g_malloc(sizeof(DistributionItem));
	    candidate_item->service = item->service;
	    candidate_item->targets = g_array_new(FALSE, FALSE, sizeof(gchar*));
	    
	    if(item->targets->len > 0)
		g_array_append_val(candidate_item->targets, g_array_index(item->targets, gchar*, 0));
	
	    g_array_append_val(candidate_target_array, candidate_item);
	    g_array_sort(candidate_target_array, (GCompareFunc)compare_target_mapping_item);
	}
    }
}

void multiwaycut(gchar *distribution_xml)
{
    GArray *candidate_target_array = create_candidate_target_array(distribution_xml);
    GArray *target_mapping_array = create_target_mapping_array(candidate_target_array);
    GArray *filtered; 
    GArray *distmapping;
    unsigned int i;
    
    filtered = filter_cuts(target_mapping_array);
    
    discard_heaviest_cut(filtered);
    
    distmapping = create_candidate_target_mapping_from(filtered);
    
    fix_unmapped_services(candidate_target_array, distmapping);
    
    print_expr_of_candidate_target_array(distmapping);
    
    /* Cleanup */
    
    for(i = 0; i < distmapping->len; i++)
    {
	DistributionItem *item = g_array_index(distmapping, DistributionItem*, i);
	g_array_free(item->targets, TRUE);
	g_free(item);
    }
    
    g_array_free(distmapping, TRUE);
    
    delete_target_mapping_array(filtered);
    delete_target_mapping_array(target_mapping_array);
    delete_candidate_target_array(candidate_target_array);
}
