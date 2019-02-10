#include "serviceproperties.h"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <xmlutil.h>

#define BUFFER_SIZE 4096

gchar *generate_service_xml_from_expr(char *service_expr)
{
    int pipefd[2];
    
    if(pipe(pipefd) == 0)
    {
        int status = fork();
        
        if(status == -1)
        {
            g_printerr("Error with forking dydisnix-xml process!\n");
            return NULL;
        }
        else if(status == 0)
        {
            char *const args[] = { "dydisnix-xml", "-s", service_expr, NULL };
            
            close(pipefd[0]); /* Close read-end of pipe */
            dup2(pipefd[1], 1);
            execvp("dydisnix-xml", args);
            _exit(1);
        }
        else
        {
            char line[BUFFER_SIZE];
            ssize_t line_size;
        
            close(pipefd[1]); /* Close write-end of pipe */
            
            line_size = read(pipefd[0], line, BUFFER_SIZE - 1);
            line[line_size - 1] = '\0'; /* Replace linefeed char with termination */

            close(pipefd[0]);
            
            wait(&status);
            
            if(WEXITSTATUS(status) == 0)
                return g_strdup(line);
            else
                return NULL;
        }
    }
    else
    {
        g_printerr("Error with creating a pipe\n");
        return NULL;
    }
}

GPtrArray *create_service_property_array(const gchar *services_xml_file)
{
    /* Declarations */
    xmlDocPtr doc;
    xmlNodePtr node_root;
    xmlXPathObjectPtr result;
    GPtrArray *service_property_array = NULL;

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
	service_property_array = g_ptr_array_new();
	
        for(i = 0; i < nodeset->nodeNr; i++)
        {
	    Service *service = (Service*)g_malloc(sizeof(Service));
	    GPtrArray *property = g_ptr_array_new();
	    xmlAttr *service_properties = nodeset->nodeTab[i]->properties;
	    xmlNodePtr service_children = nodeset->nodeTab[i]->children;
	    
	    /* Assign values */
	    service->name = NULL;
	    service->property = property;
	    service->connects_to = g_ptr_array_new();
	    service->depends_on = g_ptr_array_new();
	    service->group_node = FALSE;

	    /* Read name attribute */
	    while(service_properties != NULL)
	    {
		if(xmlStrcmp(service_properties->name, (const xmlChar*) "name") == 0)
		    service->name = g_strdup((gchar*)service_properties->children->content);
	    
		service_properties = service_properties->next;
	    }
	    
	    /* Iterate over the service properties */
	    
	    while(service_children != NULL)
	    {
		if(xmlStrcmp(service_children->name, (const xmlChar*) "connectsTo") == 0)
		{
		    xmlNodePtr connects_to_children = service_children->children;

		    while(connects_to_children != NULL)
		    {
			gchar *dependency = g_strdup((gchar*)connects_to_children->children->content);
			g_ptr_array_add(service->connects_to, dependency);
			connects_to_children = connects_to_children->next;
		    }
		}
		else if(xmlStrcmp(service_children->name, (const xmlChar*) "dependsOn") == 0)
		{
		    xmlNodePtr depends_on_children = service_children->children;

		    while(depends_on_children != NULL)
		    {
			gchar *dependency = g_strdup((gchar*)depends_on_children->children->content);
			g_ptr_array_add(service->depends_on, dependency);
			depends_on_children = depends_on_children->next;
		    }
		}
		else if(xmlStrcmp(service_children->name, (const xmlChar*) "properties") == 0)
		{
		    xmlNodePtr properties_children = service_children->children;

		    while(properties_children != NULL)
		    {
			ServiceProperty *service_property = (ServiceProperty*)g_malloc(sizeof(ServiceProperty));

			if(xmlStrcmp(properties_children->name, (const xmlChar*) "text") != 0)
			{
			    service_property->name = g_strdup((gchar*)properties_children->name);

			    if(properties_children->children == NULL) /* !!! also support lists in the future */
				service_property->value = NULL;
			    else
				service_property->value = g_strdup((gchar*)properties_children->children->content);
			
			    g_ptr_array_add(property, service_property);
			}

			properties_children = properties_children->next;
		    }
		}

		service_children = service_children->next;
	    }
	    
	    g_ptr_array_add(service_property_array, service);
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

void delete_service(Service *service)
{
    unsigned int i;

    g_free(service->name);

    for(i = 0; i < service->property->len; i++)
    {
        ServiceProperty *service_property = g_ptr_array_index(service->property, i);

        g_free(service_property->name);
        g_free(service_property->value);
        g_free(service_property);
    }

    g_ptr_array_free(service->property, TRUE);

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
            unsigned int j;
            
            g_print("Service name: %s\n", service->name);
            g_print("Properties:\n");

            for(j = 0; j < service->property->len; j++)
            {
                ServiceProperty *service_property = g_ptr_array_index(service->property, j);
                g_print("  name: %s, value: %s\n", service_property->name, service_property->value);
            }
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

static gint compare_service_property_keys(const ServiceProperty **l, const ServiceProperty **r)
{
    const ServiceProperty *left = *l;
    const ServiceProperty *right = *r;
    
    return g_strcmp0(left->name, right->name);
}

ServiceProperty *find_service_property(const Service *service, gchar *name)
{
    ServiceProperty serviceProperty;
    ServiceProperty **ret, *servicePropertyPtr = &serviceProperty;
    
    servicePropertyPtr->name = name;
    
    ret = bsearch(&servicePropertyPtr, service->property->pdata, service->property->len, sizeof(gpointer), (int (*)(const void*, const void*)) compare_service_property_keys);

    if(ret == NULL)
        return NULL;
    else
        return *ret;
}

gchar *find_service_property_value(const Service *service, gchar *name)
{
    ServiceProperty *prop = find_service_property(service, name);

    if(prop == NULL)
        return NULL;
    else
        return prop->value;
}
