#ifndef __DYDISNIX_PARTIALCOLOREDGRAPH_H
#define __DYDISNIX_PARTIALCOLOREDGRAPH_H
#include <glib.h>

typedef struct
{
    GHashTable *uncolored_nodes_table;
    GHashTable *colored_nodes_table;
    GPtrArray *colors;
}
PartialColoredGraph;

PartialColoredGraph *create_partial_colored_graph(void);

void delete_partial_colored_graph(PartialColoredGraph *graph);

#endif
