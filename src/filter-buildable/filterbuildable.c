#include "candidatetargetmapping.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

static int instantiate(gchar *services_expr, gchar *infrastructure_expr, gchar *distribution_expr, gchar *serviceName, gchar *targetName)
{
    int status = fork();
    
    if(status == 0)
    {
	char *args[] = {"nix-instantiate", "--argstr", "servicesFile", services_expr, "--argstr", "infrastructureFile", infrastructure_expr, "--argstr", "distributionFile", distribution_expr, "--argstr", "serviceName", serviceName, "--argstr", "targetName", targetName, DATADIR "/dydisnix/try-build.nix", NULL};
	dup2(2, 1);
	execvp("nix-instantiate", args);
	_exit(1);
    }
    
    wait(&status);
    
    return WEXITSTATUS(status);
}

static void delete_filtered_target_array(GArray *filtered_target_array)
{
    unsigned int i;
    
    for(i = 0; i < filtered_target_array->len; i++)
    {
	DistributionItem *item = g_array_index(filtered_target_array, DistributionItem*, i);
	g_free(item);
    }
    
    g_array_free(filtered_target_array, TRUE);
}

int filter_buildable(char *services_expr, char *infrastructure_expr, char *distribution_expr, char *distribution_xml)
{
    unsigned int i;
    GArray *candidate_target_array = create_candidate_target_array(distribution_xml);

    if(candidate_target_array == NULL)
    {
	g_printerr("Error opening candidate host mapping!\n");
	return 1;
    }
    else
    {
	GArray *filtered_target_array = g_array_new(FALSE, FALSE, sizeof(DistributionItem*));
    
	for(i = 0; i < candidate_target_array->len; i++)
	{
	    DistributionItem *filter_item = g_malloc(sizeof(DistributionItem));
    	    DistributionItem *item = g_array_index(candidate_target_array, DistributionItem*, i);
	    unsigned int j;
	
	    filter_item->service = item->service;
	    filter_item->targets = g_array_new(FALSE, FALSE, sizeof(gchar*));
	
	    for(j = 0; j < item->targets->len; j++)
	    {
		gchar *target = g_array_index(item->targets, gchar*, j);
	    
		if(instantiate(services_expr, infrastructure_expr, distribution_expr, item->service, target) == 0)
	    	    g_array_append_val(filter_item->targets, target);
	    }
	
	    g_array_append_val(filtered_target_array, filter_item);
	}
    
	/* Print resulting expression */
	print_expr_of_candidate_target_array(filtered_target_array);
    
	/* Cleanup */
	delete_filtered_target_array(filtered_target_array);
	delete_candidate_target_array(candidate_target_array);
	
	return 0;
    }
}
