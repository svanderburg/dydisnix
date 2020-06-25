#include "multiwaycut.h"
#include "targetmapping.h"
#include "candidatetargetmappingtable.h"

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

static GHashTable *create_candidate_target_mapping_from(GPtrArray *target_mapping_array)
{
    unsigned int i;
    GHashTable *result_table = g_hash_table_new(g_str_hash, g_str_equal);

    for(i = 0; i < target_mapping_array->len; i++)
    {
        TargetMappingItem *target_mapping_item = g_ptr_array_index(target_mapping_array, i);
        unsigned int j;

        for(j = 0; j < target_mapping_item->services->len; j++)
        {
            gchar *service = g_ptr_array_index(target_mapping_item->services, j);
            GPtrArray *targets = g_hash_table_lookup(result_table, service);

            if(targets == NULL)
            {
                targets = g_ptr_array_new();
                g_hash_table_insert(result_table, service, targets);
            }

            g_ptr_array_add(targets, target_mapping_item->target);
        }
    }

    return result_table;
}

static void fix_unmapped_services(GHashTable *initial_candidate_target_table, GHashTable *candidate_target_table)
{
    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, initial_candidate_target_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        gchar *service = (gchar*)key;
        GPtrArray *targets = (GPtrArray*)value;

        if(g_hash_table_lookup(candidate_target_table, service) == NULL)
        {
            GPtrArray *candidate_targets = g_ptr_array_new();

            if(targets->len > 0)
                g_ptr_array_add(candidate_targets, g_ptr_array_index(targets, 0));

            g_hash_table_insert(candidate_target_table, service, candidate_targets);
        }
    }
}

int multiwaycut(gchar *distribution, gchar *infrastructure, const unsigned int flags)
{
    NixXML_bool automapped;
    GHashTable *candidate_target_table = create_candidate_target_table(distribution, NULL, flags & DYDISNIX_FLAG_XML, &automapped);

    if(candidate_target_table == NULL)
    {
        g_printerr("Error opening candidate target host mapping!\n");
        return 1;
    }
    else
    {
        GPtrArray *target_mapping_array = create_target_mapping_array(candidate_target_table);
        GPtrArray *filtered;
        GHashTable *distmapping;
        GHashTableIter iter;
        gpointer key, value;

        filtered = filter_cuts(target_mapping_array);

        discard_heaviest_cut(filtered);

        distmapping = create_candidate_target_mapping_from(filtered);

        fix_unmapped_services(candidate_target_table, distmapping);

        if(flags & DYDISNIX_FLAG_OUTPUT_XML)
            print_candidate_target_table_xml(stdout, distmapping, 0, NULL, NULL);
        else
            print_candidate_target_table_nix(stdout, distmapping, 0, &automapped);

        /* Cleanup */

        g_hash_table_iter_init(&iter, distmapping);
        while(g_hash_table_iter_next(&iter, &key, &value))
        {
            GPtrArray *targets = (GPtrArray*)value;
            g_ptr_array_free(targets, TRUE);
        }

        g_hash_table_destroy(distmapping);

        delete_target_mapping_array(filtered);
        delete_target_mapping_array(target_mapping_array);
        delete_candidate_target_table(candidate_target_table);

        return 0;
    }
}
