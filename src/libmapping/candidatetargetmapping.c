#include "candidatetargetmapping.h"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <xmlutil.h>

#define BUFFER_SIZE 4096

gchar *generate_distribution_xml_from_expr(char *distribution_expr, char *infrastructure_expr)
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
            char *const args[] = { "dydisnix-xml", "-i", infrastructure_expr, "-d", distribution_expr, NULL };
            
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
		    service = g_strdup((gchar*)distributionitem_children->children->content);
		else if(xmlStrcmp(distributionitem_children->name, (xmlChar*) "targets") == 0)
		{
		    xmlNodePtr targets_children = distributionitem_children->children;
		    targets = g_ptr_array_new();
		    
		    /* Iterate over the targets children */
		    
		    while(targets_children != NULL)
		    {
			if(xmlStrcmp(targets_children->name, (xmlChar*) "target") == 0) /* Only iterate over target nodes */
			{
			    gchar *target = g_strdup((gchar*)targets_children->children->content);
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
    if(candidate_target_array != NULL)
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
