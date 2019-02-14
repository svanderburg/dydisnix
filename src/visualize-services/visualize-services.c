#include "visualize-services.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <serviceproperties.h>
#include <servicegroup.h>

static void print_type(FILE *fd, const Service *service)
{
    ServiceProperty *prop = find_service_property(service, "type");

    if(prop != NULL)
        fprintf(fd, "\\n(%s)", prop->value);
}

static void generate_architecture_diagram(FILE *fd, const GPtrArray *service_property_array)
{
    unsigned int i;

    fprintf(fd, "digraph G {\n");
    fprintf(fd, "node [style=filled,fillcolor=white,color=black];\n");

    /* Generate vertexes */
    for(i = 0; i < service_property_array->len; i++)
    {
         Service *current_service = g_ptr_array_index(service_property_array, i);
         fprintf(fd, "\"%s\" [ label = \"%s", current_service->name, current_service->name);
         print_type(fd, current_service);
         fprintf(fd, "\"");

         if(current_service->group_node)
             fprintf(fd, ", style=dashed");

         fprintf(fd, " ]\n");
    }

    /* Generate edges */
    fprintf(fd, "\n");

    for(i = 0; i < service_property_array->len; i++)
    {
         Service *current_service = g_ptr_array_index(service_property_array, i);
         unsigned int j;

         /* Inter-dependencies with ordering requirement */
         for(j = 0; j < current_service->depends_on->len; j++)
         {
             gchar *dependency = g_ptr_array_index(current_service->depends_on, j);
             fprintf(fd, "\"%s\" -> \"%s\"\n", current_service->name, dependency);
         }

         /* Inter-dependencies without ordering requirement */
         for(j = 0; j < current_service->connects_to->len; j++)
         {
             gchar *dependency = g_ptr_array_index(current_service->connects_to, j);
             fprintf(fd, "\"%s\" -> \"%s\" [style=dashed]\n", current_service->name, dependency);
         }
    }

    fprintf(fd, "}\n");
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

static void add_interdependent_services(Service *service, GPtrArray *dependencies, GHashTable *interdependent_services_table, GHashTable *queried_services_table)
{
    unsigned int i;

    for(i = 0; i < dependencies->len; i++)
    {
        gchar *dependency = g_ptr_array_index(dependencies, i);

        if(g_hash_table_lookup(queried_services_table, dependency) != NULL && g_hash_table_lookup(interdependent_services_table, service->name) == NULL)
            g_hash_table_insert(interdependent_services_table, service->name, service);
    }
}

static GPtrArray *copy_non_dangling_dependencies(GPtrArray *dependencies, GHashTable *queried_services_table)
{
    GPtrArray *copy_array = g_ptr_array_new();
    unsigned int i;

    for(i = 0; i < dependencies->len; i++)
    {
        gchar *dependency = g_ptr_array_index(dependencies, i);

        if(g_hash_table_lookup(queried_services_table, dependency) != NULL)
            g_ptr_array_add(copy_array, g_strdup(dependency));
    }

    return copy_array;
}

static GPtrArray *query_direct_dependencies(GHashTable *queried_services_table, GPtrArray *service_property_array)
{
    GHashTableIter iter;
    gpointer *key;
    gpointer *value;

    GPtrArray *dep_service_property_array = g_ptr_array_new();

    g_hash_table_iter_init(&iter, queried_services_table);

    while(g_hash_table_iter_next(&iter, (gpointer*)&key, (gpointer*)&value))
    {
        Service *current_service = (Service*)value;
        add_outside_group_dependencies(queried_services_table, current_service->connects_to, service_property_array, dep_service_property_array);
        add_outside_group_dependencies(queried_services_table, current_service->depends_on, service_property_array, dep_service_property_array);
    }

    return dep_service_property_array;
}

static GHashTable *query_inderdependent_services(GHashTable *queried_services_table, GPtrArray *service_property_array)
{
    unsigned int i;
    GHashTable *interdependent_services_table = g_hash_table_new(g_str_hash, g_str_equal);

    for(i = 0; i < service_property_array->len; i++)
    {
        Service *current_service = g_ptr_array_index(service_property_array, i);
        add_interdependent_services(current_service, current_service->connects_to, interdependent_services_table, queried_services_table);
        add_interdependent_services(current_service, current_service->depends_on, interdependent_services_table, queried_services_table);
    }

    return interdependent_services_table;
}

static void append_dependencies_to_table(GHashTable *queried_services_table, GPtrArray *dep_service_property_array)
{
    unsigned int i;

    for(i = 0; i < dep_service_property_array->len; i++)
    {
        Service *current_service = g_ptr_array_index(dep_service_property_array, i);
        g_hash_table_insert(queried_services_table, current_service->name, current_service);
    }
}

static void append_interdependent_services_to_table(GHashTable *queried_services_table, GHashTable *interdependent_services_table)
{
    GHashTableIter iter;
    gpointer *key;
    gpointer *value;

    g_hash_table_iter_init(&iter, interdependent_services_table);

    while(g_hash_table_iter_next(&iter, (gpointer*)&key, (gpointer*)&value))
    {
        Service *current_service = (Service*)value;

        if(g_hash_table_lookup(queried_services_table, current_service->name) == NULL)
        {
            Service *service = (Service*)g_malloc(sizeof(Service));
            service->name = g_strdup(current_service->name);
            service->property = copy_properties(current_service->property);

            /* Copy dependencies but drop non-existent dependencies */
            service->depends_on = copy_non_dangling_dependencies(current_service->depends_on, queried_services_table);
            service->connects_to = copy_non_dangling_dependencies(current_service->connects_to, queried_services_table);

            service->group_node = FALSE;
            g_hash_table_insert(queried_services_table, service->name, service);
        }
    }
}

static GHashTable *query_services_in_group_with_context(GPtrArray *service_property_array, gchar *group)
{
    /* Query all services that fit within the requested group */
    GHashTable *queried_services_table = query_services_in_group(service_property_array, group);

    /* Query all the the dependencies (but not transitive dependencies) that are outside the requested group. */
    /* The dependencies of the outside group dependencies are discarded, because their internals should be disregarded */
    GPtrArray *dep_service_property_array = query_direct_dependencies(queried_services_table, service_property_array);

    /* Add all interdependent services that are outside the requested group and discard the dependencies on the services that are not in the group */
    GHashTable *interdependent_services_table = query_inderdependent_services(queried_services_table, service_property_array);

    /* Add dependencies to queried services table */
    append_dependencies_to_table(queried_services_table, dep_service_property_array);
    g_ptr_array_free(dep_service_property_array, TRUE);

    /* Add interdependent services to queried services table */
    append_interdependent_services_to_table(queried_services_table, interdependent_services_table);
    g_hash_table_destroy(interdependent_services_table);

    /* Return queries services table */
    return queried_services_table;
}

int visualize_services(gchar *services, int xml, int group_subservices, gchar *group)
{
    GPtrArray *service_property_array = create_service_property_array(services, xml);

    if(service_property_array == NULL)
    {
        g_printerr("Cannot open services XML file!\n");
        return 1;
    }
    else
    {
        // HACK
        GHashTable *table = query_services_in_group_with_context(service_property_array, group);
        delete_service_property_array(service_property_array);

        if(group_subservices)
        {
            GHashTable *group_table = group_services(table, group);
            delete_services_table(table);
            table = group_table;
        }

        service_property_array = create_service_property_array_from_table(table);
        generate_architecture_diagram(stdout, service_property_array);
        delete_services_table(table);
        g_ptr_array_free(service_property_array, TRUE);

        return 0;
    }
}

static GPtrArray *query_unique_groups(GPtrArray *service_property_array)
{
    unsigned int i;

    GHashTable *unique_groups_table = g_hash_table_new(g_str_hash, g_str_equal);

    for(i = 0; i < service_property_array->len; i++)
    {
        Service *current_service = g_ptr_array_index(service_property_array, i);
        gchar *current_group = find_service_property_value(current_service, "group");

        if(current_group != NULL && g_strcmp0(current_group, "") != 0)
        {
            if(g_hash_table_lookup(unique_groups_table, current_group) == NULL)
                g_hash_table_insert(unique_groups_table, current_group, NULL);
        }
    }

    GHashTableIter iter;
    gpointer *key;
    gpointer *value;
    GPtrArray *unique_groups_array = g_ptr_array_new();

    g_hash_table_iter_init(&iter, unique_groups_table);

    while(g_hash_table_iter_next(&iter, (gpointer*)&key, (gpointer*)&value))
        g_ptr_array_add(unique_groups_array, (gchar*)key);

    g_hash_table_destroy(unique_groups_table);

    return unique_groups_array;
}

static void mkdirp(const char *dir)
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

static void render_group(GPtrArray *service_property_array, gchar *group, gchar *output_dir)
{
    GHashTable *table = query_services_in_group_with_context(service_property_array, group);
    GHashTable *group_table = group_services(table, group);
    GPtrArray *grouped_service_property_array = create_service_property_array_from_table(group_table);

    gchar *dirname = g_strjoin("", output_dir, "/", group, NULL);
    gchar *filename = g_strjoin("", dirname, "/", "diagram.dot", NULL);

    mkdirp(dirname);
    FILE *file = fopen(filename, "w");
    generate_architecture_diagram(file, grouped_service_property_array);
    fclose(file);

    g_free(filename);
    g_free(dirname);
    delete_services_table(group_table);
    g_ptr_array_free(grouped_service_property_array, TRUE);
    delete_services_table(table);
}

int visualize_services_batch(gchar *services, int xml, int group_subservices, gchar *output_dir)
{
    GPtrArray *service_property_array = create_service_property_array(services, xml);

    if(service_property_array == NULL)
    {
        g_printerr("Cannot open services XML file!\n");
        return 1;
    }
    else
    {
        GPtrArray *unique_groups_array = query_unique_groups(service_property_array);
        unsigned int i;

        render_group(service_property_array, "", output_dir);

        for(i = 0; i < unique_groups_array->len; i++)
        {
            gchar *group = g_ptr_array_index(unique_groups_array, i);
            render_group(service_property_array, group, output_dir);
        }

        delete_service_property_array(service_property_array);

        return 0;
    }
}
