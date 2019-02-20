#include "document-services.h"
#include <stdio.h>
#include <serviceproperties.h>
#include <servicegroup.h>
#include "docs-config.h"

static void display_property(FILE *fd, Service *service, gchar *name)
{
    ServiceProperty *prop = find_service_property(service, name);

    fprintf(fd, "                <td>");
    if(prop != NULL)
        fprintf(fd, "%s", prop->value);
    fprintf(fd, "</td>\n");
}

static gchar *compose_relative_root_path(gchar *group)
{
    if(group == NULL || g_strcmp0(group, "") == 0)
        return g_strdup(".");
    else
    {
        gchar **group_components = g_strsplit(group, "/", -1);
        gchar *result = g_strdup("");

        unsigned int i = 0;

        while(group_components[i] != NULL)
        {
            gchar *old_result = result;

            if(g_strcmp0(old_result, "") == 0)
                result = g_strdup("..");
            else
                result = g_strjoin("", old_result, "/", "..", NULL);

            g_free(old_result);
            i++;
        }

        g_strfreev(group_components);

        return result;
    }
}

static void print_title(FILE *fd, gchar *group, gchar *group_description)
{
    fprintf(fd, "Functional architecture documentation");

    if(g_strcmp0(group, "") != 0)
    {
        fprintf(fd, " for group: ");

        if(group_description == NULL)
            fprintf(fd, "%s", group);
        else
            fprintf(fd, "%s", group_description);
    }
}

static int generate_architecture_description(gchar *filepath, gchar *image_format, gchar *group, void *data, const GPtrArray *service_property_array)
{
    unsigned int i;
    FILE *fd;
    int first = TRUE;
    gchar *root_path = compose_relative_root_path(group);
    DocsConfig *docs_config = (DocsConfig*)data;

    gchar *group_description;

    if(docs_config == NULL || g_strcmp0(group, "") == 0)
        group_description = NULL;
    else
        group_description = find_group(docs_config, group);

    if(filepath == NULL)
        fd = stdout;
    else
    {
        fd = fopen(filepath, "w");
        if(fd == NULL)
        {
            g_printerr("Can't open file: %s\n", filepath);
            return FALSE;
        }
    }

    fprintf(fd, "<!DOCTYPE html>\n\n");
    fprintf(fd, "<html>\n");
    fprintf(fd, "    <head>\n");
    fprintf(fd, "        <title>");
    print_title(fd, group, group_description);
    fprintf(fd, "</title>\n");
    fprintf(fd, "        <link rel=\"stylesheet\" type=\"text/css\" href=\"%s/style.css\">\n", root_path);
    fprintf(fd, "    </head>\n");

    fprintf(fd, "    <body>\n");

    if(g_strcmp0(group, "") != 0)
    {
        fprintf(fd, "        <h1>");

        if(group_description == NULL)
            fprintf(fd, "%s", group);
        else
            fprintf(fd, "%s", group_description);

        fprintf(fd, "</h1>\n");
    }

    if(image_format != NULL)
    {
        fprintf(fd, "        <p>\n");
        fprintf(fd, "            <img src=\"diagram.dot.%s\" alt=\"Architecture diagram\">\n", image_format);
        fprintf(fd, "        </p>\n");
    }

    for(i = 0; i < service_property_array->len; i++)
    {
        Service *current_service = g_ptr_array_index(service_property_array, i);

        if(!current_service->group_node)
        {
            if(first)
            {
                fprintf(fd, "        <h2>Services</h2>\n");
                fprintf(fd, "        <table>\n");
                fprintf(fd, "            <tr>\n");
                fprintf(fd, "                <th>Name</th>\n");
                fprintf(fd, "                <th>Description</th>\n");
                fprintf(fd, "                <th>Type</th>\n");
                fprintf(fd, "            </tr>\n");
                first = FALSE;
            }

            fprintf(fd, "            <tr>\n");
            fprintf(fd, "                <td>%s</td>\n", current_service->name);
            display_property(fd, current_service, "description");
            display_property(fd, current_service, "type");
            fprintf(fd, "            </tr>\n");
        }
    }

    if(!first)
        fprintf(fd, "        </table>\n");

    first = TRUE;

    for(i = 0; i < service_property_array->len; i++)
    {
        Service *current_service = g_ptr_array_index(service_property_array, i);

        if(current_service->group_node)
        {
            if(first)
            {
                fprintf(fd, "        <h2>Groups</h2>\n");
                fprintf(fd, "        <table>\n");
                fprintf(fd, "            <tr>\n");
                fprintf(fd, "                <th>Name</th>\n");
                fprintf(fd, "                <th>Description</th>\n");
                fprintf(fd, "            </tr>\n");
                first = FALSE;
            }

            fprintf(fd, "            <tr>\n");
            fprintf(fd, "                <td><a href=\"%s/index.html\">%s</a></td>\n", current_service->name, current_service->name);

            if(docs_config == NULL)
                fprintf(fd, "                <td></td>\n");
            else
            {
                gchar *full_group;

                if(g_strcmp0(group, "") == 0)
                    full_group = g_strdup(current_service->name);
                else
                    full_group = g_strjoin("/", group, current_service->name, NULL);

                gchar *group_description = find_group(docs_config, full_group);
                fprintf(fd, "                <td>%s</td>\n", group_description);
                g_free(full_group);
            }

            fprintf(fd, "            </tr>\n");
        }
    }

    if(!first)
        fprintf(fd, "        </table>\n");

    fprintf(fd, "    </body>\n");
    fprintf(fd, "</html>\n");

    if(filepath != NULL)
    {
        if(fclose(fd) != 0)
        {
            g_printerr("Can't invoke dot to generate image for: %s\n", filepath);
            return FALSE;
        }
    }

    g_free(root_path);
    return TRUE;
}

