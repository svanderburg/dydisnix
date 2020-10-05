#include "graphcol.h"
#include <glib.h>
#include <servicestable.h>
#include <targetstable2.h>
#include <target.h>
#include <distributiontable.h>
#include "partialcoloredgraph.h"
#include "partialcoloredgraph-transform.h"
#include "graphcol-approximate.h"

static void display_uncolored_graph_dot(GHashTable *services_table, GHashTable *targets_table)
{
    // Convert models to an application graph and colors
    PartialColoredGraph *graph = generate_uncolored_graph(services_table, targets_table);

    // Print graph representation in DOT format
    print_partial_colored_graph_dot(stdout, graph);

    // Cleanup
    delete_partial_colored_graph(graph);
}

static NixXML_bool display_colored_graph_dot(GHashTable *services_table, GHashTable *targets_table)
{
    NixXML_bool result;

    // Convert models to an application graph and colors
    PartialColoredGraph *graph = generate_uncolored_graph(services_table, targets_table);

    if(approximate_graph_coloring_dsatur(graph))
    {
        print_partial_colored_graph_dot(stdout, graph); // Print graph representation in DOT format
        result = TRUE;
    }
    else
        result = FALSE;

    // Cleanup
    delete_partial_colored_graph(graph);

    return result;
}

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

static NixXML_bool display_fragile_distribution(GHashTable *services_table, GHashTable *targets_table, NixXML_bool xml)
{
    NixXML_bool result;
    NixXML_bool automapped = TRUE;
    GHashTable *result_table = generate_maximal_network_link_distribution_with_graphcol_approximation(services_table, targets_table);

    if(result_table == NULL)
        result = FALSE;
    else
    {
        result = TRUE;

        /* Print output expression */

        if(xml)
            print_distribution_table_xml(stdout, result_table, 0, NULL, NULL);
        else
            print_distribution_table_nix(stdout, result_table, 0, &automapped);
    }

    /* Cleanup */
    delete_converted_colored_graph_result_table(result_table);

    return result;
}

int graphcol(char *services_xml, char *infrastructure_xml, const unsigned int flags, const OutputArtifactType artifact_type)
{
    /* Load input models */

    int exit_status = 1;
    NixXML_bool xml = flags & DYDISNIX_FLAG_XML;
    GHashTable *services_table = create_service_table(services_xml, xml);
    GHashTable *targets_table = create_targets_table2(infrastructure_xml, xml);

    if(services_table == NULL)
    {
        g_printerr("Error with opening the services model!\n");
        exit_status = 1;
    }
    else if(targets_table == NULL)
    {
        g_printerr("Error with opening the infrastructure model!\n");
        exit_status = 1;
    }
    else
    {
        switch(artifact_type)
        {
            case ARTIFACT_NIX:
                exit_status = !display_fragile_distribution(services_table, targets_table, FALSE);
                break;
            case ARTIFACT_XML:
                exit_status = !display_fragile_distribution(services_table, targets_table, TRUE);
                break;
            case ARTIFACT_GRAPH:
                display_uncolored_graph_dot(services_table, targets_table);
                exit_status = 0;
                break;
            case ARTIFACT_RESOLVED_GRAPH:
                exit_status = !display_colored_graph_dot(services_table, targets_table);
                break;
        }
    }

    delete_service_table(services_table);
    delete_targets_table(targets_table);

    return exit_status;
}
