#include "graphcol-approximate.h"
#include <glib.h>
#include <nixxml-ghashtable-iter.h>
#include <node.h>

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

static Node *find_node_with_unique_maximal_saturation_degree(PartialColoredGraph *graph)
{
    Node *max_saturation_degree_node = NULL;
    unsigned int max_saturation_degree = 0;
    NixXML_bool max_saturation_degree_is_unique = FALSE;

    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, graph->uncolored_nodes_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        Node *node = (Node*)value;
        unsigned int saturation_degree = compute_saturation_degree(node, graph->colored_nodes_table);

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

static Node *select_node_to_color(PartialColoredGraph *graph, GPtrArray *nodes_ordered_by_degree)
{
    // Find node with saturation degree that is the highest and unique
    Node *max_saturation_degree_node = find_node_with_unique_maximal_saturation_degree(graph);

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

static void assign_color(PartialColoredGraph *graph, Node *node, unsigned int color_index, GPtrArray *nodes_ordered_by_degree)
{
    node->value = color_index;
    g_hash_table_remove(graph->uncolored_nodes_table, node->name);
    g_hash_table_insert(graph->colored_nodes_table, node->name, node);
    g_ptr_array_index(nodes_ordered_by_degree, color_index) = NULL;
}

NixXML_bool approximate_graph_coloring_dsatur(PartialColoredGraph *graph)
{
    NixXML_bool result = TRUE;

    // Step 1: arrange the vertices by decreasing order of degrees
    GPtrArray *nodes_ordered_by_degree = order_nodes_by_degree(graph->uncolored_nodes_table);

    // Step 2: color a vertex of a maximal degree with color 1
    Node *first_node = find_node_with_highest_degree(nodes_ordered_by_degree);

    if(first_node == NULL)
        result = FALSE;
    else
    {
        assign_color(graph, first_node, 0, nodes_ordered_by_degree);

        // Step 5: If all the vertices are colored stop.
        while(g_hash_table_size(graph->uncolored_nodes_table) > 0)
        {
            /*
             * Step 3: choose a vertex with a maximal saturation degree.
             * If there is an equality, choose any vertex of a maximal degree
             * in the uncolored subgraph
             */
            Node *node = select_node_to_color(graph, nodes_ordered_by_degree);

            // Step 4: Color the vertex with the lowest numbered color
            unsigned int color_index = pick_lowest_possible_color(node, graph->colors, graph->colored_nodes_table);

            if(color_index < graph->colors->len)
                assign_color(graph, node, color_index, nodes_ordered_by_degree);
            else
            {
                g_printerr("Not enough target machines available to compute a valid graph coloring!\n");
                result = FALSE;
                break;
            }
        }
    }

    g_ptr_array_free(nodes_ordered_by_degree, TRUE);

    return result;
}
