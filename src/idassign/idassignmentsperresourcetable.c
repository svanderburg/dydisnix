#include "idassignmentsperresourcetable.h"
#include <nixxml-ghashtable.h>
#include <nixxml-print-nix.h>
#include <nixxml-print-xml.h>

GHashTable *create_empty_id_assignments_per_resource_table(void)
{
    return NixXML_create_g_hash_table();
}

GHashTable *retrieve_or_add_empty_id_assignments_table_for_resource(GHashTable *id_assignments_per_resource_table, const gchar *resource_name)
{
    GHashTable *id_assignments_table = (GHashTable*)g_hash_table_lookup(id_assignments_per_resource_table, resource_name);

    if(id_assignments_table == NULL)
        id_assignments_table = create_empty_id_assignments_table();

    g_hash_table_insert(id_assignments_per_resource_table, g_strdup(resource_name), id_assignments_table);

    return id_assignments_table;
}

void *parse_id_assignments_per_resource_table(xmlNodePtr element, void *userdata)
{
    return NixXML_parse_g_hash_table_verbose(element, "resource", "name", userdata, parse_id_assignments_table);
}

void delete_id_assignments_per_resource_table(GHashTable *id_assignments_per_resource_table)
{
    NixXML_delete_g_hash_table(id_assignments_per_resource_table, (NixXML_DeleteGHashTableValueFunc)delete_id_assignments_table);
}

void print_id_assignments_per_resource_table_nix(FILE *file, GHashTable *id_assignments_per_resource_table, const int indent_level, void *userdata)
{
    NixXML_print_g_hash_table_ordered_nix(file, id_assignments_per_resource_table, indent_level, userdata, (NixXML_PrintValueFunc)print_id_assignments_table_nix);
}

void print_id_assignments_per_resource_table_xml(FILE *file, GHashTable *id_assignments_per_resource_table, const int indent_level, const char *type_property_name, void *userdata)
{
    NixXML_print_g_hash_table_verbose_ordered_xml(file, id_assignments_per_resource_table, "resource", "name", indent_level, NULL, userdata, (NixXML_PrintXMLValueFunc)print_id_assignments_table_xml);
}

static GPtrArray *remove_invalid_service_id_assignment_entries_per_resource(GHashTable *id_assignments_per_resource_table, GHashTable *services_table, GHashTable *id_resources_table, gchar *service_property)
{
    GPtrArray *remove_keys = g_ptr_array_new();

    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, id_assignments_per_resource_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        gchar *resource_name = (gchar*)key;
        GHashTable *id_assignments_table = (GHashTable*)value;
        IdResourceType *type = g_hash_table_lookup(id_resources_table, resource_name);

        remove_invalid_service_id_assignments(id_assignments_table, resource_name, services_table, type, service_property);

        if(g_hash_table_size(id_assignments_table) == 0)
            g_ptr_array_add(remove_keys, resource_name);
    }

    return remove_keys;
}

static void remove_empty_id_assignment_tables(GHashTable *id_assignments_per_resource_table, GPtrArray *remove_keys)
{
    unsigned int i;

    for(i = 0; i < remove_keys->len; i++)
    {
        gchar *resource_name = g_ptr_array_index(remove_keys, i);
        GHashTable *id_assignments_table = g_hash_table_lookup(id_assignments_per_resource_table, resource_name);
        g_hash_table_remove(id_assignments_per_resource_table, resource_name);
        delete_id_assignments_table(id_assignments_table);
    }
}

void remove_invalid_service_id_assignments_per_resource(GHashTable *id_assignments_per_resource_table, GHashTable *services_table, GHashTable *id_resources_table, gchar *service_property)
{
    GPtrArray *remove_keys = remove_invalid_service_id_assignment_entries_per_resource(id_assignments_per_resource_table, services_table, id_resources_table, service_property);
    remove_empty_id_assignment_tables(id_assignments_per_resource_table, remove_keys);
    g_ptr_array_free(remove_keys, TRUE);
}

static GPtrArray *remove_invalid_distribution_id_assignment_entries_per_resource(GHashTable *id_assignments_per_resource_table, GHashTable *services_table, GHashTable *distribution_table, GHashTable *id_resources_table, gchar *service_property)
{
    GPtrArray *remove_keys = g_ptr_array_new();

    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, id_assignments_per_resource_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        gchar *resource_name = (gchar*)key;
        GHashTable *id_assignments_table = (GHashTable*)value;
        IdResourceType *type = g_hash_table_lookup(id_resources_table, resource_name);

        remove_invalid_distribution_id_assignments(id_assignments_table, resource_name, services_table, distribution_table, type, service_property);

        if(g_hash_table_size(id_assignments_table) == 0)
            g_ptr_array_add(remove_keys, resource_name);
    }

    return remove_keys;
}

void remove_invalid_distribution_id_assignments_per_resource(GHashTable *id_assignments_per_resource_table, GHashTable *services_table, GHashTable *distribution_table, GHashTable *id_resources_table, gchar *service_property)
{
    GPtrArray *remove_keys = remove_invalid_distribution_id_assignment_entries_per_resource(id_assignments_per_resource_table, services_table, distribution_table, id_resources_table, service_property);
    remove_empty_id_assignment_tables(id_assignments_per_resource_table, remove_keys);
    g_ptr_array_free(remove_keys, TRUE);
}
