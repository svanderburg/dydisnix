#ifndef __DYDISNIX_ASSIGNDISTRIBUTION_H
#define __DYDISNIX_ASSIGNDISTRIBUTION_H
#include <glib.h>
#include <nixxml-types.h>
#include "idsconfig.h"

NixXML_bool assign_ids_to_distributed_services(GHashTable *id_resources_table, IdsConfig *ids_config, GHashTable *services_table, GHashTable *distribution_table, GHashTable *target_to_service_table, gchar *service_property);

#endif
