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
    int nelem = mesh->get_N_elements();
    int nnodes = mesh->get_N_nodes();
    int nfe = mesh->get_N_face_elements();
    idx_t* eptr;
    idx_t* eind;
    idx_t* ne;
    idx_t* nn = &nnodes;
    idx_t numflag = 0;
    idx_t ncommon = 1;

    ne = &nelem;
    int ofs = mesh->getOffset()[nfe];
    eptr = new idx_t[mesh->get_N_elements() + 1];

    for (int i = nfe, j = 0; i < mesh->getOffset().size(); i++, j++)
    {
        eptr[j] = mesh->getOffset()[i] - ofs;
    }

    eind = &mesh->getConn()[ofs];

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

void UpdateMeshArrays(Mesh* mesh, int* sort, int** new_conn, int** new_offset)
{
    int ne;
    int nfe = mesh->get_N_face_elements();
    int skip;
    int n_colors;
    int* mesh_coloring;
    int count = 0;
    int color = 1;

    ne = mesh->get_N_elements();
    skip = nfe;
    mesh_coloring = mesh->get_Mesh_coloring_internal();
    n_colors = mesh->get_N_internal_colors();

    for(int i = skip, j = 0 ; i <= skip + ne ; i++, j++)
        mesh->setOffsetPosition((*new_offset)[j], i);

    for(int i = mesh->getOffset()[skip], j = 0 ; i < mesh->getOffset()[skip + ne] ; i++, j++)
        mesh->setConnPosition((*new_conn)[j], i);

    int* mesh_coloringAux = new int [n_colors];
    
    for(int i = 0 ; i < n_colors ; i++)
        mesh_coloringAux[i] = 0;

    for(int i = 0 ; i < ne ; i++)
    {
        mesh_coloringAux[mesh_coloring[i]-1]++;
    }

    delete [] mesh->get_Mesh_coloring_internal();
    mesh->set_Mesh_coloring_internal(mesh_coloringAux);
}

void ReorderElements(Mesh* mesh, int* sort, int** new_conn, int** new_offset)
{
    int ne;
    int skip;
    int nfe = mesh->get_N_face_elements();

    ne = mesh->get_N_elements();
    skip = nfe;
    *new_conn = new int [mesh->getOffset().back() - mesh->getOffset()[skip]];

    *new_offset = new int [ne + 1];
    int count_conn = 0;
    int count_offset = 1;

    (*new_offset)[0] = mesh->getOffset()[skip];

    for(int i = 0 ; i < ne ; i++)
    {
        int start = mesh->getOffset()[skip + sort[i]];
        int end = mesh->getOffset()[skip + sort[i] + 1];

        (*new_offset)[count_offset] = (*new_offset)[count_offset - 1] + (end-start);
        count_offset++;
        for(int j = start ; j < end ; j++)
        {
            (*new_conn)[count_conn] = mesh->getConn()[j];
            count_conn++;
        }
    }
}

void CreateSort(Mesh* mesh, int* sort)
{
    int ne;
    int* mesh_coloring;

    ne = mesh->get_N_elements();
    mesh_coloring = mesh->get_Mesh_coloring_internal();

    int count = 0;

    for(int i = 1 ; i <= mesh->get_N_internal_colors() ; i++)
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
    int ne = mesh->get_N_elements();
    int* elements_color;

    elements_color = new int [ne];
    int n_colors = 1; // Número total de cores da malha

    MeshToDualGraph(mesh, &xadj, &adjncy);

    for(int i = 0 ; i < ne ; i++)
        elements_color[i] = -1; // flag para elemento sem cor

    for(int i = 0 ; i < ne ; i++)
    {
        int start = xadj[i];
        int end = xadj[i+1];

        int count = 1;
        int j = start;

        while(j < end)
        {
            int elem_adj = adjncy[j];

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


    delete [] mesh->get_Mesh_coloring_internal(); // delete do new feito na função MeshGmshReader onde inicializa todo o vetor mesh_coloring_bound com -1
    mesh->set_Mesh_coloring_internal(elements_color);
    
    METIS_Free(xadj);
    METIS_Free(adjncy);

    return n_colors;
}

void Mesh::MeshColoring()
{
    cout << "Starting mesh coloring..." << endl;
    int* sort_internal = new int [this->get_N_elements()];
    int* new_conn;
    int* new_offset;
    int n_colors;

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