#include "document-services.h"
#include <stdio.h>
#include <serviceproperties.h>
#include <servicegroup.h>

static void display_property(FILE *fd, Service *service, gchar *name)
{
    ServiceProperty *prop = find_service_property(service, name);

    fprintf(fd, "                <td>");
    if(prop != NULL)
        fprintf(fd, "%s", prop->value);
    fprintf(fd, "</td>\n");
}

static void generate_architecture_description(FILE *fd, const GPtrArray *service_property_array)
{
    unsigned int i;

    fprintf(fd, "<!DOCTYPE html>\n\n");
    fprintf(fd, "<html>\n");
    fprintf(fd, "    <head>\n");
    fprintf(fd, "        <title>Documentation</title>\n");
    fprintf(fd, "    </head>\n");

    fprintf(fd, "    <body>\n");
    fprintf(fd, "        <h2>Services</h2>\n");
    fprintf(fd, "        <table>\n");
    fprintf(fd, "            <tr>\n");
    fprintf(fd, "                <th>Name</th>\n");
    fprintf(fd, "                <th>Description</th>\n");
    fprintf(fd, "                <th>Type</th>\n");
    fprintf(fd, "            </tr>\n");

    for(i = 0; i < service_property_array->len; i++)
    {
        Service *current_service = g_ptr_array_index(service_property_array, i);

        if(!current_service->group_node)
        {
            fprintf(fd, "            <tr>\n");
            fprintf(fd, "                <td>%s</td>\n", current_service->name);
            display_property(fd, current_service, "description");
            display_property(fd, current_service, "type");
            fprintf(fd, "            </tr>\n");
        }
    }

    fprintf(fd, "        </table>\n");

    fprintf(fd, "        <h2>Groups</h2>\n");
    fprintf(fd, "        <table>\n");
    fprintf(fd, "            <tr>\n");
    fprintf(fd, "                <th>Name</th>\n");
    fprintf(fd, "                <th>Description</th>\n");
    fprintf(fd, "            </tr>\n");

    for(i = 0; i < service_property_array->len; i++)
    {
        Service *current_service = g_ptr_array_index(service_property_array, i);

        if(current_service->group_node)
        {
            fprintf(fd, "            <tr>\n");
            fprintf(fd, "                <td>%s</td>\n", current_service->name);
            display_property(fd, current_service, "description");
            fprintf(fd, "            </tr>\n");
        }
    }

    fprintf(fd, "        </table>\n");
    fprintf(fd, "    </body>\n");
    fprintf(fd, "</html>\n");
}

int document_services(gchar *services, gchar *group, int xml, int group_subservices)
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
        GHashTable *table = query_services_in_group(service_property_array, group);
        delete_service_property_array(service_property_array);

        if(group_subservices)
        {
            GHashTable *group_table = group_services(table, group);
            delete_services_table(table);
            table = group_table;
        }

        service_property_array = create_service_property_array_from_table(table);

        generate_architecture_description(stdout, service_property_array);

        delete_services_table(table);
        g_ptr_array_free(service_property_array, TRUE);

        return 0;
    }
}

static void generate_architecture_descriptions_for_group(GPtrArray *service_property_array, gchar *group, gchar *output_dir)
{
    GHashTable *table = query_services_in_group(service_property_array, group);
    generate_group_artifacts(table, group, output_dir, "index.html", generate_architecture_description);
    delete_services_table(table);
}

int document_services_batch(gchar *services, int xml, int group_subservices, gchar *output_dir)
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

        generate_architecture_descriptions_for_group(service_property_array, "", output_dir);

        for(i = 0; i < unique_groups_array->len; i++)
        {
            gchar *group = g_ptr_array_index(unique_groups_array, i);
            generate_architecture_descriptions_for_group(service_property_array, group, output_dir);
        }

        delete_service_property_array(service_property_array);

        return 0;
    }
}
