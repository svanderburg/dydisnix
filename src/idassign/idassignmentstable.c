#include "idassignmentstable.h"
#include <stdlib.h>
#include <nixxml-ghashtable.h>
#include <nixxml-print-nix.h>
#include <nixxml-print-xml.h>
#include <nixxml-node.h>
#include <service.h>
#include "idresourcetype.h"
#include "boundaries.h"
#include "idtoservicetable.h"

GHashTable *create_empty_id_assignments_table(void)
{
    return NixXML_create_g_hash_table();
}

void *parse_id_assignments_table(xmlNodePtr element, void *userdata)
{
    return NixXML_parse_g_hash_table_verbose(element, "assignment", "name", userdata, NixXML_parse_int);
}

void delete_id_assignments_table(GHashTable *id_assignments_table)
{
    NixXML_delete_g_hash_table(id_assignments_table, (NixXML_DeleteGHashTableValueFunc)free);
}

void print_id_assignments_table_nix(FILE *file, GHashTable *id_assignments_table, const int indent_level, void *userdata)
{
    NixXML_print_g_hash_table_ordered_nix(file, id_assignments_table, indent_level, userdata, (NixXML_PrintValueFunc)NixXML_print_int_nix);
}

void print_id_assignments_table_xml(FILE *file, GHashTable *id_assignments_table, const int indent_level, const char *type_property_name, void *userdata)
{
    NixXML_print_g_hash_table_verbose_ordered_xml(file, id_assignments_table, "assignment", "name", indent_level, NULL, userdata, (NixXML_PrintXMLValueFunc)NixXML_print_int_xml);
}

static void remove_id_assignments(GHashTable *id_assignments_table, GPtrArray *remove_keys)
{
    unsigned int i;

    for(i = 0; i < remove_keys->len; i++)
    {
        gchar *service_name = g_ptr_array_index(remove_keys, i);
        int *id = g_hash_table_lookup(id_assignments_table, service_name);
        g_hash_table_remove(id_assignments_table, service_name);
        free(id);
    }
}

static NixXML_bool service_requires_unique_resource_id(Service *service, gchar *resource_name, gchar *service_property)
{
    NixXML_Node *requires_unique_ids_for_node = g_hash_table_lookup(service->properties, service_property);

    if(requires_unique_ids_for_node == NULL)
        return FALSE;
    else if(requires_unique_ids_for_node->type == NIX_XML_TYPE_LIST)
    {
        unsigned int i;
        GPtrArray *require_unique_ids_for = (GPtrArray*)requires_unique_ids_for_node->value;

        for(i = 0; i < require_unique_ids_for->len; i++)
        {
            NixXML_Node *value_node = (NixXML_Node*)g_ptr_array_index(require_unique_ids_for, i);
            if(g_strcmp0(value_node->value, resource_name) == 0)
                return TRUE;
        }

        return FALSE;
    }
    else
        return FALSE;
}

static NixXML_bool check_service_assignment_is_valid(GHashTable *services_table, gchar *service_name, gchar *resource_name, gchar *service_property, IdResourceType *type, int id)
{
    Service *service = g_hash_table_lookup(services_table, service_name);
    return (service != NULL && type != NULL && service_requires_unique_resource_id(service, resource_name, service_property) && !id_is_outside_range(type, id));
}

static GPtrArray *determine_invalid_service_assignments(GHashTable *id_assignments_table, gchar *resource_name, GHashTable *services_table, IdResourceType *type, gchar *service_property)
{
    GPtrArray *remove_keys = g_ptr_array_new();

    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, id_assignments_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        gchar *service_name = (gchar*)key;
        int *id = (int*)value;

        if(!check_service_assignment_is_valid(services_table, service_name, resource_name, service_property, type, *id))
            g_ptr_array_add(remove_keys, service_name);
    }

    return remove_keys;
}

void remove_invalid_service_id_assignments(GHashTable *id_assignments_table, gchar *resource_name, GHashTable *services_table, IdResourceType *type, gchar *service_property)
{
    GPtrArray *invalid_keys = determine_invalid_service_assignments(id_assignments_table, resource_name, services_table, type, service_property);
    remove_id_assignments(id_assignments_table, invalid_keys);
    g_ptr_array_free(invalid_keys, TRUE);
}

static NixXML_bool check_distribution_assignment_is_valid(GHashTable *services_table, GHashTable *distribution_table, gchar *service_name, gchar *resource_name, gchar *service_property, IdResourceType *type, int id)
{
    GPtrArray *targets = g_hash_table_lookup(distribution_table, service_name);

    return (targets != NULL && targets->len == 1 && check_service_assignment_is_valid(services_table, service_name, resource_name, service_property, type, id));
}

static GPtrArray *determine_invalid_distribution_assignments(GHashTable *id_assignments_table, gchar *resource_name, GHashTable *services_table, GHashTable *distribution_table, IdResourceType *type, gchar *service_property)
{
    GPtrArray *remove_keys = g_ptr_array_new();

    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, id_assignments_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        gchar *service_name = (gchar*)key;
        int *id = (int*)value;

        if(!check_distribution_assignment_is_valid(services_table, distribution_table, service_name, resource_name, service_property, type, *id))
            g_ptr_array_add(remove_keys, service_name);
    }

    return remove_keys;
}

void remove_invalid_distribution_id_assignments(GHashTable *id_assignments_table, gchar *resource_name, GHashTable *services_table, GHashTable *distribution_table, IdResourceType *type, gchar *service_property)
{
    GPtrArray *invalid_keys = determine_invalid_distribution_assignments(id_assignments_table, resource_name, services_table, distribution_table, type, service_property);
    remove_id_assignments(id_assignments_table, invalid_keys);
    g_ptr_array_free(invalid_keys, TRUE);
}

static void assign_id(gchar *service_name, int id, GHashTable *id_assignments_table, GHashTable *id_to_services_table)
{
    int *id_ptr = (int*)malloc(sizeof(int));
    *id_ptr = id;
    g_hash_table_insert(id_assignments_table, g_strdup(service_name), id_ptr);
    g_hash_table_insert(id_to_services_table, id_ptr, service_name);
}

NixXML_bool create_id_assignments_for_services(GHashTable *id_assignments_table, GPtrArray *service_names, gchar *resource_name, IdResourceType *type, GHashTable *services_table, gchar *service_property)
{
    NixXML_bool result = TRUE;

    GHashTable *id_to_services_table = derive_id_to_service_table(id_assignments_table);
    Boundaries *boundaries = compute_boundaries(type, id_assignments_table, service_names);

    unsigned int i;

    for(i = 0; i < service_names->len; i++)
    {
        gchar *service_name = g_ptr_array_index(service_names, i);
        Service *service = g_hash_table_lookup(services_table, service_name);

        if(service != NULL && service_requires_unique_resource_id(service, resource_name, service_property) && !g_hash_table_contains(id_assignments_table, service_name))
        {
            int id;

            if(!derive_next_id(type, boundaries, id_to_services_table, &id))
            {
                result = FALSE;
                break;
            }

            assign_id(service_name, id, id_assignments_table, id_to_services_table);
        }
    }

    g_free(boundaries);
    g_hash_table_destroy(id_to_services_table);

    return result;
}
