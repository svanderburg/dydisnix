#include "infrastructureproperties.h"
#include <xmlutil.h>
#include <stdlib.h>

#define BUFFER_SIZE 1024

GPtrArray *create_infrastructure_property_array(const gchar *infrastructure_xml_file)
{
    /* Declarations */
    xmlDocPtr doc;
    xmlNodePtr node_root;
    xmlXPathObjectPtr result;
    GPtrArray *infrastructure_property_array = NULL;
    
    /* Parse the XML document */
    
    if((doc = xmlParseFile(infrastructure_xml_file)) == NULL)
    {
	g_printerr("Error with parsing the infrastructure XML file!\n");
	xmlCleanupParser();
	return NULL;
    }
    
    /* Check if the document has a root */
    node_root = xmlDocGetRootElement(doc);
    
    if(node_root == NULL)
    {
        g_printerr("The infrastructure XML file is empty!\n");
	xmlFreeDoc(doc);
	xmlCleanupParser();
	return NULL;
    }
    
    /* Query all the targets from the XML document */
    result = executeXPathQuery(doc, "/infrastructure/target");
    
    /* Iterate over all targets and add them to the array */
            
    if(result)
    {
	unsigned int i;
        xmlNodeSetPtr nodeset = result->nodesetval;
        infrastructure_property_array = g_ptr_array_new();
	
	for(i = 0; i < nodeset->nodeNr; i++)
        {
	    Target *target = (Target*)g_malloc(sizeof(Target));
	    GPtrArray *property = g_ptr_array_new();
	    xmlAttr *infrastructure_properties = nodeset->nodeTab[i]->properties;
	    xmlNodePtr infrastructure_children = nodeset->nodeTab[i]->children;
	    
	    /* Assign values */
	    target->name = NULL;
	    target->property = property;
	    
	    /* Read name attribute */
	    while(infrastructure_properties != NULL)
	    {
		if(xmlStrcmp(infrastructure_properties->name, (const xmlChar*) "name") == 0)
		    target->name = g_strdup(infrastructure_properties->children->content);
	    
		infrastructure_properties = infrastructure_properties->next;
	    }
	    
	    /* Iterate over the infrastructure properties */
	    
	    while(infrastructure_children != NULL)
	    {
		InfrastructureProperty *infrastructure_property = (InfrastructureProperty*)g_malloc(sizeof(InfrastructureProperty));
		
		if(xmlStrcmp(infrastructure_children->name, (xmlChar*) "text") != 0)
		{
		    infrastructure_property->name = g_strdup(infrastructure_children->name);
		    
		    if(infrastructure_children->children == NULL) /* !!! also support lists in the future */
			infrastructure_property->value = NULL;
		    else
			infrastructure_property->value = g_strdup(infrastructure_children->children->content);
			
		    g_ptr_array_add(property, infrastructure_property);
		}
		
		infrastructure_children = infrastructure_children->next;
	    }
	    
	    g_ptr_array_add(infrastructure_property_array, target);
	}
	
	xmlXPathFreeObject(result);
    }
    else
	g_printerr("No targets found!\n");
    
    /* Cleanup */
    xmlFreeDoc(doc);
    xmlCleanupParser();

    /* Return the infrastructure property array */
    return infrastructure_property_array;
}

void delete_infrastructure_property_array(GPtrArray *infrastructure_property_array)
{
    unsigned int i;
    
    for(i = 0; i < infrastructure_property_array->len; i++)
    {
	Target *target = g_ptr_array_index(infrastructure_property_array, i);
	unsigned int j;
	
	g_free(target->name);
	
	for(j = 0; j < target->property->len; j++)
	{
	    InfrastructureProperty *infrastructure_property = g_ptr_array_index(target->property, j);
	    
	    g_free(infrastructure_property->name);
	    g_free(infrastructure_property->value);
	    g_free(infrastructure_property);
	}
	
	g_ptr_array_free(target->property, TRUE);
	g_free(target);
    }
    
    g_ptr_array_free(infrastructure_property_array, TRUE);
}

void print_infrastructure_property_array(const GPtrArray *infrastructure_property_array)
{
    unsigned int i;
    
    for(i = 0; i < infrastructure_property_array->len; i++)
    {
	Target *target = g_ptr_array_index(infrastructure_property_array, i);
	unsigned int j;
	
	g_print("Target name: %s\n", target->name);
	g_print("Properties:\n");
	
	for(j = 0; j < target->property->len; j++)
	{
	    InfrastructureProperty *infrastructure_property = g_ptr_array_index(target->property, j);
	    g_print("  name: %s, value: %s\n", infrastructure_property->name, infrastructure_property->value);
	}
    }
}

static gint compare_target_keys(const Target **l, const Target **r)
{
    const Target *left = *l;
    const Target *right = *r;
    
    return g_strcmp0(left->name, right->name);
}

Target *find_target(GPtrArray *target_array, gchar *name)
{
    Target target;
    Target **ret, *targetPtr = &target;
    
    targetPtr->name = name;
    
    ret = bsearch(&targetPtr, target_array->pdata, target_array->len, sizeof(gpointer), (int (*)(const void*, const void*)) compare_target_keys);

    if(ret == NULL)
        return NULL;
    else
        return *ret;
}

static gint compare_infrastructure_property_keys(const InfrastructureProperty **l, const InfrastructureProperty **r)
{
    const InfrastructureProperty *left = *l;
    const InfrastructureProperty *right = *r;
    
    return g_strcmp0(left->name, right->name);
}

InfrastructureProperty *find_infrastructure_property(Target *target, gchar *name)
{
    InfrastructureProperty prop;
    InfrastructureProperty **ret, *propPtr = &prop;
    
    propPtr->name = name;
    
    ret = bsearch(&propPtr, target->property->pdata, target->property->len, sizeof(gpointer), (int (*)(const void*, const void*)) compare_infrastructure_property_keys);

    if(ret == NULL)
        return NULL;
    else
        return *ret;
}

void substract_target_value(Target *target, gchar *property_name, int amount)
{
    gchar buffer[BUFFER_SIZE];
    InfrastructureProperty *property = find_infrastructure_property(target, property_name);
    int value = atoi(property->value) - amount;
    g_sprintf(buffer, "%d", value);
    g_free(property->value);
    property->value = g_strdup(buffer);
}
