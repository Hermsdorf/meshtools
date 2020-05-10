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

void UpdateMeshArrays(mesh_t* mesh, int* newConn, int* newOffset)
{
    int ne = mesh->n_elements;
    int nfe = mesh->n_face_elements;

    for(int i = nfe, j = 0 ; i <= nfe + ne ; i++, j++)
        mesh->offset[i] = newOffset[j];

    for(int i = mesh->offset[nfe], j = 0 ; i < mesh->conn.size() ; i++, j++)
        mesh->conn[i] = newConn[j];

    /*cout << "NOVO CONN: ";
    for(int i = mesh->offset[nfe]; i < mesh->conn.size() ; i++)
        cout << mesh->conn[i] << " ";
    cout << endl << "NOVO OFFSET: ";
    for(int i = nfe ; i <= nfe + ne ; i++)
        cout << mesh->offset[i] << " ";
    cout << endl;*/
}

void ReorderElements(mesh_t* mesh, int* sort)
{
    int ne = mesh->n_elements;
    int nfe = mesh->n_face_elements;
    int* newConn = new int [mesh->offset.back() - mesh->offset[nfe]];
    int* newOffset = new int [ne + 1];
    unsigned int countConn = 0;
    unsigned int countOffset = 1;

    newOffset[0] = mesh->offset[nfe];

    for(int i = 0 ; i < ne ; i++)
    {
        int start = mesh->offset[nfe + sort[i]];
        int end = mesh->offset[nfe + sort[i] + 1];

        newOffset[countOffset] = newOffset[countOffset - 1] + (end-start);
        countOffset++;
        for(int j = start ; j < end ; j++)
        {
            newConn[countConn] = mesh->conn[j];
            countConn++;
        }
    }

    qsort(mesh->mesh_coloring, ne, sizeof(int), compare_int);

    UpdateMeshArrays(mesh, newConn, newOffset);

    delete [] newConn;
    delete [] newOffset;   
}

void CreateSort(mesh_t* mesh, int biggestColor)
{
    int ne = mesh->n_elements;
    int* sort = new int [ne];
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

    ReorderElements(mesh, sort);

    delete [] sort;
}

void MeshColoring(mesh_t* mesh)
{
    idx_t* xadj;
    idx_t* adjncy;
    int ne = mesh->n_elements;
    int* elementsColor = new int [ne];
    int biggestColor = 1; // variavel importante para a função MeshColoringReorder()

    cout << "Starting mesh coloring..." << endl;
    
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

    cout << "Finished mesh coloring..." << endl;

    CreateSort(mesh, biggestColor);

    METIS_Free(xadj);
    METIS_Free(adjncy);
}