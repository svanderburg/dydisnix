#include "candidatetargetsarray.h"
#include <nixxml-gptrarray.h>

void *parse_candidate_targets_array_from_element(xmlNodePtr element, void *userdata)
{
    return NixXML_parse_g_ptr_array(element, "mapping", userdata, parse_candidate_target_mapping);
}

void delete_candidate_targets_array(GPtrArray *candidate_targets_array)
{
    NixXML_delete_g_ptr_array(candidate_targets_array, (NixXML_DeleteGPtrArrayElementFunc)delete_candidate_target_mapping);
}

void print_candidate_targets_array_nix(FILE *file, const void *value, const int indent_level, void *userdata)
{
    NixXML_print_g_ptr_array_nix(file, value, indent_level, userdata, (NixXML_PrintValueFunc)print_candidate_target_mapping_nix);
}

void print_candidate_targets_array_xml(FILE *file, const void *value, const int indent_level, const char *type_property_name, void *userdata)
{
    NixXML_print_g_ptr_array_xml(file, value, "target", indent_level, type_property_name, userdata, (NixXML_PrintXMLValueFunc)print_candidate_target_mapping_xml);
}
