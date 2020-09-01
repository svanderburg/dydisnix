#ifndef __DYDISNIX_APPLICATIONHOSTGRAPH_TRANSFORM_H
#define __DYDISNIX_APPLICATIONHOSTGRAPH_TRANSFORM_H
#include <glib.h>
#include "applicationhostgraph.h"

ApplicationHostGraph *generate_application_host_graph(GHashTable *services_table, GHashTable *candidate_target_table, GHashTable *target_to_services_table);

GHashTable *generate_distribution_table_from_application_host_graph(ApplicationHostGraph *graph);

void delete_application_host_graph_result_table(GHashTable *result_table);

#endif
