#include "candidatetargetmapping.h"
#include <nixxml-parse.h>
#include <nixxml-print-nix.h>
#include <nixxml-print-xml.h>

static void *create_candidate_target_mapping_from_element(xmlNodePtr element, void *userdata)
{
    return g_malloc0(sizeof(CandidateTargetMapping));
}

static void insert_candidate_target_mapping_attribute(void *table, const xmlChar *key, void *value, void *userdata)
{
    CandidateTargetMapping *mapping = (CandidateTargetMapping*)table;

    if(xmlStrcmp(key, (xmlChar*) "target") == 0)
        mapping->target = value;
    else if(xmlStrcmp(key, (xmlChar*) "container") == 0)
        mapping->container = value;
    else
        xmlFree(value);
}

void *parse_candidate_target_mapping(xmlNodePtr element, void *userdata)
{
    return NixXML_parse_simple_attrset(element, userdata, create_candidate_target_mapping_from_element, NixXML_parse_value, insert_candidate_target_mapping_attribute);
}

void delete_candidate_target_mapping(CandidateTargetMapping *mapping)
{
    if(mapping != NULL)
    {
        xmlFree(mapping->target);
        xmlFree(mapping->container);
        g_free(mapping);
    }
}

void print_candidate_target_mapping_nix(FILE *file, const CandidateTargetMapping *mapping, const int indent_level, void *userdata)
{
    // TODO: support the verbose notation as well
    NixXML_print_string_nix(file, mapping->target, indent_level, userdata);
}

void print_candidate_target_mapping_xml(FILE *file, const CandidateTargetMapping *mapping, const int indent_level, const char *type_property_name, void *userdata)
{
    // TODO: support the verbose notation as well
    NixXML_print_string_xml(file, mapping->target, indent_level, type_property_name, userdata);
}
