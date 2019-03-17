#include "serviceproperties.h"
#include <string.h>
#include <stdlib.h>
#include <xmlutil.h>
#include <procreact_future.h>

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

static gpointer parse_service(xmlNodePtr element)
{
    Service *service = (Service*)g_malloc0(sizeof(Service));
    xmlNodePtr element_children = element->children;

    while(element_children != NULL)
    {
        if(xmlStrcmp(element_children->name, (xmlChar*) "name") == 0)
            service->name = parse_value(element_children);
        else if(xmlStrcmp(element_children->name, (xmlChar*) "properties") == 0)
            service->property = parse_dictionary(element_children, parse_value);
        else if(xmlStrcmp(element_children->name, (xmlChar*) "connectsTo") == 0)
            service->connects_to = parse_list(element_children, "dependency", parse_value);
        else if(xmlStrcmp(element_children->name, (xmlChar*) "dependsOn") == 0)
            service->depends_on = parse_list(element_children, "dependency", parse_value);

        element_children = element_children->next;
    }

    return service;
}

GPtrArray *create_service_property_array_from_xml(const gchar *services_xml_file)
{
    /* Declarations */
    xmlDocPtr doc;
    xmlNodePtr node_root;
    GPtrArray *service_property_array;

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
    service_property_array = parse_list(node_root, "service", parse_service);

    /* Cleanup */
    xmlFreeDoc(doc);
    xmlCleanupParser();

    /* Return the service property array */
    return service_property_array;
}

GPtrArray *create_service_property_array_from_nix(gchar *services_nix)
{
    char *services_xml = generate_service_xml_from_expr(services_nix);
    GPtrArray *service_property_array = create_service_property_array_from_xml(services_xml);
    free(services_xml);
    return service_property_array;
}

GPtrArray *create_service_property_array(gchar *services, const int xml)
{
    if(xml)
        return create_service_property_array_from_xml(services);
    else
        return create_service_property_array_from_nix(services);
}

void delete_service(Service *service)
{
    unsigned int i;
    GHashTableIter iter;
    gpointer key, value;

    g_free(service->name);

    g_hash_table_iter_init(&iter, service->property);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        g_free(key);
        g_free(value);
    }

    g_hash_table_destroy(service->property);

    for(i = 0; i < service->connects_to->len; i++)
    {
        gchar *dependency = g_ptr_array_index(service->connects_to, i);
        g_free(dependency);
    }

    g_ptr_array_free(service->connects_to, TRUE);

    for(i = 0; i < service->depends_on->len; i++)
    {
        gchar *dependency = g_ptr_array_index(service->depends_on, i);
        g_free(dependency);
    }

    g_ptr_array_free(service->depends_on, TRUE);

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

    GHashTable *copy_hash_table = g_hash_table_new(g_str_hash, g_str_equal);

    g_hash_table_iter_init(&iter, properties);
    while(g_hash_table_iter_next(&iter, &key, &value))
        g_hash_table_insert(copy_hash_table, g_strdup((gchar*)key), g_strdup((gchar*)value));

    return copy_hash_table;
}

Service *copy_service(const Service *service)
{
    Service *new_service = (Service*)g_malloc(sizeof(Service));
    new_service->name = g_strdup(service->name);
    new_service->property = copy_properties(service->property);
    new_service->depends_on = copy_dependencies(service->depends_on);
    new_service->connects_to = copy_dependencies(service->connects_to);
    new_service->group_node = service->group_node;

    return new_service;
}

void delete_service_property_array(GPtrArray *service_property_array)
{
    if(service_property_array != NULL)
    {
        unsigned int i;

        for(i = 0; i < service_property_array->len; i++)
        {
            Service *service = g_ptr_array_index(service_property_array, i);
            delete_service(service);
        }

        g_ptr_array_free(service_property_array, TRUE);
    }
}

void print_service_property_array(const GPtrArray *service_property_array)
{
    if(service_property_array != NULL)
    {
        unsigned int i;

        for(i = 0; i < service_property_array->len; i++)
        {
            Service *service = g_ptr_array_index(service_property_array, i);
            GHashTableIter iter;
            gpointer key, value;

            g_print("Service name: %s\n", service->name);
            g_print("Properties:\n");

            g_hash_table_iter_init(&iter, service->property);
            while(g_hash_table_iter_next(&iter, &key, &value))
                g_print("  name: %s, value: %s\n", (gchar*)key, (gchar*)value);
        }
    }
}

static gint compare_service_keys(const Service **l, const Service **r)
{
    const Service *left = *l;
    const Service *right = *r;
    
    return g_strcmp0(left->name, right->name);
}

Service *find_service(GPtrArray *service_array, gchar *name)
{
    Service service;
    Service **ret, *servicePtr = &service;
    
    servicePtr->name = name;
    
    ret = bsearch(&servicePtr, service_array->pdata, service_array->len, sizeof(gpointer), (int (*)(const void*, const void*)) compare_service_keys);
    
    if(ret == NULL)
        return NULL;
    else
        return *ret;
}

gchar *find_service_property(const Service *service, gchar *name)
{
    if(service->property == NULL)
        return NULL;
    else
        return g_hash_table_lookup(service->property, name);
}
