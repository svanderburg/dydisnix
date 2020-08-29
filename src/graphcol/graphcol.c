#include "graphcol.h"
#include <glib.h>
#include <servicestable.h>
#include <targetstable2.h>
#include <target.h>
#include <distributiontable.h>
#include "partialcoloredgraph.h"
#include "partialcoloredgraph-transform.h"
#include "graphcol-approximate.h"

static GHashTable *generate_maximal_network_link_distribution_with_graphcol_approximation(GHashTable *services_table, GHashTable *targets_table)
{
    GHashTable *result_table;

    // Convert models to an application graph and colors
    PartialColoredGraph *graph = generate_uncolored_graph(services_table, targets_table);

    // Execute approximation algorithm
    if(approximate_graph_coloring_dsatur(graph))
        result_table = convert_colored_graph_to_distribution_table(graph); // Convert back to distribution model
    else
        result_table = NULL;

    // Cleanup
    delete_partial_colored_graph(graph);

    return result_table;
}

int graphcol(char *services_xml, char *infrastructure_xml, const unsigned int flags)
{
    /* Load input models */

    NixXML_bool xml = flags & DYDISNIX_FLAG_XML;
    GHashTable *services_table = create_service_table(services_xml, xml);
    GHashTable *targets_table = create_targets_table2(infrastructure_xml, xml);

    if(services_table == NULL || targets_table == NULL)
    {
        g_printerr("Error opening one of the input models!\n");
        return 1;
    }
    else
    {
        NixXML_bool automapped = TRUE;
        int exit_status;

        /* Generate distribution using graph coloring approximation */
        GHashTable *result_table = generate_maximal_network_link_distribution_with_graphcol_approximation(services_table, targets_table);

        if(result_table == NULL)
            exit_status = 1;
        else
        {
            exit_status = 0;

            /* Print output expression */

            if(flags & DYDISNIX_FLAG_OUTPUT_XML)
                print_distribution_table_xml(stdout, result_table, 0, NULL, NULL);
            else
                print_distribution_table_nix(stdout, result_table, 0, &automapped);
        }

        /* Cleanup */

        delete_converted_colored_graph_result_table(result_table);
        delete_service_table(services_table);
        delete_targets_table(targets_table);

        return exit_status;
    }
}
