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

void MeshToDualGraph(mesh_t *mesh, idx_t **xadj, idx_t **adjncy)
{
    int result;
    idx_t* ne = &mesh->n_elements;
    idx_t* nn = &mesh->n_nodes;
    idx_t numflag = 0;
    idx_t ncommon = 1;

    int ofs = mesh->offset[mesh->n_face_elements];
    int* eptr = new int[mesh->n_elements + 1];
    for (int i = mesh->n_face_elements, j = 0; i < mesh->offset.size(); i++, j++)
    {
        eptr[j] = mesh->offset[i] - ofs;
    }

    idx_t* eind = &mesh->conn[ofs];

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

void UpdateMeshArrays(mesh_t* mesh, int* sort, int** newConn, int** newOffset)
{
    int ne = mesh->n_elements;
    int nfe = mesh->n_face_elements;

    for(int i = nfe, j = 0 ; i <= nfe + ne ; i++, j++)
        mesh->offset[i] = (*newOffset)[j];

    for(int i = mesh->offset[nfe], j = 0 ; i < mesh->conn.size() ; i++, j++)
        mesh->conn[i] = (*newConn)[j];


    int biggestColor = mesh->mesh_coloring[sort[ne-1]];
    int* mesh_coloringAux = new int [biggestColor];
    int count = 0;
    int color = 1;

    for(int i = 0 ; i < biggestColor ; i++)
        mesh_coloringAux[i] = 0;

    for(int i = 0 ; i < ne ; i++)
    {
        mesh_coloringAux[mesh->mesh_coloring[i]-1]++;
    }

    delete [] mesh->mesh_coloring;

    mesh->mesh_coloring = mesh_coloringAux;
}

void ReorderElements(mesh_t* mesh, int* sort, int** newConn, int** newOffset)
{
    int ne = mesh->n_elements;
    int nfe = mesh->n_face_elements;
    *newConn = new int [mesh->offset.back() - mesh->offset[nfe]];
    *newOffset = new int [ne + 1];
    unsigned int countConn = 0;
    unsigned int countOffset = 1;

    (*newOffset)[0] = mesh->offset[nfe];

    for(int i = 0 ; i < ne ; i++)
    {
        int start = mesh->offset[nfe + sort[i]];
        int end = mesh->offset[nfe + sort[i] + 1];

        (*newOffset)[countOffset] = (*newOffset)[countOffset - 1] + (end-start);
        countOffset++;
        for(int j = start ; j < end ; j++)
        {
            (*newConn)[countConn] = mesh->conn[j];
            countConn++;
        }
    }
}

void CreateSort(mesh_t* mesh, int biggestColor, int* sort)
{
    int ne = mesh->n_elements;
    int count = 0;

    for(int i = 1 ; i <= biggestColor ; i++)
    {
        for(int j = 0 ; j < ne ; j++)
        {
            if(mesh->mesh_coloring[j] == i)
            {
                sort[count] = j;
                count++;
            }
        }
    }
}

int Coloring(mesh_t* mesh)
{
    idx_t* xadj;
    idx_t* adjncy;
    int ne = mesh->n_elements;
    int* elementsColor = new int [ne];
    int biggestColor = 1; // variavel importante para a função CreateSort()
    
    MeshToDualGraph(mesh, &xadj, &adjncy);

    for(int i = 0 ; i < ne ; i++)
        elementsColor[i] = -1; // flag para elemento sem cor

    for(int i = 0 ; i < ne ; i++)
    {
        int start = xadj[i];
        int end = xadj[i+1];

        int count = 1;
        int j = start;

        while(j < end)
        {
            int elem_adj = adjncy[j];

            if(elementsColor[elem_adj] == count)
            {
                count++;
                j = start;
            }
            else
                j++;
        }

        elementsColor[i] = count;

        if(count > biggestColor)
            biggestColor = count;
    }
    
    delete [] mesh->mesh_coloring; // delete do new feito na função MeshGmshReader onde inicializa todo o vetor mesh_coloring com -1

    mesh->mesh_coloring = elementsColor;

    METIS_Free(xadj);
    METIS_Free(adjncy);

    return biggestColor;
}

void MeshColoring(mesh_t* mesh)
{
    cout << "Starting mesh coloring..." << endl;
    int* sort = new int [mesh->n_elements];
    int* newConn;
    int* newOffset;
    int biggestColor;

    biggestColor = Coloring(mesh);
    CreateSort(mesh, biggestColor, sort);
    ReorderElements(mesh, sort, &newConn, &newOffset);
    UpdateMeshArrays(mesh, sort, &newConn, &newOffset);
    mesh->biggestColor = biggestColor;
    cout << "Finished mesh coloring..." << endl;

    delete [] sort;
    delete [] newConn;
    delete [] newOffset;   
}