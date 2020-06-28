#ifndef __DYDISNIX_TARGETSTABLE2_H
#define __DYDISNIX_TARGETSTABLE2_H
#include <glib.h>
#include <targetstable.h>
#include <nixxml-types.h>

char *generate_infrastructure_xml_from_expr(char *infrastructure_expr);

GHashTable *create_targets_table_from_nix_file(gchar *infrastructure_nix);

GHashTable *create_targets_table2(gchar *infrastructure, const NixXML_bool xml);

void substract_target_value(Target *target, const gchar *property_name, int amount);

#endif
