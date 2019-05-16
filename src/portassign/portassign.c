#include "portassign.h"
#include "serviceproperties.h"
#include "infrastructureproperties.h"
#include "candidatetargetmapping.h"
#include "portconfiguration.h"
#include <unistd.h>
#include <string.h>

int portassign(gchar *services, gchar *infrastructure, gchar *distribution, gchar *ports, gchar *service_property, const unsigned int flags)
{
    int xml = flags & DYDISNIX_FLAG_XML;
    GPtrArray *service_property_array = create_service_property_array(services, xml);
    GPtrArray *targets_array = create_target_property_array(infrastructure, xml);
    GHashTable *candidate_target_table = create_candidate_target_table(distribution, infrastructure, xml);

    if(service_property_array == NULL || targets_array == NULL || candidate_target_table == NULL)
    {
        g_printerr("Error with opening one of the models!\n");
        return 1;
    }
    else
    {
        PortConfiguration *port_configuration;

        if(ports == NULL)
            port_configuration = create_empty_port_configuration(); /* If no ports config is given, initialise an empty one */
        else
            port_configuration = open_port_configuration(ports, xml); /* Otherwise, open the ports config */

        /* Clean obsolete reservations */
        clean_obsolete_reservations(port_configuration, candidate_target_table, service_property_array, service_property);

        g_print("{\n");
        g_print("  ports = {\n");

        GHashTableIter iter;
        gpointer key, value;

        g_hash_table_iter_init(&iter, candidate_target_table);
        while(g_hash_table_iter_next(&iter, &key, &value))
        {
            gchar *service_name = (gchar*)key;
            GPtrArray *targets = (GPtrArray*)value;

            Service *service = find_service(service_property_array, service_name);
            gchar *prop_value;

            if(service == NULL)
                prop_value = NULL;
            else
                prop_value = find_service_property(service, service_property);

            /* For each service that has a port property that is unassigned, assign a port */

            if(prop_value != NULL)
            {
                if(g_strcmp0(prop_value, "shared") == 0) /* If a shared port is requested, consult the shared ports pool */
                {
                    gint port = assign_or_reuse_port(port_configuration, NULL, service_name);
                    g_print("    %s = %d;\n", service_name, port);
                }
                else if(g_strcmp0(prop_value, "private") == 0) /* If a private port is requested, consult the machine's ports pool */
                {
                    if(targets->len > 0)
                    {
                        gchar *target = g_ptr_array_index(targets, 0);
                        gint port = assign_or_reuse_port(port_configuration, target, service_name);
                        g_print("    %s = %d;\n", service_name, port);
                    }
                    else
                        g_printerr("WARNING: %s is not distributed to any machine. Skipping port assignment...\n", service_name);
                }
            }
        }

        g_print("  };\n");
        print_port_configuration(port_configuration);
        g_print("}\n");

        /* Cleanup */
        delete_port_configuration(port_configuration);
        delete_service_property_array(service_property_array);
        delete_target_array(targets_array);
        delete_candidate_target_table(candidate_target_table);

        return 0;
    }
}
