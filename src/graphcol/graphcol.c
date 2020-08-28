#include "graphcol.h"
#include <glib.h>
#include <nixxml-ghashtable-iter.h>
#include <node.h>
#include <servicestable.h>
#include <targetstable2.h>
#include <target.h>
#include <candidatetargetmappingtable.h>
#include "partialcoloredgraph.h"
#include "partialcoloredgraph-transform.h"

static gint compare_node_degrees(gconstpointer a, gconstpointer b)
{
    const Node *node1 = *((Node**)a);
    const Node *node2 = *((Node**)b);

    return node_degree(node2) - node_degree(node1);
}

static GPtrArray *order_nodes_by_degree(GHashTable *nodes_table)
{
    GPtrArray *nodes = g_ptr_array_new();

    /* Create an array with all values from the node table */

    NixXML_GHashTableOrderedIter iter;
    gchar *host_name;
    gpointer value;

    NixXML_g_hash_table_ordered_iter_init(&iter, nodes_table);

    while(NixXML_g_hash_table_ordered_iter_next(&iter, &host_name, &value))
    {
        Node *node = (Node*)value;
        g_ptr_array_add(nodes, node);
    }

    /* Sort the array by degree */
    g_ptr_array_sort(nodes, compare_node_degrees);

    NixXML_g_hash_table_ordered_iter_destroy(&iter);

    return nodes;
}

static unsigned int compute_saturation_degree(Node *node, GHashTable *colored_nodes)
{
    unsigned int i;
    unsigned int degree = 0;

    for(i = 0; i < node->links->len; i++)
    {
        Node *linked_node = g_ptr_array_index(node->links, i);
        if(g_hash_table_contains(colored_nodes, linked_node->name))
            degree++;
    }

    return degree;
}

static Node *find_node_with_unique_maximal_saturation_degree(GHashTable *uncolored_nodes_table, GHashTable *colored_nodes)
{
    Node *max_saturation_degree_node = NULL;
    unsigned int max_saturation_degree = 0;
    NixXML_bool max_saturation_degree_is_unique = FALSE;

    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, uncolored_nodes_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        Node *node = (Node*)value;
        unsigned int saturation_degree = compute_saturation_degree(node, colored_nodes);

        if(max_saturation_degree_node == NULL || saturation_degree > max_saturation_degree)
        {
            max_saturation_degree = saturation_degree;
            max_saturation_degree_node = node;
            max_saturation_degree_is_unique = TRUE;
        }
        else if(saturation_degree == max_saturation_degree)
            max_saturation_degree_is_unique = FALSE;
    }

    if(max_saturation_degree_is_unique)
        return max_saturation_degree_node;
    else
        return NULL;
}

static Node *find_node_with_highest_degree(GPtrArray *nodes_ordered_by_degree)
{
    unsigned int i;
    Node *node = NULL;

    for(i = 0; i < nodes_ordered_by_degree->len; i++)
    {
        node = g_ptr_array_index(nodes_ordered_by_degree, i);

        if(node != NULL)
            break;
    }

    return node;
}

static Node *select_node_to_color(GHashTable *uncolored_nodes_table, GHashTable *colored_nodes, GPtrArray *nodes_ordered_by_degree)
{
    // Find node with saturation degree that is the highest and unique
    Node *max_saturation_degree_node = find_node_with_unique_maximal_saturation_degree(uncolored_nodes_table, colored_nodes);

    if(max_saturation_degree_node == NULL)
        return find_node_with_highest_degree(nodes_ordered_by_degree); // If no node with a unique maximum saturation degree, take the first node with the highest regular degree
    else
        return max_saturation_degree_node;
}

static NixXML_bool has_colored_adjacent_node(Node *node, unsigned int color_index, GHashTable *colored_nodes_table)
{
    NixXML_bool found = FALSE;
    unsigned int i;

    for(i = 0; i < node->links->len; i++)
    {
        Node *linked_node = g_ptr_array_index(node->links, i);

        if(g_hash_table_contains(colored_nodes_table, linked_node->name) && linked_node->value == color_index)
        {
            found = TRUE;
            break;
        }
    }

    return found;
}

