#ifndef __DYDISNIX_SERVICEPROPERTIES_H
#define __DYDISNIX_SERVICEPROPERTIES_H
#include <glib.h>
#include <nixxml-parse.h>

typedef struct
{
    xmlChar *name;

    xmlChar *type;

    xmlChar *group;

    GPtrArray *connects_to;

    GPtrArray *depends_on;

    GHashTable *properties;

    int group_node;
}
Service;

char *generate_service_xml_from_expr(char *service_expr);

GHashTable *create_service_table_from_xml(const gchar *services_xml_file);

GHashTable *create_service_table_from_nix(gchar *services_nix);

GHashTable *create_service_table(gchar *services, const int xml);

void delete_service(Service *service);

GHashTable *copy_properties(GHashTable *properties);

Service *copy_service(const Service *service);

void delete_service_table(GHashTable *service_table);

xmlChar *find_service_property(Service *service, gchar *service_name);

#endif
