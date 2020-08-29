#include "targetmappingtable.h"
#include <stdlib.h>
#include <distributiontable.h>

GHashTable *create_target_mapping_table(GHashTable *distribution_table)
{
    GHashTable *result_table = g_hash_table_new(g_str_hash, g_str_equal);

    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, distribution_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        gchar *service = (gchar*)key;
        GPtrArray *targets = (GPtrArray*)value;
        unsigned int i;

        for(i = 0; i < targets->len; i++)
        {
            DistributionMapping *distribution_mapping = g_ptr_array_index(targets, i);
            GPtrArray *services = g_hash_table_lookup(result_table, distribution_mapping->target);

            if(services == NULL)
            {
                services = g_ptr_array_new();
                g_hash_table_insert(result_table, distribution_mapping->target, services);
            }

            g_ptr_array_add(services, service);
        }
    }

    return result_table;
}

void delete_target_mapping_table(GHashTable *target_mapping_table)
{
    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, target_mapping_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        GPtrArray *services = (GPtrArray*)value;
        g_ptr_array_free(services, TRUE);
    }

    g_hash_table_destroy(target_mapping_table);
}
