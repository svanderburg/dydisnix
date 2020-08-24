#include "multiwaycut.h"
#include <nixxml-ghashtable-iter.h>
#include "applicationhostgraph.h"
#include <service.h>
#include <servicestable.h>
#include <candidatetargetmappingtable.h>
#include <candidatetargetmapping.h>
#include <targetmappingtable.h>
#include "node.h"

static void generate_initial_application_nodes(ApplicationHostGraph *graph, GHashTable *candidate_target_table)
{
    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, candidate_target_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        gchar *service_name = (gchar*)key;
        Node *app_node = create_node(TRUE, service_name);
        g_hash_table_insert(graph->appnodes_table, service_name, app_node);
    }
}

static void generate_links_for_service_dependency_type(Node *app_node, ApplicationHostGraph *graph, GPtrArray *dependencies)
{
    unsigned int i;

    for(i = 0; i < dependencies->len; i++)
    {
        gchar *dependency_name = (gchar*)g_ptr_array_index(dependencies, i);
        Node *dependency_app_node = g_hash_table_lookup(graph->appnodes_table, dependency_name);

        link_nodes_bidirectional(app_node, dependency_app_node);
    }
}

static void generate_edges_for_application_dependencies(ApplicationHostGraph *graph, GHashTable *services_table)
{
    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, graph->appnodes_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        Node *app_node = (Node*)value;
        Service *service = (Service*)g_hash_table_lookup(services_table, app_node->name);

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
static void generate_application_graph(ApplicationHostGraph *graph, GHashTable *services_table, GHashTable *candidate_target_table)
{
    /* Create application nodes */
    generate_initial_application_nodes(graph, candidate_target_table);

    /* Create edges between application nodes */
    generate_edges_for_application_dependencies(graph, services_table);
}

static void generate_host_to_app_node_links(Node *host_node, GPtrArray *services, GHashTable *appnodes_table)
{
    unsigned int i;

    for(i = 0; i < services->len; i++)
    {
        gchar *service = (gchar*)g_ptr_array_index(services, i);
        Node *app_node = g_hash_table_lookup(appnodes_table, service);

        link_nodes_bidirectional(host_node, app_node);
    }
}

/*
 * This step corresponds to the mapping of host machines described in the paper.
 *
 * - Every target in the infrastructure model becomes a host node in the graph
 * - For each target machine that is a candidate for a service, a bidirectional link is created with a very heavy weight: n^2
 */
static void generate_and_attach_host_nodes(ApplicationHostGraph *graph, GHashTable *target_mapping_table)
{
    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, target_mapping_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        gchar *target = (gchar*)key;
        GPtrArray *services = (GPtrArray*)value;

        Node *host_node = create_node(FALSE, target);
        g_hash_table_insert(graph->hosts_table, target, host_node);

        generate_host_to_app_node_links(host_node, services, graph->appnodes_table);
    }
}

/*
 * This function converts the Disnix models to a host-application graph described in the paper:
 * - Every service becomes an app node. Every inter-dependencies translates to a bidirectional link with weight: 1
 * - Every target machines becomes a host. When a target machine is a candidate deployment target for a service, a link gets created with a very heavy weight: n^2
 */
static ApplicationHostGraph *generate_application_host_graph(GHashTable *services_table, GHashTable *candidate_target_table, GHashTable *target_mapping_table)
{
    ApplicationHostGraph *graph = create_application_host_graph();

    /* Construct application graph first */
    generate_application_graph(graph, services_table, candidate_target_table);

    /* Create host nodes and attach them to the application nodes */
    generate_and_attach_host_nodes(graph, target_mapping_table);

    /* Return result */
    return graph;
}

/*
 * Examines every terminal (host node) in the graph and checks for each connected
 * app node whether there is a link to another host node. If it exists, we cut
 * the connection between the host node and app node.

 * This corresponds to the
 * minimum cut described in the paper -- the terminal host must be included in
 * the cut and to ensure the minimum value, we should not include any other links
 * because that will increase the weight.
 */
