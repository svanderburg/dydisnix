#ifndef __DYDISNIX_APPLICATIONHOSTGRAPH_TRANSFORM_H
#define __DYDISNIX_APPLICATIONHOSTGRAPH_TRANSFORM_H
#include <glib.h>
#include "applicationhostgraph.h"

/**
 * Transforms a services model (including dependencies between services) and
 * a distribution model (that maps services to machines in the network) to a
 * weighted undirected graph.
 *
 * Every service becomes an application node, every target machine a host node.
 *
 * Dependencies between services translate to bidirectional links with a weight
 * of: 1
 *
 * A candidate mapping between a service and target machine translates to a
 * bidirectional link with a weight of: n^2 representing a very large
 * number.
 *
 * This transformation approach is described in the paper:
 * "Reliable Deployment of Component-based Applications into Distributed Environments"
 * by A. Heydarnoori and F. Mavaddat
 *
 * @param services_table A hash table with services
 * @param distribution_table A hash table that maps services to zero or more target machines in the network.
 * @param target_to_services_table The inverse model of the distribution_table, specifying for each machine that services that it can host.
 * @return The application-host graph, a weighted undirected graph
 */
ApplicationHostGraph *generate_application_host_graph(GHashTable *services_table, GHashTable *distribution_table, GHashTable *target_to_services_table);

GHashTable *generate_distribution_table_from_application_host_graph(ApplicationHostGraph *graph);

void delete_application_host_graph_result_table(GHashTable *result_table);

#endif
