#include "candidatetargetmapping.h"
#include <stdlib.h>
#include <xmlutil.h>

GPtrArray *create_candidate_target_array(const char *candidate_mapping_file)
{
    /* Declarations */
    xmlDocPtr doc;
    xmlNodePtr node_root;
    xmlXPathObjectPtr result;
    GPtrArray *candidate_target_array = NULL;
    
    /* Parse the XML document */
    
    if((doc = xmlParseFile(candidate_mapping_file)) == NULL)
    {
	fprintf(stderr, "Error with candidate mapping XML file!\n");
	xmlCleanupParser();
	return NULL;
    }

    /* Retrieve root element */
    node_root = xmlDocGetRootElement(doc);
    
    if(node_root == NULL)
    {
        fprintf(stderr, "The candidate mapping XML file is empty!\n");
	xmlFreeDoc(doc);
	xmlCleanupParser();
	return NULL;
    }

    /* Query the mapping elements */
    result = executeXPathQuery(doc, "/distribution/distributionitem");
    
    /* Iterate over all the mapping elements and add them to the array */
    
    if(result)
    {
	unsigned int i;
	xmlNodeSetPtr nodeset = result->nodesetval;
	
	/* Create a candidate target array */
        candidate_target_array = g_ptr_array_new();
	
	/* Iterate over all the distributionitem elements */
	for(i = 0; i < nodeset->nodeNr; i++)
        {
	    xmlNodePtr distributionitem_children = nodeset->nodeTab[i]->children;
	    DistributionItem *item = (DistributionItem*)g_malloc(sizeof(DistributionItem));
	    gchar *service = NULL;
	    GPtrArray *targets = NULL;
	    
	    /* Iterate over all the distributionitem children (derivation and target elements) */
	    
	    while(distributionitem_children != NULL)
	    {
		if(xmlStrcmp(distributionitem_children->name, (xmlChar*) "service") == 0)
		    service = g_strdup(distributionitem_children->children->content);
		else if(xmlStrcmp(distributionitem_children->name, (xmlChar*) "targets") == 0)
		{
		    xmlNodePtr targets_children = distributionitem_children->children;
		    targets = g_ptr_array_new();
		    
		    /* Iterate over the targets children */
		    
		    while(targets_children != NULL)
		    {
			if(xmlStrcmp(targets_children->name, (xmlChar*) "target") == 0) /* Only iterate over target nodes */
			{
			    gchar *target = g_strdup(targets_children->children->content);
			    g_ptr_array_add(targets, target);
			}
			
			targets_children = targets_children->next;
		    }
		}
		    
		distributionitem_children = distributionitem_children->next;
	    }
	    
	    /* Add the distributionitem to the array */
	    
	    item->service = service;
	    item->targets = targets;
	    g_ptr_array_add(candidate_target_array, item);
	}
    }
    
    /* Cleanup */
    xmlXPathFreeObject(result);
    xmlFreeDoc(doc);
    xmlCleanupParser();
    
    /* Return the candidate host array */
    return candidate_target_array;
}

void delete_candidate_target_array(GPtrArray *candidate_target_array)
{
    unsigned int i;
    
    for(i = 0; i < candidate_target_array->len; i++)
    {
	DistributionItem *item = g_ptr_array_index(candidate_target_array, i);
	unsigned int j;
	
	g_free(item->service);
	
	if(item->targets != NULL)
	{
	    for(j = 0; j < item->targets->len; j++)
	    {
		gchar *target = g_ptr_array_index(item->targets, j);
		g_free(target);
	    }
	}
	
	g_ptr_array_free(item->targets, TRUE);
	g_free(item);
    }
    
    g_ptr_array_free(candidate_target_array, TRUE);
}

void print_candidate_target_array(const GPtrArray *candidate_target_array)
{
    unsigned int i;
    
    for(i = 0; i < candidate_target_array->len; i++)
    {
	DistributionItem *item = g_ptr_array_index(candidate_target_array, i);
	unsigned int j;
	
	g_print("service: %s\n", item->service);
	g_print("targets:\n");
	
	for(j = 0; j < item->targets->len; j++)
	{
	    gchar *target = g_ptr_array_index(item->targets, j);
	    
	    g_print("  target: %s\n", target);
	}
    }
}

void print_expr_of_candidate_target_array(const GPtrArray *candidate_target_array)
{
    unsigned int i;
    
    g_print("{\n");
    
    for(i = 0; i < candidate_target_array->len; i++)
    {
	DistributionItem *item = g_ptr_array_index(candidate_target_array, i);
	unsigned int j;
	
	g_print("  %s = [\n", item->service);
	
	for(j = 0; j < item->targets->len; j++)
	{
	    gchar *target = g_ptr_array_index(item->targets, j);
	    g_print("    \"%s\"\n", target);
	}
	
	g_print("  ];\n");
    }
    
    g_print("}\n");
}

static gint compare_distribution_item_keys(const DistributionItem **l, const DistributionItem **r)
{
    const DistributionItem *left = *l;
    const DistributionItem *right = *r;
    
    return g_strcmp0(left->service, right->service);
}

DistributionItem *find_distribution_item(GPtrArray *candidate_target_array, gchar *service)
{
    DistributionItem distributionItem;
    DistributionItem *distributionItemPtr = &distributionItem, **ret;
    
    distributionItemPtr->service = service;
    
    ret = bsearch(&distributionItemPtr, candidate_target_array->pdata, candidate_target_array->len, sizeof(gpointer), (int (*)(const void*, const void*)) compare_distribution_item_keys);

    if(ret == NULL)
        return NULL;
    else
        return *ret;
}
