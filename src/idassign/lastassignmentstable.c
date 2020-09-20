#include "lastassignmentstable.h"
#include <nixxml-ghashtable.h>

GHashTable *create_empty_last_assignments_table(void)
{
    return NixXML_create_g_hash_table();
}

void *parse_last_assignments_table(xmlNodePtr element, void *userdata)
{
    return NixXML_parse_g_hash_table_verbose(element, "assignment", "name", userdata, NixXML_parse_int);
}

void delete_last_assignments_table(GHashTable *last_assignments_table)
{
    NixXML_delete_g_hash_table(last_assignments_table, (NixXML_DeleteGHashTableValueFunc)free);
}

void print_last_assignments_table_nix(FILE *file, GHashTable *last_assignments_table, const int indent_level, void *userdata)
{
    NixXML_print_g_hash_table_ordered_nix(file, last_assignments_table, indent_level, userdata, (NixXML_PrintValueFunc)NixXML_print_int_nix);
}

void print_last_assignments_table_xml(FILE *file, GHashTable *last_assignments_table, const int indent_level, const char *type_property_name, void *userdata)
{
    NixXML_print_g_hash_table_verbose_ordered_xml(file, last_assignments_table, "assignment", "name", indent_level, NULL, userdata, (NixXML_PrintXMLValueFunc)NixXML_print_int_xml);
}

static GPtrArray *determine_invalid_last_assignments(GHashTable *last_assignments_table, GHashTable *id_resources_table)
{
    GPtrArray *remove_keys = g_ptr_array_new();

    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, last_assignments_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        gchar *resource_name = (gchar*)key;

        if(!g_hash_table_contains(id_resources_table, resource_name))
            g_ptr_array_add(remove_keys, resource_name);
    }

    return remove_keys;
}

static void remove_last_assignments(GHashTable *last_assignments_table, GPtrArray *remove_keys)
{
    unsigned int i;

    for(i = 0; i < remove_keys->len; i++)
    {
        gchar *resource_name = (gchar*)g_ptr_array_index(remove_keys, i);
        int *id = (int*)g_hash_table_lookup(last_assignments_table, resource_name);
        g_hash_table_remove(last_assignments_table, resource_name);
        free(id);
    }
}

void remove_invalid_last_assignments(GHashTable *last_assignments_table, GHashTable *id_resources_table)
{
    GPtrArray *remove_keys = determine_invalid_last_assignments(last_assignments_table, id_resources_table);
    remove_last_assignments(last_assignments_table, remove_keys);
    g_ptr_array_free(remove_keys, TRUE);
}