static GHashTable *generate_minimum_cuts_per_terminal(ApplicationHostGraph *graph)
{
    GHashTable *cuts_per_terminal_table = g_hash_table_new(g_str_hash, g_str_equal);

    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, graph->hosts_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        gchar *host_name = (gchar*)key;
        Node *host_node = (Node*)value;
        GPtrArray *cut_app_nodes_array = g_ptr_array_new();

        unsigned int i = 0;

        for(i = 0; i < host_node->links->len; i++)
        {
            Node *app_node = (Node*)g_ptr_array_index(host_node->links, i);

            GHashTableIter iter2;
            gpointer key2, value2;

            g_hash_table_iter_init(&iter2, graph->hosts_table);
            while(g_hash_table_iter_next(&iter2, &key2, &value2))
            {
                Node *target_host_node = (Node*)value2;

                if(host_node != target_host_node)
                {
                    mark_all_nodes_unvisited(graph);

                    if(search_node_breadth_first(app_node, check_nodes_have_indirect_connection, target_host_node) != NULL)
                        g_ptr_array_add(cut_app_nodes_array, app_node);
                }
            }
        }

        g_hash_table_insert(cuts_per_terminal_table, host_name, cut_app_nodes_array);
    }

    return cuts_per_terminal_table;
}

static gchar *determine_heaviest_host(GHashTable *cuts_per_terminal_table)
{
    gchar *heaviest_host = NULL;
    unsigned int heaviest_weight = 0;

    NixXML_GHashTableOrderedIter iter;
    gchar *host_name;
    gpointer value;

    NixXML_g_hash_table_ordered_iter_init(&iter, cuts_per_terminal_table);

    while(NixXML_g_hash_table_ordered_iter_next(&iter, &host_name, &value))
    {
        GPtrArray *cut_app_nodes_array = (GPtrArray*)value;

        if(cut_app_nodes_array->len > heaviest_weight)
        {
            heaviest_weight = cut_app_nodes_array->len;
            heaviest_host = host_name;
        }
    }

    NixXML_g_hash_table_ordered_iter_destroy(&iter);

    return heaviest_host;
}

/*
 * Enumerates the weights of all the minimum cuts per terminal.
 * Since each host-app link has the same weight (n^2), we only have
 * to count the amount of connections.
 *
 * The entry with the most amount of connections to app nodes, gets discarded.
 */
static void discard_heaviest_cut(GHashTable *cuts_per_terminal_table)
{
    gchar *heaviest_host = determine_heaviest_host(cuts_per_terminal_table);

    if(heaviest_host != NULL)
    {
        GPtrArray *cut_app_nodes_array = g_hash_table_lookup(cuts_per_terminal_table, heaviest_host);
        g_ptr_array_free(cut_app_nodes_array, TRUE);
        g_hash_table_remove(cuts_per_terminal_table, heaviest_host);
    }
}

/*
 * Unlinks all host-app node connections that are still in the table of
 * remaining minimum cuts per terminal. The host-app links that remain in the graph
 * are the actual deployments the algorithm recommends. Some app nodes might get
 * orphaned from all host nodes and need to be fixed.
 */
static void unlink_cuts_in_graph(ApplicationHostGraph *graph, GHashTable *cuts_per_terminal_table)
{
    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, cuts_per_terminal_table);

    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        gchar *host_name = (gchar*)key;
        GPtrArray *cut_app_nodes_array = (GPtrArray*)value;
        Node *host_node = g_hash_table_lookup(graph->hosts_table, host_name);
        unsigned int i;

        for(i = 0; i < cut_app_nodes_array->len; i++)
        {
            Node *cut_app_node = g_ptr_array_index(cut_app_nodes_array, i);
            unlink_nodes_bidirectional(host_node, cut_app_node);
        }
    }
}

static NixXML_bool check_app_node_is_orphaned_from_host_node(Node *app_node)
{
    unsigned int i;

    for(i = 0; i < app_node->links->len; i++)
    {
        Node *linked_node = (Node*)g_ptr_array_index(app_node->links, i);
        if(!linked_node->application_node)
            return FALSE;
    }

    return TRUE;
}

static void attach_app_node_to_first_host_node(Node *app_node, GHashTable *candidate_target_table, GHashTable *hosts_table)
{
    GPtrArray *targets = g_hash_table_lookup(candidate_target_table, app_node->name);
    CandidateTargetMapping *first_mapping = g_ptr_array_index(targets, 0);
    Node *host_node = g_hash_table_lookup(hosts_table, first_mapping->target);
    link_nodes_bidirectional(app_node, host_node);
}

