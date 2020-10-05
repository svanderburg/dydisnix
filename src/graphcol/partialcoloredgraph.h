#ifndef __DYDISNIX_PARTIALCOLOREDGRAPH_H
#define __DYDISNIX_PARTIALCOLOREDGRAPH_H
#include <stdio.h>
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

void print_partial_colored_graph_dot(FILE *file, PartialColoredGraph *graph);

#endif
