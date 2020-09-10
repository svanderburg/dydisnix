#ifndef __DYDISNIX_IDTOSERVICETABLE_H
#define __DYDISNIX_IDTOSERVICETABLE_H
#include <glib.h>
#include <nixxml-types.h>
#include "idresourcetype.h"
#include "boundaries.h"

NixXML_bool derive_next_id(IdResourceType *type, Boundaries *boundaries, GHashTable *id_to_service_table, int *next_id);

GHashTable *derive_id_to_service_table(GHashTable *id_assignments_table);

#endif
