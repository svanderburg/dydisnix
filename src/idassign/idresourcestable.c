#include "idresourcestable.h"
#include <unistd.h>
#include <procreact_future.h>
#include <nixxml-ghashtable.h>

static ProcReact_Future generate_id_resources_xml_from_expr_async(char *id_resources_expr)
{
    ProcReact_Future future = procreact_initialize_future(procreact_create_string_type());

    if(future.pid == 0)
    {
        char *const args[] = {"dydisnix-xml", "--id-resources", id_resources_expr, "--no-out-link", NULL};
        dup2(future.fd, 1); /* Attach write-end to stdout */
        execvp(args[0], args);
        _exit(1);
    }

    return future;
}

char *generate_id_resources_xml_from_expr(char *id_resources_expr)
{
    ProcReact_Status status;
    ProcReact_Future future = generate_id_resources_xml_from_expr_async(id_resources_expr);
    char *path = procreact_future_get(&future, &status);

    if(status == PROCREACT_STATUS_OK && path != NULL)
    {
        path[strlen(path) - 1] = '\0';
        return path;
    }
    else
        return NULL;
}

GHashTable *create_id_resources_table_from_xml(const gchar *id_resources_xml_file)
{
    /* Declarations */
    xmlDocPtr doc;
    xmlNodePtr node_root;
    GHashTable *id_resources_table;

    /* Parse the XML document */

    if((doc = xmlParseFile(id_resources_xml_file)) == NULL)
    {
        g_printerr("Error with parsing the ID resources XML file!\n");
        xmlCleanupParser();
        return NULL;
    }

    /* Check if the document has a root */
    node_root = xmlDocGetRootElement(doc);

    if(node_root == NULL)
    {
        g_printerr("The ID resources XML file is empty!\n");
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return NULL;
    }

    /* Parse the resources */
    id_resources_table = NixXML_parse_g_hash_table_verbose(node_root, "resource", "name", NULL, parse_id_resource_type);

    /* Cleanup */
    xmlFreeDoc(doc);
    xmlCleanupParser();

    /* Return the ID resources table */
    return id_resources_table;
}

GHashTable *create_id_resources_table_from_nix(gchar *id_resources_nix)
{
    char *id_resources_xml = generate_id_resources_xml_from_expr(id_resources_nix);

    if(id_resources_xml == NULL)
        return NULL;
    else
    {
        GHashTable *id_resources_table = create_id_resources_table_from_xml(id_resources_xml);
        free(id_resources_xml);
        return id_resources_table;
    }
}

GHashTable *create_id_resources_table(gchar *id_resources, const NixXML_bool xml)
{
    if(xml)
        return create_id_resources_table_from_xml(id_resources);
    else
        return create_id_resources_table_from_nix(id_resources);
}

void delete_id_resources_table(GHashTable *id_resources_table)
{
    NixXML_delete_g_hash_table(id_resources_table, (NixXML_DeleteGHashTableValueFunc)delete_id_resource_type);
}
