#ifndef __DYDISNIX_IDTOSERVICETABLE_H
#define __DYDISNIX_IDTOSERVICETABLE_H
#include <glib.h>
#include <nixxml-types.h>
#include "idresourcetype.h"

NixXML_bool derive_next_id(const IdResourceType *type, GHashTable *id_to_service_table, int *last_id, int *next_id);

GHashTable *derive_id_to_service_table(GHashTable *id_assignments_table, GPtrArray *service_names);

#endif
