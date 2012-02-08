#include "candidatetargetmapping.h"
#include <xmlutil.h>

GArray *create_candidate_target_array(const char *candidate_mapping_file)
{
    /* Declarations */    
    xmlDocPtr doc;
    xmlNodePtr node_root;
    xmlXPathObjectPtr result;
    GArray *candidate_target_array = NULL;
    
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
        candidate_target_array = g_array_new(FALSE, FALSE, sizeof(DistributionItem*));
	
	/* Iterate over all the distributionitem elements */
	for(i = 0; i < nodeset->nodeNr; i++)
        {
	    xmlNodePtr distributionitem_children = nodeset->nodeTab[i]->children;
	    DistributionItem *item = (DistributionItem*)g_malloc(sizeof(DistributionItem));
	    gchar *service = NULL;
	    GArray *targets = NULL;
	    
	    /* Iterate over all the distributionitem children (derivation and target elements) */
	    
	    while(distributionitem_children != NULL)
	    {
		if(xmlStrcmp(distributionitem_children->name, (xmlChar*) "service") == 0)
		    service = g_strdup(distributionitem_children->children->content);
		else if(xmlStrcmp(distributionitem_children->name, (xmlChar*) "targets") == 0)
		{
		    xmlNodePtr targets_children = distributionitem_children->children;
		    targets = g_array_new(FALSE, FALSE, sizeof(gchar*));
		    
		    /* Iterate over the targets children */
		    
		    while(targets_children != NULL)
		    {
			if(xmlStrcmp(targets_children->name, (xmlChar*) "target") == 0) /* Only iterate over target nodes */
			{
			    gchar *target = g_strdup(targets_children->children->content);
			    g_array_append_val(targets, target);
			}
			
			targets_children = targets_children->next;
		    }
		}
		    
		distributionitem_children = distributionitem_children->next;
	    }
	    
	    /* Add the distributionitem to the array */
	    
	    item->service = service;
	    item->targets = targets;
	    g_array_append_val(candidate_target_array, item);
	}	
    }
    
    /* Cleanup */
    xmlXPathFreeObject(result);
    xmlFreeDoc(doc);
    xmlCleanupParser();
    
    /* Return the candidate host array */
    return candidate_target_array;
}

void delete_candidate_target_array(GArray *candidate_target_array)
{
    unsigned int i;
    
    for(i = 0; i < candidate_target_array->len; i++)
    {
	DistributionItem *item = g_array_index(candidate_target_array, DistributionItem*, i);
	unsigned int j;
	
	g_free(item->service);
	
	if(item->targets != NULL)
	{
	    for(j = 0; j < item->targets->len; j++)
	    {
		gchar *target = g_array_index(item->targets, gchar*, j);
		g_free(target);	    
	    }
	}
	
	g_array_free(item->targets, TRUE);
	g_free(item);
    }
    
    g_array_free(candidate_target_array, TRUE);
}

void print_candidate_target_array(const GArray *candidate_target_array)
{
    unsigned int i;
    
    for(i = 0; i < candidate_target_array->len; i++)
    {
	DistributionItem *item = g_array_index(candidate_target_array, DistributionItem*, i);
	unsigned int j;
	
	g_print("service: %s\n", item->service);
	g_print("targets:\n");
	
	for(j = 0; j < item->targets->len; j++)
	{
	    gchar *target = g_array_index(item->targets, gchar*, j);
	    
	    g_print("  target: %s\n", target);
	}
    }
}

void print_expr_of_candidate_target_array(const GArray *candidate_target_array)
{
    unsigned int i;
    
    g_print("{\n");
    
    for(i = 0; i < candidate_target_array->len; i++)
    {
	DistributionItem *item = g_array_index(candidate_target_array, DistributionItem*, i);
	unsigned int j;
	
	g_print("  %s = [\n", item->service);
	
	for(j = 0; j < item->targets->len; j++)
	{
	    gchar *target = g_array_index(item->targets, gchar*, j);
	    g_print("    \"%s\"\n", target);
	}
	
	g_print("  ];\n");
    }
    
    g_print("}\n");
}

gint distribution_item_index(GArray *candidate_target_array, gchar *service)
{
    gint left = 0;
    gint right = candidate_target_array->len - 1;
    
    while(left <= right)
    {
	gint mid = (left + right) / 2;
	DistributionItem *mid_distribution_item = g_array_index(candidate_target_array, DistributionItem*, mid);
        gint status = g_strcmp0(mid_distribution_item->service, service);
	
	if(status == 0)
            return mid; /* Return index of the found service */
	else if(status > 0)
	    right = mid - 1;
	else if(status < 0)
	    left = mid + 1;
    }
    
    return -1; /* service not found */
}

DistributionItem *lookup_distribution_item(GArray *candidate_target_array, gchar *service)
{
    gint index = distribution_item_index(candidate_target_array, service);
    
    if(index == -1)
	return NULL;
    else
	return g_array_index(candidate_target_array, DistributionItem*, index);
}
