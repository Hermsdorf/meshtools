#ifndef FEADAPTOR_H
#define FEADAPTOR_H

#include "mesh.h"
#include "meshtools_config.h"

#ifdef CATALYST_LEGACY_ENABLE

#ifdef __cplusplus
extern "C" {
#endif
void CatalystInitialize(int numScripts, char* scripts[]);

void CatalystFinalize();

void CatalystCoProcess(Mesh* mesh, double* velocityData, float* pressureData, double time,
  unsigned int timeStep, int lastTimeStep);
#ifdef __cplusplus
}
#endif
#endif
#endif /* FEADAPTOR_H */
