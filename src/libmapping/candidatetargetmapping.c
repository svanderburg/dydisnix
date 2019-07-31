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
    {
        int *automapped = (int*)userdata; /* If encouter a container, then we can no longer print the mappings with automapping notation */
        *automapped = FALSE;

        mapping->container = value;
    }
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

void print_candidate_target_mapping_attributes_nix(FILE *file, const void *value, const int indent_level, void *userdata, NixXML_PrintValueFunc print_value)
{
    const CandidateTargetMapping *mapping = (const CandidateTargetMapping*)value;

    NixXML_print_attribute_nix(file, "target", mapping->target, indent_level, userdata, NixXML_print_string_nix);
    if(mapping->container != NULL)
        NixXML_print_attribute_nix(file, "container", mapping->container, indent_level, userdata, NixXML_print_string_nix);
}

void print_candidate_target_mapping_nix(FILE *file, const CandidateTargetMapping *mapping, const int indent_level, void *userdata)
{
    int *automapped = (int*)userdata;

    if(*automapped)
        NixXML_print_string_nix(file, mapping->target, indent_level, userdata);
    else
        NixXML_print_attrset_nix(file, mapping, indent_level, userdata, print_candidate_target_mapping_attributes_nix, NULL);
}

static void print_candidate_target_mapping_attributes_xml(FILE *file, const void *value, const int indent_level, const char *type_property_name, void *userdata, NixXML_PrintXMLValueFunc print_value)
{
    const CandidateTargetMapping *mapping = (const CandidateTargetMapping*)value;

    NixXML_print_simple_attribute_xml(file, "target", mapping->target, indent_level, type_property_name, userdata, NixXML_print_string_xml);
    if(mapping->container != NULL)
        NixXML_print_simple_attribute_xml(file, "container", mapping->container, indent_level, type_property_name, userdata, NixXML_print_string_xml);
}

void print_candidate_target_mapping_xml(FILE *file, const CandidateTargetMapping *mapping, const int indent_level, const char *type_property_name, void *userdata)
{
    NixXML_print_simple_attrset_xml(file, mapping, indent_level, type_property_name, userdata, print_candidate_target_mapping_attributes_xml, NULL);
}
