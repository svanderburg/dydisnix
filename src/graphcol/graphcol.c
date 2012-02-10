#include "graphcol.h"
#include "serviceproperties.h"
#include "infrastructureproperties.h"

typedef struct
{
    gchar *name;
    gint degree;
}
ServiceDegree;

static int compare_service_degree(gconstpointer a, gconstpointer b)
{
    const ServiceDegree *l = (ServiceDegree*)a;
    const ServiceDegree *r = (ServiceDegree*)b;
    
    if(l->degree < r->degree)
	return 1;
    else if(l->degree > r->degree)
	return -1;
    else
	return 0;
}

typedef struct
{
    gchar *service;
    GArray *adjacentServices;
    gchar *target;
}
VertexAdjacency;

int graphcol(const char *services_xml, const char *infrastructure_xml)
{
    GArray *service_property_array = create_service_property_array(services_xml);
    GArray *infrastructure_property_array = create_infrastructure_property_array(infrastructure_xml);
    GArray *adjacency_array = g_array_new(FALSE, FALSE, sizeof(VertexAdjacency*));
    VertexAdjacency *max_adjacency = NULL;
    VertexAdjacency *max_saturation_adjacency = NULL;
    unsigned int i;
    int maxSaturationDegree = 0;
    int colored_vertices = 0;
    
    /* Create adjacency array */
    
    for(i = 0; i < service_property_array->len; i++)
    {
	Service *current_service = g_array_index(service_property_array, Service*, i);
	ServiceProperty *dependsOn = lookup_service_property(current_service, "dependsOn");
	
	VertexAdjacency *vertex_adjacency = (VertexAdjacency*)g_malloc(sizeof(VertexAdjacency));

	vertex_adjacency->service = current_service->name;
	vertex_adjacency->adjacentServices = g_array_new(FALSE, FALSE, sizeof(gchar*));
	
	if(dependsOn != NULL && dependsOn->value != NULL)
	{
	    unsigned int j;
	    gchar **dependencies = g_strsplit(dependsOn->value, " ", 0);
	
	    for(j = 0; j < g_strv_length(dependencies) - 1; j++)
	        g_array_append_val(vertex_adjacency->adjacentServices, dependencies[j]);
	
	    g_free(dependencies);
	}
	
	vertex_adjacency->target = NULL;
	
	g_array_append_val(adjacency_array, vertex_adjacency);
    }
    
    /* Add interdependent services on given service to the adjacency array */
    for(i = 0; i < service_property_array->len; i++)
    {
	Service *current_service = g_array_index(service_property_array, Service*, i);
	ServiceProperty *dependsOn = lookup_service_property(current_service, "dependsOn");
	
	if(dependsOn != NULL && dependsOn->value != NULL)
	{
	    unsigned int j;
	    gchar **dependencies = g_strsplit(dependsOn->value, " ", 0);
	    
	    for(j = 0; j < g_strv_length(dependencies) - 1; j++)
	    {
		gint index = service_index(service_property_array, dependencies[j]);
		VertexAdjacency *vertex_adjacency = g_array_index(adjacency_array, VertexAdjacency*, index);
		
		g_array_append_val(vertex_adjacency->adjacentServices, current_service->name);
	    }
	    
	    g_free(dependencies);
	}
    }
    
    /* Determine a vertex with max degree */
    
    for(i = 0; i < adjacency_array->len; i++)
    {
	VertexAdjacency *current_adjacency = g_array_index(adjacency_array, VertexAdjacency*, i);
	
	if((max_adjacency == NULL) || (current_adjacency->adjacentServices->len > max_adjacency->adjacentServices->len))
	    max_adjacency = current_adjacency;
    }
    
    /* Color the max degree vertex with the first color */
    
    max_adjacency->target = (g_array_index(infrastructure_property_array, Target*, 0))->name;
    colored_vertices++;
    
    while(colored_vertices < adjacency_array->len)
    {
	gchar *pickTarget = NULL;
	
	/* Determine the uncolored vertex with maximum saturation degree */
	
	for(i = 0; i < adjacency_array->len; i++)
	{
	    int saturationDegree = 0;
	    VertexAdjacency *current_adjacency = g_array_index(adjacency_array, VertexAdjacency*, i);
	    unsigned int j;
	    
	    if(current_adjacency->target == NULL)
	    {
		for(j = 0; j < current_adjacency->adjacentServices->len; j++)
		{
		    gchar *adjacentServiceName = g_array_index(current_adjacency->adjacentServices, gchar*, j);
		    int index = service_index(service_property_array, adjacentServiceName);
		    VertexAdjacency *adjacentService = g_array_index(adjacency_array, VertexAdjacency*, index);
		
		    if(adjacentService->target != NULL)
			saturationDegree++;
		}
		
		if((max_saturation_adjacency == NULL) || (saturationDegree > maxSaturationDegree))
		{
		    maxSaturationDegree = saturationDegree;
		    max_saturation_adjacency = current_adjacency;
		}
	    }
	}
	
	/* Color the vertex with max saturation degree, with the lowest available color */
	
	/* Determine which colors are already used by the neighbours */
	
	GArray *used_targets = g_array_new(FALSE, FALSE, sizeof(gchar*));
	
	for(i = 0; i < max_saturation_adjacency->adjacentServices->len; i++)
	{
	    gchar *adjacentServiceName = g_array_index(max_saturation_adjacency->adjacentServices, gchar*, i);
	    int index = service_index(service_property_array, adjacentServiceName);
	    VertexAdjacency *adjacentService = g_array_index(adjacency_array, VertexAdjacency*, index);
	    
	    if(adjacentService->target != NULL)
		g_array_append_val(used_targets, adjacentService->target);
	}
	
	/* Look in the targets array for the first target that is not in used targets (which we mean by the lowest color) */
	
	for(i = 0; i < infrastructure_property_array->len; i++)
	{
	    unsigned int j;
	    Target *current_target = g_array_index(infrastructure_property_array, Target*, i);
	    int exists = FALSE;
	    
	    for(j = 0; j < used_targets->len; j++)
	    {
		gchar *current_used_target = g_array_index(used_targets, char*, j);
		
		if(g_strcmp0(current_target->name, current_used_target) == 0)
		{
		    exists = TRUE;
		    break;
		}
	    }

	    if(!exists)
	    {
		pickTarget = current_target->name;
		break;
	    }
	}
	
	g_array_free(used_targets, TRUE);
	
	if(pickTarget == NULL)
	{
	    g_printerr("Cannot pick a target for service: %s\n", max_saturation_adjacency->service);
	    return 1;
	}
	else
	{
	    max_saturation_adjacency->target = pickTarget;
	    colored_vertices++;
	}
    }
    
    /* Print output expression */
    
    g_print("{\n");
    
    for(i = 0; i < adjacency_array->len; i++)
    {
	VertexAdjacency *current_adjacency = g_array_index(adjacency_array, VertexAdjacency*, i);
	
	if(current_adjacency->target == NULL)
	    g_print("  %s = [];\n", current_adjacency->service);
	else
	    g_print("  %s = [ \"%s\" ];\n", current_adjacency->service, current_adjacency->target);
    }
    
    g_print("}\n");
    
    /* Cleanup */
    delete_service_property_array(service_property_array);
    delete_infrastructure_property_array(infrastructure_property_array);
    
    return 0;
}
