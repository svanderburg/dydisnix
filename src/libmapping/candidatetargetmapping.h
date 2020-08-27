#ifndef __DYDISNIX_CANDIDATETARGETMAPPING_H
#define __DYDISNIX_CANDIDATETARGETMAPPING_H
#include <libxml/parser.h>
#include <glib.h>

typedef struct
{
    xmlChar *target;
    xmlChar *container;
}
CandidateTargetMapping;

CandidateTargetMapping *create_candidate_target_mapping(xmlChar *target, xmlChar *container);

CandidateTargetMapping *create_candidate_target_auto_mapping(xmlChar *target);

void *parse_candidate_target_mapping(xmlNodePtr element, void *userdata);

void delete_candidate_target_mapping(CandidateTargetMapping *mapping);

void print_candidate_target_mapping_nix(FILE *file, const CandidateTargetMapping *mapping, const int indent_level, void *userdata);

void print_candidate_target_mapping_xml(FILE *file, const CandidateTargetMapping *mapping, const int indent_level, const char *type_property_name, void *userdata);

#endif
