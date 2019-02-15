#include "portconfiguration.h"
#include <stdlib.h>
#include <string.h>
#include <targetmapping.h>
#include <serviceproperties.h>
#include <candidatetargetmapping.h>
#include <xmlutil.h>
#include <procreact_future.h>

static ProcReact_Future generate_ports_xml_from_expr_async(char *ports_expr)
{
    ProcReact_Future future = procreact_initialize_future(procreact_create_string_type());

    if(future.pid == 0)
    {
        char *const args[] = {"dydisnix-xml", "-p", ports_expr, "--no-out-link", NULL};
        dup2(future.fd, 1); /* Attach write-end to stdout */
        execvp(args[0], args);
        _exit(1);
    }

    return future;
}

char *generate_ports_xml_from_expr(char *ports_expr)
{
    ProcReact_Status status;
    ProcReact_Future future = generate_ports_xml_from_expr_async(ports_expr);
    char *path = procreact_future_get(&future, &status);
    path[strlen(path) - 1] = '\0';
    return path;
}

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
    target_config->ports_to_services = g_hash_table_new(g_int_hash, g_int_equal);
    
    return target_config;
}

static void delete_target_config(TargetConfig *target_config)
{
    if(target_config != NULL)
    {
        g_hash_table_destroy(target_config->services_to_ports);
        g_hash_table_destroy(target_config->ports_to_services);
        g_free(target_config);
    }
}

static void delete_target_key(gpointer data)
{
    g_free(data);
}

static void delete_config_value(gpointer data)
{
    TargetConfig *target_config = (TargetConfig*)data;
    delete_target_config(target_config);
}

void init_port_configuration(PortConfiguration *port_configuration)
{
    port_configuration->global_config = NULL;
    port_configuration->target_configs = g_hash_table_new_full(g_str_hash, g_str_equal, delete_target_key, delete_config_value);
}

static TargetConfig *parse_target_config(xmlNodePtr config_node)
{
    gint last_port = 0;
    gint min_port = 0;
    gint max_port = 0;
    
    /* Construct a target config */
    TargetConfig *target_config = create_target_config(last_port, min_port, max_port);
    
    /* Iterate over the config node's children */
    xmlNodePtr config_node_children = config_node->children;
    
    while(config_node_children != NULL)
    {
        if(xmlStrcmp(config_node_children->name, (xmlChar*) "lastPort") == 0)
            last_port = atoi((char*)config_node_children->children->content);
        else if(xmlStrcmp(config_node_children->name, (xmlChar*) "minPort") == 0)
            min_port = atoi((char*)config_node_children->children->content);
        else if(xmlStrcmp(config_node_children->name, (xmlChar*) "maxPort") == 0)
            max_port = atoi((char*)config_node_children->children->content);
        else if(xmlStrcmp(config_node_children->name, (xmlChar*) "servicesToPorts") == 0)
        {
            xmlNodePtr service_to_ports_children = config_node_children->children;
            
            while(service_to_ports_children != NULL)
            {
                if(xmlStrcmp(service_to_ports_children->name, (xmlChar*) "service") == 0)
                {
                    gint *port = (gint*)g_malloc(sizeof(gint));
                    gchar *service = NULL;
                    
                    xmlAttrPtr service_properties = service_to_ports_children->properties;
                    *port = atoi((char*)service_to_ports_children->children->content);
                    
                    while(service_properties != NULL)
                    {
                        if(xmlStrcmp(service_properties->name, (xmlChar*) "name") == 0)
                            service = g_strdup((gchar*)service_properties->children->content);
                        
                        service_properties = service_properties->next;
                    }
                    
                    g_hash_table_insert(target_config->services_to_ports, service, port);
                    g_hash_table_insert(target_config->ports_to_services, port, service);
                }
                
                service_to_ports_children = service_to_ports_children->next;
            }
        }
        
        config_node_children = config_node_children->next;
    }

    /* Set remainder of configuration values */
    target_config->last_port = last_port;
    target_config->min_port = min_port;
    target_config->max_port = max_port;
    
    /* Return parsed target config */
    return target_config;
}

