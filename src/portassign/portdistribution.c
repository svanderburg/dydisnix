#include "portdistribution.h"
#include <nixxml-ghashtable.h>
#include "serviceproperties.h"
#include "candidatetargetmappingtable.h"

GHashTable *create_port_distribution_table(PortConfiguration *port_configuration, GHashTable *service_table, GHashTable *candidate_target_table, gchar *service_property)
{
    GHashTable *port_distribution_table = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, g_free);

    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, candidate_target_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        gchar *service_name = (gchar*)key;
        GPtrArray *targets = (GPtrArray*)value;

        Service *service = g_hash_table_lookup(service_table, service_name);
        gchar *prop_value;

        if(service == NULL)
            prop_value = NULL;
        else
            prop_value = g_hash_table_lookup(service->properties, service_property);

        /* For each service that has a port property that is unassigned, assign a port */

        if(prop_value != NULL)
        {
            if(g_strcmp0(prop_value, "shared") == 0) /* If a shared port is requested, consult the shared ports pool */
            {
                gint *port = g_malloc(sizeof(gint));
                *port = assign_or_reuse_port(port_configuration, NULL, service_name);
                g_hash_table_insert(port_distribution_table, service_name, port);
            }
            else if(g_strcmp0(prop_value, "private") == 0) /* If a private port is requested, consult the machine's ports pool */
            {
                if(targets->len > 0)
                {
                    CandidateTargetMapping *mapping = g_ptr_array_index(targets, 0);
                    gint *port = g_malloc(sizeof(gint));
                    *port = assign_or_reuse_port(port_configuration, mapping->target, service_name);
                    g_hash_table_insert(port_distribution_table, service_name, port);
                }
                else
                    g_printerr("WARNING: %s is not distributed to any machine. Skipping port assignment...\n", service_name);
            }
        }
    }

    return port_distribution_table;
}

void delete_port_distribution_table(GHashTable *port_distribution_table)
{
    g_hash_table_destroy(port_distribution_table);
}

void print_port_distribution_table_nix(FILE *file, const void *value, const int indent_level, void *userdata)
{
    NixXML_print_g_hash_table_nix(file, (GHashTable*)value, indent_level, userdata, NixXML_print_int_nix);
}

void print_port_distribution_table_xml(FILE *file, const void *value, const int indent_level, const char *type_property_name, void *userdata)
{
    NixXML_print_g_hash_table_verbose_xml(file, (GHashTable*)value, "service", "name", indent_level, type_property_name, userdata, NixXML_print_int_xml);
}
