#ifndef __DYDISNIX_PARTIALCOLOREDGRAPH_TRANSFORM_H
#define __DYDISNIX_PARTIALCOLOREDGRAPH_TRANSFORM_H
#include <glib.h>
#include "partialcoloredgraph.h"

PartialColoredGraph *generate_uncolored_graph(GHashTable *services_table, GHashTable *targets_table);

GHashTable *convert_colored_graph_to_distribution_table(PartialColoredGraph *graph);

void delete_converted_colored_graph_result_table(GHashTable *result_table);

#endif
