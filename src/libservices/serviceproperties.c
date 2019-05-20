#include "serviceproperties.h"
#include <string.h>
#include <stdlib.h>
#include <procreact_future.h>
#include <nixxml-gptrarray.h>
#include <nixxml-ghashtable.h>

static ProcReact_Future generate_service_xml_from_expr_async(char *service_expr)
{
    ProcReact_Future future = procreact_initialize_future(procreact_create_string_type());

    if(future.pid == 0)
    {
        char *const args[] = {"dydisnix-xml", "-s", service_expr, "--no-out-link", NULL};
        dup2(future.fd, 1); /* Attach write-end to stdout */
        execvp(args[0], args);
        _exit(1);
    }

    return future;
}

char *generate_service_xml_from_expr(char *service_expr)
{
    ProcReact_Status status;
    ProcReact_Future future = generate_service_xml_from_expr_async(service_expr);
    char *path = procreact_future_get(&future, &status);
    path[strlen(path) - 1] = '\0';
    return path;
}

static void *create_service(xmlNodePtr element, void *userdata)
{
    Service *service = (Service*)g_malloc(sizeof(Service));
    service->name = NULL;
    service->type = NULL;
    service->group = NULL;
    service->depends_on = NULL;
    service->connects_to = NULL;
    service->properties = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, xmlFree);
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
        gchar *value = NixXML_parse_value(element, userdata);
        g_hash_table_insert(service->properties, g_strdup((gchar*)key), value);
    }
}

static void *parse_service(xmlNodePtr element, void *userdata)
{
    return NixXML_parse_verbose_heterogeneous_attrset(element, "property", "name", NULL, create_service, parse_and_insert_service_attributes);
}

GHashTable *create_service_table_from_xml(const gchar *services_xml_file)
{
    /* Declarations */
    xmlDocPtr doc;
    xmlNodePtr node_root;
    GHashTable *service_table;

    /* Parse the XML document */

    if((doc = xmlParseFile(services_xml_file)) == NULL)
    {
        g_printerr("Error with parsing the services XML file!\n");
        xmlCleanupParser();
        return NULL;
    }

    /* Check if the document has a root */
    node_root = xmlDocGetRootElement(doc);

    if(node_root == NULL)
    {
        g_printerr("The services XML file is empty!\n");
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return NULL;
    }

    /* Parse the services */
    service_table = NixXML_parse_g_hash_table_verbose(node_root, "service", "name", NULL, parse_service);

    /* Cleanup */
    xmlFreeDoc(doc);
    xmlCleanupParser();

    /* Return the service table */
    return service_table;
}

GHashTable *create_service_table_from_nix(gchar *services_nix)
{
    char *services_xml = generate_service_xml_from_expr(services_nix);
    GHashTable *service_table = create_service_table_from_xml(services_xml);
    free(services_xml);
    return service_table;
}

GHashTable *create_service_table(gchar *services, const int xml)
{
    if(xml)
        return create_service_table_from_xml(services);
    else
        return create_service_table_from_nix(services);
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
    xmlFree(service->name);
    xmlFree(service->type);
    xmlFree(service->group);

    g_hash_table_destroy(service->properties);

    delete_dependencies(service->connects_to);
    delete_dependencies(service->depends_on);

    g_free(service);
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

    GHashTable *copy_hash_table = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, xmlFree);

    g_hash_table_iter_init(&iter, properties);
    while(g_hash_table_iter_next(&iter, &key, &value))
        g_hash_table_insert(copy_hash_table, g_strdup((gchar*)key), xmlStrdup((xmlChar*)value));

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

void delete_service_table(GHashTable *service_table)
{
    if(service_table != NULL)
    {
        GHashTableIter iter;
        gpointer key, value;

        g_hash_table_iter_init(&iter, service_table);
        while(g_hash_table_iter_next(&iter, &key, &value))
        {
            Service *service = (Service*)value;
            delete_service(service);
        }

        g_hash_table_destroy(service_table);
    }
}
