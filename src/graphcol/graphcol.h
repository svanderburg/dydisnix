#ifndef __DYDISNIX_GRAPHCOL_H
#define __DYDISNIX_GRAPHCOL_H
#include <checkoptions.h>

typedef enum
{
  ARTIFACT_NIX,
  ARTIFACT_XML,
  ARTIFACT_GRAPH,
  ARTIFACT_RESOLVED_GRAPH
}
OutputArtifactType;

int graphcol(char *services, char *infrastructure, const unsigned int flags, const OutputArtifactType artifact_type);

#endif
