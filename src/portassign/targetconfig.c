#include "targetconfig.h"
#include <nixxml-node.h>
#include <nixxml-print-nix.h>
#include <nixxml-print-xml.h>
#include <nixxml-ghashtable.h>
#include <serviceproperties.h>

TargetConfig *create_target_config(gint last_port, gint min_port, gint max_port)
{
    TargetConfig *target_config = (TargetConfig*)g_malloc(sizeof(TargetConfig));

    target_config->last_port = last_port;
    target_config->min_port = min_port;
    target_config->max_port = max_port;
    target_config->services_to_ports = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
    target_config->ports_to_services = g_hash_table_new(g_int_hash, g_int_equal);

    return target_config;
}

void delete_target_config(TargetConfig *target_config)
{
    if(target_config != NULL)
    {
        GHashTableIter iter;
        gpointer key, value;

        g_hash_table_iter_init(&iter, target_config->services_to_ports);
        while(g_hash_table_iter_next(&iter, &key, &value))
            free(value);

        g_hash_table_destroy(target_config->services_to_ports);
        g_hash_table_destroy(target_config->ports_to_services);
        g_free(target_config);
    }
}

static void *create_target_config_from_element(xmlNodePtr element, void *userdata)
{
    return create_target_config(0, 0, 0);
}

static void parse_and_insert_target_config_attributes(xmlNodePtr element, void *table, const xmlChar *key, void *userdata)
{
    TargetConfig *target_config = (TargetConfig*)table;

    if(xmlStrcmp(key, (xmlChar*) "lastPort") == 0)
    {
        gchar *value = NixXML_parse_value(element, NULL);
        target_config->last_port = atoi(value);
        g_free(value);
    }
    else if(xmlStrcmp(key, (xmlChar*) "minPort") == 0)
    {
        gchar *value = NixXML_parse_value(element, NULL);
        target_config->min_port = atoi(value);
        g_free(value);
    }
    else if(xmlStrcmp(key, (xmlChar*) "maxPort") == 0)
    {
        gchar *value = NixXML_parse_value(element, NULL);
        target_config->max_port = atoi(value);
        g_free(value);
    }
    else if(xmlStrcmp(key, (xmlChar*) "servicesToPorts") == 0)
        target_config->services_to_ports = NixXML_parse_g_hash_table_simple(element, userdata, NixXML_parse_int);
}

void *parse_target_config(xmlNodePtr element, void *userdata)
{
    return NixXML_parse_simple_heterogeneous_attrset(element, userdata, create_target_config_from_element, parse_and_insert_target_config_attributes);
}

/* Print Nix */

static void print_services_to_ports_nix(FILE *file, const void *value, const int indent_level, void *userdata)
{
    NixXML_print_g_hash_table_nix(file, (GHashTable*)value, indent_level, userdata, NixXML_print_int_nix);
}

static void print_target_config_attributes_nix(FILE *file, const void *value, const int indent_level, void *userdata, NixXML_PrintValueFunc print_value)
{
    const TargetConfig *target_config = (const TargetConfig*)value;

    NixXML_print_attribute_nix(file, "lastPort", &target_config->last_port, indent_level, userdata, NixXML_print_int_nix);
    NixXML_print_attribute_nix(file, "minPort", &target_config->min_port, indent_level, userdata, NixXML_print_int_nix);
    NixXML_print_attribute_nix(file, "maxPort", &target_config->max_port, indent_level, userdata, NixXML_print_int_nix);
    NixXML_print_attribute_nix(file, "servicesToPorts", target_config->services_to_ports, indent_level, userdata, print_services_to_ports_nix);
}

void print_target_config_nix(FILE *file, const void *value, const int indent_level, void *userdata)
{
    NixXML_print_attrset_nix(file, value, indent_level, userdata, print_target_config_attributes_nix, NULL);
}

/* Print XML */

static void print_services_to_ports_xml(FILE *file, const void *value, const int indent_level, const char *type_property_name, void *userdata)
{
    NixXML_print_g_hash_table_verbose_xml(file, (GHashTable*)value, "service", "name", indent_level, type_property_name, userdata, NixXML_print_int_xml);
}

static void print_target_config_attributes_xml(FILE *file, const void *value, const int indent_level, const char *type_property_name, void *userdata, NixXML_PrintXMLValueFunc print_value)
{
    const TargetConfig *target_config = (const TargetConfig*)value;

    NixXML_print_simple_attribute_xml(file, "lastPort", &target_config->last_port, indent_level, type_property_name, userdata, NixXML_print_int_xml);
    NixXML_print_simple_attribute_xml(file, "minPort", &target_config->min_port, indent_level, type_property_name, userdata, NixXML_print_int_xml);
    NixXML_print_simple_attribute_xml(file, "maxPort", &target_config->max_port, indent_level, type_property_name, userdata, NixXML_print_int_xml);
    NixXML_print_simple_attribute_xml(file, "servicesToPorts", target_config->services_to_ports, indent_level, type_property_name, userdata, print_services_to_ports_xml);
}

void print_target_config_xml(FILE *file, const void *value, const int indent_level, const char *type_property_name, void *userdata)
{
    NixXML_print_simple_attrset_xml(file, value, indent_level, type_property_name, userdata, print_target_config_attributes_xml, NULL);
}

typedef struct
{
    GHashTable *candidate_target_table;
    GHashTable *service_table;
    gchar *service_property;
    gchar *service_property_value;
}
RemoveParams;

static gboolean remove_service_port_pair(gchar *service_name, RemoveParams *params)
{
    GPtrArray *targets = g_hash_table_lookup(params->candidate_target_table, service_name);

    if(targets == NULL)
        return TRUE;
    else
    {
        Service *service = g_hash_table_lookup(params->service_table, service_name);

        if(service == NULL)
            return TRUE;
        else
        {
            NixXML_Node *value = g_hash_table_lookup(service->properties, params->service_property);

            if(value == NULL)
                return TRUE;
            else
                return (xmlStrcmp(value->value, (xmlChar*) params->service_property_value) != 0);
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

/* Remove global port reservations for services that are no longer deployed */

void clean_obsolete_services_to_ports(TargetConfig *target_config, GHashTable *candidate_target_table, GHashTable *service_table, gchar *service_property, gchar *service_property_value)
{
    RemoveParams params;

    /* Remove obsolete service to port mappings */
    params.candidate_target_table = candidate_target_table;
    params.service_table = service_table;
    params.service_property = service_property;
    params.service_property_value = service_property_value;

    g_hash_table_foreach_remove(target_config->ports_to_services, remove_port_to_service_mapping, &params);
    g_hash_table_foreach_remove(target_config->services_to_ports, remove_service_to_port_mapping, &params);
}
