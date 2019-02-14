#include "servicegroup.h"
#include "serviceproperties.h"

int is_subgroup_of(gchar *current_group, gchar *group)
{
    if(strcmp(group, "") == 0)
        return TRUE;
    else
    {
        gchar *prefix = g_strjoin("", group, "/", NULL);
        int status = g_str_has_prefix(current_group, prefix);
        g_free(prefix);
        return status;
    }
}

static int is_in_group(gchar *current_group, gchar *group)
{
    if(current_group == NULL)
        return FALSE;
    else
    {
        if(g_strcmp0(current_group, group) == 0)
            return TRUE;
        else
            return is_subgroup_of(current_group, group);
    }
}

GHashTable *query_services_in_group(GPtrArray *service_property_array, gchar *group)
{
    unsigned int i;
    GHashTable *queried_services_table = g_hash_table_new(g_str_hash, g_str_equal);

    for(i = 0; i < service_property_array->len; i++)
    {
        Service *current_service = g_ptr_array_index(service_property_array, i);
        gchar *current_group = find_service_property_value(current_service, "group");

        if(g_strcmp0(group, "") == 0 || is_in_group(current_group, group))
        {
            Service *new_service = copy_service(current_service);
            g_hash_table_insert(queried_services_table, new_service->name, new_service);
        }
    }

    return queried_services_table;
}

GPtrArray *create_service_property_array_from_table(GHashTable *table)
{
    GPtrArray *copy_array = g_ptr_array_new();
    GHashTableIter iter;
    gpointer *key;
    gpointer *value;

    g_hash_table_iter_init(&iter, table);

    while(g_hash_table_iter_next(&iter, (gpointer*)&key, (gpointer*)&value))
        g_ptr_array_add(copy_array, value);

    return copy_array;
}

void delete_services_table(GHashTable *services_table)
{
    GHashTableIter iter;
    gpointer *key;
    gpointer *value;

    g_hash_table_iter_init(&iter, services_table);

    while(g_hash_table_iter_next(&iter, (gpointer*)&key, (gpointer*)&value))
        delete_service((Service*)value);

    g_hash_table_destroy(services_table);
}

static void merge_dependencies(GPtrArray *service_dependencies, GPtrArray *group_dependencies, gchar *current_group)
{
    unsigned int i;

    for(i = 0; i < service_dependencies->len; i++)
    {
        gchar *dependency = g_ptr_array_index(service_dependencies, i);

        if(g_strcmp0(dependency, current_group) != 0) /* Ignore dependency on itself */
        {
            unsigned int j;
            int found = FALSE;

            for(j = 0; j < group_dependencies->len; j++)
            {
                gchar *group_dependency = g_ptr_array_index(group_dependencies, j);

                if(g_strcmp0(group_dependency, dependency) == 0)
                {
                    found = TRUE;
                    break;
                }
            }

            if(!found)
            {
                /* If dependency is not a dependency of the group yet, then add it, otherwise ignore to prevent duplicates */
                gchar *group_dependency = g_strdup(dependency);
                g_ptr_array_add(group_dependencies, group_dependency);
            }
        }
    }
}

static void remove_self_reference(GPtrArray *dependencies, gchar *current_group)
{
    unsigned int i;

    for(i = 0; i < dependencies->len; i++)
    {
        gchar *dependency = g_ptr_array_index(dependencies, i);

        if(g_strcmp0(dependency, current_group) == 0)
        {
            g_ptr_array_remove_index(dependencies, i);
            break;
        }
    }
}

static GPtrArray *replace_service_dependency_by_group_dependency(Service *service, GPtrArray *dependencies, gchar *current_group)
{
    unsigned int i;
    GPtrArray *replaced_dependencies = g_ptr_array_new();
    int has_group = FALSE;

    for(i = 0; i < dependencies->len; i++)
    {
        gchar *dependency = g_ptr_array_index(dependencies, i);

        if(g_strcmp0(dependency, current_group) == 0)
        {
            if(!has_group)
                g_ptr_array_add(replaced_dependencies, g_strdup(current_group));

            has_group = TRUE;
            g_free(dependency);
        }
        else if(g_strcmp0(dependency, service->name) == 0)
        {
            if(!has_group)
            {
                g_ptr_array_add(replaced_dependencies, g_strdup(current_group));
                has_group = TRUE;
            }
            g_free(dependency);
        }
        else
            g_ptr_array_add(replaced_dependencies, dependency);
    }

    g_ptr_array_free(dependencies, TRUE);

    return replaced_dependencies;
}

