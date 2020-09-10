#include "idassign.h"
#include <servicestable.h>
#include <nixxml-node.h>
#include <nixxml-ghashtable-iter.h>
#include <distributiontable.h>
#include <targettoservicestable.h>
#include "idresourcetype.h"
#include "idresourcestable.h"
#include "idassignmentstable.h"
#include "idassignmentsperresourcetable.h"
#include "assignservices.h"
#include "assigndistribution.h"

static NixXML_bool print_assignments_per_resource_table(GHashTable *id_assignments_per_resource_table, const char *output_file, NixXML_bool xml, NixXML_bool automapped)
{
    FILE *file;

    if(output_file == NULL)
        file = stdout;
    else
        file = fopen(output_file, "w");

    if(file == NULL)
    {
        g_printerr("Cannot write to output file: %s\n", output_file);
        return FALSE;
    }

    if(xml)
        print_id_assignments_per_resource_table_xml(file, id_assignments_per_resource_table, 0, NULL, NULL);
    else
        print_id_assignments_per_resource_table_nix(file, id_assignments_per_resource_table, 0, &automapped);

    if(output_file != NULL)
        fclose(file);

    return TRUE;
}

static int idassign_to_services(gchar *services, gchar *resources, gchar *ids, gchar *service_property, const char *output_file, const unsigned int flags)
{
    int exit_status = 0;
    NixXML_bool xml = flags & DYDISNIX_FLAG_XML;

    GHashTable *services_table = create_service_table(services, xml);
    GHashTable *id_resources_table = create_id_resources_table(resources, xml);

    GHashTable *id_assignments_per_resource_table;

    if(ids == NULL)
        id_assignments_per_resource_table = create_empty_id_assignments_per_resource_table();
    else
        id_assignments_per_resource_table = create_id_assignments_per_resource_table(ids, xml);

    if(services_table == NULL)
    {
        g_printerr("Error with opening the services model!\n");
        exit_status = 1;
    }
    else if(id_resources_table == NULL)
    {
        g_printerr("Error with opening the ID resources model!\n");
        exit_status = 1;
    }
    else if(id_assignments_per_resource_table == NULL)
    {
        g_printerr("Error with opening the IDs model!\n");
        exit_status = 1;
    }
    else
    {
        remove_invalid_service_id_assignments_per_resource(id_assignments_per_resource_table, services_table, id_resources_table, service_property);

        if(assign_ids_to_services(id_resources_table, id_assignments_per_resource_table, services_table, service_property))
        {
           NixXML_bool automapped = TRUE;
            if(!print_assignments_per_resource_table(id_assignments_per_resource_table, output_file, flags & DYDISNIX_FLAG_OUTPUT_XML, automapped))
                exit_status = 1;
        }
        else
            exit_status = 1;
    }

    // Cleanup
    delete_id_assignments_per_resource_table(id_assignments_per_resource_table);
    delete_id_resources_table(id_resources_table);
    delete_service_table(services_table);

    return exit_status;
}

static int idassign_to_distribution(gchar *services, gchar *infrastructure, gchar *distribution, gchar *resources, gchar *ids, gchar *service_property, const char *output_file, const unsigned int flags)
{
    int exit_status = 0;
    NixXML_bool automapped;
    NixXML_bool xml = flags & DYDISNIX_FLAG_XML;

    GHashTable *services_table = create_service_table(services, xml);
    GHashTable *distribution_table = create_distribution_table(distribution, infrastructure, xml, &automapped);

    GHashTable *id_resources_table = create_id_resources_table(resources, xml);

    GHashTable *id_assignments_per_resource_table;

    if(ids == NULL)
        id_assignments_per_resource_table = create_empty_id_assignments_per_resource_table();
    else
        id_assignments_per_resource_table = create_id_assignments_per_resource_table(ids, xml);

    if(services_table == NULL)
    {
        g_printerr("Error with opening the services model!\n");
        exit_status = 1;
    }
    if(distribution_table == NULL)
    {
        g_printerr("Error with opening the distribution model!\n");
        exit_status = 1;
    }
    else if(id_resources_table == NULL)
    {
        g_printerr("Error with opening the ID resources model!\n");
        exit_status = 1;
    }
    else if(id_assignments_per_resource_table == NULL)
    {
        g_printerr("Error with opening the IDs model!\n");
        exit_status = 1;
    }
    else
    {
        GHashTable *target_to_services_table = create_target_to_services_table(distribution_table);

        remove_invalid_distribution_id_assignments_per_resource(id_assignments_per_resource_table, services_table, distribution_table, id_resources_table, service_property);

        if(assign_ids_to_distributed_services(id_resources_table, id_assignments_per_resource_table, services_table, distribution_table, target_to_services_table, service_property))
        {
            if(!print_assignments_per_resource_table(id_assignments_per_resource_table, output_file, flags & DYDISNIX_FLAG_OUTPUT_XML, automapped))
                exit_status = 1;
        }
        else
            exit_status = 1;

        delete_target_to_services_table(target_to_services_table);
    }

    // Cleanup
    delete_id_assignments_per_resource_table(id_assignments_per_resource_table);
    delete_id_resources_table(id_resources_table);
    delete_distribution_table(distribution_table);
    delete_service_table(services_table);

    return exit_status;
}

int idassign(gchar *services, gchar *infrastructure, gchar *distribution, gchar *resources, gchar *ids, gchar *service_property, const char *output_file, const unsigned int flags)
{
    if(distribution == NULL)
        return idassign_to_services(services, resources, ids, service_property, output_file, flags);
    else
        return idassign_to_distribution(services, infrastructure, distribution, resources, ids, service_property, output_file, flags);
}
