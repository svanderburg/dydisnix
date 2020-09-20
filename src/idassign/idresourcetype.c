#include "idresourcetype.h"
#include <stdlib.h>
#include <glib.h>
#include <nixxml-node.h>
#include <nixxml-parse.h>

static void *create_id_resource_type_from_element(xmlNodePtr element, void *userdata)
{
    return g_malloc0(sizeof(IdResourceType));
}

static void parse_and_insert_id_resource_type_properties(xmlNodePtr element, void *table, const xmlChar *key, void *userdata)
{
    IdResourceType *type = (IdResourceType*)table;

    if(xmlStrcmp(key, (xmlChar*) "min") == 0)
    {
        int *min_value = NixXML_parse_int(element, userdata);

        if(min_value == NULL)
            type->min = 0;
        else
        {
            type->min = *min_value;
            free(min_value);
        }
    }
    else if(xmlStrcmp(key, (xmlChar*) "max") == 0)
    {
        int *max_value = NixXML_parse_int(element, userdata);

        if(max_value == NULL)
            type->max = 0;
        else
        {
            type->max = *max_value;
            free(max_value);
        }
    }
    else if(xmlStrcmp(key, (xmlChar*) "scope") == 0)
        type->scope = NixXML_parse_value(element, userdata);
}

void *parse_id_resource_type(xmlNodePtr element, void *userdata)
{
    IdResourceType *type = NixXML_parse_simple_heterogeneous_attrset(element, userdata, create_id_resource_type_from_element, parse_and_insert_id_resource_type_properties);

    if(type->scope == NULL || xmlStrcmp(type->scope, (xmlChar*) "") == 0)
    {
        xmlFree(type->scope);
        type->scope = xmlStrdup((xmlChar*) "global");
    }

    return type;
}

void delete_id_resource_type(IdResourceType *type)
{
    if(type != NULL)
    {
        xmlFree(type->scope);
        g_free(type);
    }
}

NixXML_bool id_is_outside_range(IdResourceType *type, int id)
{
    return (id < type->min || id > type->max);
}

NixXML_bool service_requires_unique_resource_id(Service *service, gchar *resource_name, gchar *service_property)
{
    NixXML_Node *requires_unique_ids_for_node = g_hash_table_lookup(service->properties, service_property);

    if(requires_unique_ids_for_node == NULL)
        return FALSE;
    else if(requires_unique_ids_for_node->type == NIX_XML_TYPE_LIST)
    {
        unsigned int i;
        GPtrArray *require_unique_ids_for = (GPtrArray*)requires_unique_ids_for_node->value;

        for(i = 0; i < require_unique_ids_for->len; i++)
        {
            NixXML_Node *value_node = (NixXML_Node*)g_ptr_array_index(require_unique_ids_for, i);
            if(g_strcmp0(value_node->value, resource_name) == 0)
                return TRUE;
        }

        return FALSE;
    }
    else
        return FALSE;
}
