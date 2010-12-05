#include "targetmapping.h"
#include <candidatetargetmapping.h>

gint compare_target_mapping_item(const TargetMappingItem **l, const TargetMappingItem **r)
{
    const TargetMappingItem *left = *l;
    const TargetMappingItem *right = *r;
    
    return g_strcmp0(left->target, right->target);
}

gint target_mapping_index(GArray *target_mapping_array, gchar *target)
{
    gint left = 0;
    gint right = target_mapping_array->len - 1;
    
    TargetMappingItem compare_mapping;
    TargetMappingItem *compare_mapping_ptr = &compare_mapping;
    
    compare_mapping.target = target;
    
    while(left <= right)
    {
	gint mid = (left + right) / 2;
	TargetMappingItem *mid_target_mapping_item = g_array_index(target_mapping_array, TargetMappingItem*, mid);
        gint status = compare_target_mapping_item((const TargetMappingItem**) &mid_target_mapping_item, (const TargetMappingItem**) &compare_mapping_ptr);
	
	if(status == 0)
            return mid; /* Return index of the found target mapping */
	else if(status > 0)
	    right = mid - 1;
	else if(status < 0)
	    left = mid + 1;
    }
    
    return -1; /* Target mapping not found */
}

GArray *create_target_mapping_array(GArray *candidate_target_array)
{
    GArray *target_mapping_array = g_array_new(FALSE, FALSE, sizeof(TargetMappingItem*));
    unsigned int i;
    
    for(i = 0; i < candidate_target_array->len; i++)
    {
	DistributionItem *distribution_item = g_array_index(candidate_target_array, DistributionItem*, i);
	GArray *targets = distribution_item->targets;
	unsigned int j;
	
	for(j = 0; j < targets->len; j++)
	{
	    gchar *target = g_array_index(targets, gchar*, j);
	    int target_index = target_mapping_index(target_mapping_array, target);
	    TargetMappingItem *mapping;
	    
	    if(target_index == -1)
	    {
		mapping = (TargetMappingItem*)g_malloc(sizeof(TargetMappingItem));
		mapping->target = target;
		mapping->services = g_array_new(FALSE, FALSE, sizeof(gchar*));
		
		g_array_append_val(target_mapping_array, mapping);
		g_array_sort(target_mapping_array, (GCompareFunc)compare_target_mapping_item);
	    }
	    else
		mapping = g_array_index(target_mapping_array, TargetMappingItem*, target_index);
	
	    g_array_append_val(mapping->services, distribution_item->service);
	}
    }
    
    return target_mapping_array;
}

void delete_target_mapping_array(GArray *target_mapping_array)
{
    unsigned int i;
    
    for(i = 0; i < target_mapping_array->len; i++)
    {
	TargetMappingItem *item = g_array_index(target_mapping_array, TargetMappingItem*, i);
	g_array_free(item->services, TRUE);
	g_free(item);
    }
    
    g_array_free(target_mapping_array, TRUE);
}

void print_target_mapping_array(GArray *target_mapping_array)
{
    unsigned int i;
    
    for(i = 0; i < target_mapping_array->len; i++)
    {
	unsigned int j;
	TargetMappingItem *item = g_array_index(target_mapping_array, TargetMappingItem*, i);
	
	g_print("target: %s\n", item->target);
	
	for(j = 0; j < item->services->len; j++)
	{
	    gchar *service = g_array_index(item->services, gchar*, j);
	    g_print("  service: %s\n", service);
	}
    }
}

int service_name_index(TargetMappingItem *item, gchar *service)
{
    gint left = 0;
    gint right = item->services->len - 1;
    
    while(left <= right)
    {
	gint mid = (left + right) / 2;
	gchar *mid_service = g_array_index(item->services, gchar*, mid);
        gint status = g_strcmp0(mid_service, service);
	
	if(status == 0)
            return mid; /* Return index of the found service */
	else if(status > 0)
	    right = mid - 1;
	else if(status < 0)
	    left = mid + 1;
    }
    
    return -1; /* service not found */
}
