#include "idtoservicetable.h"
#include <distributiontable.h>

static int increase_search_id(int search_id, const IdResourceType *type)
{
    search_id++;

    if(search_id > type->max)
        search_id = type->min;

    return search_id;
}

NixXML_bool derive_next_id(const IdResourceType *type, GHashTable *id_to_service_table, int *last_id, int *next_id)
{
    int max_num_of_ids = type->max - type->min + 1;

    if(g_hash_table_size(id_to_service_table) >= max_num_of_ids) // If the size of the assignments table equals the max number of ids, we have depleted our resource pool
    {
        g_printerr("The amount of IDs is depleted!\n");
        return FALSE;
    }
    else
    {
        int search_id;

        if(last_id == NULL)
            search_id = type->min;
        else
            search_id = increase_search_id(*last_id, type);

        while(TRUE)
        {
            if(g_hash_table_contains(id_to_service_table, &search_id))
                search_id = increase_search_id(search_id, type);
            else
            {
                *next_id = search_id;
                break;
            }
        }

        return TRUE;
    }
}

GHashTable *derive_id_to_service_table(GHashTable *id_assignments_table, GPtrArray *service_names)
{
    unsigned int i;
    GHashTable *id_to_service_table = g_hash_table_new(g_int_hash, g_int_equal);

    for(i = 0; i < service_names->len; i++)
    {
        gchar *service_name = (gchar*)g_ptr_array_index(service_names, i);
        int *id = (int*)g_hash_table_lookup(id_assignments_table, service_name);

        if(id != NULL)
            g_hash_table_insert(id_to_service_table, id, service_name);
    }

    return id_to_service_table;
}
