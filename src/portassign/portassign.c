#include "portassign.h"
#include "servicestable.h"
#include "targetstable2.h"
#include "distributiontable.h"
#include "portconfiguration.h"
#include "portdistribution.h"
#include <unistd.h>
#include <string.h>
#include <nixxml-print-nix.h>
#include <nixxml-print-xml.h>

typedef struct
{
    GHashTable *port_distribution_table;

    PortConfiguration *port_configuration;
}
PortsAssignment;

static void clean_ports_assignment(PortsAssignment *assignment)
{
    delete_port_distribution_table(assignment->port_distribution_table);
    delete_port_configuration(assignment->port_configuration);
}

static void print_ports_assignment_attributes_nix(FILE *file, const void *value, const int indent_level, void *userdata, NixXML_PrintValueFunc print_value)
{
    const PortsAssignment *assignment = (const PortsAssignment*)value;
    NixXML_print_attribute_nix(file, "ports", assignment->port_distribution_table, indent_level, userdata, print_port_distribution_table_nix);
    NixXML_print_attribute_nix(file, "portConfiguration", assignment->port_configuration, indent_level, userdata, print_port_configuration_nix);
}

static void print_ports_assignment_nix(const PortsAssignment *assignment)
{
    NixXML_print_attrset_nix(stdout, assignment, 0, NULL, print_ports_assignment_attributes_nix, NULL);
}

static void print_ports_assignment_attributes_xml(FILE *file, const void *value, const int indent_level, const char *type_property_name, void *userdata, NixXML_PrintXMLValueFunc print_value)
{
    const PortsAssignment *assignment = (const PortsAssignment*)value;
    NixXML_print_simple_attribute_xml(file, "ports", assignment->port_distribution_table, indent_level, type_property_name, userdata, print_port_distribution_table_xml);
    NixXML_print_simple_attribute_xml(file, "portConfiguration", assignment->port_configuration, indent_level, type_property_name, userdata, print_port_configuration_xml);
}

static void print_ports_assignment_xml(const PortsAssignment *assignment)
{
    NixXML_print_open_root_tag(stdout, "portAssignment");
    NixXML_print_simple_attrset_xml(stdout, assignment, 0, NULL, NULL, print_ports_assignment_attributes_xml, NULL);
    NixXML_print_close_root_tag(stdout, "portAssignment");
}

int portassign(gchar *services, gchar *infrastructure, gchar *distribution, gchar *ports, gchar *service_property, const unsigned int flags)
{
    NixXML_bool automapped;
    NixXML_bool xml = flags & DYDISNIX_FLAG_XML;
    GHashTable *service_table = create_service_table(services, xml);
    GHashTable *targets_table = create_targets_table2(infrastructure, xml);
    GHashTable *distribution_table = create_distribution_table(distribution, infrastructure, xml, &automapped);

    if(service_table == NULL || targets_table == NULL || distribution_table == NULL)
    {
        g_printerr("Error with opening one of the models!\n");
        return 1;
    }
    else
    {
        PortsAssignment assignment;

        if(ports == NULL)
            assignment.port_configuration = create_empty_port_configuration(); /* If no ports config is given, initialise an empty one */
        else
            assignment.port_configuration = open_port_configuration(ports, xml); /* Otherwise, open the ports config */

        /* Clean obsolete reservations */
        clean_obsolete_reservations(assignment.port_configuration, distribution_table, service_table, service_property);

        /* Create ports distribution table */
        assignment.port_distribution_table = create_port_distribution_table(assignment.port_configuration, service_table, distribution_table, service_property);

        /* Print ports configuration */
        if(flags & DYDISNIX_OPTION_OUTPUT_XML)
            print_ports_assignment_xml(&assignment);
        else
            print_ports_assignment_nix(&assignment);

        /* Cleanup */
        clean_ports_assignment(&assignment);
        delete_service_table(service_table);
        delete_targets_table(targets_table);
        delete_distribution_table(distribution_table);

        return 0;
    }
}
