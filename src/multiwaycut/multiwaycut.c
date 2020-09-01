#include "multiwaycut.h"
#include <nixxml-ghashtable-iter.h>
#include <servicestable.h>
#include <distributionmapping.h>
#include <distributiontable.h>
#include <targettoservicestable.h>
#include "applicationhostgraph.h"
#include "applicationhostgraph-transform.h"
#include "multiwaycut-approximate.h"

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

    return result_table;
}

int multiwaycut(gchar *services, gchar *distribution, gchar *infrastructure, const unsigned int flags)
{
    int exit_status;
    NixXML_bool xml = flags & DYDISNIX_FLAG_XML;
    NixXML_bool automapped;

    GHashTable *services_table = create_service_table(services, xml);
    GHashTable *distribution_table = create_distribution_table(distribution, infrastructure, xml, &automapped);

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
        GHashTable *result_table = generate_reliable_distribution_using_multiway_cut_approximation(services_table, distribution_table, target_to_services_table);

        /* Print Nix expression of the result */
        if(flags & DYDISNIX_FLAG_OUTPUT_XML)
            print_distribution_table_xml(stdout, result_table, 0, NULL, NULL);
        else
            print_distribution_table_nix(stdout, result_table, 0, &automapped);

        /* Cleanup */
        delete_application_host_graph_result_table(result_table);
        delete_target_to_services_table(target_to_services_table);

        exit_status = 0;
    }

    /* Cleanup */
    delete_distribution_table(distribution_table);
    delete_service_table(services_table);

    /* Return exit status */
    return exit_status;
}
