#include "infrastructureproperties.h"
#include <stdlib.h>
#include <string.h>
#include <glib/gprintf.h>
#include <procreact_future.h>
#include <nixxml-node.h>

#define BUFFER_SIZE 1024

static ProcReact_Future generate_infrastructure_xml_from_expr_async(char *infrastructure_expr)
{
    ProcReact_Future future = procreact_initialize_future(procreact_create_string_type());

    if(future.pid == 0)
    {
        char *const args[] = {"dydisnix-xml", "-i", infrastructure_expr, "--no-out-link", NULL};
        dup2(future.fd, 1); /* Attach write-end to stdout */
        execvp(args[0], args);
        _exit(1);
    }

    return future;
}

char *generate_infrastructure_xml_from_expr(char *infrastructure_expr)
{
    ProcReact_Status status;
    ProcReact_Future future = generate_infrastructure_xml_from_expr_async(infrastructure_expr);
    char *path = procreact_future_get(&future, &status);
    path[strlen(path) - 1] = '\0';
    return path;
}

GHashTable *create_target_property_table_from_xml(const gchar *infrastructure_xml_file)
{
    GHashTable *targets_table;
    xmlDocPtr doc;

    /* Parse the XML document */

    if((doc = xmlParseFile(infrastructure_xml_file)) == NULL)
    {
        g_printerr("Error with parsing the infrastructure XML file!\n");
        xmlCleanupParser();
        return NULL;
    }

    /* Create a target array from the XML document */
    targets_table = create_targets_table_from_doc(doc);

    /* Cleanup */
    xmlFreeDoc(doc);
    xmlCleanupParser();

    /* Return the target table */
    return targets_table;
}

GHashTable *create_target_property_table_from_nix(gchar *infrastructure_nix)
{
    char *infrastructure_xml = generate_infrastructure_xml_from_expr(infrastructure_nix);
    GHashTable *targets_table = create_targets_table_from_xml(infrastructure_xml);
    free(infrastructure_xml);
    return targets_table;
}

GHashTable *create_target_property_table(gchar *infrastructure, const int xml)
{
    if(xml)
        return create_target_property_table_from_xml(infrastructure);
    else
        return create_target_property_table_from_nix(infrastructure);
}

void substract_target_value(Target *target, gchar *property_name, int amount)
{
    gchar buffer[BUFFER_SIZE];
    NixXML_Node *node = g_hash_table_lookup(target->properties_table, property_name);

    int result = atoi((char*)node->value) - amount;
    g_sprintf(buffer, "%d", result);
    xmlFree(node->value);
    node->value = xmlStrdup((xmlChar*)buffer);
}
