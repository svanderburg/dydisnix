#include "filterbuildable.h"
#include "distributiontable.h"
#include <unistd.h>
#include <procreact_pid.h>

static pid_t instantiate_all_services_on_all_targets_async(gchar *services_expr, gchar *infrastructure_expr, gchar *distribution_expr, gchar *serviceName, gchar *targetName, char *interface, char *target_property, char *extra_params)
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
            "--arg", "disnix", "builtins.storePath " DISNIX_PREFIX,
            "--arg", "extraParams", extra_params,
            DATADIR "/dydisnix/try-build.nix", NULL};
        dup2(2, 1);
        execvp(args[0], args);
        _exit(1);
    }

    return pid;
}

static ProcReact_bool instantiate_all_services_on_all_targets(gchar *services_expr, gchar *infrastructure_expr, gchar *distribution_expr, gchar *serviceName, gchar *targetName, char *interface, char *target_property, char *extra_params)
{
    ProcReact_Status status;
    pid_t pid = instantiate_all_services_on_all_targets_async(services_expr, infrastructure_expr, distribution_expr, serviceName, targetName, interface, target_property, extra_params);
    ProcReact_bool exit_status = procreact_wait_for_boolean(pid, &status);

    if(status == PROCREACT_STATUS_OK)
        return exit_status;
    else
        return FALSE;
}

GHashTable *filter_unbuildable_derivations_from_distribution_table(GHashTable *distribution_table, char *services_expr, char *infrastructure_expr, char *distribution_expr, char *interface, char *target_property, char *extra_params)
{
    GHashTable *filtered_distribution_table = g_hash_table_new(g_str_hash, g_str_equal);

    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, distribution_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        gchar *service = (gchar*)key;
        GPtrArray *targets = (GPtrArray*)value;
        unsigned int i;
        GPtrArray *filtered_targets = g_ptr_array_new();

        for(i = 0; i < targets->len; i++)
        {
            DistributionMapping *mapping = g_ptr_array_index(targets, i);

            if(instantiate_all_services_on_all_targets(services_expr, infrastructure_expr, distribution_expr, service, (gchar*)mapping->target, interface, target_property, extra_params))
                g_ptr_array_add(filtered_targets, mapping);
        }

        g_hash_table_insert(filtered_distribution_table, service, filtered_targets);
    }

    return filtered_distribution_table;
}

static void delete_filtered_distribution_table(GHashTable *filtered_distribution_table)
{
    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, filtered_distribution_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        GPtrArray *targets = (GPtrArray*)value;
        g_ptr_array_free(targets, TRUE);
    }

    g_hash_table_destroy(filtered_distribution_table);
}

int filter_buildable(char *services_expr, char *infrastructure_expr, char *distribution_expr, const unsigned int flags, char *interface, char *target_property, char *extra_params)
{
    NixXML_bool automapped;
    GHashTable *distribution_table = create_distribution_table(distribution_expr, infrastructure_expr, extra_params, flags & DYDISNIX_FLAG_XML, &automapped);

    if(distribution_table == NULL)
    {
        g_printerr("Error opening distribution model!\n");
        return 1;
    }
    else
    {
        GHashTable *filtered_distribution_table = filter_unbuildable_derivations_from_distribution_table(distribution_table, services_expr, infrastructure_expr, distribution_expr, interface, target_property, extra_params);

        /* Print resulting expression */
        if(flags & DYDISNIX_FLAG_OUTPUT_XML)
            print_distribution_table_xml(stdout, filtered_distribution_table, 0, NULL, NULL);
        else
            print_distribution_table_nix(stdout, filtered_distribution_table, 0, &automapped);

        /* Cleanup */
        delete_filtered_distribution_table(filtered_distribution_table);
        delete_distribution_table(distribution_table);

        return 0;
    }
}
