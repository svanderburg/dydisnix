#include "boundaries.h"

Boundaries *compute_boundaries(IdResourceType *type, GHashTable *id_assignments_table, GPtrArray *service_names)
{
    Boundaries *boundaries = (Boundaries*)g_malloc(sizeof(Boundaries));

    NixXML_bool first = TRUE;
    unsigned int i;

    for(i = 0; i < service_names->len; i++)
    {
        gchar *service_name = g_ptr_array_index(service_names, i);
        int *id = g_hash_table_lookup(id_assignments_table, service_name);

        if(id != NULL)
        {
            if(first)
            {
                boundaries->lowest_id = *id;
                boundaries->highest_id = *id;
                first = FALSE;
            }
            else
            {
                if(*id < boundaries->lowest_id)
                    boundaries->lowest_id = *id;

                if(*id > boundaries->highest_id)
                    boundaries->highest_id = *id;
            }
        }
    }

    if(first)
    {
        boundaries->lowest_id = type->min;
        boundaries->highest_id = type->min - 1;
    }

    return boundaries;
}