static unsigned int pick_lowest_possible_color(Node *node, GPtrArray *colors, GHashTable *colored_nodes_table)
{
    unsigned int i = 0;

    for(i = 0; i < colors->len; i++)
    {
        if(!has_colored_adjacent_node(node, i, colored_nodes_table))
            return i;
    }

    return i;
}

static void assign_color(Node *node, unsigned int color_index, GHashTable *uncolored_nodes_table, GHashTable *colored_nodes_table, GPtrArray *nodes_ordered_by_degree)
{
    node->value = color_index;
    g_hash_table_remove(uncolored_nodes_table, node->name);
    g_hash_table_insert(colored_nodes_table, node->name, node);
    g_ptr_array_index(nodes_ordered_by_degree, color_index) = NULL;
}

static GHashTable *graphcol_dsatur_approximation(GHashTable *services_table, GHashTable *targets_table)
{
    if(g_hash_table_size(services_table) > 0)
    {
        PartialColoredGraph *graph = generate_uncolored_graph(services_table, targets_table);

        // Convert models to an application graph and colors
        GPtrArray *nodes_ordered_by_degree = order_nodes_by_degree(graph->uncolored_nodes_table);

        // Approximation starts here
        Node *first_node = find_node_with_highest_degree(nodes_ordered_by_degree);

        assign_color(first_node, 0, graph->uncolored_nodes_table, graph->colored_nodes_table, nodes_ordered_by_degree);

        while(g_hash_table_size(graph->uncolored_nodes_table) > 0)
        {
            Node *node = select_node_to_color(graph->uncolored_nodes_table, graph->colored_nodes_table, nodes_ordered_by_degree);
            unsigned int color_index = pick_lowest_possible_color(node, graph->colors, graph->colored_nodes_table);

            g_printerr("NODE: %s, picked color: %d, targets table size: %d\n", node->name, color_index, g_hash_table_size(targets_table));

            if(color_index < g_hash_table_size(targets_table))
                assign_color(node, color_index, graph->uncolored_nodes_table, graph->colored_nodes_table, nodes_ordered_by_degree);
            else
            {
                g_printerr("Not enough target machines available to compute a valid graph coloring!\n");
                g_ptr_array_free(nodes_ordered_by_degree, TRUE);
                delete_partial_colored_graph(graph);
                return NULL;
            }
        }

        // Convert back to distribution model
        GHashTable *result_table = convert_colored_graph_to_candidate_target_mapping_table(graph);

        // Cleanup
        g_ptr_array_free(nodes_ordered_by_degree, TRUE);
        delete_partial_colored_graph(graph);

        return result_table;
    }
    else
        return g_hash_table_new(g_str_hash, g_str_equal);
}

int graphcol(char *services_xml, char *infrastructure_xml, const unsigned int flags)
{
    /* Load input models */

    NixXML_bool xml = flags & DYDISNIX_FLAG_XML;
    GHashTable *services_table = create_service_table(services_xml, xml);
    GHashTable *targets_table = create_targets_table2(infrastructure_xml, xml);

    if(services_table == NULL || targets_table == NULL)
    {
        g_printerr("Error opening one of the input models!\n");
        return 1;
    }
    else
    {
        NixXML_bool automapped = TRUE;
        int exit_status;

        /* Generate distribution using graph coloring approximation */
        GHashTable *result_table = graphcol_dsatur_approximation(services_table, targets_table);

        if(result_table == NULL)
            exit_status = 1;
        else
        {
            exit_status = 0;

            /* Print output expression */

            if(flags & DYDISNIX_FLAG_OUTPUT_XML)
                print_candidate_target_table_xml(stdout, result_table, 0, NULL, NULL);
            else
                print_candidate_target_table_nix(stdout, result_table, 0, &automapped);
        }

        /* Cleanup */

        delete_converted_colored_graph_result_table(result_table);
        delete_service_table(services_table);
        delete_targets_table(targets_table);

        return exit_status;
    }
}
