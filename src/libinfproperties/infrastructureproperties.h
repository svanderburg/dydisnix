#ifndef __INFRASTRUCTUREPROPERTIES_H
#define __INFRASTRUCTUREPROPERTIES_H
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
    
    GArray *property;
}
Target;

GArray *create_infrastructure_property_array(gchar *infrastructure_xml_file);

void delete_infrastructure_property_array(GArray *infrastructure_property_array);

void print_infrastructure_property_array(GArray *infrastructure_property_array);

gint infrastructure_index(GArray *infrastructure_property_array, gchar *name);

gint infrastructure_property_index(Target *target, gchar *name);

void substract_target_value(Target *target, gchar *property_name, int amount);

#endif
