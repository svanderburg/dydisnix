#include "distributiontable.h"
#include <stdlib.h>
#include <procreact_future.h>
#include <nixxml-parse.h>
#include <nixxml-ghashtable.h>
#include <nixxml-print-nix.h>
#include <nixxml-print-xml.h>

static ProcReact_Future generate_distribution_xml_from_expr_async(char *distribution_expr, char *infrastructure_expr, char *extra_params)
{
    ProcReact_Future future = procreact_initialize_future(procreact_create_string_type());

    if(future.pid == 0)
    {
        char *const args[] = {"dydisnix-xml", "-i", infrastructure_expr, "-d", distribution_expr, "--extra-params", extra_params, "--no-out-link", NULL};
        dup2(future.fd, 1); /* Attach write-end to stdout */
        execvp(args[0], args);
        _exit(1);
    }

    return future;
}

char *generate_distribution_xml_from_expr(char *distribution_expr, char *infrastructure_expr, char *extra_params)
{
    ProcReact_Status status;
    ProcReact_Future future = generate_distribution_xml_from_expr_async(distribution_expr, infrastructure_expr, extra_params);
    char *path = procreact_future_get(&future, &status);

    if(status == PROCREACT_STATUS_OK && path != NULL)
    {
        path[strlen(path) - 1] = '\0';
        return path;
    }
    else
        return NULL;
}

GHashTable *create_distribution_table_from_xml(const char *distribution_file, NixXML_bool *automapped)
{
    /* Declarations */
    xmlDocPtr doc;
    xmlNodePtr node_root;
    GHashTable *distribution_table;

    /* Parse the XML document */

    if((doc = xmlParseFile(distribution_file)) == NULL)
    {
        fprintf(stderr, "Error with distribution XML file!\n");
        xmlCleanupParser();
        return NULL;
    }

    /* Retrieve root element */
    node_root = xmlDocGetRootElement(doc);

    if(node_root == NULL)
    {
        fprintf(stderr, "The distribution XML file is empty!\n");
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return NULL;
    }

    /* Parse mappings */
    *automapped = TRUE;
    distribution_table = NixXML_parse_g_hash_table_verbose(node_root, "service", "name", &automapped, parse_distribution_mapping_array_from_element);

    /* Cleanup */
    xmlFreeDoc(doc);
    xmlCleanupParser();

    /* Return the distribution table */
    return distribution_table;
}

GHashTable *create_distribution_table_from_nix(gchar *distribution_expr, gchar *infrastructure_expr, gchar *extra_params, NixXML_bool *automapped)
{
    char *distribution_xml = generate_distribution_xml_from_expr(distribution_expr, infrastructure_expr, extra_params);

    if(distribution_xml == NULL)
        return NULL;
    else
    {
        GHashTable *distribution_table = create_distribution_table_from_xml(distribution_xml, automapped);
        free(distribution_xml);
        return distribution_table;
    }
}

GHashTable *create_distribution_table(gchar *distribution_expr, gchar *infrastructure_expr, gchar *extra_params, NixXML_bool xml, NixXML_bool *automapped)
{
    if(xml)
        return create_distribution_table_from_xml(distribution_expr, automapped);
    else
        return create_distribution_table_from_nix(distribution_expr, infrastructure_expr, extra_params, automapped);
}

void delete_distribution_table(GHashTable *distribution_table)
{
    NixXML_delete_g_hash_table(distribution_table, (NixXML_DeleteGHashTableValueFunc)delete_distribution_mapping_array);
}

void print_distribution_table_nix(FILE *file, GHashTable *distribution_table, const int indent_level, NixXML_bool *automapped)
{
    NixXML_print_g_hash_table_nix(file, distribution_table, indent_level, automapped, print_distribution_mapping_array_nix);
}

void print_distribution_table_xml(FILE *file, GHashTable *distribution_table, const int indent_level, const char *type_property_name, void *userdata)
{
    NixXML_print_open_root_tag(file, "distribution");
    NixXML_print_g_hash_table_verbose_xml(file, distribution_table, "service", "name", indent_level, type_property_name, userdata, print_distribution_mapping_array_xml);
    NixXML_print_close_root_tag(file, "distribution");
}