int document_services(gchar *services, gchar *group, int xml, int group_subservices, gchar *docs)
{
    GPtrArray *service_property_array = create_service_property_array(services, xml);

    if(service_property_array == NULL)
    {
        g_printerr("Cannot open services XML file!\n");
        return 1;
    }
    else
    {
        int status;
        DocsConfig *docs_config;

        if(docs == NULL)
            docs_config = NULL;
        else
            docs_config = create_docs_config(docs, xml);

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

        status = generate_architecture_description(NULL, NULL, "", docs_config, service_property_array);

        delete_services_table(table);
        delete_docs_config(docs_config);
        g_ptr_array_free(service_property_array, TRUE);

        return !status;
    }
}

static int generate_architecture_descriptions_for_group(GPtrArray *service_property_array, gchar *group, gchar *output_dir, gchar *image_format, DocsConfig *docs_config)
{
    int status;
    GHashTable *table = query_services_in_group(service_property_array, group);
    status = generate_group_artifacts(table, group, output_dir, "index.html", image_format, docs_config, generate_architecture_description);
    delete_services_table(table);
    return status;
}

static int copy_stylesheet(gchar *output_dir)
{
    FILE *src, *dest;
    gchar *output_file;

    src = fopen(DATADIR "/style.css", "r");
    if(src == NULL)
    {
        g_printerr("Cannot open stylesheet!\n");
        return FALSE;
    }

    output_file = g_strjoin("/", output_dir, "style.css", NULL);

    dest = fopen(output_file, "w");
    if(dest == NULL)
    {
        g_printerr("Cannot write stylesheet to destination!\n");
        g_free(output_file);
        return FALSE;
    }

    int c;
    while((c = fgetc(src)) != EOF)
        fputc(c, dest);

    fclose(dest);
    fclose(src);
    g_free(output_file);

    return TRUE;
}

int document_services_batch(gchar *services, int xml, int group_subservices, gchar *output_dir, gchar *image_format, gchar *docs)
{
    GPtrArray *service_property_array = create_service_property_array(services, xml);

    if(service_property_array == NULL)
    {
        g_printerr("Cannot open services XML file!\n");
        return 1;
    }
    else
    {
        int status;
        DocsConfig *docs_config;

        if(docs == NULL)
            docs_config = NULL;
        else
            docs_config = create_docs_config(docs, xml);

        if((status = generate_architecture_descriptions_for_group(service_property_array, "", output_dir ,image_format, docs_config)))
        {
            GPtrArray *unique_groups_array = query_unique_groups(service_property_array);
            unsigned int i;

            for(i = 0; i < unique_groups_array->len; i++)
            {
                gchar *group = g_ptr_array_index(unique_groups_array, i);
                status = generate_architecture_descriptions_for_group(service_property_array, group, output_dir, image_format, docs_config);
                if(!status)
                    break;
            }

            delete_docs_config(docs_config);
            delete_service_property_array(service_property_array);
        }

        if(status)
            status = copy_stylesheet(output_dir);

        return !status;
    }
}