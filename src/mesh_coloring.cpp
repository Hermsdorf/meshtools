#include <iostream>
#include <algorithm>

using namespace std;

#include "metis.h"
#include "mesh.h"


int compare_int(const void *a, const void *b)
{
    const int *da = (const int *)a;
    const int *db = (const int *)b;

    return (*da > *db);
}

void MeshToDualGraph(Mesh *mesh, idx_t **xadj, idx_t **adjncy)
{
    int result;
    int nelem = mesh->get_n_elements();
    int nnodes = mesh->get_n_nodes();
    idx_t* eptr;
    idx_t* eind;
    idx_t* ne;
    idx_t* nn = &nnodes;
    idx_t numflag = 0;
    idx_t ncommon = 1;

    ne = &nelem;
    unsigned int ofs = mesh->getElementOffset(0)[0];
    eptr = new idx_t[mesh->get_n_elements() + 1];

    int* offset_aux = (int*)mesh->getElementOffset(0);
    for (int i = 0, j = 0; i <= nelem ; i++, j++)
    {
        eptr[j] = offset_aux[i] - ofs;
    }

    eind = (idx_t*)mesh->getElementConn(0);

    result = METIS_MeshToDual(ne, nn, eptr, eind, &ncommon, &numflag, xadj, adjncy);

    if (result == METIS_OK)
    {
        cout << "Mesh to Dual Graph sucessfully applied" << endl;
    }
    else
    {
        if (result == METIS_ERROR_INPUT)
        {
            cout << "Input error" << endl;
            exit(1);
        }
        else
        {
            if (result == METIS_ERROR_MEMORY)
            {
                cout << "Memory error" << endl;
                exit(1);
            }
            else
            {
                cout << "Another kind of error" << endl;
                exit(1);
            }
        }
    }

    delete[] eptr;
}

void UpdateMeshArrays(Mesh* mesh, int* sort, unsigned int** new_conn, unsigned int** new_offset)
{
    unsigned int ne = mesh->get_n_elements();
    unsigned int nfe = mesh->get_n_face_elements();
    unsigned int n_colors;
    int* mesh_coloring;
    unsigned int count = 0;
    unsigned int color = 1;

    mesh_coloring = mesh->get_mesh_coloring_internal();
    n_colors = mesh->get_n_internal_colors();

    for(unsigned int i = nfe, j = 0 ; i <= nfe + ne ; i++, j++)
        mesh->setOffsetPosition((*new_offset)[j], i);

    for(unsigned int i = mesh->getElementOffset(0)[0], j = 0 ; i < mesh->getElementOffset(ne)[0] ; i++, j++)
        mesh->setConnPosition((*new_conn)[j], i);

    int* mesh_coloringAux = new int [n_colors];
    
    for(int i = 0 ; i < n_colors ; i++)
        mesh_coloringAux[i] = 0;

    for(int i = 0 ; i < ne ; i++)
    {
        mesh_coloringAux[mesh_coloring[i]-1]++;
    }

    delete [] mesh->get_mesh_coloring_internal();
    mesh->set_mesh_coloring_internal(mesh_coloringAux);
}

void ReorderElements(Mesh* mesh, int* sort, unsigned int** new_conn, unsigned int** new_offset)
{
    unsigned int ne = mesh->get_n_elements();
    *new_conn = new unsigned int [mesh->getOffset().back() - mesh->getElementOffset(0)[0]];

    *new_offset = new unsigned int [ne + 1];
    unsigned int count_conn = 0;
    unsigned int count_offset = 1;

    (*new_offset)[0] = mesh->getElementOffset(0)[0];

    for(unsigned int i = 0 ; i < ne ; i++)
    {
        unsigned int start = mesh->getElementOffset(sort[i])[0];
        unsigned int end = mesh->getElementOffset(sort[i] + 1)[0];

        (*new_offset)[count_offset] = (*new_offset)[count_offset - 1] + (end-start);
        count_offset++;
        for(unsigned int j = start ; j < end ; j++)
        {
            (*new_conn)[count_conn] = mesh->getConn()[j];
            count_conn++;
        }
    }
}

void CreateSort(Mesh* mesh, int* sort)
{
    unsigned int ne = mesh->get_n_elements();
    int* mesh_coloring = mesh->get_mesh_coloring_internal();

    unsigned int count = 0;

    for(unsigned int i = 1 ; i <= mesh->get_n_internal_colors() ; i++)
    {
        for(int j = 0 ; j < ne ; j++)
        {
            if(mesh_coloring[j] == i)
            {
                sort[count] = j;
                count++;
            }
        }
    }
}

int Coloring(Mesh* mesh)
{
    idx_t* xadj;
    idx_t* adjncy;
    unsigned int ne = mesh->get_n_elements();
    int* elements_color = new int[ne];
    unsigned int n_colors = 1; // Número total de cores da malha

    MeshToDualGraph(mesh, &xadj, &adjncy);

    for(unsigned int i = 0 ; i < ne ; i++)
        elements_color[i] = -1; // flag para elemento sem cor

    for(unsigned int i = 0 ; i < ne ; i++)
    {
        unsigned int start = xadj[i];
        unsigned int end = xadj[i+1];

        unsigned int count = 1;
        unsigned int j = start;

        while(j < end)
        {
            unsigned int elem_adj = adjncy[j];

            if(elements_color[elem_adj] == count)
            {
                count++;
                j = start;
            }
            else
                j++;
        }

        elements_color[i] = count;

        if(count > n_colors)
            n_colors = count;
    }


    delete [] mesh->get_mesh_coloring_internal(); // delete do new feito na função MeshGmshReader onde inicializa todo o vetor mesh_coloring_bound com -1
    mesh->set_mesh_coloring_internal(elements_color);
    
    METIS_Free(xadj);
    METIS_Free(adjncy);

    return n_colors;
}

void Mesh::MeshColoring()
{
    cout << "Starting mesh coloring..." << endl;
    int* sort_internal = new int [this->get_n_elements()];
    unsigned int* new_conn;
    unsigned int* new_offset;
    unsigned int n_colors;

    n_colors = Coloring(this);
    this->n_internal_colors = n_colors;

    CreateSort(this, sort_internal);
    ReorderElements(this, sort_internal, &new_conn, &new_offset);
    UpdateMeshArrays(this, sort_internal, &new_conn, &new_offset);
    
    cout << "Finished mesh coloring..." << endl;  

    delete [] sort_internal;
    delete [] new_conn;
    delete [] new_offset; 
}