#include "idtoservicetable.h"
#include <distributiontable.h>

NixXML_bool derive_next_id(IdResourceType *type, Boundaries *boundaries, GHashTable *id_to_service_table, int *next_id)
{
    int max_num_of_ids = type->max - type->min + 1;

    if(g_hash_table_size(id_to_service_table) >= max_num_of_ids) // If the size of the assignments table equals the max number of ids, we have depleted our resource pool
    {
        g_printerr("The amount of IDs is depleted!\n");
        return FALSE;
    }
    else if(boundaries->lowest_id > type->min)
    {
        boundaries->lowest_id--;
        *next_id = boundaries->lowest_id;
        return TRUE;
    }
    else if(boundaries->highest_id < type->max)
    {
        boundaries->highest_id++;
        *next_id = boundaries->highest_id;
        return TRUE;
    }
    else
    {
        int i;

        for(i = type->min; i <= type->max; i++)
        {
            if(!g_hash_table_contains(id_to_service_table, &i))
            {
                *next_id = i;
                return TRUE;
            }
        }

        return FALSE;
    }
}

GHashTable *derive_id_to_service_table(GHashTable *id_assignments_table)
{
    GHashTable *id_to_service_table = g_hash_table_new(g_int_hash, g_int_equal);

    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, id_assignments_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        gchar *service_name = (gchar*)key;
        int *id = (int*)value;
        g_hash_table_insert(id_to_service_table, id, service_name);
    }

    return id_to_service_table;
}
