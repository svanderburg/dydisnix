#include "portassign.h"
#include "serviceproperties.h"
#include "infrastructureproperties.h"
#include "candidatetargetmapping.h"
#include "portconfiguration.h"

int portassign(gchar *service_xml, gchar *infrastructure_xml, gchar *distribution_xml, gchar *service_property)
{
    GPtrArray *service_property_array = create_service_property_array(service_xml);
    GPtrArray *infrastructure_property_array = create_infrastructure_property_array(infrastructure_xml);
    GPtrArray *candidate_target_array = create_candidate_target_array(distribution_xml);
    
    if(service_property_array == NULL || infrastructure_property_array == NULL || candidate_target_array == NULL)
    {
        g_printerr("Error with opening one of the models!\n");
        return 1;
    }
    else
    {
        /* TODO: open existing file */
        
        PortConfiguration port_configuration;
        unsigned int i;
        
        init_port_configuration(&port_configuration);
        
        g_print("{\n");
        
        for(i = 0; i < candidate_target_array->len; i++)
        {
            DistributionItem *distribution_item = g_ptr_array_index(candidate_target_array, i);
            Service *service = find_service(service_property_array, distribution_item->service);
            ServiceProperty *prop = find_service_property(service, service_property);
            
            /* For each service that has a port property that is unassigned, assign a port */

            if(prop != NULL)
            {
                if(g_strcmp0(prop->value, "shared") == 0 || g_strcmp0(prop->value, "") == 0) /* If a shared port is requested, consult the shared ports pool */
                {
                    gint port = assign_or_reuse_port(&port_configuration, NULL, distribution_item->service);
                    g_print("  %s = %d;\n", distribution_item->service, port);
                }
                else if(g_strcmp0(prop->value, "private") == 0) /* If a private port is requested, consult the machine's ports pool */
                {
                    if(distribution_item->targets->len > 0)
                    {
                        gchar *target = g_ptr_array_index(distribution_item->targets, 0);
                        gint port = assign_or_reuse_port(&port_configuration, target, distribution_item->service);
                        g_print("  %s = %d;\n", distribution_item->service, port);
                    }
                    else
                        g_printerr("WARNING: %s is not distributed to any machine. Skipping port assignment...\n");
                }
                else /* If a port has been assigned already, reuse the value */
                    g_print("  %s = %s;\n", distribution_item->service, prop->value);
            }
        }
        
        g_print("}\n");
        
        destroy_port_configuration(&port_configuration);
        
        return 0;
    }
}
