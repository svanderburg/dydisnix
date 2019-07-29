#ifndef __DYDISNIX_INFRASTRUCTUREPROPERTIES_H
#define __DYDISNIX_INFRASTRUCTUREPROPERTIES_H
#include <glib.h>
#include <targetstable.h>

char *generate_infrastructure_xml_from_expr(char *infrastructure_expr);

GHashTable *create_target_property_table_from_xml(const gchar *infrastructure_xml_file);

GHashTable *create_target_property_table_from_nix(gchar *infrastructure_nix);

GHashTable *create_target_property_table(gchar *infrastructure, const int xml);

void substract_target_value(Target *target, gchar *property_name, int amount);

#endif
