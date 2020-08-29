#ifndef __DYDISNIX_DISTRIBUTIONMAPPING_H
#define __DYDISNIX_DISTRIBUTIONMAPPING_H
#include <libxml/parser.h>
#include <glib.h>

typedef struct
{
    xmlChar *target;
    xmlChar *container;
}
DistributionMapping;

DistributionMapping *create_distribution_mapping(xmlChar *target, xmlChar *container);

DistributionMapping *create_distribution_auto_mapping(xmlChar *target);

void *parse_distribution_mapping(xmlNodePtr element, void *userdata);

void delete_distribution_mapping(DistributionMapping *mapping);

void print_distribution_mapping_nix(FILE *file, const DistributionMapping *mapping, const int indent_level, void *userdata);

void print_distribution_mapping_xml(FILE *file, const DistributionMapping *mapping, const int indent_level, const char *type_property_name, void *userdata);

#endif
