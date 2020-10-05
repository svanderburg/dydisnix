#include "partialcoloredgraph.h"
#include <node.h>

PartialColoredGraph *create_partial_colored_graph(void)
{
    PartialColoredGraph *graph = (PartialColoredGraph*)g_malloc(sizeof(PartialColoredGraph));
    graph->uncolored_nodes_table = g_hash_table_new(g_str_hash, g_str_equal);
    graph->colored_nodes_table = g_hash_table_new(g_str_hash, g_str_equal);
    graph->colors = g_ptr_array_new();
    return graph;
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

void delete_partial_colored_graph(PartialColoredGraph *graph)
{
    if(graph != NULL)
    {
        delete_nodes_table(graph->uncolored_nodes_table);
        delete_nodes_table(graph->colored_nodes_table);
        g_ptr_array_free(graph->colors, TRUE);
        g_free(graph);
    }
}

static void print_uncolored_nodes(FILE *file, PartialColoredGraph *graph)
{
    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, graph->uncolored_nodes_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        Node *node = (Node*)value;
        fprintf(stderr, "\"%s\" [ label = \"%s\" ]\n", node->name, node->name);
    }
}

static void print_colored_nodes(FILE *file, PartialColoredGraph *graph)
{
    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, graph->colored_nodes_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        Node *node = (Node*)value;
        fprintf(stderr, "\"%s\" [ label = \"<f0> %s|<f1> %d\" ]\n", node->name, node->name, node->value);
    }
}

static void print_undirected_node_links(FILE *file, GHashTable *nodes_table)
{
    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, nodes_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        Node *node = (Node*)value;
        unsigned int i;

        for(i = 0; i < node->links->len; i++)
        {
            Node *linked_node = (Node*)g_ptr_array_index(node->links, i);

            if(g_strcmp0(node->name, linked_node->name) <= 0) /* Only print an edge from the perspective of the lowest node name. Otherwise, we will get a double connection */
                fprintf(file, "\"%s\" -- \"%s\"\n", node->name, linked_node->name);
        }
    }
}

void print_partial_colored_graph_dot(FILE *file, PartialColoredGraph *graph)
{
    fprintf(file, "graph {\n");
    fprintf(file, "node [shape=Mrecord];\n");
    print_uncolored_nodes(file, graph);
    print_colored_nodes(file, graph);
    print_undirected_node_links(file, graph->uncolored_nodes_table);
    print_undirected_node_links(file, graph->colored_nodes_table);
    fprintf(file, "}\n");
}
