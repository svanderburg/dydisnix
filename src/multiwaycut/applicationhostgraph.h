#ifndef __DYDISNIX_APPLICATIONHOSTGRAPH_H
#define __DYDISNIX_APPLICATIONHOSTGRAPH_H
#include <glib.h>
#include <stdio.h>
#include "node.h"

typedef struct
{
    GHashTable *app_nodes_table;
    GHashTable *host_nodes_table;
}
ApplicationHostGraph;

ApplicationHostGraph *create_application_host_graph(void);

void delete_application_host_graph(ApplicationHostGraph *graph);

void mark_all_nodes_unvisited(ApplicationHostGraph *graph);

Node *create_app_node(gchar *name);

Node *create_host_node(gchar *name);

void delete_app_or_host_node(Node *node);

NixXML_bool node_is_app_node(Node *node);

void print_application_host_graph_dot(FILE *file, ApplicationHostGraph *graph);

#endif
