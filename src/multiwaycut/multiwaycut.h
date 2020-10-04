#ifndef __DYDISNIX_MULTIWAYCUT_H
#define __DYDISNIX_MULTIWAYCUT_H
#include <glib.h>
#include <checkoptions.h>

typedef enum
{
  ARTIFACT_NIX,
  ARTIFACT_XML,
  ARTIFACT_GRAPH,
  ARTIFACT_RESOLVED_GRAPH
}
OutputArtifactType;

int multiwaycut(gchar *services, gchar *distribution, gchar *infrastructure, const unsigned int flags, const OutputArtifactType artifact_type);

#endif
