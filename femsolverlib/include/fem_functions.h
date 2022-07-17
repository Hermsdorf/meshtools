#ifndef FEM_FUNCTIONS_H
#define FEM_FUNCTIONS_H

void QGaussTri3(int nqp, double qp[][2], double qw[]);
void ComputeTRI3Functions(double gp[], double qw, double *coords, double xyqp[2],double phi[3], double dphi[3][2], double *JxW);

#endif /* FEM_FUNCTIONS_H */
