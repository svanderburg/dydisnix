#include "assignservices.h"
#include <nixxml-ghashtable-iter.h>
#include <service.h>
#include "idassignmentsperresourcetable.h"
#include "idassignmentstable.h"

static GPtrArray *derive_service_names(GHashTable *services_table)
{
    GPtrArray *service_names = g_ptr_array_new();

    NixXML_GHashTableOrderedIter iter;
    gchar *service_name;
    gpointer value;

    NixXML_g_hash_table_ordered_iter_init(&iter, services_table);

    while(NixXML_g_hash_table_ordered_iter_next(&iter, &service_name, &value))
    {
        Service *service = (Service*)value;
        g_ptr_array_add(service_names, service->name);
    }

    NixXML_g_hash_table_ordered_iter_destroy(&iter);

    return service_names;
}

static NixXML_bool assign_global_ids_to_services(GHashTable *id_assignments_table, GHashTable *last_assignments_table, gchar *resource_name, IdResourceType *type, GHashTable *services_table, gchar *service_property)
{
    GPtrArray *service_names = derive_service_names(services_table);
    NixXML_bool result = create_id_assignments_for_services(id_assignments_table, last_assignments_table, service_names, resource_name, type, services_table, service_property);
    g_ptr_array_free(service_names, TRUE);
    return result;
}

NixXML_bool assign_ids_to_services(GHashTable *id_resources_table, IdsConfig *ids_config, GHashTable *services_table, gchar *service_property)
{
    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, id_resources_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        gchar *resource_name = (gchar*)key;
        IdResourceType *type = (IdResourceType*)value;

        // Create ID assignments table
        GHashTable *id_assignments_table = retrieve_or_add_empty_id_assignments_table_for_resource(ids_config->id_assignments_per_resource_table, resource_name);

        // Assign IDs
        if(xmlStrcmp(type->scope, (xmlChar*) "global") == 0)
        {
            if(!assign_global_ids_to_services(id_assignments_table, ids_config->last_assignments_table, resource_name, type, services_table, service_property))
                return FALSE;
        }
        else
        {
            g_printerr("Unknown scope: %s\n", type->scope);
            return FALSE;
        }
    }

    return TRUE;
}
