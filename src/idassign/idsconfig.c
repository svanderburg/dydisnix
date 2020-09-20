#include "idsconfig.h"
#include <procreact_future.h>
#include <nixxml-parse.h>
#include <nixxml-print-nix.h>
#include <nixxml-print-xml.h>
#include <service.h>
#include "idassignmentsperresourcetable.h"
#include "lastassignmentstable.h"
#include "lastassignmentspertargettable.h"
#include "idresourcetype.h"
#include "idtoservicetable.h"

static void *create_ids_config_from_element(xmlNodePtr element, void *userdata)
{
    return (IdsConfig*)g_malloc0(sizeof(IdsConfig));
}

static void parse_and_insert_ids_config_attributes(xmlNodePtr element, void *table, const xmlChar *key, void *userdata)
{
    IdsConfig *ids_config = (IdsConfig*)table;

    if(xmlStrcmp(key, (xmlChar*) "ids") == 0)
        ids_config->id_assignments_per_resource_table = parse_id_assignments_per_resource_table(element, userdata);
    else if(xmlStrcmp(key, (xmlChar*) "lastAssignments") == 0)
        ids_config->last_assignments_table = parse_last_assignments_table(element, userdata);
    else if(xmlStrcmp(key, (xmlChar*) "lastAssignmentsPerTarget") == 0)
        ids_config->last_assignments_per_target_table = parse_last_assignments_per_target_table(element, userdata);
}

static ProcReact_Future generate_ids_xml_from_expr_async(char *ids_expr)
{
    ProcReact_Future future = procreact_initialize_future(procreact_create_string_type());

    if(future.pid == 0)
    {
        char *const args[] = {"dydisnix-xml", "--ids", ids_expr, "--no-out-link", NULL};
        dup2(future.fd, 1); /* Attach write-end to stdout */
        execvp(args[0], args);
        _exit(1);
    }

    return future;
}

IdsConfig *create_empty_ids_config(void)
{
    IdsConfig *ids_config = (IdsConfig*)g_malloc(sizeof(IdsConfig));
    ids_config->id_assignments_per_resource_table = create_empty_id_assignments_per_resource_table();
    ids_config->last_assignments_table = create_empty_last_assignments_table();
    ids_config->last_assignments_per_target_table = create_empty_last_assignments_per_target_table();
    return ids_config;
}

char *generate_ids_xml_from_expr(char *ids_expr)
{
    ProcReact_Status status;
    ProcReact_Future future = generate_ids_xml_from_expr_async(ids_expr);
    char *path = procreact_future_get(&future, &status);

    if(status == PROCREACT_STATUS_OK && path != NULL)
    {
        path[strlen(path) - 1] = '\0';
        return path;
    }
    else
        return NULL;
}

IdsConfig *create_ids_config_from_xml(const gchar *ids_xml_file)
{
    /* Declarations */
    xmlDocPtr doc;
    xmlNodePtr node_root;
    IdsConfig *ids_config;

    /* Parse the XML document */

    if((doc = xmlParseFile(ids_xml_file)) == NULL)
    {
        g_printerr("Error with parsing the IDs XML file!\n");
        xmlCleanupParser();
        return NULL;
    }

    /* Check if the document has a root */
    node_root = xmlDocGetRootElement(doc);

    if(node_root == NULL)
    {
        g_printerr("The IDs XML file is empty!\n");
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return NULL;
    }

    /* Parse the ID assignments per resources table */
    ids_config = NixXML_parse_simple_heterogeneous_attrset(node_root, NULL, create_ids_config_from_element, parse_and_insert_ids_config_attributes);

    if(ids_config->id_assignments_per_resource_table == NULL)
        ids_config->id_assignments_per_resource_table = create_empty_id_assignments_per_resource_table();
    if(ids_config->last_assignments_table == NULL)
        ids_config->last_assignments_table = create_empty_last_assignments_table();
    if(ids_config->last_assignments_per_target_table == NULL)
        ids_config->last_assignments_per_target_table = create_empty_last_assignments_per_target_table();

    /* Cleanup */
    xmlFreeDoc(doc);
    xmlCleanupParser();

    /* Return the configuration*/
    return ids_config;
}

IdsConfig *create_ids_config_from_nix(gchar *ids_nix)
{
    char *ids_xml = generate_ids_xml_from_expr(ids_nix);

    if(ids_xml == NULL)
        return NULL;
    else
    {
        IdsConfig *ids_config = create_ids_config_from_xml(ids_xml);
        free(ids_xml);
        return ids_config;
    }
}

IdsConfig *create_ids_config(gchar *ids, const NixXML_bool xml)
{
    if(xml)
        return create_ids_config_from_xml(ids);
    else
        return create_ids_config_from_nix(ids);
}

void delete_ids_config(IdsConfig *ids_config)
{
    if(ids_config != NULL)
    {
        delete_id_assignments_per_resource_table(ids_config->id_assignments_per_resource_table);
        delete_last_assignments_table(ids_config->last_assignments_table);
        delete_last_assignments_per_target_table(ids_config->last_assignments_per_target_table);
        g_free(ids_config);
    }
}

/* Nix printing infrastructure */