static gchar *generate_group_root(gchar *current_group, gchar *group)
{
    if(current_group == NULL)
        return NULL;
    else
    {
        if(is_subgroup_of(current_group, group))
        {
            gchar *subgroup;
            if(g_strcmp0(group, "") == 0)
                subgroup = current_group;
            else
                subgroup = current_group + strlen(group) + 1;

            gchar **group_tokens = g_strsplit(subgroup, "/", -1);
            gchar *subgroup_root;

            if(group_tokens[0] == NULL)
                subgroup_root = NULL;
            else
                subgroup_root = g_strdup(group_tokens[0]);

            g_strfreev(group_tokens);
            return subgroup_root;
        }
        else
            return NULL;
    }
}

static void replace_service_dependencies_by_groups(Service *service, GHashTable *table, gchar *subgroup_root)
{
    GHashTableIter iter;
    gpointer *key;
    gpointer *value;

    g_hash_table_iter_init(&iter, table);

    while(g_hash_table_iter_next(&iter, (gpointer*)&key, (gpointer*)&value))
    {
        Service *current_service = (Service*)value;

        if(current_service != service)
        {
            current_service->depends_on = replace_service_dependency_by_group_dependency(service, current_service->depends_on, subgroup_root);
            current_service->connects_to = replace_service_dependency_by_group_dependency(service, current_service->connects_to, subgroup_root);
        }
    }
}

static void group_service(GHashTable *queried_services_table, GHashTable *grouped_services_table, Service *service, gchar *group)
{
    gchar *current_group = find_service_property_value(service, "group");
    gchar *subgroup_root = generate_group_root(current_group, group);

    if(subgroup_root != NULL)
    {
        /* If a service is part of a subgroup in this group, then create a single sub group node that abstracts over its properties */
        Service *group_service = g_hash_table_lookup(grouped_services_table, subgroup_root);

        if(group_service == NULL)
        {
            group_service = (Service*)g_malloc(sizeof(Service));
            group_service->name = subgroup_root;
            group_service->property = g_ptr_array_new();
            group_service->connects_to = g_ptr_array_new();
            group_service->depends_on = g_ptr_array_new();
            group_service->group_node = TRUE;
            g_hash_table_insert(grouped_services_table, group_service->name, group_service);
        }
        else
            g_free(subgroup_root);

        /* For all services, substitute a dependency on this service by the group */
        replace_service_dependencies_by_groups(service, queried_services_table, group_service->name);
        replace_service_dependencies_by_groups(service, grouped_services_table, group_service->name);

        /* Eliminate self references on group dependencies */
        remove_self_reference(group_service->connects_to, group_service->name);
        remove_self_reference(group_service->depends_on, group_service->name);

        /* Merge dependencies of the service with the group dependencies */
        merge_dependencies(service->depends_on, group_service->depends_on, group_service->name);
        merge_dependencies(service->connects_to, group_service->connects_to, group_service->name);
    }
    else
    {
        /* If a service does not fit within a sub group, just add it verbatim */
        Service *leaf_service = copy_service(service);
        g_hash_table_insert(grouped_services_table, leaf_service->name, leaf_service);
    }
}

GHashTable *group_services(GHashTable *queried_services_table, gchar *group)
{
    GHashTable *grouped_services_table = g_hash_table_new(g_str_hash, g_str_equal);

    /* Iterate over all queried services and put them into the right sub groups */

    GHashTableIter iter;
    gpointer *key;
    gpointer *value;

    g_hash_table_iter_init(&iter, queried_services_table);

    while(g_hash_table_iter_next(&iter, (gpointer*)&key, (gpointer*)&value))
        group_service(queried_services_table, grouped_services_table, (Service*)value, group);

    return grouped_services_table;
}
