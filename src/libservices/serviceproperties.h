#ifndef __DYDISNIX_SERVICEPROPERTIES_H
#define __DYDISNIX_SERVICEPROPERTIES_H
#include <glib.h>

typedef struct
{
    gchar *name;

    GHashTable *property;

    GPtrArray *connects_to;

    GPtrArray *depends_on;

    int group_node;
}
Service;

char *generate_service_xml_from_expr(char *service_expr);

GPtrArray *create_service_property_array_from_xml(const gchar *services_xml_file);

GPtrArray *create_service_property_array_from_nix(gchar *services_nix);

GPtrArray *create_service_property_array(gchar *services, const int xml);

void delete_service(Service *service);

GHashTable *copy_properties(GHashTable *properties);

Service *copy_service(const Service *service);

void delete_service_property_array(GPtrArray *service_property_array);

void print_service_property_array(const GPtrArray *service_property_array);

Service *find_service(GPtrArray *service_array, gchar *name);

gchar *find_service_property(const Service *service, gchar *name);

#endif
