#ifndef __DYDISNIX_APPLICATIONHOSTGRAPH_H
#define __DYDISNIX_APPLICATIONHOSTGRAPH__H
#include <glib.h>

typedef struct
{
    GHashTable *appnodes_table;
    GHashTable *hosts_table;
}
ApplicationHostGraph;

ApplicationHostGraph *create_application_host_graph(void);

void delete_application_host_graph(ApplicationHostGraph *graph);

void mark_all_nodes_unvisited(ApplicationHostGraph *graph);

#endif
