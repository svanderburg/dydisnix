#include "targetstable2.h"
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

GHashTable *create_targets_table_from_nix_file(gchar *infrastructure_nix)
{
    char *infrastructure_xml = generate_infrastructure_xml_from_expr(infrastructure_nix);
    GHashTable *targets_table = create_targets_table_from_xml(infrastructure_xml, NULL, NULL);
    free(infrastructure_xml);
    return targets_table;
}

GHashTable *create_targets_table2(gchar *infrastructure, const int xml)
{
    if(xml)
        return create_targets_table_from_xml(infrastructure, NULL, NULL);
    else
        return create_targets_table_from_nix_file(infrastructure);
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
