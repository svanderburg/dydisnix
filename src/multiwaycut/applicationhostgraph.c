#include "applicationhostgraph.h"
#include "node.h"

ApplicationHostGraph *create_application_host_graph(void)
{
    ApplicationHostGraph *graph = (ApplicationHostGraph*)g_malloc(sizeof(ApplicationHostGraph));
    graph->hosts_table = g_hash_table_new(g_str_hash, g_str_equal);
    graph->appnodes_table = g_hash_table_new(g_str_hash, g_str_equal);
    return graph;
}

void delete_application_host_graph(ApplicationHostGraph *graph)
{
    if(graph != NULL)
    {
        g_hash_table_destroy(graph->hosts_table);
        g_hash_table_destroy(graph->appnodes_table);
        g_free(graph);
    }
}

void mark_all_nodes_unvisited(ApplicationHostGraph *graph)
{
    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, graph->hosts_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        Node *host_node = (Node*)value;
        host_node->visited = FALSE;
    }

    g_hash_table_iter_init(&iter, graph->appnodes_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        Node *app_node = (Node*)value;
        app_node->visited = FALSE;
    }
}
