#ifndef __DYDISNIX_INFRASTRUCTUREPROPERTIES_H
#define __DYDISNIX_INFRASTRUCTUREPROPERTIES_H
#include <glib.h>

typedef struct
{
    gchar *name;
    
    gchar *value;
}
InfrastructureProperty;

typedef struct
{
    gchar *name;
    
    GPtrArray *property;
}
Target;

GPtrArray *create_infrastructure_property_array(const gchar *infrastructure_xml_file);

void delete_infrastructure_property_array(GPtrArray *infrastructure_property_array);

void print_infrastructure_property_array(const GPtrArray *infrastructure_property_array);

void substract_target_value(Target *target, gchar *property_name, int amount);

Target *find_target(GPtrArray *target_array, gchar *name);

InfrastructureProperty *find_infrastructure_property(Target *target, gchar *name);

#endif
