#include "multiwaycut-approximate.h"
#include <nixxml-ghashtable-iter.h>
#include <distributionmapping.h>
#include <node.h>

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

    g_hash_table_iter_init(&iter, graph->host_nodes_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        gchar *host_name = (gchar*)key;
        Node *host_node = (Node*)value;
        GPtrArray *cut_app_nodes_array = g_ptr_array_new();

        unsigned int i;

        for(i = 0; i < host_node->links->len; i++)
        {
            Node *app_node = (Node*)g_ptr_array_index(host_node->links, i);

            GHashTableIter iter2;
            gpointer key2, value2;

            g_hash_table_iter_init(&iter2, graph->host_nodes_table);
            while(g_hash_table_iter_next(&iter2, &key2, &value2))
            {
                Node *target_host_node = (Node*)value2;

                if(host_node != target_host_node)
                {
                    mark_all_nodes_unvisited(graph);

                    if(search_node_breadth_first(app_node, check_nodes_have_indirect_connection, target_host_node) != NULL)
                    {
                        g_ptr_array_add(cut_app_nodes_array, app_node);
                        break;
                    }
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
        Node *host_node = g_hash_table_lookup(graph->host_nodes_table, host_name);
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
        if(!node_is_app_node(linked_node))
            return FALSE;
    }

    return TRUE;
}

static void attach_app_node_to_first_host_node(gchar *service_name, Node *app_node, GHashTable *distribution_table, GHashTable *host_nodes_table)
{
    GPtrArray *targets = g_hash_table_lookup(distribution_table, service_name);
    DistributionMapping *first_mapping = g_ptr_array_index(targets, 0);
    Node *host_node = g_hash_table_lookup(host_nodes_table, first_mapping->target);
    link_nodes_bidirectional(app_node, host_node);
}

/*
 * Some app nodes might get orphaned after applying the approximation algorithm.
 * We should reattach these to the first available host node.
 */
static void fix_orphaned_services(ApplicationHostGraph *graph, GHashTable *distribution_table)
{
    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, graph->app_nodes_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        gchar *service_name = (gchar*)key;
        Node *app_node = (Node*)value;

        if(check_app_node_is_orphaned_from_host_node(app_node))
            attach_app_node_to_first_host_node(service_name, app_node, distribution_table, graph->host_nodes_table);
    }
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

void approximate_multiway_cut_solution(ApplicationHostGraph *graph, GHashTable *distribution_table)
{
    // The three steps of the approximation algorithm

    /*
     * Step 1: For each terminal (target machine) find a minimum-cost set of
     * edges (distribution mappings from target machine to services) whose
     * removal disconnects the terminal (machine) from the rest of the terminals
     * (other machines).
     */
    GHashTable *cuts_per_terminal_table = generate_minimum_cuts_per_terminal(graph);

    // Step 2: Discard the cut (host -> service mapping) whose cost is the heaviest
    discard_heaviest_cut(cuts_per_terminal_table);

    /*
     * Step 3: Output the union of the rest.
     * Equivalent step: the union contains the cuts that need to be removed from
     * the graph. After removal, what remains is the intended/near optimal
     * mapping of targets to services -- the terminals that remain attached to
     * the graph, specify to which machines the services need to be distributed.
     */
    unlink_cuts_in_graph(graph, cuts_per_terminal_table);

    // Fix orphaned services (also described in the paper, as a fixup after applying the approximation algorithm)
    fix_orphaned_services(graph, distribution_table);

    // Cleanup
    delete_cuts_per_terminal_table(cuts_per_terminal_table);
}
