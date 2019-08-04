#include "servicegroup.h"
#include "servicestable.h"
#include <sys/stat.h>
#include <sys/types.h>

int is_subgroup_of(xmlChar *current_group, gchar *group)
{
    if(g_strcmp0(group, "") == 0)
        return TRUE;
    else
    {
        gchar *prefix = g_strjoin("", group, "/", NULL);
        int status = g_str_has_prefix((gchar*)current_group, prefix);
        g_free(prefix);
        return status;
    }
}

static int is_in_group(xmlChar *current_group, gchar *group)
{
    if(current_group == NULL)
        return FALSE;
    else
    {
        if(xmlStrcmp(current_group, (xmlChar*) group) == 0)
            return TRUE;
        else
            return is_subgroup_of(current_group, group);
    }
}

GHashTable *query_services_in_group(GHashTable *service_table, gchar *group)
{
    GHashTable *queried_services_table = g_hash_table_new(g_str_hash, g_str_equal);

    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, service_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        Service *current_service = (Service*)value;

        if(xmlStrcmp(current_service->group, (xmlChar*) "") == 0 || is_in_group(current_service->group, group))
        {
            Service *new_service = copy_service(current_service);
            g_hash_table_insert(queried_services_table, new_service->name, new_service);
        }
    }

    return queried_services_table;
}

