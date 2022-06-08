#ifndef FEM_FUNCTIONS_H__
#define FEM_FUNCTIONS_H__

void ComputeTRI3Functions(double *xy, double qpoints[2], double phi[3], double dphi[3][2], double *JxW);

#endif /* FEM_FUNCTIONS_H__ */
