#include "infrastructureproperties.h"
#include <stdlib.h>
#include <glib/gprintf.h>

#define BUFFER_SIZE 1024

GPtrArray *create_target_array_from_xml(const gchar *infrastructure_xml_file)
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

static gint compare_target_names(const Target **l, const Target **r)
{
    const Target *left = *l;
    const Target *right = *r;
    
    return g_strcmp0(left->name, right->name);
}

Target *find_target_by_name(GPtrArray *target_array, gchar *name)
{
    Target target;
    Target **ret, *targetPtr = &target;
    
    targetPtr->name = name;
    
    ret = bsearch(&targetPtr, target_array->pdata, target_array->len, sizeof(gpointer), (int (*)(const void*, const void*)) compare_target_names);

    if(ret == NULL)
        return NULL;
    else
        return *ret;
}

static gint compare_target_property_keys(const TargetProperty **l, const TargetProperty **r)
{
    const TargetProperty *left = *l;
    const TargetProperty *right = *r;
    
    return g_strcmp0(left->name, right->name);
}

TargetProperty *find_target_property(Target *target, gchar *name)
{
    if(target->properties == NULL)
        return NULL;
    else
    {
        TargetProperty prop;
        TargetProperty **ret, *propPtr = &prop;
        
        propPtr->name = name;
        
        ret = bsearch(&propPtr, target->properties->pdata, target->properties->len, sizeof(gpointer), (int (*)(const void*, const void*)) compare_target_property_keys);
        
        if(ret == NULL)
            return NULL;
        else
            return *ret;
    }
}

void substract_target_value(Target *target, gchar *property_name, int amount)
{
    gchar buffer[BUFFER_SIZE];
    TargetProperty *property = find_target_property(target, property_name);
    int value = atoi(property->value) - amount;
    g_sprintf(buffer, "%d", value);
    g_free(property->value);
    property->value = g_strdup(buffer);
}
