#ifndef __DYDISNIX_IDRESOURCESTABLE_H
#define __DYDISNIX_IDRESOURCESTABLE_H
#include <libxml/parser.h>
#include <glib.h>
#include <nixxml-types.h>
#include "idresourcetype.h"

GHashTable *create_id_resources_table_from_xml(const gchar *id_resources_xml_file);

GHashTable *create_id_resources_table_from_nix(gchar *id_resources_nix);

GHashTable *create_id_resources_table(gchar *id_resources, const NixXML_bool xml);

void delete_id_resources_table(GHashTable *id_resources_table);

#endif
