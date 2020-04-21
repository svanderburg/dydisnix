#ifndef __DYDISNIX_SERVICE_H
#define __DYDISNIX_SERVICE_H
#include <glib.h>
#include <libxml/parser.h>

typedef struct
{
    xmlChar *name;

    xmlChar *type;

    xmlChar *group;

    xmlChar *provides_container;

    GHashTable *provides_containers_table;

    GPtrArray *connects_to;

    GPtrArray *depends_on;

    GHashTable *properties;

    int group_node;
}
Service;

void *parse_service(xmlNodePtr element, void *userdata);

void delete_service(Service *service);

GHashTable *copy_properties(GHashTable *properties);

Service *copy_service(const Service *service);

xmlChar *find_service_property(Service *service, gchar *service_name);

#endif
