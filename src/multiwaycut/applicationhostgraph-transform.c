#include "applicationhostgraph-transform.h"
#include <string.h>
#include <node.h>
#include <service.h>
#include <distributionmapping.h>

static void generate_initial_application_nodes(ApplicationHostGraph *graph, GHashTable *distribution_table)
{
    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, distribution_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        gchar *service_name = (gchar*)key;
        Node *app_node = create_app_node(service_name);
        g_hash_table_insert(graph->app_nodes_table, service_name, app_node);
    }
}

static void generate_links_for_service_dependency_type(Node *app_node, ApplicationHostGraph *graph, GPtrArray *dependencies)
{
    unsigned int i;

    for(i = 0; i < dependencies->len; i++)
    {
        gchar *dependency_name = (gchar*)g_ptr_array_index(dependencies, i);
        Node *dependency_app_node = g_hash_table_lookup(graph->app_nodes_table, dependency_name);

        link_nodes_bidirectional_with_annotation(app_node, dependency_app_node, "1");
    }
}

static void generate_edges_for_application_dependencies(ApplicationHostGraph *graph, GHashTable *services_table)
{
    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, graph->app_nodes_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        gchar *service_name = (gchar*)key;
        Node *app_node = (Node*)value;
        Service *service = (Service*)g_hash_table_lookup(services_table, service_name);

        generate_links_for_service_dependency_type(app_node, graph, service->depends_on);
        generate_links_for_service_dependency_type(app_node, graph, service->connects_to);
    }
}

/*
 * This step corresponds to the generation of the application graph as described in the paper:
 *
 * - Every service in the service model is an application node.
 * - Every inter-dependency (dependsOn, connectsTo) translates to a bidirectional link between corresponding app nodes with weight: 1
 */
static void generate_application_graph(ApplicationHostGraph *graph, GHashTable *services_table, GHashTable *distribution_table)
{
    /* Create application nodes */
    generate_initial_application_nodes(graph, distribution_table);

    /* Create edges between application nodes */
    generate_edges_for_application_dependencies(graph, services_table);
}

static void generate_host_to_app_node_links(Node *host_node, GPtrArray *services, GHashTable *app_nodes_table)
{
    unsigned int i;

    for(i = 0; i < services->len; i++)
    {
        gchar *service = (gchar*)g_ptr_array_index(services, i);
        Node *app_node = g_hash_table_lookup(app_nodes_table, service);

        link_nodes_bidirectional_with_annotation(host_node, app_node, "n&#178;");
    }
}

/*
 * This step corresponds to the mapping of host machines described in the paper.
 *
 * - Every target in the infrastructure model becomes a host node in the graph
 * - For each target machine that is a candidate for a service, a bidirectional link is created with a very heavy weight: n^2
 */
static void generate_and_attach_host_nodes(ApplicationHostGraph *graph, GHashTable *target_to_services_table)
{
    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, target_to_services_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        gchar *target = (gchar*)key;
        GPtrArray *services = (GPtrArray*)value;

        Node *host_node = create_host_node(target);
        g_hash_table_insert(graph->host_nodes_table, target, host_node);

        generate_host_to_app_node_links(host_node, services, graph->app_nodes_table);
    }
}

/*
 * This function converts the Disnix models to a host-application graph described in the paper:
 * - Every service becomes an app node. Every inter-dependencies translates to a bidirectional link with weight: 1
 * - Every target machines becomes a host. When a target machine is a candidate deployment target for a service, a link gets created with a very heavy weight: n^2
 */
ApplicationHostGraph *generate_application_host_graph(GHashTable *services_table, GHashTable *distribution_table, GHashTable *target_to_services_table)
{
    ApplicationHostGraph *graph = create_application_host_graph();

    /* Construct application graph first */
    generate_application_graph(graph, services_table, distribution_table);

    /* Create host nodes and attach them to the application nodes */
    generate_and_attach_host_nodes(graph, target_to_services_table);

    /* Return result */
    return graph;
}

GHashTable *generate_distribution_table_from_application_host_graph(ApplicationHostGraph *graph)
{
    GHashTable *distribution_table = g_hash_table_new(g_str_hash, g_str_equal);
    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, graph->app_nodes_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        gchar *service_name = (gchar*)key;
        Node *app_node = (Node*)value;

        GPtrArray *targets = g_ptr_array_new();
        unsigned int i;

        for(i = 0; i < app_node->links->len; i++)
        {
            Node *link_node = (Node*)g_ptr_array_index(app_node->links, i);
            if(!node_is_app_node(link_node))
            {
                gchar *host_name = g_strdup(link_node->name + strlen("host:"));
                DistributionMapping *mapping = create_distribution_auto_mapping((xmlChar*)host_name);
                g_ptr_array_add(targets, mapping);
            }
        }

        g_hash_table_insert(distribution_table, service_name, targets);
    }

    return distribution_table;
}

void delete_application_host_graph_result_table(GHashTable *result_table)
{
    if(result_table != NULL)
    {
        GHashTableIter iter;
        gpointer key, value;

        g_hash_table_iter_init(&iter, result_table);
        while(g_hash_table_iter_next(&iter, &key, &value))
        {
            GPtrArray *mappings_array = (GPtrArray*)value;
            unsigned int i;

            for(i = 0; i < mappings_array->len; i++)
            {
                DistributionMapping *mapping = (DistributionMapping*)g_ptr_array_index(mappings_array, i);
                delete_distribution_mapping(mapping);
            }

            g_ptr_array_free(mappings_array, TRUE);
        }

        g_hash_table_destroy(result_table);
    }
}
