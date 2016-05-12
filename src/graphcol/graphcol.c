#include "graphcol.h"
#include <stdlib.h>
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
    GPtrArray *adjacentServices;
    gchar *target;
}
VertexAdjacency;

static gint compare_vertex_adjacency_keys(const VertexAdjacency **l, const VertexAdjacency **r)
{
    const VertexAdjacency *left = *l;
    const VertexAdjacency *right = *r;
    
    return g_strcmp0(left->service, right->service);
}

static VertexAdjacency *find_vertex_adjacency_item(GPtrArray *adjacency_array, gchar *service)
{
    VertexAdjacency vertexAdjacency;
    VertexAdjacency **ret, *vertexAdjacencyPtr = &vertexAdjacency;
    
    vertexAdjacencyPtr->service = service;
    
    ret = bsearch(&vertexAdjacencyPtr, adjacency_array->pdata, adjacency_array->len, sizeof(gpointer), (int (*)(const void*, const void*)) compare_vertex_adjacency_keys);

    if(ret == NULL)
        return NULL;
    else
        return *ret;
}

int graphcol(const char *services_xml, const char *infrastructure_xml)
{
    GPtrArray *service_property_array = create_service_property_array(services_xml);
    GPtrArray *targets_array = create_target_array_from_xml(infrastructure_xml);
    GPtrArray *adjacency_array = g_ptr_array_new();
    VertexAdjacency *max_adjacency = NULL;
    unsigned int i;
    
    int colored_vertices = 0;
    
    /* Create adjacency array */
    
    for(i = 0; i < service_property_array->len; i++)
    {
	Service *current_service = g_ptr_array_index(service_property_array, i);
	ServiceProperty *dependsOn = find_service_property(current_service, "dependsOn");
	
	VertexAdjacency *vertex_adjacency = (VertexAdjacency*)g_malloc(sizeof(VertexAdjacency));

	vertex_adjacency->service = current_service->name;
	vertex_adjacency->adjacentServices = g_ptr_array_new();
	
	if(dependsOn != NULL && dependsOn->value != NULL)
	{
	    unsigned int j;
	    gchar **dependencies = g_strsplit(dependsOn->value, " ", 0);
	
	    for(j = 0; j < g_strv_length(dependencies) - 1; j++)
	        g_ptr_array_add(vertex_adjacency->adjacentServices, dependencies[j]);
	
	    g_free(dependencies);
	}
	
	vertex_adjacency->target = NULL;
	
	g_ptr_array_add(adjacency_array, vertex_adjacency);
    }
    
    /* Add interdependent services on given service to the adjacency array */
    for(i = 0; i < service_property_array->len; i++)
    {
	Service *current_service = g_ptr_array_index(service_property_array, i);
	ServiceProperty *dependsOn = find_service_property(current_service, "dependsOn");
	
	if(dependsOn != NULL && dependsOn->value != NULL)
	{
	    unsigned int j;
	    gchar **dependencies = g_strsplit(dependsOn->value, " ", 0);
	    
	    for(j = 0; j < g_strv_length(dependencies) - 1; j++)
	    {
		VertexAdjacency *vertex_adjacency = find_vertex_adjacency_item(adjacency_array, dependencies[j]);
		g_ptr_array_add(vertex_adjacency->adjacentServices, current_service->name);
	    }
	    
	    g_free(dependencies);
	}
    }
    
    /* Determine a vertex with max degree */
    
    for(i = 0; i < adjacency_array->len; i++)
    {
	VertexAdjacency *current_adjacency = g_ptr_array_index(adjacency_array, i);
	
	if((max_adjacency == NULL) || (current_adjacency->adjacentServices->len > max_adjacency->adjacentServices->len))
	    max_adjacency = current_adjacency;
    }
    
    /* Color the max degree vertex with the first color */
    
    max_adjacency->target = ((Target*)g_ptr_array_index(targets_array, 0))->name;
    colored_vertices++;
    
    while(colored_vertices < adjacency_array->len)
    {
	gchar *pickTarget = NULL;
	VertexAdjacency *max_saturation_adjacency = NULL;
	int maxSaturationDegree = 0;
	
	/* Determine the uncolored vertex with maximum saturation degree */
	
	for(i = 0; i < adjacency_array->len; i++)
	{
	    VertexAdjacency *current_adjacency = g_ptr_array_index(adjacency_array, i);
	    unsigned int j;
	    
	    if(current_adjacency->target == NULL)
	    {
		int saturationDegree = 0;
		
		for(j = 0; j < current_adjacency->adjacentServices->len; j++)
		{
		    gchar *adjacentServiceName = g_ptr_array_index(current_adjacency->adjacentServices, j);
		    VertexAdjacency *adjacentService = find_vertex_adjacency_item(adjacency_array, adjacentServiceName);
		
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
	
	GPtrArray *used_targets = g_ptr_array_new();
	
	for(i = 0; i < max_saturation_adjacency->adjacentServices->len; i++)
	{
	    gchar *adjacentServiceName = g_ptr_array_index(max_saturation_adjacency->adjacentServices, i);
	    VertexAdjacency *adjacentService = find_vertex_adjacency_item(adjacency_array, adjacentServiceName);
	    
	    if(adjacentService->target != NULL)
		g_ptr_array_add(used_targets, adjacentService->target);
	}
	
	/* Look in the targets array for the first target that is not in used targets (which we mean by the lowest color) */
	
	for(i = 0; i < targets_array->len; i++)
	{
	    unsigned int j;
	    Target *current_target = g_ptr_array_index(targets_array, i);
	    int exists = FALSE;
	    
	    for(j = 0; j < used_targets->len; j++)
	    {
		gchar *current_used_target = g_ptr_array_index(used_targets, j);
		
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
	
	g_ptr_array_free(used_targets, TRUE);
	
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
	VertexAdjacency *current_adjacency = g_ptr_array_index(adjacency_array, i);
	
	if(current_adjacency->target == NULL)
	    g_print("  %s = [];\n", current_adjacency->service);
	else
	    g_print("  %s = [ \"%s\" ];\n", current_adjacency->service, current_adjacency->target);
    }
    
    g_print("}\n");
    
    /* Cleanup */
    delete_service_property_array(service_property_array);
    delete_target_array(targets_array);
    
    return 0;
}
