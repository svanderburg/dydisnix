#ifndef __DYDISNIX_APPLICATIONHOSTGRAPH_H
#define __DYDISNIX_APPLICATIONHOSTGRAPH__H
#include <glib.h>
#include "node.h"

typedef struct
{
    GHashTable *appnodes_table;
    GHashTable *hosts_table;
}
ApplicationHostGraph;

ApplicationHostGraph *create_application_host_graph(void);

void delete_application_host_graph(ApplicationHostGraph *graph);

void mark_all_nodes_unvisited(ApplicationHostGraph *graph);

Node *create_app_node(gchar *name);

Node *create_host_node(gchar *name);

NixXML_bool node_is_app_node(Node *node);

#endif
