#include <iostream>
#include <cmath>
#include <string>

#include "petsc.h"

using namespace std;

int main(int argc, char *argv[])
{
    /*

      | 1   1 |  | x_1 |  =  | 1 |
      | 1  -2 |  | x_2 |     | 0 |

      x_1 = 0.667,
      x_2 = 0.333

    */

    PetscInitialize(&argc,&argv,0,0);
    PetscScalar b_arr[2] = {1, 0};
    PetscScalar a_mat[2][2];
    a_mat[0][0] = 1;
    a_mat[0][1] = 1;
    a_mat[1][0] = 1;
    a_mat[1][1] = -2;
    PetscInt col = 0;
    PetscInt pos[2] = {0, 1};

    PetscViewer v_view, m_view;
    Vec b;
    VecCreate(PETSC_COMM_WORLD, &b);
    VecSetSizes(b,2,PETSC_DECIDE);
    VecSetFromOptions(b);

    VecSetValues(b, 2, &col, b_arr, INSERT_VALUES);
    VecAssemblyBegin(b); 
    VecAssemblyEnd(b); 
    cout << "\nVetor B: ";
    VecView(b, v_view);

    Vec x;
    cout << "\nVetor X: ";
    VecDuplicate(b, &x);
    VecView(x, v_view);

    Mat A;
    MatCreateSeqAIJ(PETSC_COMM_WORLD, 2, 2, 2, NULL, &A);
    MatSetFromOptions(A);
    MatSeqAIJSetPreallocation(A, 2, NULL);
    MatSetValues(A, 2, &pos[0], 2, &pos[0], &a_mat[0][0], INSERT_VALUES);
    MatAssemblyBegin(A, MAT_FINAL_ASSEMBLY);
    MatAssemblyEnd(A, MAT_FINAL_ASSEMBLY);

    cout << "\nMatriz A: ";
    MatView(A, m_view);

    KSP ksp;
    // Creating linear solver
    KSPCreate(PETSC_COMM_WORLD, &ksp);

    KSPSetOperators(ksp, A, A);
    KSPSetFromOptions(ksp);

    KSPSolve(ksp, b, x);

    cout << "Resultado: ";
    VecView(x, v_view);
    PetscFinalize();
    return 0;
}