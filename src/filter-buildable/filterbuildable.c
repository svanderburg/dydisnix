#include "candidatetargetmapping.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

static int instantiate(gchar *services_expr, gchar *infrastructure_expr, gchar *distribution_expr, gchar *serviceName, gchar *targetName)
{
    int status = fork();
    
    if(status == 0)
    {
	char *const args[] = {"nix-instantiate", "--argstr", "servicesFile", services_expr, "--argstr", "infrastructureFile", infrastructure_expr, "--argstr", "distributionFile", distribution_expr, "--argstr", "serviceName", serviceName, "--argstr", "targetName", targetName, DATADIR "/dydisnix/try-build.nix", NULL};
	dup2(2, 1);
	execvp("nix-instantiate", args);
	_exit(1);
    }
    
    wait(&status);
    
    return WEXITSTATUS(status);
}

static void delete_filtered_target_array(GPtrArray *filtered_target_array)
{
    unsigned int i;
    
    for(i = 0; i < filtered_target_array->len; i++)
    {
	DistributionItem *item = g_ptr_array_index(filtered_target_array, i);
	g_free(item);
    }
    
    g_ptr_array_free(filtered_target_array, TRUE);
}

int filter_buildable(char *services_expr, char *infrastructure_expr, char *distribution_expr)
{
    unsigned int i;
    GPtrArray *candidate_target_array = create_candidate_target_array_from_nix(distribution_expr, infrastructure_expr);

    if(candidate_target_array == NULL)
    {
	g_printerr("Error opening candidate host mapping!\n");
	return 1;
    }
    else
    {
	GPtrArray *filtered_target_array = g_ptr_array_new();
    
	for(i = 0; i < candidate_target_array->len; i++)
	{
	    DistributionItem *filter_item = g_malloc(sizeof(DistributionItem));
    	    DistributionItem *item = g_ptr_array_index(candidate_target_array, i);
	    unsigned int j;
	
	    filter_item->service = item->service;
	    filter_item->targets = g_ptr_array_new();
	
	    for(j = 0; j < item->targets->len; j++)
	    {
		gchar *target = g_ptr_array_index(item->targets, j);
	    
		if(instantiate(services_expr, infrastructure_expr, distribution_expr, item->service, target) == 0)
	    	    g_ptr_array_add(filter_item->targets, target);
	    }
	
	    g_ptr_array_add(filtered_target_array, filter_item);
	}
    
	/* Print resulting expression */
	print_expr_of_candidate_target_array(filtered_target_array);
    
	/* Cleanup */
	delete_filtered_target_array(filtered_target_array);
	delete_candidate_target_array(candidate_target_array);
	
	return 0;
    }
}
