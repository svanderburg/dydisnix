#include "candidatetargetmapping.h"
#include <unistd.h>
#include <procreact_pid.h>

static int instantiate_async(gchar *services_expr, gchar *infrastructure_expr, gchar *distribution_expr, gchar *serviceName, gchar *targetName)
{
    pid_t pid = fork();

    if(pid == 0)
    {
        char *const args[] = {"nix-instantiate", "--argstr", "servicesFile", services_expr, "--argstr", "infrastructureFile", infrastructure_expr, "--argstr", "distributionFile", distribution_expr, "--argstr", "serviceName", serviceName, "--argstr", "targetName", targetName, DATADIR "/dydisnix/try-build.nix", NULL};
        dup2(2, 1);
        execvp(args[0], args);
        _exit(1);
    }

    return pid;
}

static int instantiate(gchar *services_expr, gchar *infrastructure_expr, gchar *distribution_expr, gchar *serviceName, gchar *targetName)
{
    ProcReact_Status status;
    pid_t pid = instantiate_async(services_expr, infrastructure_expr, distribution_expr, serviceName, targetName);
    int exit_status = procreact_wait_for_exit_status(pid, &status);

    if(status != PROCREACT_STATUS_OK)
        return 1;
    else
        return exit_status;
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

int filter_buildable(char *services_expr, char *infrastructure_expr, char *distribution_expr, int xml)
{
    unsigned int i;
    GPtrArray *candidate_target_array = create_candidate_target_array(distribution_expr, infrastructure_expr, xml);

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
