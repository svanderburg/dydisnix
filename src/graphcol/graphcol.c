#include "graphcol.h"
#include <glib.h>
#include <nixxml-ghashtable-iter.h>
#include <node.h>
#include <servicestable.h>
#include <targetstable2.h>
#include <target.h>
#include <candidatetargetmappingtable.h>

static GHashTable *create_application_nodes(GHashTable *services_table)
{
    GHashTable *result_table = g_hash_table_new(g_str_hash, g_str_equal);

    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, services_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        gchar *service_name = (gchar*)key;
        Node *node = create_node(service_name);
        g_hash_table_insert(result_table, service_name, node);
    }

    return result_table;
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

static GHashTable *generate_application_graph(GHashTable *services_table)
{
    GHashTable *nodes_table = create_application_nodes(services_table);
    add_application_edges(nodes_table, services_table);
    return nodes_table;
}

static GPtrArray *convert_targets_to_colors(GHashTable *targets_table)
{
    GPtrArray *colors = g_ptr_array_new();

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

    return colors;

    // TODO: maybe also allow a priority field per target
}

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
    {
        // If no node with a unique maximum saturation degree, take the first node with the highest regular degree
        return find_node_with_highest_degree(nodes_ordered_by_degree);
    }
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

GHashTable *convert_nodes_table_to_candidate_target_mapping_table(GHashTable *colored_nodes_table, GPtrArray *colors)
{
    GHashTable *result_table = g_hash_table_new(g_str_hash, g_str_equal);

    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, colored_nodes_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        gchar *service_name = (gchar*)key;
        Node *node = (Node*)value;

        Target *target = g_ptr_array_index(colors, node->value);

        CandidateTargetMapping *mapping = (CandidateTargetMapping*)g_malloc(sizeof(CandidateTargetMapping));
        mapping->container = NULL;
        mapping->target = (xmlChar*)find_target_property(target, "hostname"); // TODO: we need the real key;

        GPtrArray *targets = g_ptr_array_new();
        g_ptr_array_add(targets, mapping);

        g_hash_table_insert(result_table, service_name, targets);
    }

    return result_table;
}

static void delete_nodes_table(GHashTable *nodes_table)
{
    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, nodes_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
         delete_node((Node*)value);

    g_hash_table_destroy(nodes_table);
}

static GHashTable *graphcol_dsatur_approximation(GHashTable *services_table, GHashTable *targets_table)
{
    // Convert models to an application graph and colors
    GHashTable *uncolored_nodes_table = generate_application_graph(services_table);
    unsigned int num_of_nodes = g_hash_table_size(uncolored_nodes_table);
    GPtrArray *colors = convert_targets_to_colors(targets_table);

    GHashTable *colored_nodes_table = g_hash_table_new(g_str_hash, g_str_equal);
    GPtrArray *nodes_ordered_by_degree = order_nodes_by_degree(uncolored_nodes_table);

    // Approximation starts here
    Node *first_node = find_node_with_highest_degree(nodes_ordered_by_degree);
    assign_color(first_node, 0, uncolored_nodes_table, colored_nodes_table, nodes_ordered_by_degree);

    while(g_hash_table_size(colored_nodes_table) < num_of_nodes)
    {
        Node *node = select_node_to_color(uncolored_nodes_table, colored_nodes_table, nodes_ordered_by_degree);
        unsigned int color_index = pick_lowest_possible_color(node, colors, colored_nodes_table);
        assign_color(node, color_index, uncolored_nodes_table, colored_nodes_table, nodes_ordered_by_degree);
    }

    GHashTable *result_table = convert_nodes_table_to_candidate_target_mapping_table(colored_nodes_table, colors);

    // Cleanup
    g_hash_table_destroy(uncolored_nodes_table);
    delete_nodes_table(colored_nodes_table);
    g_ptr_array_free(colors, TRUE);
    g_ptr_array_free(nodes_ordered_by_degree, TRUE);

    return result_table;
}

static void delete_result_table(GHashTable *result_table)
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
            CandidateTargetMapping *mapping = (CandidateTargetMapping*)g_ptr_array_index(targets, i);
            g_free(mapping);
        }

        g_ptr_array_free(targets, TRUE);
    }

    g_hash_table_destroy(result_table);
}

int graphcol(char *services_xml, char *infrastructure_xml, const unsigned int flags)
{
    /* Load input models */
    NixXML_bool automapped = TRUE;

    NixXML_bool xml = flags & DYDISNIX_FLAG_XML;
    GHashTable *services_table = create_service_table(services_xml, xml);
    GHashTable *targets_table = create_targets_table2(infrastructure_xml, xml);

    /* Generate distribution using graph coloring approximation */
    GHashTable *result_table = graphcol_dsatur_approximation(services_table, targets_table);

    /* Print output expression */

    if(flags & DYDISNIX_FLAG_OUTPUT_XML)
        print_candidate_target_table_xml(stdout, result_table, 0, NULL, NULL);
    else
        print_candidate_target_table_nix(stdout, result_table, 0, &automapped);

    /* Cleanup */

    delete_result_table(result_table);
    delete_service_table(services_table);
    delete_targets_table(targets_table);

    return 0;
}