/*
 * Some app nodes might get orphaned after applying the approximation algorithm.
 * We should reattach these to the first available host node.
 */
static void fix_orphaned_services(ApplicationHostGraph *graph, GHashTable *candidate_target_table)
{
    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, graph->appnodes_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        Node *app_node = (Node*)value;

        if(check_app_node_is_orphaned_from_host_node(app_node))
            attach_app_node_to_first_host_node(app_node, candidate_target_table, graph->hosts_table);
    }
}

static GHashTable *generate_candidate_target_table_from_application_host_graph(ApplicationHostGraph *graph)
{
    GHashTable *candidate_target_table = g_hash_table_new(g_str_hash, g_str_equal);
    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, graph->appnodes_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        Node *app_node = (Node*)value;
        GPtrArray *targets = g_ptr_array_new();
        unsigned int i;

        for(i = 0; i < app_node->links->len; i++)
        {
            Node *link_node = (Node*)g_ptr_array_index(app_node->links, i);
            if(!link_node->application_node)
            {
                CandidateTargetMapping *mapping = (CandidateTargetMapping*)g_malloc(sizeof(CandidateTargetMapping));
                mapping->target = (xmlChar*)link_node->name;
                mapping->container = NULL;
                g_ptr_array_add(targets, mapping);
            }
        }

        g_hash_table_insert(candidate_target_table, app_node->name, targets);
    }

    return candidate_target_table;
}

static void delete_cuts_per_terminal_table(GHashTable *cuts_per_terminal_table)
{
    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, cuts_per_terminal_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
        g_ptr_array_free((GPtrArray*)value, TRUE);

    g_hash_table_destroy(cuts_per_terminal_table);
}

static GHashTable *generate_reliable_distribution_using_multiway_cut_approximation(GHashTable *services_table, GHashTable *candidate_target_table, GHashTable *target_mapping_table)
{
    // Convert deployment models to a host/application graph
    ApplicationHostGraph *graph = generate_application_host_graph(services_table, candidate_target_table, target_mapping_table);

    // The three steps of the approximation algorithm
    GHashTable *cuts_per_terminal_table = generate_minimum_cuts_per_terminal(graph);
    discard_heaviest_cut(cuts_per_terminal_table);
    unlink_cuts_in_graph(graph, cuts_per_terminal_table);

    // Fix orphaned services
    fix_orphaned_services(graph, candidate_target_table);

    // Convert back to mapping table and return the result
    GHashTable *result_table = generate_candidate_target_table_from_application_host_graph(graph);

    // Cleanup
    delete_cuts_per_terminal_table(cuts_per_terminal_table);
    delete_application_host_graph(graph);

    return result_table;
}

static void delete_result_table(GHashTable *result_table)
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
            CandidateTargetMapping *mapping = (CandidateTargetMapping*)g_ptr_array_index(mappings_array, i);
            g_free(mapping);
        }

        g_ptr_array_free(mappings_array, TRUE);
    }

    g_hash_table_destroy(result_table);
}

int multiwaycut(gchar *services, gchar *distribution, gchar *infrastructure, const unsigned int flags)
{
    NixXML_bool xml = flags & DYDISNIX_FLAG_XML;
    NixXML_bool automapped;
    GHashTable *services_table = create_service_table(services, xml);
    GHashTable *candidate_target_table = create_candidate_target_table(distribution, infrastructure, xml, &automapped);

    if(services_table == NULL || candidate_target_table == NULL)
    {
        g_printerr("Error opening one of the input models!\n");
        return 1;
    }
    else
    {
        GHashTable *target_mapping_table = create_target_mapping_table(candidate_target_table);
        GHashTable *result_table = generate_reliable_distribution_using_multiway_cut_approximation(services_table, candidate_target_table, target_mapping_table);

        /* Print Nix expression of the result */
        if(flags & DYDISNIX_FLAG_OUTPUT_XML)
            print_candidate_target_table_xml(stdout, result_table, 0, NULL, NULL);
        else
            print_candidate_target_table_nix(stdout, result_table, 0, &automapped);

        /* Cleanup */
        delete_result_table(result_table);
        delete_target_mapping_table(target_mapping_table);
        delete_candidate_target_table(candidate_target_table);
        delete_service_table(services_table);

        /* Return exit status */
        return 0;
    }
}
