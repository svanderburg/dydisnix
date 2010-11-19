#include "infrastructureproperties.h"
#include <xmlutil.h>
#include <stdlib.h>

#define BUFFER_SIZE 1024

GArray *create_infrastructure_property_array(gchar *infrastructure_xml_file)
{
    /* Declarations */
    xmlDocPtr doc;
    xmlNodePtr node_root;
    xmlXPathObjectPtr result;
    GArray *infrastructure_property_array = NULL;
    
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
        infrastructure_property_array = g_array_new(FALSE, FALSE, sizeof(Target*));
	
	for(i = 0; i < nodeset->nodeNr; i++)
        {
	    Target *target = (Target*)g_malloc(sizeof(Target));
	    GArray *property = g_array_new(FALSE, FALSE, sizeof(InfrastructureProperty*));
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
			
		    g_array_append_val(property, infrastructure_property);
		}
		
		infrastructure_children = infrastructure_children->next;
	    }
	    
	    g_array_append_val(infrastructure_property_array, target);
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

void delete_infrastructure_property_array(GArray *infrastructure_property_array)
{
    unsigned int i;
    
    for(i = 0; i < infrastructure_property_array->len; i++)
    {
	Target *target = g_array_index(infrastructure_property_array, Target*, i);
	unsigned int j;
	
	g_free(target->name);
	
	for(j = 0; j < target->property->len; j++)
	{
	    InfrastructureProperty *infrastructure_property = g_array_index(target->property, InfrastructureProperty*, j);
	    
	    g_free(infrastructure_property->name);
	    g_free(infrastructure_property->value);
	    g_free(infrastructure_property);
	}
	
	g_array_free(target->property, TRUE);
	g_free(target);
    }
    
    g_array_free(infrastructure_property_array, TRUE);
}

void print_infrastructure_property_array(GArray *infrastructure_property_array)
{
    unsigned int i;
    
    for(i = 0; i < infrastructure_property_array->len; i++)
    {
	Target *target = g_array_index(infrastructure_property_array, Target*, i);
	unsigned int j;
	
	g_print("Target name: %s\n", target->name);
	g_print("Properties:\n");
	
	for(j = 0; j < target->property->len; j++)
	{
	    InfrastructureProperty *infrastructure_property = g_array_index(target->property, InfrastructureProperty*, j);
	    g_print("  name: %s, value: %s\n", infrastructure_property->name, infrastructure_property->value);
	}
    }
}

gint infrastructure_index(GArray *infrastructure_property_array, gchar *name)
{
    gint left = 0;
    gint right = infrastructure_property_array->len - 1;
    
    while(left <= right)
    {
	gint mid = (left + right) / 2;
	Target *mid_target = g_array_index(infrastructure_property_array, Target*, mid);
        gint status = g_strcmp0(mid_target->name, name);
	
	if(status == 0)
            return mid; /* Return index of the found target */
	else if(status > 0)
	    right = mid - 1;
	else if(status < 0)
	    left = mid + 1;
    }
    
    return -1; /* Target not found */
}

gint infrastructure_property_index(Target *target, gchar *name)
{
    GArray *property = target->property;
    
    gint left = 0;
    gint right = property->len - 1;
    
    while(left <= right)
    {
	gint mid = (left + right) / 2;
	InfrastructureProperty *mid_property = g_array_index(property, InfrastructureProperty*, mid);
        gint status = g_strcmp0(mid_property->name, name);
	
	if(status == 0)
            return mid; /* Return index of the found target property */
	else if(status > 0)
	    right = mid - 1;
	else if(status < 0)
	    left = mid + 1;
    }
    
    return -1; /* Target property not found */
}

void substract_target_value(Target *target, gchar *property_name, int amount)
{
    gchar buffer[BUFFER_SIZE];
    int index = infrastructure_property_index(target, property_name);
    InfrastructureProperty *property = g_array_index(target->property, InfrastructureProperty*, index);
    int value = atoi(property->value) - amount;
    g_sprintf(buffer, "%d", value);
    g_free(property->value);
    property->value = g_strdup(buffer);
}
