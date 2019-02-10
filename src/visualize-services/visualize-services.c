#include "visualize-services.h"
#include <serviceproperties.h>

static void print_type(const Service *service)
{
    ServiceProperty *prop = find_service_property(service, "type");

    if(prop != NULL)
        g_print("\\n(%s)", prop->value);
}

static void generate_architecture_diagram(const GPtrArray *service_property_array)
{
    unsigned int i;

    g_print("digraph G {\n");
    g_print("node [style=filled,fillcolor=white,color=black];\n");

    /* Generate vertexes */
    for(i = 0; i < service_property_array->len; i++)
    {
         Service *current_service = g_ptr_array_index(service_property_array, i);
         g_print("\"%s\" [ label = \"%s", current_service->name, current_service->name);
         print_type(current_service);
         g_print("\"");

         if(current_service->group_node)
             g_print(", style=dashed");

         g_print(" ]\n");
    }

    /* Generate edges */
    g_print("\n");

    for(i = 0; i < service_property_array->len; i++)
    {
         Service *current_service = g_ptr_array_index(service_property_array, i);
         unsigned int j;

         /* Inter-dependencies with ordering requirement */
         for(j = 0; j < current_service->depends_on->len; j++)
         {
             gchar *dependency = g_ptr_array_index(current_service->depends_on, j);
             g_print("\"%s\" -> \"%s\"\n", current_service->name, dependency);
         }

         /* Inter-dependencies without ordering requirement */
         for(j = 0; j < current_service->connects_to->len; j++)
         {
             gchar *dependency = g_ptr_array_index(current_service->connects_to, j);
             g_print("\"%s\" -> \"%s\" [style=dashed]\n", current_service->name, dependency);
         }
    }

    g_print("}\n");
}

