#include "candidatetargetmappingtable.h"
#include <stdlib.h>
#include <procreact_future.h>
#include <nixxml-parse.h>
#include <nixxml-gptrarray.h>
#include <nixxml-ghashtable.h>
#include <nixxml-print-nix.h>
#include <nixxml-print-xml.h>

static ProcReact_Future generate_distribution_xml_from_expr_async(char *distribution_expr, char *infrastructure_expr)
{
    ProcReact_Future future = procreact_initialize_future(procreact_create_string_type());

    if(future.pid == 0)
    {
        char *const args[] = {"dydisnix-xml", "-i", infrastructure_expr, "-d", distribution_expr, "--no-out-link", NULL};
        dup2(future.fd, 1); /* Attach write-end to stdout */
        execvp(args[0], args);
        _exit(1);
    }

    return future;
}

char *generate_distribution_xml_from_expr(char *distribution_expr, char *infrastructure_expr)
{
    ProcReact_Status status;
    ProcReact_Future future = generate_distribution_xml_from_expr_async(distribution_expr, infrastructure_expr);
    char *path = procreact_future_get(&future, &status);
    path[strlen(path) - 1] = '\0';
    return path;
}

static void *parse_targets(xmlNodePtr element, void *userdata)
{
    return NixXML_parse_g_ptr_array(element, "mapping", userdata, parse_candidate_target_mapping);
}

GHashTable *create_candidate_target_table_from_xml(const char *candidate_mapping_file)
{
    /* Declarations */
    xmlDocPtr doc;
    xmlNodePtr node_root;
    GHashTable *candidate_target_table;

    /* Parse the XML document */

    if((doc = xmlParseFile(candidate_mapping_file)) == NULL)
    {
        fprintf(stderr, "Error with candidate mapping XML file!\n");
        xmlCleanupParser();
        return NULL;
    }

    /* Retrieve root element */
    node_root = xmlDocGetRootElement(doc);

    if(node_root == NULL)
    {
        fprintf(stderr, "The candidate mapping XML file is empty!\n");
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return NULL;
    }

    /* Parse mappings */
    candidate_target_table = NixXML_parse_g_hash_table_verbose(node_root, "service", "name", NULL, parse_targets);

    /* Cleanup */
    xmlFreeDoc(doc);
    xmlCleanupParser();

    /* Return the candidate host array */
    return candidate_target_table;
}

GHashTable *create_candidate_target_table_from_nix(gchar *distribution_expr, gchar *infrastructure_expr)
{
    char *distribution_xml = generate_distribution_xml_from_expr(distribution_expr, infrastructure_expr);
    GHashTable *candidate_target_table = create_candidate_target_table_from_xml(distribution_xml);
    free(distribution_xml);
    return candidate_target_table;
}

GHashTable *create_candidate_target_table(gchar *distribution_expr, gchar *infrastructure_expr, int xml)
{
    if(xml)
        return create_candidate_target_table_from_xml(distribution_expr);
    else
        return create_candidate_target_table_from_nix(distribution_expr, infrastructure_expr);
}

void delete_candidate_target_table(GHashTable *candidate_target_table)
{
    if(candidate_target_table != NULL)
    {
        GHashTableIter iter;
        gpointer key, value;

        g_hash_table_iter_init(&iter, candidate_target_table);
        while(g_hash_table_iter_next(&iter, &key, &value))
        {
            GPtrArray *targets = (GPtrArray*)value;

            if(targets != NULL)
            {
                unsigned int i;

                for(i = 0; i < targets->len; i++)
                {
                    CandidateTargetMapping *mapping = g_ptr_array_index(targets, i);
                    delete_candidate_target_mapping(mapping);
                }

                g_ptr_array_free(targets, TRUE);
            }
        }

        g_hash_table_destroy(candidate_target_table);
    }
}

static void print_targets_nix(FILE *file, const void *value, const int indent_level, void *userdata)
{
    NixXML_print_g_ptr_array_nix(file, value, indent_level, userdata, (NixXML_PrintValueFunc)print_candidate_target_mapping_nix);
}

void print_candidate_target_table_nix(GHashTable *candidate_target_table)
{
    NixXML_print_g_hash_table_nix(stdout, candidate_target_table, 0, NULL, print_targets_nix);
}

static void print_targets_xml(FILE *file, const void *value, const int indent_level, const char *type_property_name, void *userdata)
{
    NixXML_print_g_ptr_array_xml(file, value, "target", indent_level, type_property_name, userdata, (NixXML_PrintXMLValueFunc)print_candidate_target_mapping_xml);
}

void print_candidate_target_table_xml(GHashTable *candidate_target_table)
{
    NixXML_print_open_root_tag(stdout, "distribution");
    NixXML_print_g_hash_table_verbose_xml(stdout, candidate_target_table, "service", "name", 0, NULL, NULL, print_targets_xml);
    NixXML_print_close_root_tag(stdout, "distribution");
}
