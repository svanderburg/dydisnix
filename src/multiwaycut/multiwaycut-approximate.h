#ifndef __DYDISNIX_MULTIWAYCUT_APPROXIMATE_H
#define __DYDISNIX_MULTIWAYCUT_APPROXIMATE_H
#include <glib.h>
#include "applicationhostgraph.h"

void approximate_multiway_cut_solution(ApplicationHostGraph *graph, GHashTable *distribution_table);

#endif
