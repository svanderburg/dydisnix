#include "serviceproperties.h"
#include <xmlutil.h>

GArray *create_service_property_array(const gchar *services_xml_file)
{
    /* Declarations */
    xmlDocPtr doc;
    xmlNodePtr node_root;
    xmlXPathObjectPtr result;
    GArray *service_property_array = NULL;

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

    /* Query all the services from the XML document */
    result = executeXPathQuery(doc, "/services/service");
    
    /* Iterate over all targets and add them to the array */
            
    if(result)
    {
        unsigned int i;
        xmlNodeSetPtr nodeset = result->nodesetval;
	service_property_array = g_array_new(FALSE, FALSE, sizeof(Service*));
	
        for(i = 0; i < nodeset->nodeNr; i++)
        {
	    Service *service = (Service*)g_malloc(sizeof(Service));
	    GArray *property = g_array_new(FALSE, FALSE, sizeof(ServiceProperty*));
	    xmlAttr *service_properties = nodeset->nodeTab[i]->properties;
	    xmlNodePtr service_children = nodeset->nodeTab[i]->children;
	    
	    /* Assign values */
	    service->name = NULL;
	    service->property = property;
	    
	    /* Read name attribute */
	    while(service_properties != NULL)
	    {
		if(xmlStrcmp(service_properties->name, (const xmlChar*) "name") == 0)
		    service->name = g_strdup(service_properties->children->content);
	    
		service_properties = service_properties->next;
	    }
	    
	    /* Iterate over the service properties */
	    
	    while(service_children != NULL)
	    {
		ServiceProperty *service_property = (ServiceProperty*)g_malloc(sizeof(ServiceProperty));
		
		if(xmlStrcmp(service_children->name, (xmlChar*) "text") != 0)
		{
		    service_property->name = g_strdup(service_children->name);
		    
		    if(service_children->children == NULL) /* !!! also support lists in the future */
			service_property->value = NULL;
		    else
			service_property->value = g_strdup(service_children->children->content);
			
		    g_array_append_val(property, service_property);
		}
	    
		service_children = service_children->next;
	    }
	    
	    g_array_append_val(service_property_array, service);
	}
	
	xmlXPathFreeObject(result);
    }
    else
        g_printerr("No services found!\n");

    /* Cleanup */
    xmlFreeDoc(doc);
    xmlCleanupParser();

    /* Return the service property array */
    return service_property_array;
}

void delete_service_property_array(GArray *service_property_array)
{
    unsigned int i;
    
    for(i = 0; i < service_property_array->len; i++)
    {
	Service *service = g_array_index(service_property_array, Service*, i);
	unsigned int j;
		
	g_free(service->name);
	
	for(j = 0; j < service->property->len; j++)
	{
	    ServiceProperty *service_property = g_array_index(service->property, ServiceProperty*, j);
	    
	    g_free(service_property->name);
	    g_free(service_property->value);
	    g_free(service_property);
	}
	
	g_array_free(service->property, TRUE);
	g_free(service);
    }
    
    g_array_free(service_property_array, TRUE);
}

void print_service_property_array(const GArray *service_property_array)
{
    unsigned int i;
    
    for(i = 0; i < service_property_array->len; i++)
    {
	Service *service = g_array_index(service_property_array, Service*, i);
	unsigned int j;
	
	g_print("Service name: %s\n", service->name);
	g_print("Properties:\n");
	
	for(j = 0; j < service->property->len; j++)
	{
	    ServiceProperty *service_property = g_array_index(service->property, ServiceProperty*, j);	    
	    g_print("  name: %s, value: %s\n", service_property->name, service_property->value);
	}
    }
}

gint service_index(GArray *service_property_array, gchar *name)
{
    gint left = 0;
    gint right = service_property_array->len - 1;
    
    while(left <= right)
    {
	gint mid = (left + right) / 2;
	Service *mid_service = g_array_index(service_property_array, Service*, mid);
        gint status = g_strcmp0(mid_service->name, name);
	
	if(status == 0)
            return mid; /* Return index of the found service */
	else if(status > 0)
	    right = mid - 1;
	else if(status < 0)
	    left = mid + 1;
    }
    
    return -1; /* Service not found */
}

Service *lookup_service(GArray *service_property_array, gchar *name)
{
    gint index = service_index(service_property_array, name);
    
    if(index == -1)
	return NULL;
    else
	return g_array_index(service_property_array, Service*, index);	
}

static gint service_property_index(Service *service, gchar *name)
{
    GArray *property = service->property;
    
    gint left = 0;
    gint right = property->len - 1;
    
    while(left <= right)
    {
	gint mid = (left + right) / 2;
	ServiceProperty *mid_property = g_array_index(property, ServiceProperty*, mid);
        gint status = g_strcmp0(mid_property->name, name);
	
	if(status == 0)
            return mid; /* Return index of the found service property */
	else if(status > 0)
	    right = mid - 1;
	else if(status < 0)
	    left = mid + 1;
    }
    
    return -1; /* Service property not found */
}

ServiceProperty *lookup_service_property(Service *service, gchar *name)
{
    gint index = service_property_index(service, name);
    
    if(index == -1)
	return NULL;
    else
	return g_array_index(service->property, ServiceProperty*, index);
}
