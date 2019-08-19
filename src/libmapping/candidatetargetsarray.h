#ifndef __DYDISNIX_CANDIDATETARGETSARRAY_H
#define __DYDISNIX_CANDIDATETARGETSARRAY_H
#include <stdio.h>
#include <libxml/parser.h>
#include <glib.h>
#include "candidatetargetmapping.h"

void *parse_candidate_targets_array_from_element(xmlNodePtr element, void *userdata);

void delete_candidate_targets_array(GPtrArray *candidate_targets_array);

void print_candidate_targets_array_nix(FILE *file, const void *value, const int indent_level, void *userdata);

void print_candidate_targets_array_xml(FILE *file, const void *value, const int indent_level, const char *type_property_name, void *userdata);

#endif
