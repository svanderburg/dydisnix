#ifndef __DYDISNIX_BOUNDARIES_H
#define __DYDISNIX_BOUNDARIES_H
#include <glib.h>
#include "idresourcetype.h"

typedef struct
{
    int lowest_id;
    int highest_id;
}
Boundaries;

Boundaries *compute_boundaries(IdResourceType *type, GHashTable *id_assignments_table, GPtrArray *service_names);

#endif