int open_port_configuration_from_xml(PortConfiguration *port_configuration, const gchar *port_configuration_file)
{
    /* Declarations */
    xmlDocPtr doc;
    xmlNodePtr node_root;
    xmlXPathObjectPtr result;

    /* Parse the XML document */
    
    if((doc = xmlParseFile(port_configuration_file)) == NULL)
    {
        g_printerr("Error with parsing the port configuration XML file!\n");
        xmlCleanupParser();
        return FALSE;
    }
    
    /* Check if the document has a root */
    node_root = xmlDocGetRootElement(doc);
    
    if(node_root == NULL)
    {
        g_printerr("The port configuration XML file is empty!\n");
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return FALSE;
    }
    
    /* Query the global config properties */
    result = executeXPathQuery(doc, "/portConfiguration/globalConfig");
    
    if(result)
    {
        unsigned int i;
        xmlNodeSetPtr nodeset = result->nodesetval;
        
        for(i = 0; i < nodeset->nodeNr; i++)
            port_configuration->global_config = parse_target_config(nodeset->nodeTab[i]);
    }
    else
        port_configuration->global_config = NULL;
    
    /* Query the target config properties */
    result = executeXPathQuery(doc, "/portConfiguration/targetConfigs");
    
    port_configuration->target_configs = g_hash_table_new_full(g_str_hash, g_str_equal, delete_target_key, delete_config_value);
    
    if(result)
    {
        unsigned int i;
        xmlNodeSetPtr nodeset = result->nodesetval;
        
        for(i = 0; i < nodeset->nodeNr; i++)
        {
            xmlNodePtr target_node_children = nodeset->nodeTab[i]->children;
            
            while(target_node_children != NULL)
            {
                if(xmlStrcmp(target_node_children->name, (xmlChar *) "text") != 0)
                {
                    TargetConfig *target_config = parse_target_config(target_node_children);
                    gchar *target = g_strdup((gchar*)target_node_children->name);
                
                    g_hash_table_insert(port_configuration->target_configs, target, target_config);
                }
                
                target_node_children = target_node_children->next;
            }
        }
    }
    
    /* Cleanup */
    xmlFreeDoc(doc);
    xmlCleanupParser();
    
    return TRUE;
}

int open_port_configuration_from_nix(PortConfiguration *port_configuration, gchar *port_configuration_file)
{
    char *ports_xml = generate_ports_xml_from_expr(port_configuration_file);
    int status = open_port_configuration_from_xml(port_configuration, ports_xml);
    free(ports_xml);
    return status;
}

int open_port_configuration(PortConfiguration *port_configuration, gchar *port_configuration_file, int xml)
{
    if(xml)
        return open_port_configuration_from_xml(port_configuration, port_configuration_file);
    else
        return open_port_configuration_from_nix(port_configuration, port_configuration_file);
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

static void print_indent(unsigned int indent_level)
{
    unsigned int i;
    
    for(i = 0; i < indent_level; i++)
        g_print("  ");
}

static void print_target_config(TargetConfig *target_config, unsigned int indent_level) {
    GHashTableIter iter;
    gpointer key, value;
    
    print_indent(indent_level);
    g_print("lastPort = %d;\n", target_config->last_port);
    print_indent(indent_level);
    g_print("minPort = %d;\n", target_config->min_port);
    print_indent(indent_level);
    g_print("maxPort = %d;\n", target_config->max_port);
    print_indent(indent_level);
    g_print("servicesToPorts = {\n");
    
    g_hash_table_iter_init(&iter, target_config->services_to_ports);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        gint *port = (gint*)value;
        print_indent(indent_level + 1);
        g_print("%s = %d;\n", (gchar*)key, *port);
    }
    
    print_indent(indent_level);
    g_print("};\n");
}

