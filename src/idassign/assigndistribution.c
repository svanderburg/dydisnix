#include "assigndistribution.h"
#include <nixxml-ghashtable-iter.h>
#include "idassignmentsperresourcetable.h"
#include "idassignmentstable.h"

static GPtrArray *derive_names_of_services_that_are_distributed(GHashTable *distribution_table)
{
    GPtrArray *service_names = g_ptr_array_new();

    NixXML_GHashTableOrderedIter iter;
    gchar *service_name;
    gpointer value;

    NixXML_g_hash_table_ordered_iter_init(&iter, distribution_table);

    while(NixXML_g_hash_table_ordered_iter_next(&iter, &service_name, &value))
    {
        GPtrArray *targets = (GPtrArray*)value;
        if(targets->len > 0)
            g_ptr_array_add(service_names, service_name);
    }

    NixXML_g_hash_table_ordered_iter_destroy(&iter);

    return service_names;
}

static NixXML_bool assign_global_ids_to_distributed_services(GHashTable *id_assignments_table, gchar *resource_name, IdResourceType *type, GHashTable *services_table, GHashTable *distribution_table, gchar *service_property)
{
    GPtrArray *service_names = derive_names_of_services_that_are_distributed(distribution_table);
    NixXML_bool result = create_id_assignments_for_services(id_assignments_table, service_names, resource_name, type, services_table, service_property);
    g_ptr_array_free(service_names, TRUE);
    return result;
}

static NixXML_bool assign_machine_level_ids_to_distributed_services(GHashTable *id_assignments_table, gchar *resource_name, IdResourceType *type, GHashTable *services_table, gchar *service_property, GHashTable *target_to_service_table)
{
    NixXML_bool result = TRUE;

    NixXML_GHashTableOrderedIter iter;
    gchar *target_name;
    gpointer value;

    NixXML_g_hash_table_ordered_iter_init(&iter, target_to_service_table);

    while(NixXML_g_hash_table_ordered_iter_next(&iter, &target_name, &value))
    {
        GPtrArray *service_names = (GPtrArray*)value;
        if(!create_id_assignments_for_services(id_assignments_table, service_names, resource_name, type, services_table, service_property))
        {
            result = FALSE;
            break;
        }
    }

    NixXML_g_hash_table_ordered_iter_destroy(&iter);

    return result;
}

NixXML_bool assign_ids_to_distributed_services(GHashTable *id_resources_table, GHashTable *id_assignments_per_resource_table, GHashTable *services_table, GHashTable *distribution_table, GHashTable *target_to_service_table, gchar *service_property)
{
    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, id_resources_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        gchar *resource_name = (gchar*)key;
        IdResourceType *type = (IdResourceType*)value;

        // Create ID assignments table
        GHashTable *id_assignments_table = retrieve_or_add_empty_id_assignments_table_for_resource(id_assignments_per_resource_table, resource_name);

        // Assign IDs

        if(xmlStrcmp(type->scope, (xmlChar*) "global") == 0)
        {
            if(!assign_global_ids_to_distributed_services(id_assignments_table, resource_name, type, services_table, distribution_table, service_property))
                return FALSE;
        }
        else if(xmlStrcmp(type->scope, (xmlChar*) "machine") == 0)
        {
            if(!assign_machine_level_ids_to_distributed_services(id_assignments_table, resource_name, type, services_table, service_property, target_to_service_table))
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
