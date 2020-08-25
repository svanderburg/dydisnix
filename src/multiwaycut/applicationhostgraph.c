#include "applicationhostgraph.h"

ApplicationHostGraph *create_application_host_graph(void)
{
    ApplicationHostGraph *graph = (ApplicationHostGraph*)g_malloc(sizeof(ApplicationHostGraph));
    graph->hosts_table = g_hash_table_new(g_str_hash, g_str_equal);
    graph->appnodes_table = g_hash_table_new(g_str_hash, g_str_equal);
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
        delete_node(node);
    }

    g_hash_table_destroy(table);
}

void delete_application_host_graph(ApplicationHostGraph *graph)
{
    if(graph != NULL)
    {
        delete_nodes_table(graph->hosts_table);
        delete_nodes_table(graph->appnodes_table);
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
    mark_all_nodes_unvisited_in_table(graph->hosts_table);
    mark_all_nodes_unvisited_in_table(graph->appnodes_table);
}

Node *create_app_node(gchar *name)
{
    return create_node_with_value(name, TRUE);
}

Node *create_host_node(gchar *name)
{
    return create_node_with_value(name, FALSE);
}

NixXML_bool node_is_app_node(Node *node)
{
    return node->value;
}
