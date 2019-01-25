#ifndef __DYDISNIX_SERVICEPROPERTIES_H
#define __DYDISNIX_SERVICEPROPERTIES_H
#include <glib.h>

typedef struct
{
    gchar *name;

    gchar *value;
}
ServiceProperty;

typedef struct
{
    gchar *name;

    GPtrArray *property;

    GPtrArray *connects_to;

    GPtrArray *depends_on;
}
Service;

GPtrArray *create_service_property_array(const gchar *services_xml_file);

void delete_service_property_array(GPtrArray *service_property_array);

void print_service_property_array(const GPtrArray *service_property_array);

Service *find_service(GPtrArray *service_array, gchar *name);

ServiceProperty *find_service_property(const Service *service, gchar *name);

#endif
