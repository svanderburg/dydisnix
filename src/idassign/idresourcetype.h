#ifndef __DYDISNIX_IDRESOURCETYPE_H
#define __DYDISNIX_IDRESOURCETYPE_H
#include <libxml/parser.h>
#include <nixxml-types.h>

typedef struct
{
    int min;

    int max;

    xmlChar *scope;
}
IdResourceType;

void *parse_id_resource_type(xmlNodePtr element, void *userdata);

void delete_id_resource_type(IdResourceType *type);

NixXML_bool id_is_outside_range(IdResourceType *type, int id);

#endif