void print_port_configuration(PortConfiguration *port_configuration)
{
    GHashTableIter iter;
    gpointer key, value;

    g_print("  portConfiguration = {\n");
    
    if(port_configuration->global_config != NULL)
    {
        g_print("    globalConfig = {\n");
        print_target_config(port_configuration->global_config, 3);
        g_print("    };\n");
    }
    
    g_print("    targetConfigs = {\n");
    g_hash_table_iter_init(&iter, port_configuration->target_configs);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        TargetConfig *target_config = (TargetConfig*)value;
        
        g_print("      %s = {\n", (gchar*)key);
        print_target_config(target_config, 4);
        g_print("      };\n");
    }
    g_print("    };\n");
    
    g_print("  };\n");
}

typedef struct
{
    GPtrArray *candidate_target_array;
    GPtrArray *service_property_array;
    gchar *service_property;
    gchar *service_property_value;
}
RemoveParams;

static gboolean remove_service_port_pair(gchar *service, RemoveParams *params)
{
    DistributionItem *item = find_distribution_item(params->candidate_target_array, service);
    
    if(item == NULL)
        return TRUE;
    else
    {
        Service *service = find_service(params->service_property_array, item->service);
        
        if(service == NULL)
            return TRUE;
        else
        {
            ServiceProperty *property = find_service_property(service, params->service_property);
            
            if(property == NULL)
                return TRUE;
            else
                return (g_strcmp0(property->value, params->service_property_value) != 0);
        }
    }

}

static gboolean remove_service_to_port_mapping(gpointer key, gpointer value, gpointer user_data)
{
    gchar *service = (gchar*)key;
    RemoveParams *params = (RemoveParams*)user_data;
    return remove_service_port_pair(service, params);
}

static gboolean remove_port_to_service_mapping(gpointer key, gpointer value, gpointer user_data)
{
    gchar *service = (gchar*)value;
    RemoveParams *params = (RemoveParams*)user_data;
    return remove_service_port_pair(service, params);
}

/** Remove global port reservations for services that are no longer deployed */

static void clean_obsolete_services_to_ports(TargetConfig *target_config, GPtrArray *candidate_target_array, GPtrArray *service_property_array, gchar *service_property, gchar *service_property_value)
{
    RemoveParams params;
    
    /* Remove obsolete service to port mappings */
    params.candidate_target_array = candidate_target_array;
    params.service_property_array = service_property_array;
    params.service_property = service_property;
    params.service_property_value = service_property_value;
    
    g_hash_table_foreach_remove(target_config->ports_to_services, remove_port_to_service_mapping, &params);
    g_hash_table_foreach_remove(target_config->services_to_ports, remove_service_to_port_mapping, &params);
}

static gboolean remove_obsolete_target_config(gpointer key, gpointer value, gpointer user_data)
{
    gchar *target = (gchar*)key;
    GPtrArray *target_mapping_array = (GPtrArray*)user_data;
    
    return (find_target_mapping_item(target_mapping_array, target) == NULL);
}

void clean_obsolete_reservations(PortConfiguration *port_configuration, GPtrArray *candidate_target_array, GPtrArray *service_property_array, gchar *service_property)
{
    /* Generate inverse mapping to determine which machines are in use */
    GPtrArray *target_mapping_array = create_target_mapping_array(candidate_target_array);
    
    GHashTableIter iter;
    gpointer key, value;
    
    /* Clean the global target config */
    if(port_configuration->global_config != NULL)
        clean_obsolete_services_to_ports(port_configuration->global_config, candidate_target_array, service_property_array, service_property, "shared");
    
    /* Remove all port reservations for a machine, if it does not exist anymore */
    g_hash_table_foreach_remove(port_configuration->target_configs, remove_obsolete_target_config, target_mapping_array);
    
    g_hash_table_iter_init(&iter, port_configuration->target_configs);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        gchar *target = (gchar*)key;
        TargetConfig *target_config = (TargetConfig*)value;
        
        TargetMappingItem *target_mapping_item = find_target_mapping_item(target_mapping_array, target);
        
        if(target_mapping_item != NULL)
            clean_obsolete_services_to_ports(target_config, candidate_target_array, service_property_array, service_property, "private");
    }
    
    /* Clean inverse mapping array from memory */
    delete_target_mapping_array(target_mapping_array);
}
