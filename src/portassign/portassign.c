#include "portassign.h"
#include "serviceproperties.h"
#include "infrastructureproperties.h"
#include "candidatetargetmapping.h"
#include "portconfiguration.h"
#include <unistd.h>
#include <string.h>

int portassign(gchar *services, gchar *infrastructure, gchar *distribution, gchar *ports, gchar *service_property, int xml)
{
    GPtrArray *service_property_array = create_service_property_array(services, xml);
    GPtrArray *targets_array = create_target_property_array(infrastructure, xml);
    GPtrArray *candidate_target_array = create_candidate_target_array(infrastructure, distribution, xml);

    if(service_property_array == NULL || targets_array == NULL || candidate_target_array == NULL)
    {
        g_printerr("Error with opening one of the models!\n");
        return 1;
    }
    else
    {
        PortConfiguration port_configuration;
        unsigned int i;

        if(ports == NULL)
            init_port_configuration(&port_configuration); /* If no ports config is given, initialise an empty one */
        else
            open_port_configuration(&port_configuration, ports, xml); /* Otherwise, open the ports config */

        /* Clean obsolete reservations */
        clean_obsolete_reservations(&port_configuration, candidate_target_array, service_property_array, service_property);

        g_print("{\n");
        g_print("  ports = {\n");

        for(i = 0; i < candidate_target_array->len; i++)
        {
            DistributionItem *distribution_item = g_ptr_array_index(candidate_target_array, i);
            Service *service = find_service(service_property_array, distribution_item->service);
            ServiceProperty *prop;

            if(service == NULL)
                prop = NULL;
            else
                prop = find_service_property(service, service_property);

            /* For each service that has a port property that is unassigned, assign a port */

            if(prop != NULL)
            {
                if(g_strcmp0(prop->value, "shared") == 0) /* If a shared port is requested, consult the shared ports pool */
                {
                    gint port = assign_or_reuse_port(&port_configuration, NULL, distribution_item->service);
                    g_print("    %s = %d;\n", distribution_item->service, port);
                }
                else if(g_strcmp0(prop->value, "private") == 0) /* If a private port is requested, consult the machine's ports pool */
                {
                    if(distribution_item->targets->len > 0)
                    {
                        gchar *target = g_ptr_array_index(distribution_item->targets, 0);
                        gint port = assign_or_reuse_port(&port_configuration, target, distribution_item->service);
                        g_print("    %s = %d;\n", distribution_item->service, port);
                    }
                    else
                        g_printerr("WARNING: %s is not distributed to any machine. Skipping port assignment...\n", distribution_item->service);
                }
            }
        }

        g_print("  };\n");
        print_port_configuration(&port_configuration);
        g_print("}\n");

        destroy_port_configuration(&port_configuration);

        return 0;
    }
}
