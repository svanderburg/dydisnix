#include "partialcoloredgraph-transform.h"
#include <nixxml-ghashtable-iter.h>
#include <node.h>
#include <service.h>
#include <target.h>
#include <distributionmapping.h>

static void generate_application_nodes(GHashTable *services_table, GHashTable *nodes_table)
{
    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, services_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        gchar *service_name = (gchar*)key;
        Node *node = create_node(service_name);
        g_hash_table_insert(nodes_table, service_name, node);
    }
}

static void generate_edges_for_dependency_type(Node *node, GHashTable *nodes_table, GPtrArray *dependencies)
{
    unsigned int i;

    for(i = 0; i < dependencies->len; i++)
    {
        gchar *dependency_name = g_ptr_array_index(dependencies, i);
        Node *dependency_node = g_hash_table_lookup(nodes_table, dependency_name);

        link_nodes_bidirectional(node, dependency_node);
    }
}

static void add_application_edges(GHashTable *nodes_table, GHashTable *services_table)
{
    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, nodes_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        gchar *service_name = (gchar*)key;
        Node *node = (Node*)value;
        Service *service = g_hash_table_lookup(services_table, service_name);

        generate_edges_for_dependency_type(node, nodes_table, service->depends_on);
        generate_edges_for_dependency_type(node, nodes_table, service->connects_to);
    }
}

static void generate_application_graph(GHashTable *services_table, GHashTable *nodes_table)
{
    generate_application_nodes(services_table, nodes_table);
    add_application_edges(nodes_table, services_table);
}

static void generate_colors(GHashTable *targets_table, GPtrArray *colors)
{
    NixXML_GHashTableOrderedIter iter;
    gchar *host_name;
    gpointer value;

    NixXML_g_hash_table_ordered_iter_init(&iter, targets_table);

    while(NixXML_g_hash_table_ordered_iter_next(&iter, &host_name, &value))
    {
        Target *target = (Target*)value;
        g_ptr_array_add(colors, target);
    }

    NixXML_g_hash_table_ordered_iter_destroy(&iter);

    // TODO: maybe also allow a priority field per target
}

PartialColoredGraph *generate_uncolored_graph(GHashTable *services_table, GHashTable *targets_table)
{
    PartialColoredGraph *graph = create_partial_colored_graph();
    generate_application_graph(services_table, graph->uncolored_nodes_table);
    generate_colors(targets_table, graph->colors);
    return graph;
}

GHashTable *convert_colored_graph_to_distribution_table(PartialColoredGraph *graph)
{
    GHashTable *result_table = g_hash_table_new(g_str_hash, g_str_equal);

    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, graph->colored_nodes_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        gchar *service_name = (gchar*)key;
        Node *node = (Node*)value;

        Target *target = g_ptr_array_index(graph->colors, node->value);

        xmlChar *target_name = (xmlChar*)find_target_property(target, "hostname"); // TODO: we need the real key
        DistributionMapping *mapping = create_distribution_auto_mapping(target_name);

        GPtrArray *targets = g_ptr_array_new();
        g_ptr_array_add(targets, mapping);

        g_hash_table_insert(result_table, service_name, targets);
    }

    return result_table;
}

void delete_converted_colored_graph_result_table(GHashTable *result_table)
{
    if(result_table != NULL)
    {
        GHashTableIter iter;
        gpointer key, value;

        g_hash_table_iter_init(&iter, result_table);
        while(g_hash_table_iter_next(&iter, &key, &value))
        {
            GPtrArray *targets = (GPtrArray*)value;
            unsigned int i;

            for(i = 0; i < targets->len; i++)
            {
                DistributionMapping *mapping = (DistributionMapping*)g_ptr_array_index(targets, i);
                g_free(mapping);
            }

            g_ptr_array_free(targets, TRUE);
        }

        g_hash_table_destroy(result_table);
    }
}
