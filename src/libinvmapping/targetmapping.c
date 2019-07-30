#include "targetmapping.h"
#include <stdlib.h>
#include <candidatetargetmappingtable.h>

GPtrArray *create_target_mapping_array(GHashTable *candidate_target_table)
{
    GPtrArray *target_mapping_array = g_ptr_array_new();

    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, candidate_target_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        gchar *service = (gchar*)key;
        GPtrArray *targets = (GPtrArray*)value;
        unsigned int i;

        for(i = 0; i < targets->len; i++)
        {
            CandidateTargetMapping *target_mapping = g_ptr_array_index(targets, i);
            TargetMappingItem *mapping = find_target_mapping_item(target_mapping_array, (gchar*)target_mapping->target);

            if(mapping == NULL)
            {
                mapping = (TargetMappingItem*)g_malloc(sizeof(TargetMappingItem));
                mapping->target = (gchar*)target_mapping->target;
                mapping->services = g_ptr_array_new();

                g_ptr_array_add(target_mapping_array, mapping);
                g_ptr_array_sort(target_mapping_array, (GCompareFunc)compare_target_mapping_item);
            }

            g_ptr_array_add(mapping->services, service);
        }
    }
    
    return target_mapping_array;
}

void delete_target_mapping_array(GPtrArray *target_mapping_array)
{
    if(target_mapping_array != NULL)
    {
        unsigned int i;
        
        for(i = 0; i < target_mapping_array->len; i++)
        {
            TargetMappingItem *item = g_ptr_array_index(target_mapping_array, i);
            g_ptr_array_free(item->services, TRUE);
            g_free(item);
        }
        
        g_ptr_array_free(target_mapping_array, TRUE);
    }
}

void print_target_mapping_array(GPtrArray *target_mapping_array)
{
    unsigned int i;
    
    for(i = 0; i < target_mapping_array->len; i++)
    {
	unsigned int j;
	TargetMappingItem *item = g_ptr_array_index(target_mapping_array, i);
	
	g_print("target: %s\n", item->target);
	
	for(j = 0; j < item->services->len; j++)
	{
	    gchar *service = g_ptr_array_index(item->services, j);
	    g_print("  service: %s\n", service);
	}
    }
}

static gint compare_services(const gchar **l, const gchar **r)
{
    return g_strcmp0(*l, *r);
}

gchar *find_service_name(TargetMappingItem *item, gchar *service)
{
    gchar **ret = bsearch(&service, item->services->pdata, item->services->len, sizeof(gpointer), (int (*)(const void*, const void*)) compare_services);

    if(ret == NULL)
        return NULL;
    else
        return *ret;
}

gint compare_target_mapping_item(const TargetMappingItem **l, const TargetMappingItem **r)
{
    const TargetMappingItem *left = *l;
    const TargetMappingItem *right = *r;
    
    return g_strcmp0(left->target, right->target);
}

TargetMappingItem *find_target_mapping_item(GPtrArray *target_mapping_array, gchar *target)
{
    TargetMappingItem item;
    TargetMappingItem **ret, *itemPtr = &item;
    
    itemPtr->target = target;
    
    ret = bsearch(&itemPtr, target_mapping_array->pdata, target_mapping_array->len, sizeof(gpointer), (int (*)(const void*, const void*)) compare_target_mapping_item);

    if(ret == NULL)
        return NULL;
    else
        return *ret;
}