static void merge_dependencies(GPtrArray *service_dependencies, GPtrArray *group_dependencies, xmlChar *current_group)
{
    unsigned int i;

    for(i = 0; i < service_dependencies->len; i++)
    {
        gchar *dependency = g_ptr_array_index(service_dependencies, i);

        if(xmlStrcmp((xmlChar*) dependency, current_group) != 0) /* Ignore dependency on itself */
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

static void remove_self_reference(GPtrArray *dependencies, xmlChar *current_group)
{
    unsigned int i;

    for(i = 0; i < dependencies->len; i++)
    {
        gchar *dependency = g_ptr_array_index(dependencies, i);

        if(xmlStrcmp((xmlChar*) dependency, current_group) == 0)
        {
            g_ptr_array_remove_index(dependencies, i);
            break;
        }
    }
}

static GPtrArray *replace_service_dependency_by_group_dependency(Service *service, GPtrArray *dependencies, xmlChar *current_group)
{
    unsigned int i;
    GPtrArray *replaced_dependencies = g_ptr_array_new();
    int has_group = FALSE;

    for(i = 0; i < dependencies->len; i++)
    {
        gchar *dependency = g_ptr_array_index(dependencies, i);

        if(xmlStrcmp((xmlChar*) dependency, current_group) == 0)
        {
            if(!has_group)
                g_ptr_array_add(replaced_dependencies, xmlStrdup(current_group));

            has_group = TRUE;
            g_free(dependency);
        }
        else if(xmlStrcmp((xmlChar*) dependency, service->name) == 0)
        {
            if(!has_group)
            {
                g_ptr_array_add(replaced_dependencies, xmlStrdup(current_group));
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

static void replace_service_dependencies_by_groups(Service *service, GHashTable *table, xmlChar *subgroup_root)
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

static int exactly_fits_in_group(gchar **current_group_tokens, gchar **group_tokens)
{
    if(g_strv_length(current_group_tokens) <= g_strv_length(group_tokens))
    {
        int num_of_tokens, i;

        if(g_strv_length(current_group_tokens) <= g_strv_length(group_tokens))
            num_of_tokens = g_strv_length(current_group_tokens);
        else
            num_of_tokens = g_strv_length(group_tokens);

        for(i = 0; i < num_of_tokens; i++)
        {
            if(g_strcmp0(current_group_tokens[i], group_tokens[i]) != 0)
                return FALSE;
        }

        return TRUE;
    }
    else
        return FALSE;
}

static xmlChar *derive_sub_group_root(gchar **current_group_tokens, gchar **group_tokens)
{
    unsigned int i = 0;

    while(current_group_tokens[i] != NULL && group_tokens[i] != NULL)
    {
        if(g_strcmp0(current_group_tokens[i], group_tokens[i]) != 0)
            return xmlStrdup((xmlChar*)current_group_tokens[i]);

        i++;
    }

    if(current_group_tokens[i] == NULL)
        return NULL;
    else
        return xmlStrdup((xmlChar*) current_group_tokens[i]);
}

static void group_service(GHashTable *queried_services_table, GHashTable *grouped_services_table, Service *service, gchar **group_tokens)
{
    gchar **current_group_tokens;

    if(service->group == NULL)
        current_group_tokens = NULL;
    else
        current_group_tokens = g_strsplit((gchar*)service->group, "/", -1);

    if(service->group == NULL || xmlStrcmp(service->group, (xmlChar*) "") == 0 || exactly_fits_in_group(current_group_tokens, group_tokens))
    {
        /* If a service exactly fits in this group, just add it verbatim */
        Service *leaf_service = copy_service(service);
        g_hash_table_insert(grouped_services_table, leaf_service->name, leaf_service);
    }
    else
    {
        xmlChar *subgroup_root = derive_sub_group_root(current_group_tokens, group_tokens);

        /* If a service is part of a subgroup in this group, then create a single sub group node that abstracts over its properties */
        Service *group_service = g_hash_table_lookup(grouped_services_table, subgroup_root);

        if(group_service == NULL)
        {
            group_service = (Service*)g_malloc(sizeof(Service));
            group_service->name = subgroup_root;
            group_service->type = NULL;
            group_service->group = NULL;
            group_service->properties = g_hash_table_new(g_str_hash, g_str_equal);
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

    g_strfreev(current_group_tokens);
}

GHashTable *group_services(GHashTable *queried_services_table, gchar *group)
{
    GHashTable *grouped_services_table = g_hash_table_new(g_str_hash, g_str_equal);
    gchar **group_tokens;

    if(group == NULL)
        group_tokens = NULL;
    else
        group_tokens = g_strsplit(group, "/", -1);

    /* Iterate over all queried services and put them into the right sub groups */

    GHashTableIter iter;
    gpointer *key;
    gpointer *value;

    g_hash_table_iter_init(&iter, queried_services_table);

    while(g_hash_table_iter_next(&iter, (gpointer*)&key, (gpointer*)&value))
        group_service(queried_services_table, grouped_services_table, (Service*)value, group_tokens);

    g_strfreev(group_tokens);
    return grouped_services_table;
}

GPtrArray *query_unique_groups(GHashTable *service_table)
{
    GHashTable *unique_groups_table = g_hash_table_new(g_str_hash, g_str_equal);
    GHashTableIter iter;
    gpointer *key;
    gpointer *value;
    GPtrArray *unique_groups_array = g_ptr_array_new();

    g_hash_table_iter_init(&iter, service_table);

    while(g_hash_table_iter_next(&iter, (gpointer*)&key, (gpointer*)&value))
    {
        Service *current_service = (Service*)value;

        if(current_service->group != NULL && xmlStrcmp(current_service->group, (xmlChar*) "") != 0)
        {
            if(g_hash_table_lookup(unique_groups_table, current_service->group) == NULL)
                g_hash_table_insert(unique_groups_table, current_service->group, NULL);
        }
    }

    g_hash_table_iter_init(&iter, unique_groups_table);

    while(g_hash_table_iter_next(&iter, (gpointer*)&key, (gpointer*)&value))
        g_ptr_array_add(unique_groups_array, (gchar*)key);

    g_hash_table_destroy(unique_groups_table);

    return unique_groups_array;
}

void mkdirp(const char *dir)
{
    gchar *tmp = g_strdup(dir);
    size_t len = strlen(tmp);
    unsigned int i;

    if(tmp[len - 1] == '/') /* Make it ignore trailing / */
        tmp[len - 1] = '\0';

    for(i = 1; tmp[i] != '\0'; i++)
    {
        if(tmp[i] == '/')
        {
            tmp[i] = '\0';
            mkdir(tmp, S_IRWXU);
            tmp[i] = '/';
        }
    }

    mkdir(tmp, S_IRWXU);
    g_free(tmp);
}

int generate_group_artifacts(GHashTable *table, gchar *group, gchar *output_dir, gchar *filename, gchar *image_format, void *data, int (*generate_artifact) (gchar *filepath, gchar *image_format, gchar *group, void *data, GHashTable *service_table) )
{
    GHashTable *group_table = group_services(table, group);

    gchar *basedir = g_strjoin("", output_dir, "/", group, NULL);
    gchar *filepath = g_strjoin("", basedir, "/", filename, NULL);

    int status;

    mkdirp(basedir);
    status = generate_artifact(filepath, image_format, group, data, group_table);

    g_free(filepath);
    g_free(basedir);
    delete_service_table(group_table);

    return status;
}
