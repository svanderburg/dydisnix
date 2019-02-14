#include "document-services.h"
#include <serviceproperties.h>
#include <servicegroup.h>

static void display_property(Service *service, gchar *name)
{
    ServiceProperty *prop = find_service_property(service, name);

    g_print("                <td>");
    if(prop != NULL)
        g_print("%s", prop->value);
    g_print("</td>\n");
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

        GHashTableIter iter;
        gpointer *key;
        gpointer *value;

        g_print("<!DOCTYPE html>\n\n");
        g_print("<html>\n");
        g_print("    <head>\n");
        g_print("        <title>Documentation</title>\n");
        g_print("    </head>\n");

        g_print("    <body>\n");
        g_print("        <h2>Services</h2>\n");
        g_print("        <table>\n");
        g_print("            <tr>\n");
        g_print("                <th>Name</th>\n");
        g_print("                <th>Description</th>\n");
        g_print("                <th>Type</th>\n");
        g_print("            </tr>\n");

        g_hash_table_iter_init(&iter, table);

        while(g_hash_table_iter_next(&iter, (gpointer*)&key, (gpointer*)&value))
        {
            Service *current_service = (Service*)value;

            if(!current_service->group_node)
            {
                g_print("            <tr>\n");
                g_print("                <td>%s</td>\n", current_service->name);
                display_property(current_service, "description");
                display_property(current_service, "type");
                g_print("            </tr>\n");
            }
        }

        g_print("        </table>\n");

        g_print("        <h2>Groups</h2>\n");
        g_print("        <table>\n");
        g_print("            <tr>\n");
        g_print("                <th>Name</th>\n");
        g_print("                <th>Description</th>\n");
        g_print("            </tr>\n");

        g_hash_table_iter_init(&iter, table);

        while(g_hash_table_iter_next(&iter, (gpointer*)&key, (gpointer*)&value))
        {
            Service *current_service = (Service*)value;

            if(current_service->group_node)
            {
                g_print("            <tr>\n");
                g_print("                <td>%s</td>\n", current_service->name);
                display_property(current_service, "description");
                g_print("            </tr>\n");
            }
        }

        g_print("        </table>\n");
        g_print("    </body>\n");
        g_print("</html>\n");

        delete_services_table(table);
        g_ptr_array_free(service_property_array, TRUE);

        return 0;
    }
}
