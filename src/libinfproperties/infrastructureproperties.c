#include "infrastructureproperties.h"
#include <stdlib.h>
#include <string.h>
#include <glib/gprintf.h>
#include <procreact_future.h>

#define BUFFER_SIZE 1024

static ProcReact_Future generate_infrastructure_xml_from_expr_async(char *infrastructure_expr)
{
    ProcReact_Future future = procreact_initialize_future(procreact_create_string_type());

    if(future.pid == 0)
    {
        char *const args[] = {"dydisnix-xml", "-i", infrastructure_expr, "--no-out-link", NULL};
        dup2(future.fd, 1); /* Attach write-end to stdout */
        execvp(args[0], args);
        _exit(1);
    }

    return future;
}

char *generate_infrastructure_xml_from_expr(char *infrastructure_expr)
{
    ProcReact_Status status;
    ProcReact_Future future = generate_infrastructure_xml_from_expr_async(infrastructure_expr);
    char *path = procreact_future_get(&future, &status);
    path[strlen(path) - 1] = '\0';
    return path;
}

GPtrArray *create_target_property_array_from_xml(const gchar *infrastructure_xml_file)
{
    GPtrArray *targets_array;
    xmlDocPtr doc;
    
    /* Parse the XML document */
    
    if((doc = xmlParseFile(infrastructure_xml_file)) == NULL)
    {
	g_printerr("Error with parsing the infrastructure XML file!\n");
	xmlCleanupParser();
	return NULL;
    }
    
    /* Create a target array from the XML document */
    targets_array = create_target_array_from_doc(doc);
    
    /* Cleanup */
    xmlFreeDoc(doc);
    xmlCleanupParser();
    
    /* Return the target array */
    return targets_array;
}

GPtrArray *create_target_property_array_from_nix(gchar *infrastructure_nix)
{
    char *infrastructure_xml = generate_infrastructure_xml_from_expr(infrastructure_nix);
    GPtrArray *target_array = create_target_array_from_xml(infrastructure_xml);
    free(infrastructure_xml);
    return target_array;
}

GPtrArray *create_target_property_array(gchar *infrastructure, const int xml)
{
    if(xml)
        return create_target_property_array_from_xml(infrastructure);
    else
        return create_target_property_array_from_nix(infrastructure);
}

static gint compare_target_names(const Target **l, const Target **r)
{
    const Target *left = *l;
    const Target *right = *r;
    
    return xmlStrcmp(left->name, right->name);
}

Target *find_target_by_name(GPtrArray *target_array, gchar *name)
{
    Target target;
    Target **ret, *targetPtr = &target;
    
    targetPtr->name = (xmlChar*) name;
    
    ret = bsearch(&targetPtr, target_array->pdata, target_array->len, sizeof(gpointer), (int (*)(const void*, const void*)) compare_target_names);

    if(ret == NULL)
        return NULL;
    else
        return *ret;
}

void substract_target_value(Target *target, gchar *property_name, int amount)
{
    gchar buffer[BUFFER_SIZE];
    gchar *value = find_target_property(target, property_name);
    int result = atoi(value) - amount;
    g_sprintf(buffer, "%d", result);
    g_free(value);
    g_hash_table_insert(target->properties_table, g_strdup(property_name), g_strdup(buffer));
}
