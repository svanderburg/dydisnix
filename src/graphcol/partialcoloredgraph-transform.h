#ifndef __DYDISNIX_PARTIALCOLOREDGRAPH_TRANSFORM_H
#define __DYDISNIX_PARTIALCOLOREDGRAPH_TRANSFORM_H
#include <glib.h>
#include "partialcoloredgraph.h"

/**
 * This function converts a candidate distribution (a cartesian product of all
 * services in the services table to the targets in the targets table) to an
 * undirected partially colored graph.
 *
 * Each node in the graph refers to a service, each service that has a dependency
 * on another service, translates to a bidirectional link, and each target machine
 * translates to a color.
 *
 * @param services_table A hash table with all service instances
 * @param targets_table A table with all target machines
 * @return A partially colored undirected graph, in which no node is colored yet
 */
PartialColoredGraph *generate_uncolored_graph(GHashTable *services_table, GHashTable *targets_table);

GHashTable *convert_colored_graph_to_distribution_table(PartialColoredGraph *graph);

void delete_converted_colored_graph_result_table(GHashTable *result_table);

#endif
