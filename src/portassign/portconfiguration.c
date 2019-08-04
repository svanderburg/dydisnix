#include "portconfiguration.h"
#include <stdlib.h>
#include <string.h>
#include <targetmapping.h>
#include <servicestable.h>
#include <candidatetargetmappingtable.h>
#include <procreact_future.h>
#include <nixxml-parse.h>
#include <nixxml-print-nix.h>
#include <nixxml-print-xml.h>
#include <nixxml-ghashtable.h>

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

static void delete_config_value(gpointer data)
{
    TargetConfig *target_config = (TargetConfig*)data;
    delete_target_config(target_config);
}

PortConfiguration *create_empty_port_configuration(void)
{
    PortConfiguration *port_configuration = (PortConfiguration*)g_malloc(sizeof(PortConfiguration));
    port_configuration->global_config = NULL;
    port_configuration->target_configs = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, delete_config_value);
    return port_configuration;
}

static void *create_port_configuration_from_element(xmlNodePtr element, void *userdata)
{
    return g_malloc0(sizeof(PortConfiguration));
}

static void parse_and_insert_port_configuration_attributes_nix(xmlNodePtr element, void *table, const xmlChar *key, void *userdata)
{
    PortConfiguration *port_configuration = (PortConfiguration*)table;

    if(xmlStrcmp(key, (xmlChar*) "globalConfig") == 0)
        port_configuration->global_config = parse_target_config(element, NULL);
    else if(xmlStrcmp(key, (xmlChar*) "targetConfigs") == 0)
        port_configuration->target_configs = NixXML_parse_g_hash_table_simple(element, NULL, parse_target_config);
}

void *parse_port_configuration(xmlNodePtr element, void *userdata)
{
    return NixXML_parse_simple_heterogeneous_attrset(element, userdata, create_port_configuration_from_element, parse_and_insert_port_configuration_attributes_nix);
}

PortConfiguration *open_port_configuration_from_xml(const gchar *port_configuration_file)
{
    /* Declarations */
    xmlDocPtr doc;
    xmlNodePtr node_root;
    PortConfiguration *port_configuration;

    /* Parse the XML document */

    if((doc = xmlParseFile(port_configuration_file)) == NULL)
    {
        g_printerr("Error with parsing the port configuration XML file!\n");
        xmlCleanupParser();
        return NULL;
    }

    /* Check if the document has a root */
    node_root = xmlDocGetRootElement(doc);

    if(node_root == NULL)
    {
        g_printerr("The port configuration XML file is empty!\n");
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return NULL;
    }

    /* Parse the target config */
    port_configuration = parse_port_configuration(node_root, NULL);

    /* Cleanup */
    xmlFreeDoc(doc);
    xmlCleanupParser();

    return port_configuration;
}

PortConfiguration *open_port_configuration_from_nix(char *port_configuration_file)
{
    char *ports_xml = generate_ports_xml_from_expr(port_configuration_file);
    PortConfiguration *port_configuration = open_port_configuration_from_xml(ports_xml);
    free(ports_xml);
    return port_configuration;
}

PortConfiguration *open_port_configuration(gchar *port_configuration_file, int xml)
{
    if(xml)
        return open_port_configuration_from_xml(port_configuration_file);
    else
        return open_port_configuration_from_nix(port_configuration_file);
}

void delete_port_configuration(PortConfiguration *port_configuration)
{
    if(port_configuration != NULL)
    {
        delete_target_config(port_configuration->global_config);
        g_hash_table_destroy(port_configuration->target_configs);
        g_free(port_configuration);
    }
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

/* Print Nix */

static void print_target_configs_nix(FILE *file, const void *value, const int indent_level, void *userdata)
{
    NixXML_print_g_hash_table_nix(file, (GHashTable*)value, indent_level, userdata, print_target_config_nix);
}

static void print_port_configuration_attributes_nix(FILE *file, const void *value, const int indent_level, void *userdata, NixXML_PrintValueFunc print_value)
{
    const PortConfiguration *port_configuration = (const PortConfiguration*)value;

    if(port_configuration->global_config != NULL)
        NixXML_print_attribute_nix(file, "globalConfig", port_configuration->global_config, indent_level, userdata, print_target_config_nix);
    NixXML_print_attribute_nix(file, "targetConfigs", port_configuration->target_configs, indent_level, userdata, print_target_configs_nix);
}

void print_port_configuration_nix(FILE *file, const void *value, const int indent_level, void *userdata)
{
    NixXML_print_attrset_nix(file, value, indent_level, userdata, print_port_configuration_attributes_nix, NULL);
}

/* Print XML */

static void print_target_configs_xml(FILE *file, const void *value, const int indent_level, const char *type_property_name, void *userdata)
{
    NixXML_print_g_hash_table_simple_xml(file, (GHashTable*)value, indent_level, type_property_name, userdata, print_target_config_xml);
}

static void print_port_configuration_attributes_xml(FILE *file, const void *value, const int indent_level, const char *type_property_name, void *userdata, NixXML_PrintXMLValueFunc print_value)
{
    const PortConfiguration *port_configuration = (const PortConfiguration*)value;

    if(port_configuration->global_config != NULL)
        NixXML_print_simple_attribute_xml(file, "globalConfig", port_configuration->global_config, indent_level, type_property_name, userdata, print_target_config_xml);
    NixXML_print_simple_attribute_xml(file, "targetConfigs", port_configuration->target_configs, indent_level, type_property_name, userdata, print_target_configs_xml);
}

void print_port_configuration_xml(FILE *file, const void *value, const int indent_level, const char *type_property_name, void *userdata)
{
    NixXML_print_simple_attrset_xml(file, value, indent_level, type_property_name, userdata, print_port_configuration_attributes_xml, NULL);
}

static gboolean remove_obsolete_target_config(gpointer key, gpointer value, gpointer user_data)
{
    gchar *target = (gchar*)key;
    GPtrArray *target_mapping_array = (GPtrArray*)user_data;
    
    return (find_target_mapping_item(target_mapping_array, target) == NULL);
}

void clean_obsolete_reservations(PortConfiguration *port_configuration, GHashTable *candidate_target_table, GHashTable *service_table, gchar *service_property)
{
    /* Generate inverse mapping to determine which machines are in use */
    GPtrArray *target_mapping_array = create_target_mapping_array(candidate_target_table);

    GHashTableIter iter;
    gpointer key, value;

    /* Clean the global target config */
    if(port_configuration->global_config != NULL)
        clean_obsolete_services_to_ports(port_configuration->global_config, candidate_target_table, service_table, service_property, "shared");

    /* Remove all port reservations for a machine, if it does not exist anymore */
    g_hash_table_foreach_remove(port_configuration->target_configs, remove_obsolete_target_config, target_mapping_array);

    g_hash_table_iter_init(&iter, port_configuration->target_configs);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        gchar *target = (gchar*)key;
        TargetConfig *target_config = (TargetConfig*)value;

        TargetMappingItem *target_mapping_item = find_target_mapping_item(target_mapping_array, target);

        if(target_mapping_item != NULL)
            clean_obsolete_services_to_ports(target_config, candidate_target_table, service_table, service_property, "private");
    }

    /* Clean inverse mapping array from memory */
    delete_target_mapping_array(target_mapping_array);
}
