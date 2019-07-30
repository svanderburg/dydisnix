#include "filterbuildable.h"
#include "candidatetargetmappingtable.h"
#include <unistd.h>
#include <procreact_pid.h>

static int instantiate_async(gchar *services_expr, gchar *infrastructure_expr, gchar *distribution_expr, gchar *serviceName, gchar *targetName, char *interface, char *target_property)
{
    pid_t pid = fork();

    if(pid == 0)
    {
        char *const args[] = {"nix-instantiate",
            "--argstr", "servicesFile", services_expr,
            "--argstr", "infrastructureFile", infrastructure_expr,
            "--argstr", "distributionFile", distribution_expr,
            "--argstr", "serviceName", serviceName,
            "--argstr", "targetName", targetName,
            "--argstr", "defaultClientInterface", interface,
            "--argstr", "defaultTargetProperty", target_property,
            DATADIR "/dydisnix/try-build.nix", NULL};
        dup2(2, 1);
        execvp(args[0], args);
        _exit(1);
    }

    return pid;
}

static int instantiate(gchar *services_expr, gchar *infrastructure_expr, gchar *distribution_expr, gchar *serviceName, gchar *targetName, char *interface, char *target_property)
{
    ProcReact_Status status;
    pid_t pid = instantiate_async(services_expr, infrastructure_expr, distribution_expr, serviceName, targetName, interface, target_property);
    int exit_status = procreact_wait_for_exit_status(pid, &status);

    if(status != PROCREACT_STATUS_OK)
        return 1;
    else
        return exit_status;
}

static void delete_filtered_target_table(GHashTable *filtered_target_table)
{
    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, filtered_target_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        GPtrArray *targets = (GPtrArray*)value;
        g_ptr_array_free(targets, TRUE);
    }

    g_hash_table_destroy(filtered_target_table);
}

int filter_buildable(char *services_expr, char *infrastructure_expr, char *distribution_expr, const unsigned int flags, char *interface, char *target_property)
{
    GHashTable *candidate_target_table = create_candidate_target_table(distribution_expr, infrastructure_expr, flags & DYDISNIX_FLAG_XML);

    if(candidate_target_table == NULL)
    {
        g_printerr("Error opening candidate host mapping!\n");
        return 1;
    }
    else
    {
        GHashTable *filtered_target_table = g_hash_table_new(g_str_hash, g_str_equal);

        GHashTableIter iter;
        gpointer key, value;

        g_hash_table_iter_init(&iter, candidate_target_table);
        while(g_hash_table_iter_next(&iter, &key, &value))
        {
            gchar *service = (gchar*)key;
            GPtrArray *targets = (GPtrArray*)value;
            unsigned int i;
            GPtrArray *filtered_targets = g_ptr_array_new();

            for(i = 0; i < targets->len; i++)
            {
                CandidateTargetMapping *mapping = g_ptr_array_index(targets, i);

                if(instantiate(services_expr, infrastructure_expr, distribution_expr, service, (gchar*)mapping->target, interface, target_property) == 0)
                    g_ptr_array_add(filtered_targets, mapping);
            }

            g_hash_table_insert(filtered_target_table, service, filtered_targets);
        }

        /* Print resulting expression */
        if(flags & DYDISNIX_FLAG_OUTPUT_XML)
            print_candidate_target_table_xml(filtered_target_table);
        else
            print_candidate_target_table_nix(filtered_target_table);

        /* Cleanup */
        delete_filtered_target_table(filtered_target_table);
        delete_candidate_target_table(candidate_target_table);

        return 0;
    }
}
