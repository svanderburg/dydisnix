#include "idassignmentstable.h"
#include <nixxml-ghashtable.h>
#include <nixxml-print-nix.h>
#include <nixxml-print-xml.h>

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