static int is_subgroup_of(gchar *current_group, gchar *group)
{
    if(strcmp(group, "") == 0)
        return TRUE;
    else
    {
        gchar *prefix = g_strjoin(group, "/", NULL);
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

static void replace_service_dependency_by_group_dependency(Service *service, GPtrArray *dependencies, gchar *current_group)
{
    unsigned int i;
    int found = FALSE;

    /* Check if group dependency already exists */
    for(i = 0; i < dependencies->len; i++)
    {
        gchar *dependency = g_ptr_array_index(dependencies, i);
        if(g_strcmp0(dependency, current_group) == 0)
        {
            found = TRUE;
            break;
        }
    }

    if(!found)
    {
        /* If group dependency does not already exists, drop the service dependency and replace it by the group */
        for(i = 0; i < dependencies->len; i++)
        {
            gchar *dependency = g_ptr_array_index(dependencies, i);

            if(g_strcmp0(dependency, service->name) == 0)
            {
                g_ptr_array_index(dependencies, i) = g_strdup(current_group);
                g_free(dependency);
            }
        }
    }
}

static GPtrArray *copy_dependencies(GPtrArray *dependencies)
{
    unsigned int i;
    GPtrArray *copy_array = g_ptr_array_new();

    for(i = 0; i < dependencies->len; i++)
    {
        gchar *dependency = g_ptr_array_index(dependencies, i);
        g_ptr_array_add(copy_array, g_strdup(dependency));
    }

    return copy_array;
}

static GPtrArray *copy_properties(GPtrArray *properties)
{
    unsigned int i;
    GPtrArray *copy_array = g_ptr_array_new();

    for(i = 0; i < properties->len; i++)
    {
        ServiceProperty *prop = g_ptr_array_index(properties, i);
        ServiceProperty *copy_prop = (ServiceProperty*)g_malloc(sizeof(ServiceProperty));
        copy_prop->name = g_strdup(prop->name);
        copy_prop->value = g_strdup(prop->value);

        g_ptr_array_add(copy_array, copy_prop);
    }

    return copy_array;
}

static Service *copy_service(const Service *service)
{
    Service *new_service = (Service*)g_malloc(sizeof(Service));
    new_service->name = g_strdup(service->name);
    new_service->property = copy_properties(service->property);
    new_service->depends_on = copy_dependencies(service->depends_on);
    new_service->connects_to = copy_dependencies(service->connects_to);
    new_service->group_node = service->group_node;

    return new_service;
}

static void group_service(GHashTable *queried_services_table, GHashTable *grouped_services_table, Service *service, gchar *group)
{
    gchar *current_group = find_service_property_value(service, "group");

    if(current_group != NULL && is_subgroup_of(current_group, group))
    {
        /* If a service is part of a subgroup in this group, then create a single sub group node that abstracts over its properties */
        Service *group_service = g_hash_table_lookup(grouped_services_table, current_group);

        if(group_service == NULL)
        {
            group_service = (Service*)g_malloc(sizeof(Service));
            group_service->name = g_strdup(current_group);
            group_service->property = g_ptr_array_new();
            group_service->connects_to = g_ptr_array_new();
            group_service->depends_on = g_ptr_array_new();
            group_service->group_node = TRUE;
            g_hash_table_insert(grouped_services_table, group_service->name, group_service);
        }

        /* For all services, substitute a dependency on this service by the group */
        GHashTableIter iter;
        gpointer *key;
        gpointer *value;

        g_hash_table_iter_init(&iter, queried_services_table);

        while(g_hash_table_iter_next(&iter, (gpointer*)&key, (gpointer*)&value))
        {
            Service *current_service = (Service*)value;

            if(current_service != service)
            {
                replace_service_dependency_by_group_dependency(service, current_service->depends_on, current_group);
                replace_service_dependency_by_group_dependency(service, current_service->connects_to, current_group);
            }
        }

        g_hash_table_iter_init(&iter, grouped_services_table);

        while(g_hash_table_iter_next(&iter, (gpointer*)&key, (gpointer*)&value))
        {
            Service *current_service = (Service*)value;

            if(current_service != service)
            {
                replace_service_dependency_by_group_dependency(service, current_service->depends_on, current_group);
                replace_service_dependency_by_group_dependency(service, current_service->connects_to, current_group);
            }
        }

        /* Eliminate self references on group dependencies */
        remove_self_reference(group_service->connects_to, current_group);
        remove_self_reference(group_service->depends_on, current_group);

        /* Merge dependencies of the service with the group dependencies */
        merge_dependencies(service->depends_on, group_service->depends_on, current_group);
        merge_dependencies(service->connects_to, group_service->connects_to, current_group);
    }
    else
    {
        /* If a service does not fit within a sub group, just add it verbatim */

        Service *leaf_service = copy_service(service);
        g_hash_table_insert(grouped_services_table, leaf_service->name, leaf_service);
    }
}

static void add_outside_group_dependencies(GHashTable *queried_services_table, GPtrArray *dependencies, GPtrArray *service_property_array, GPtrArray *dep_service_property_array)
{
    unsigned int i;

    for(i = 0; i < dependencies->len; i++)
    {
        gchar *dependency = g_ptr_array_index(dependencies, i);

        if(g_hash_table_lookup(queried_services_table, dependency) == NULL)
        {
            Service *dependency_service = find_service(service_property_array, dependency); // TODO: NULL check

            Service *service = (Service*)g_malloc(sizeof(Service));
            service->name = g_strdup(dependency_service->name);
            service->property = copy_properties(dependency_service->property);
            /* Disregard its dependencies, because they are an implementation detail */
            service->depends_on = g_ptr_array_new();
            service->connects_to = g_ptr_array_new();
            service->group_node = FALSE;

            g_ptr_array_add(dep_service_property_array, service);
        }
    }
}

static GHashTable *query_services_in_group(GPtrArray *service_property_array, gchar *group)
{
    GHashTable *queried_services_table = g_hash_table_new(g_str_hash, g_str_equal);
    unsigned int i;

    /* Query all services that fit within the requested group */

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

    /* Add all the the dependencies (but not transitive dependencies) that are outside the requested group. */
    /* The dependencies of the outside group dependencies are discarded, because their internals should be disregarded */

    GPtrArray *dep_service_property_array = g_ptr_array_new();

    GHashTableIter iter;
    gpointer *key;
    gpointer *value;

    g_hash_table_iter_init(&iter, queried_services_table);

    while(g_hash_table_iter_next(&iter, (gpointer*)&key, (gpointer*)&value))
    {
        Service *current_service = (Service*)value;
        add_outside_group_dependencies(queried_services_table, current_service->connects_to, service_property_array, dep_service_property_array);
        add_outside_group_dependencies(queried_services_table, current_service->depends_on, service_property_array, dep_service_property_array);
    }

    for(i = 0; i < dep_service_property_array->len; i++)
    {
        Service *current_service = g_ptr_array_index(dep_service_property_array, i);
        g_hash_table_insert(queried_services_table, current_service->name, current_service);
    }

    g_ptr_array_free(dep_service_property_array, TRUE);

    return queried_services_table;
}

static GHashTable *group_services(GHashTable *queried_services_table, gchar *group)
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

static GPtrArray *create_service_property_array_from_table(GHashTable *table)
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

static void delete_services_table(GHashTable *services_table)
{
    GHashTableIter iter;
    gpointer *key;
    gpointer *value;

    g_hash_table_iter_init(&iter, services_table);

    while(g_hash_table_iter_next(&iter, (gpointer*)&key, (gpointer*)&value))
        delete_service((Service*)value);

    g_hash_table_destroy(services_table);
}

int visualize_services(gchar *services, int xml, int group_subservices, gchar *group)
{
    gchar *services_xml;
    GPtrArray *service_property_array;

    if(xml)
        services_xml = strdup(services);
    else
        services_xml = generate_service_xml_from_expr(services);

    service_property_array = create_service_property_array(services_xml);

    free(services_xml);

    if(service_property_array == NULL)
    {
        g_printerr("Cannot open services XML file!\n");
        return 1;
    }
    else
    {
        // HACK
        GHashTable *table = query_services_in_group(service_property_array, group);
        delete_service_property_array(service_property_array);

        if(group_subservices)
        {
            GHashTable *group_table = group_services(table, group);
            delete_services_table(table);
            table = group_table;
        }

        service_property_array = create_service_property_array_from_table(table);
        generate_architecture_diagram(service_property_array);
        delete_services_table(table);
        g_ptr_array_free(service_property_array, TRUE);

        return 0;
    }
}
