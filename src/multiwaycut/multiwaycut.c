#include "multiwaycut.h"
#include <nixxml-ghashtable-iter.h>
#include <servicestable.h>
#include <distributionmapping.h>
#include <distributiontable.h>
#include <targettoservicestable.h>
#include "applicationhostgraph.h"
#include "applicationhostgraph-transform.h"
#include "multiwaycut-approximate.h"

static void display_application_host_graph_dot(GHashTable *services_table, GHashTable *distribution_table, GHashTable *target_to_services_table)
{
    ApplicationHostGraph *graph = generate_application_host_graph(services_table, distribution_table, target_to_services_table);
    print_application_host_graph_dot(stdout, graph);
    delete_application_host_graph(graph);
}

static void display_resolved_application_host_graph_dot(GHashTable *services_table, GHashTable *distribution_table, GHashTable *target_to_services_table)
{
    // Convert deployment models to a host/application graph
    ApplicationHostGraph *graph = generate_application_host_graph(services_table, distribution_table, target_to_services_table);

    // Approximate a solution for the multiway cut problem
    approximate_multiway_cut_solution(graph, distribution_table);

    print_application_host_graph_dot(stdout, graph);
    delete_application_host_graph(graph);
}

static GHashTable *generate_reliable_distribution_using_multiway_cut_approximation(GHashTable *services_table, GHashTable *distribution_table, GHashTable *target_to_services_table)
{
    // Convert deployment models to a host/application graph
    ApplicationHostGraph *graph = generate_application_host_graph(services_table, distribution_table, target_to_services_table);

    // Approximate a solution for the multiway cut problem
    approximate_multiway_cut_solution(graph, distribution_table);

    // Convert back to mapping table and return the result
    GHashTable *result_table = generate_distribution_table_from_application_host_graph(graph);

    // Cleanup
    delete_application_host_graph(graph);

    // Return result table
    return result_table;
}

static void display_reliable_distribution(GHashTable *services_table, GHashTable *distribution_table, GHashTable *target_to_services_table, NixXML_bool xml)
{
    GHashTable *result_table = generate_reliable_distribution_using_multiway_cut_approximation(services_table, distribution_table, target_to_services_table);
    NixXML_bool automapped = TRUE;

    /* Print Nix expression of the result */
    if(xml)
        print_distribution_table_xml(stdout, result_table, 0, NULL, NULL);
    else
        print_distribution_table_nix(stdout, result_table, 0, &automapped);

    delete_application_host_graph_result_table(result_table);
}

int multiwaycut(gchar *services, gchar *distribution, gchar *infrastructure, gchar *extra_params, const unsigned int flags, const OutputArtifactType artifact_type)
{
    int exit_status;
    NixXML_bool xml = flags & DYDISNIX_FLAG_XML;
    NixXML_bool automapped;

    GHashTable *services_table = create_service_table(services, extra_params, xml);
    GHashTable *distribution_table = create_distribution_table(distribution, infrastructure, extra_params, xml, &automapped);

    if(services_table == NULL)
    {
        g_printerr("Error opening the services model!\n");
        exit_status = 1;
    }
    else if(distribution_table == NULL)
    {
        g_printerr("Error opening the distribution model!\n");
        exit_status = 1;
    }
    else
    {
        GHashTable *target_to_services_table = create_target_to_services_table(distribution_table);

        switch(artifact_type)
        {
            case ARTIFACT_NIX:
                display_reliable_distribution(services_table, distribution_table, target_to_services_table, FALSE);
                break;
            case ARTIFACT_XML:
                display_reliable_distribution(services_table, distribution_table, target_to_services_table, TRUE);
                break;
            case ARTIFACT_GRAPH:
                display_application_host_graph_dot(services_table, distribution_table, target_to_services_table);
                break;
            case ARTIFACT_RESOLVED_GRAPH:
                display_resolved_application_host_graph_dot(services_table, distribution_table, target_to_services_table);
                break;
        }

        /* Cleanup */
        delete_target_to_services_table(target_to_services_table);
        exit_status = 0;
    }

    /* Cleanup */
    delete_distribution_table(distribution_table);
    delete_service_table(services_table);

    /* Return exit status */
    return exit_status;
}
