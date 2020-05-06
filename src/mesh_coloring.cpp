#include <iostream>

using namespace std;

#include "metis.h"
#include "mesh.h"

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
        /*cout << "XADJ: ";
        for(int i = 0 ; i < *ne+1 ; i++)
            cout << (*xadj)[i] << " ";
        cout << endl << "ADJNCY: ";
        for(int i = 0 ; i < (*xadj)[*ne] ; i++)
            cout << (*adjncy)[i] << " ";
        cout << endl;*/
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

void MeshColoring(mesh_t* mesh)
{
    idx_t* xadj;
    idx_t* adjncy;
    int ne = mesh->n_elements;
    int* elementsColor = new int [ne];

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
    }
    
    mesh->mesh_coloring = elementsColor;

    /*cout << "MESH_COLORING: ";
    for(int i = 0 ; i < ne ; i++)
        cout << mesh->mesh_coloring[i] << " ";
    cout << endl;*/

    cout << "Finished mesh coloring..." << endl;
    METIS_Free(xadj);
    METIS_Free(adjncy);
}