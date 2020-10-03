#ifndef __DYDISNIX_MULTIWAYCUT_APPROXIMATE_H
#define __DYDISNIX_MULTIWAYCUT_APPROXIMATE_H
#include <glib.h>
#include "applicationhostgraph.h"

/**
 * Applies the multiway cut approximation algorithm as described in the paper:
 * "Reliable Deployment of Component-based Applications into Distributed Environments"
 * by A. Heydarnoori and F. Mavaddat
 */
void approximate_multiway_cut_solution(ApplicationHostGraph *graph, GHashTable *distribution_table);

#endif
