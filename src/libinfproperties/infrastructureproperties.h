#ifndef __DYDISNIX_INFRASTRUCTUREPROPERTIES_H
#define __DYDISNIX_INFRASTRUCTUREPROPERTIES_H
#include <glib.h>
#include <targets.h>

char *generate_infrastructure_xml_from_expr(char *infrastructure_expr);

GPtrArray *create_target_property_array_from_xml(const gchar *infrastructure_xml_file);

GPtrArray *create_target_property_array_from_nix(gchar *infrastructure_nix);

GPtrArray *create_target_property_array(gchar *infrastructure, const int xml);

void substract_target_value(Target *target, gchar *property_name, int amount);

Target *find_target_by_name(GPtrArray *target_array, gchar *name);

//TargetProperty *find_target_property(Target *target, gchar *name);

#endif
