#ifndef FEADAPTOR_HEADER
#define FEADAPTOR_HEADER

#ifdef __cplusplus
extern "C" {
#endif
void CatalystInitialize(int numScripts, char* scripts[]);

void CatalystFinalize();

void CatalystCoProcess(mesh_t* mesh, double* velocityData, float* pressureData, double time,
  unsigned int timeStep, int lastTimeStep);
#ifdef __cplusplus
}
#endif
#endif
