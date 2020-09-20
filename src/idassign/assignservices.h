#ifndef __DYDISNIX_ASSIGNSERVICES_H
#define __DYDISNIX_ASSIGNSERVICES_H
#include <glib.h>
#include <nixxml-types.h>
#include "idresourcetype.h"
#include "idsconfig.h"

NixXML_bool assign_ids_to_services(GHashTable *id_resources_table, IdsConfig *ids_config, GHashTable *services_table, gchar *service_property);

#endif
