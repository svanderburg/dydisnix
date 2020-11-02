#include "servicestable.h"
#include <string.h>
#include <stdlib.h>
#include <procreact_future.h>
#include <nixxml-ghashtable.h>

static ProcReact_Future generate_service_xml_from_expr_async(char *service_expr, char *extra_params)
{
    ProcReact_Future future = procreact_initialize_future(procreact_create_string_type());

    if(future.pid == 0)
    {
        char *const args[] = {"dydisnix-xml", "-s", service_expr, "--extra-params", extra_params, "--no-out-link", NULL};
        dup2(future.fd, 1); /* Attach write-end to stdout */
        execvp(args[0], args);
        _exit(1);
    }

    return future;
}

char *generate_service_xml_from_expr(char *service_expr, char *extra_params)
{
    ProcReact_Status status;
    ProcReact_Future future = generate_service_xml_from_expr_async(service_expr, extra_params);
    char *path = procreact_future_get(&future, &status);

    if(status == PROCREACT_STATUS_OK && path != NULL)
    {
        path[strlen(path) - 1] = '\0';
        return path;
    }
    else
        return NULL;
}

GHashTable *create_service_table_from_xml(const gchar *services_xml_file)
{
    /* Declarations */
    xmlDocPtr doc;
    xmlNodePtr node_root;
    GHashTable *service_table;

    /* Parse the XML document */

    if((doc = xmlParseFile(services_xml_file)) == NULL)
    {
        g_printerr("Error with parsing the services XML file!\n");
        xmlCleanupParser();
        return NULL;
    }

    /* Check if the document has a root */
    node_root = xmlDocGetRootElement(doc);

    if(node_root == NULL)
    {
        g_printerr("The services XML file is empty!\n");
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return NULL;
    }

    /* Parse the services */
    service_table = NixXML_parse_g_hash_table_verbose(node_root, "service", "name", NULL, parse_service);

    /* Cleanup */
    xmlFreeDoc(doc);
    xmlCleanupParser();

    /* Return the service table */
    return service_table;
}

GHashTable *create_service_table_from_nix(gchar *services_nix, gchar *extra_params)
{
    char *services_xml = generate_service_xml_from_expr(services_nix, extra_params);

    if(services_xml == NULL)
        return NULL;
    else
    {
        GHashTable *service_table = create_service_table_from_xml(services_xml);
        free(services_xml);
        return service_table;
    }
}

GHashTable *create_service_table(gchar *services, gchar *extra_params, const NixXML_bool xml)
{
    if(xml)
        return create_service_table_from_xml(services);
    else
        return create_service_table_from_nix(services, extra_params);
}

void delete_service_table(GHashTable *service_table)
{
    NixXML_delete_g_hash_table(service_table, (NixXML_DeleteGHashTableValueFunc)delete_service);
}
