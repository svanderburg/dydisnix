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
    
    GArray *property;
}
Service;

GArray *create_service_property_array(const gchar *services_xml_file);

void delete_service_property_array(GArray *service_property_array);

void print_service_property_array(const GArray *service_property_array);

Service *lookup_service(GArray *service_property_array, gchar *name);

ServiceProperty *lookup_service_property(Service *service, gchar *name);

#endif
