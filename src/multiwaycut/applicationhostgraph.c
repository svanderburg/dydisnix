#include "applicationhostgraph.h"

ApplicationHostGraph *create_application_host_graph(void)
{
    ApplicationHostGraph *graph = (ApplicationHostGraph*)g_malloc(sizeof(ApplicationHostGraph));
    graph->host_nodes_table = g_hash_table_new(g_str_hash, g_str_equal);
    graph->app_nodes_table = g_hash_table_new(g_str_hash, g_str_equal);
    return graph;
}

static void delete_nodes_table(GHashTable *table)
{
    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        Node *node = (Node*)value;
        delete_app_or_host_node(node);
    }

    g_hash_table_destroy(table);
}

void delete_application_host_graph(ApplicationHostGraph *graph)
{
    if(graph != NULL)
    {
        delete_nodes_table(graph->host_nodes_table);
        delete_nodes_table(graph->app_nodes_table);
        g_free(graph);
    }
}

static void mark_all_nodes_unvisited_in_table(GHashTable *table)
{
    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        Node *node = (Node*)value;
        node->visited = FALSE;
    }
}

void mark_all_nodes_unvisited(ApplicationHostGraph *graph)
{
    mark_all_nodes_unvisited_in_table(graph->host_nodes_table);
    mark_all_nodes_unvisited_in_table(graph->app_nodes_table);
}

Node *create_app_node(gchar *name)
{
    gchar *node_name = g_strconcat("app:", name, NULL);
    return create_node_with_value(node_name, TRUE);
}

Node *create_host_node(gchar *name)
{
    gchar *node_name = g_strconcat("host:", name, NULL);
    return create_node_with_value(node_name, FALSE);
}

void delete_app_or_host_node(Node *node)
{
    if(node != NULL)
    {
        g_free(node->name);
        delete_node(node);
    }
}

NixXML_bool node_is_app_node(Node *node)
{
    return node->value;
}

static void print_nodes_table(FILE *file, GHashTable *nodes_table)
{
    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, nodes_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        Node *node = (Node*)value;
        print_node_with_name_dot(file, node);
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
        print_node_edges_with_annotations_dot(file, node);
    }
}

void print_application_host_graph_dot(FILE *file, ApplicationHostGraph *graph)
{
    fprintf(file, "graph {\n");
    print_nodes_table(file, graph->app_nodes_table);
    print_nodes_table(file, graph->host_nodes_table);
    print_undirected_node_links(file, graph->app_nodes_table);
    print_undirected_node_links(file, graph->host_nodes_table);
    fprintf(file, "}\n");
}
