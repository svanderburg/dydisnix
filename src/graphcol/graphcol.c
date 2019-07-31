#include "graphcol.h"
#include <stdlib.h>
#include <nixxml-ghashtable-iter.h>
#include "serviceproperties.h"
#include "infrastructureproperties.h"
#include "candidatetargetmappingtable.h"

typedef struct
{
    gchar *name;
    gint degree;
}
ServiceDegree;

typedef struct
{
    xmlChar *service;
    GPtrArray *adjacentServices;
    gchar *target;
}
VertexAdjacency;

static gint compare_vertex_adjacency_keys(const VertexAdjacency **l, const VertexAdjacency **r)
{
    const VertexAdjacency *left = *l;
    const VertexAdjacency *right = *r;
    
    return xmlStrcmp(left->service, right->service);
}

static VertexAdjacency *find_vertex_adjacency_item(GPtrArray *adjacency_array, gchar *service)
{
    VertexAdjacency vertexAdjacency;
    VertexAdjacency **ret, *vertexAdjacencyPtr = &vertexAdjacency;
    
    vertexAdjacencyPtr->service = (xmlChar*)service;
    
    ret = bsearch(&vertexAdjacencyPtr, adjacency_array->pdata, adjacency_array->len, sizeof(gpointer), (int (*)(const void*, const void*)) compare_vertex_adjacency_keys);

    if(ret == NULL)
        return NULL;
    else
        return *ret;
}

static void add_dependencies_to_adjacency_array(GPtrArray *dependencies, VertexAdjacency *vertex_adjacency)
{
    unsigned int i;

    for(i = 0; i < dependencies->len; i++)
    {
        gchar *dependency = g_ptr_array_index(dependencies, i);
        g_ptr_array_add(vertex_adjacency->adjacentServices, dependency);
    }
}

static void add_interdependent_services_to_adjacency_array(GPtrArray *dependencies, GPtrArray *adjacency_array, Service *current_service)
{
    unsigned int i;

    for(i = 0; i < dependencies->len; i++)
    {
        gchar *dependency = g_ptr_array_index(dependencies, i);
        VertexAdjacency *vertex_adjacency = find_vertex_adjacency_item(adjacency_array, dependency);
        g_ptr_array_add(vertex_adjacency->adjacentServices, current_service->name);
    }
}

static void delete_targets(gpointer data)
{
    GPtrArray *targets = (GPtrArray*)data;
    g_ptr_array_free(targets, TRUE);
}

static gchar *get_first_target_name(GHashTable *targets_table)
{
    gchar *result;
    NixXML_GHashTableOrderedIter iter;
    gchar *key;
    gpointer value;
    NixXML_g_hash_table_ordered_iter_init(&iter, targets_table);

    if(NixXML_g_hash_table_ordered_iter_next(&iter, &key, &value))
        result = (gchar*)key;
    else
        result = NULL;

    NixXML_g_hash_table_ordered_iter_destroy(&iter);
    return result;
}

int graphcol(char *services_xml, char *infrastructure_xml, const unsigned int flags)
{
    int xml = flags & DYDISNIX_FLAG_XML;
    GHashTable *service_table = create_service_table(services_xml, xml);
    GHashTable *targets_table = create_target_property_table(infrastructure_xml, xml);
    GPtrArray *adjacency_array = g_ptr_array_new();
    VertexAdjacency *max_adjacency = NULL;
    GHashTableIter iter;
    gpointer key, value;
    unsigned int i;

    int colored_vertices = 0;

    /* Create adjacency array */

    g_hash_table_iter_init(&iter, service_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
	Service *current_service = (Service*)value;

	VertexAdjacency *vertex_adjacency = (VertexAdjacency*)g_malloc(sizeof(VertexAdjacency));

	vertex_adjacency->service = current_service->name;
	vertex_adjacency->adjacentServices = g_ptr_array_new();

	add_dependencies_to_adjacency_array(current_service->depends_on, vertex_adjacency);
	add_dependencies_to_adjacency_array(current_service->connects_to, vertex_adjacency);

	vertex_adjacency->target = NULL;
	
	g_ptr_array_add(adjacency_array, vertex_adjacency);
    }
    
    /* Add interdependent services on given service to the adjacency array */
    g_hash_table_iter_init(&iter, service_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
	Service *current_service = (Service*)value;

	add_interdependent_services_to_adjacency_array(current_service->depends_on, adjacency_array, current_service);
	add_interdependent_services_to_adjacency_array(current_service->connects_to, adjacency_array, current_service);
    }
    
    /* Determine a vertex with max degree */
    
    for(i = 0; i < adjacency_array->len; i++)
    {
	VertexAdjacency *current_adjacency = g_ptr_array_index(adjacency_array, i);
	
	if((max_adjacency == NULL) || (current_adjacency->adjacentServices->len > max_adjacency->adjacentServices->len))
	    max_adjacency = current_adjacency;
    }
    
    /* Color the max degree vertex with the first color */
    
    max_adjacency->target = get_first_target_name(targets_table);
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
	
	GHashTableIter iter;
	gpointer key, value;

	g_hash_table_iter_init(&iter, targets_table);
	while(g_hash_table_iter_next(&iter, &key, &value))
	{
	    unsigned int j;
	    int exists = FALSE;
	    
	    for(j = 0; j < used_targets->len; j++)
	    {
		gchar *current_used_target = g_ptr_array_index(used_targets, j);
		
		if(xmlStrcmp((xmlChar*) key, (xmlChar*) current_used_target) == 0)
		{
		    exists = TRUE;
		    break;
		}
	    }
	    
	    if(!exists)
	    {
		pickTarget = (gchar*)key;
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

    GHashTable *candidate_target_table = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, delete_targets);
    int automapped = TRUE;

    for(i = 0; i < adjacency_array->len; i++)
    {
        VertexAdjacency *current_adjacency = g_ptr_array_index(adjacency_array, i);

        GPtrArray *targets = g_ptr_array_new();

        if(current_adjacency->target != NULL)
        {
            CandidateTargetMapping *target_mapping = (CandidateTargetMapping*)g_malloc(sizeof(CandidateTargetMapping));
            target_mapping->target = (xmlChar*)current_adjacency->target;
            target_mapping->container = NULL;
            g_ptr_array_add(targets, target_mapping);
        }

        g_hash_table_insert(candidate_target_table, current_adjacency->service, targets);
    }

    if(flags & DYDISNIX_FLAG_OUTPUT_XML)
        print_candidate_target_table_xml(candidate_target_table);
    else
        print_candidate_target_table_nix(candidate_target_table, &automapped);

    /* Cleanup */

    g_hash_table_destroy(candidate_target_table); // TODO: properly clean

    delete_service_table(service_table);
    delete_targets_table(targets_table);

    return 0;
}
