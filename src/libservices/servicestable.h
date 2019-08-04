#ifndef __DYDISNIX_SERVICESTABLE_H
#define __DYDISNIX_SERVICESTABLE_H
#include <glib.h>
#include <nixxml-parse.h>
#include "service.h"

char *generate_service_xml_from_expr(char *service_expr);

GHashTable *create_service_table_from_xml(const gchar *services_xml_file);

GHashTable *create_service_table_from_nix(gchar *services_nix);

GHashTable *create_service_table(gchar *services, const int xml);

void delete_service_table(GHashTable *service_table);

#endif
