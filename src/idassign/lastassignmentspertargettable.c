#include "lastassignmentspertargettable.h"
#include <nixxml-ghashtable.h>
#include "lastassignmentstable.h"

GHashTable *create_empty_last_assignments_per_target_table(void)
{
    return NixXML_create_g_hash_table();
}

GHashTable *retrieve_or_add_empty_last_assignments_table_for_target(GHashTable *last_assignments_per_target_table, const gchar *target_name)
{
    GHashTable *last_assignments_table = (GHashTable*)g_hash_table_lookup(last_assignments_per_target_table, target_name);

    if(last_assignments_table == NULL)
        last_assignments_table = create_empty_last_assignments_table();

    g_hash_table_insert(last_assignments_per_target_table, g_strdup(target_name), last_assignments_table);

    return last_assignments_table;
}

void *parse_last_assignments_per_target_table(xmlNodePtr element, void *userdata)
{
    return NixXML_parse_g_hash_table_verbose(element, "target", "name", userdata, parse_last_assignments_table);
}

void delete_last_assignments_per_target_table(GHashTable *last_assignments_per_target_table)
{
    NixXML_delete_g_hash_table(last_assignments_per_target_table, (NixXML_DeleteGHashTableValueFunc)delete_last_assignments_table);
}

void print_last_assignments_per_target_table_nix(FILE *file, GHashTable *last_assignments_per_target_table, const int indent_level, void *userdata)
{
    NixXML_print_g_hash_table_ordered_nix(file, last_assignments_per_target_table, indent_level, userdata, (NixXML_PrintValueFunc)print_last_assignments_table_nix);
}

void print_last_assignments_per_target_table_xml(FILE *file, GHashTable *last_assignments_per_target_table, const int indent_level, const char *type_property_name, void *userdata)
{
    NixXML_print_g_hash_table_verbose_ordered_xml(file, last_assignments_per_target_table, "target", "name", indent_level, NULL, userdata, (NixXML_PrintXMLValueFunc)print_last_assignments_table_xml);
}

static GPtrArray *remove_invalid_last_assignments_entries_per_target(GHashTable *last_assignments_per_target_table, GHashTable *id_resources_table, GHashTable *target_to_service_table)
{
    GPtrArray *remove_keys = g_ptr_array_new();

    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, last_assignments_per_target_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        gchar *target_name = (gchar*)key;
        GHashTable *last_assignments_table = (GHashTable*)value;

        remove_invalid_last_assignments(last_assignments_table, id_resources_table);

        if(g_hash_table_contains(target_to_service_table, target_name))
        {
            if(g_hash_table_size(last_assignments_table) == 0)
                g_ptr_array_add(remove_keys, target_name);
        }
        else
        {
            delete_last_assignments_table(last_assignments_table);
            g_ptr_array_add(remove_keys, target_name);
        }
    }

    return remove_keys;
}

static void remove_unneeded_last_assignments_tables(GHashTable *last_assignments_per_target_table, GPtrArray *remove_keys)
{
    unsigned int i;

    for(i = 0; i < remove_keys->len; i++)
    {
        gchar *target_name = (gchar*)g_ptr_array_index(remove_keys, i);
        GHashTable *last_assignments_table = (GHashTable*)g_hash_table_lookup(last_assignments_per_target_table, target_name);
        g_hash_table_remove(last_assignments_table, target_name);
        delete_last_assignments_table(last_assignments_table);
    }
}

void remove_invalid_last_assignments_per_target(GHashTable *last_assignments_per_target_table, GHashTable *id_resources_table, GHashTable *target_to_service_table)
{
    GPtrArray *remove_keys = remove_invalid_last_assignments_entries_per_target(last_assignments_per_target_table, id_resources_table, target_to_service_table);
    remove_unneeded_last_assignments_tables(last_assignments_per_target_table, remove_keys);
    g_ptr_array_free(remove_keys, TRUE);
}