static void print_ids_config_attributes_nix(FILE *file, const void *value, const int indent_level, void *userdata, NixXML_PrintValueFunc print_value)
{
    IdsConfig *ids_config = (IdsConfig*)value;

    NixXML_print_attribute_nix(file, "ids", ids_config->id_assignments_per_resource_table, indent_level, userdata, (NixXML_PrintValueFunc)print_id_assignments_per_resource_table_nix);
    if(g_hash_table_size(ids_config->last_assignments_table) > 0)
        NixXML_print_attribute_nix(file, "lastAssignments", ids_config->last_assignments_table, indent_level, userdata, (NixXML_PrintValueFunc)print_last_assignments_table_nix);
    if(g_hash_table_size(ids_config->last_assignments_per_target_table) > 0)
        NixXML_print_attribute_nix(file, "lastAssignmentsPerTarget", ids_config->last_assignments_per_target_table, indent_level, userdata, (NixXML_PrintValueFunc)print_last_assignments_per_target_table_nix);
}

void print_ids_config_nix(FILE *file, const IdsConfig *ids_config, const int indent_level, void *userdata)
{
    NixXML_print_attrset_nix(file, ids_config, indent_level, userdata, print_ids_config_attributes_nix, NULL);
}

/* XML printing infrastructure */

static void print_ids_config_attributes_xml(FILE *file, const void *value, const int indent_level, const char *type_property_name, void *userdata, NixXML_PrintXMLValueFunc print_value)
{
    IdsConfig *ids_config = (IdsConfig*)value;

    NixXML_print_simple_attribute_xml(file, "ids", ids_config->id_assignments_per_resource_table, indent_level, NULL, userdata, (NixXML_PrintXMLValueFunc)print_id_assignments_per_resource_table_xml);
    if(g_hash_table_size(ids_config->last_assignments_table) > 0)
        NixXML_print_simple_attribute_xml(file, "lastAssignments", ids_config->last_assignments_table, indent_level, NULL, userdata, (NixXML_PrintXMLValueFunc)print_last_assignments_table_xml);
    if(g_hash_table_size(ids_config->last_assignments_per_target_table) > 0)
        NixXML_print_simple_attribute_xml(file, "lastAssignmentsPerTarget", ids_config->last_assignments_per_target_table, indent_level, NULL, userdata, (NixXML_PrintXMLValueFunc)print_last_assignments_per_target_table_xml);
}

void print_ids_config_xml(FILE *file, const IdsConfig *ids_config, const int indent_level, const char *type_property_name, void *userdata)
{
    NixXML_print_open_root_tag(file, "ids");
    NixXML_print_simple_attrset_xml(file, ids_config, indent_level, NULL, userdata, print_ids_config_attributes_xml, NULL);
    NixXML_print_close_root_tag(file, "ids");
}

void remove_invalid_service_ids(IdsConfig *ids_config, GHashTable *services_table, GHashTable *id_resources_table, gchar *service_property)
{
    remove_invalid_service_id_assignments_per_resource(ids_config->id_assignments_per_resource_table, services_table, id_resources_table, service_property);
    remove_invalid_last_assignments(ids_config->last_assignments_table, id_resources_table);
    if(g_hash_table_size(ids_config->last_assignments_per_target_table) > 0)
    {
        delete_last_assignments_per_target_table(ids_config->last_assignments_per_target_table);
        ids_config->last_assignments_per_target_table = create_empty_last_assignments_per_target_table();
    }
}

void remove_invalid_distribution_ids(IdsConfig *ids_config, GHashTable *services_table, GHashTable *distribution_table, GHashTable *target_to_service_table, GHashTable *id_resources_table, gchar *service_property)
{
    remove_invalid_distribution_id_assignments_per_resource(ids_config->id_assignments_per_resource_table, services_table, distribution_table, id_resources_table, service_property);
    remove_invalid_last_assignments(ids_config->last_assignments_table, id_resources_table);
    remove_invalid_last_assignments_per_target(ids_config->last_assignments_per_target_table, id_resources_table, target_to_service_table);
}

static void assign_id(gchar *service_name, int id, gchar *resource_name, GHashTable *id_assignments_table, GHashTable *last_assignments_table, GHashTable *id_to_services_table)
{
    int *id_ptr = (int*)malloc(sizeof(int));
    *id_ptr = id;
    g_hash_table_insert(id_assignments_table, g_strdup(service_name), id_ptr);
    g_hash_table_insert(id_to_services_table, id_ptr, service_name);

    int *id_ptr_copy = (int*)malloc(sizeof(int));
    *id_ptr_copy = id;

    int *old_id_ptr = g_hash_table_lookup(last_assignments_table, resource_name);
    free(old_id_ptr);

    g_hash_table_insert(last_assignments_table, g_strdup(resource_name), id_ptr_copy);
}

NixXML_bool create_id_assignments_for_services(GHashTable *id_assignments_table, GHashTable *last_assignments_table, GPtrArray *service_names, gchar *resource_name, IdResourceType *type, GHashTable *services_table, gchar *service_property)
{
    NixXML_bool result = TRUE;

    int *last_id = g_hash_table_lookup(last_assignments_table, resource_name);
    GHashTable *id_to_services_table = derive_id_to_service_table(id_assignments_table, service_names);

    unsigned int i;

    for(i = 0; i < service_names->len; i++)
    {
        gchar *service_name = g_ptr_array_index(service_names, i);
        Service *service = g_hash_table_lookup(services_table, service_name);

        if(service != NULL && service_requires_unique_resource_id(service, resource_name, service_property) && !g_hash_table_contains(id_assignments_table, service_name))
        {
            int id;

            if(!derive_next_id(type, id_to_services_table, last_id, &id))
            {
                result = FALSE;
                break;
            }

            last_id = &id;

            assign_id(service_name, id, resource_name, id_assignments_table, last_assignments_table, id_to_services_table);
        }
    }

    g_hash_table_destroy(id_to_services_table);

    return result;
}
