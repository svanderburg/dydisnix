#ifndef __DYDISNIX_DISTRIBUTIONMAPPINGARRAY_H
#define __DYDISNIX_DISTRIBUTIONMAPPINGARRAY_H
#include <stdio.h>
#include <libxml/parser.h>
#include <glib.h>
#include "distributionmapping.h"

void *parse_distribution_mapping_array_from_element(xmlNodePtr element, void *userdata);

void delete_distribution_mapping_array(GPtrArray *candidate_targets_array);

void print_distribution_mapping_array_nix(FILE *file, const void *value, const int indent_level, void *userdata);

void print_distribution_mapping_array_xml(FILE *file, const void *value, const int indent_level, const char *type_property_name, void *userdata);

#endif
