#include "service.h"
#include <nixxml-parse.h>
#include <nixxml-gptrarray.h>
#include <nixxml-ghashtable.h>
#include <nixxml-glib.h>

static void *create_service_from_element(xmlNodePtr element, void *userdata)
{
    Service *service = (Service*)g_malloc(sizeof(Service));
    service->name = NULL;
    service->type = NULL;
    service->group = NULL;
    service->depends_on = NULL;
    service->connects_to = NULL;
    service->properties = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
    service->group_node = FALSE;
    return service;
}

void parse_and_insert_service_attributes(xmlNodePtr element, void *table, const xmlChar *key, void *userdata)
{
    Service *service = (Service*)table;

    if(xmlStrcmp(key, (xmlChar*) "name") == 0)
        service->name = NixXML_parse_value(element, userdata);
    else if(xmlStrcmp(key, (xmlChar*) "type") == 0)
        service->type = NixXML_parse_value(element, userdata);
    else if(xmlStrcmp(key, (xmlChar*) "group") == 0)
        service->group = NixXML_parse_value(element, userdata);
    else if(xmlStrcmp(key, (xmlChar*) "connectsTo") == 0)
        service->connects_to = NixXML_parse_g_ptr_array(element, "dependency", userdata, NixXML_parse_value);
    else if(xmlStrcmp(key, (xmlChar*) "dependsOn") == 0)
        service->depends_on = NixXML_parse_g_ptr_array(element, "dependency", userdata, NixXML_parse_value);
    else
    {
        NixXML_Node *value = NixXML_generic_parse_verbose_expr_glib(element, "type", "name", NULL);
        g_hash_table_insert(service->properties, g_strdup((gchar*)key), value);
    }
}

void *parse_service(xmlNodePtr element, void *userdata)
{
    Service *service = NixXML_parse_verbose_heterogeneous_attrset(element, "property", "name", NULL, create_service_from_element, parse_and_insert_service_attributes);

    if(service->group == NULL)
        service->group = xmlStrdup((xmlChar*) "");

    return service;
}

static void delete_service_property_table(GHashTable *service_property_table)
{
    NixXML_delete_g_hash_table(service_property_table, (NixXML_DeleteGHashTableValueFunc)NixXML_delete_node_glib);
}

static void delete_dependencies(GPtrArray *dependencies)
{
    unsigned int i;

    for(i = 0; i < dependencies->len; i++)
    {
        gchar *dependency = g_ptr_array_index(dependencies, i);
        g_free(dependency);
    }

    g_ptr_array_free(dependencies, TRUE);
}

void delete_service(Service *service)
{
    if(service != NULL)
    {
        xmlFree(service->name);
        xmlFree(service->type);
        xmlFree(service->group);

        delete_service_property_table(service->properties);

        delete_dependencies(service->connects_to);
        delete_dependencies(service->depends_on);

        g_free(service);
    }
}

static GPtrArray *copy_dependencies(GPtrArray *dependencies)
{
    unsigned int i;
    GPtrArray *copy_array = g_ptr_array_new();

    for(i = 0; i < dependencies->len; i++)
    {
        gchar *dependency = g_ptr_array_index(dependencies, i);
        g_ptr_array_add(copy_array, g_strdup(dependency));
    }

    return copy_array;
}

GHashTable *copy_properties(GHashTable *properties)
{
    GHashTableIter iter;
    gpointer key, value;

    GHashTable *copy_hash_table = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

    g_hash_table_iter_init(&iter, properties);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        NixXML_Node *node = (NixXML_Node*)value;
        NixXML_Node *copy_value = (NixXML_Node*)malloc(sizeof(NixXML_Node));
        copy_value->type = node->type;
        copy_value->value = xmlStrdup((xmlChar*)node->value);

        g_hash_table_insert(copy_hash_table, g_strdup((gchar*)key), copy_value);
    }

    return copy_hash_table;
}

Service *copy_service(const Service *service)
{
    Service *new_service = (Service*)g_malloc(sizeof(Service));
    new_service->name = xmlStrdup(service->name);
    new_service->type = xmlStrdup(service->type);
    new_service->group = xmlStrdup(service->group);
    new_service->properties = copy_properties(service->properties);
    new_service->depends_on = copy_dependencies(service->depends_on);
    new_service->connects_to = copy_dependencies(service->connects_to);
    new_service->group_node = service->group_node;

    return new_service;
}

xmlChar *find_service_property(Service *service, gchar *service_name)
{
    if(g_strcmp0(service_name, "name") == 0)
        return service->name;
    else if(g_strcmp0(service_name, "type") == 0)
        return service->type;
    else if(g_strcmp0(service_name, "group") == 0)
        return service->group;
    else
    {
        NixXML_Node *value = g_hash_table_lookup(service->properties, service_name);
        if(value == NULL)
            return NULL;
        else
            return value->value;
    }
}
