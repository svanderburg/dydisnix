#ifndef __DYDISNIX_GRAPHCOL_APPROXIMATE_H
#define __DYDISNIX_GRAPHCOL_APPROXIMATE_H
#include <nixxml-types.h>
#include "partialcoloredgraph.h"

/**
 * Applies the Dsatur graph coloring approximation algorithm, as described
 * in the paper: "New Methods to Color the Vertices of a Graph" by Daniel
 * Brelaz.
 *
 * @param graph A partial colored graph instance
 * @return TRUE if all vertexes were colored, FALSE otherwise
 */
NixXML_bool approximate_graph_coloring_dsatur(PartialColoredGraph *graph);

#endif
