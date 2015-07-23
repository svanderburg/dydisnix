#include "portconfiguration.h"

static void delete_port_value(gpointer data)
{
    g_free(data);
}

static void delete_service_value(gpointer data)
{
    g_free(data);
}

static TargetConfig *create_target_config(gint last_port, gint min_port, gint max_port)
{
    TargetConfig *target_config = (TargetConfig*)g_malloc(sizeof(TargetConfig));
    
    target_config->last_port = last_port;
    target_config->min_port = min_port;
    target_config->max_port = max_port;
    target_config->services_to_ports = g_hash_table_new_full(g_str_hash, g_str_equal, delete_service_value, delete_port_value);
    target_config->ports_to_services = g_hash_table_new_full(g_int_hash, g_int_equal, delete_port_value, delete_service_value);
    
    return target_config;
}

static void delete_target_config(TargetConfig *target_config)
{
    g_hash_table_destroy(target_config->services_to_ports);
    g_hash_table_destroy(target_config->ports_to_services);
    g_free(target_config);
}

static void delete_target_key(gpointer data)
{
    g_free(data);
}

void init_port_configuration(PortConfiguration *port_configuration)
{
    port_configuration->global_config = NULL;
    port_configuration->target_configs = g_hash_table_new_full(g_str_hash, g_str_equal, delete_target_key, delete_target_config);
}

void destroy_port_configuration(PortConfiguration *port_configuration)
{
    delete_target_config(port_configuration->global_config);
    g_hash_table_destroy(port_configuration->target_configs);
}

gint assign_or_reuse_port(PortConfiguration *port_configuration, gchar *target, gchar *service)
{
    TargetConfig *target_config;
    gint *port;
    
    /* Get the target config */
    
    if(target == NULL)
    {
        target_config = port_configuration->global_config;
        
        /* If no global config is defined, create one */
        if(target_config == NULL)
        {
            target_config = create_target_config(3000, 3000, 4000);
            port_configuration->global_config = target_config;
        }
    }
    else
    {
        target_config = g_hash_table_lookup(port_configuration->target_configs, target);
        
        /* If no target config is defined, create one */
        if(target_config == NULL)
        {
            gchar *target_key = g_strdup(target);
            target_config = create_target_config(8000, 8000, 9000);
            g_hash_table_insert(port_configuration->target_configs, target_key, target_config);
        }
    }
    
    /* Lookup the service */
    port = g_hash_table_lookup(target_config->services_to_ports, service);
    
    if(port == NULL)
    {
        /* If no reservation exists, make one */
        gint *port_value;
        gchar *service_value;
        
        /* Look for a free port */
        
        do
        {
            target_config->last_port++;
            
            if(target_config->last_port > target_config->max_port) /* If the last port exceeds the the maximum port number, start looking from the beginning */
                target_config->last_port = target_config->min_port;
        }
        while(g_hash_table_lookup(target_config->ports_to_services, &target_config->last_port) != NULL);
        
        /* Reserve the free port */
        
        port_value = (gint*)g_malloc(sizeof(gint));
        *port_value = target_config->last_port;
        service_value = g_strdup(service);
        g_hash_table_insert(target_config->services_to_ports, service_value, port_value);
        g_hash_table_insert(target_config->ports_to_services, port_value, service_value);
        
        /* Return the reserved port */
        return *port_value;
    }
    else
        return *port; /* If port reservation exist already, return it */
}
