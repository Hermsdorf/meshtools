
#include <cstring>
#include <chrono>
#include <iostream>
using namespace std::chrono;

#include "mesh.h"


namespace FiniteElementKernels
{
    int nDim1,nDim2;
    double* AllocateFiniteElementMatrix(int n_elements, int number_of_element_nodes)
    {
        nDim1 = number_of_element_nodes*number_of_element_nodes;
        nDim2 = number_of_element_nodes;

        return new double[n_elements*nDim1];
    }

    double &at(double *global_matrix, int iel, int i_no, int j_no)
    {
        return global_matrix[iel*nDim1 + i_no*nDim2 + j_no];
    }

    void LocalToGlobalEBE(int iel, int n_nodes, unsigned int* conn, double *localMatrix, double* GlobalMatrix, double* localVector, double *GlobalVector)
    {

        for(int i_no = 0; i_no < n_nodes; i_no++)
        {
            int i_gid  = conn[i_no];
            GlobalVector[i_gid]  += localVector[i_no]; 
            
            for(int j_no = 0; j_no < n_nodes; j_no++)
                at(GlobalMatrix,iel,i_no,j_no) += localMatrix[i_no*n_nodes+j_no];
            
        }

    }

    // Monta o sistema Ax = b, 
    // onde A é armazenada usando element-by-element 
    void Assembly(Mesh& mesh, double *A_EBE, double *b)
    {
        int n_elements          = mesh.get_n_elements();
        int element_n_nodes     = mesh.getElementConnSize(0);
        
        double Ke[element_n_nodes][element_n_nodes];
        double Fe[element_n_nodes];

        for(int i_el = 0; i_el < n_elements; i_el++)
        {

            unsigned int *conn   = mesh.getElementConn(i_el);
            element_n_nodes      = mesh.getElementConnSize(i_el);
            
            for(int i_no = 0; i_no < element_n_nodes; ++i_no)
            {
                int i_gid = conn[i_no];

                Fe[i_no]  += 1.0; 

                for(int j_no = 0; j_no < element_n_nodes; ++j_no)
                {
                     int j_gid = conn[j_no];

                     Ke[i_no][j_no] += 1.0;
                }

                LocalToGlobalEBE(i_el,element_n_nodes,conn,&Ke[0][0],A_EBE,Fe,b);

            }
        }

    }

    void AssemblyOpenMP(Mesh& mesh, double *A_EBE, double *b)
    {
        int n_elements          = mesh.get_n_elements();
        int element_n_nodes     = mesh.getElementConnSize(0);
        int *coloring           = mesh.get_mesh_coloring_internal();
        int n_coloring          = mesh.get_n_internal_colors();
        
#pragma omp parallel firstprivate (n_elements, element_n_nodes, n_coloring)
{
        double Ke[element_n_nodes][element_n_nodes];
        double Fe[element_n_nodes];

        int start = 0;
        for(int c = 0; c < n_coloring; ++c) 
        {
            int blocksize = coloring[c];

#pragma ivdep             
#pragma omp for 
            for(int i_el = start; i_el < (start+blocksize); i_el++)
            {

                unsigned int *conn            = mesh.getElementConn(i_el);
                element_n_nodes      = mesh.getElementConnSize(i_el);
                
                for(int i_no = 0; i_no < element_n_nodes; ++i_no)
                {
                    int i_gid = conn[i_no];

                    Fe[i_no]  += 1.0; 

                    for(int j_no = 0; j_no < element_n_nodes; ++j_no)
                    {
                        int j_gid = conn[j_no];

                        Ke[i_no][j_no] += 1.0;
                    }

                    LocalToGlobalEBE(i_el,element_n_nodes,conn,&Ke[0][0],A_EBE,Fe,b);

                }
            }
            start += blocksize;
        }
    }

}

void run(Mesh& mesh)
{

    std::cout<<"Running finite element assemble kernel" << std::endl;
    int n_elements          = mesh.get_n_elements();
    int element_n_nodes     = mesh.getElementConnSize(0); 
    int n_nodes             = mesh.get_n_nodes();
    
    double * A_ebe = AllocateFiniteElementMatrix(n_elements,element_n_nodes);
    double * b     = new double[n_nodes];  

    
    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    AssemblyOpenMP(mesh,A_ebe,b);
    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    duration<double> time_span = duration_cast<duration<double>>(t2 - t1);

    std::cout << "Assembly CPU Time: " << time_span.count() << " seconds.";
    std::cout << std::endl;

    delete [] A_ebe;
    delete [] b;

}

}




