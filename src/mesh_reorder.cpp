
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <memory>

using namespace std;

#include "metis.h"
#include "mesh.h"
#include "rcm.hpp"



void convert_to_one_index( int node_num, int adj_num, int adj_row[], int adj[])
{

        for (int i = 0; i < node_num; i++)
        {
            adj_row[i] += 1;
        }
    
        for(int i = 0; i < adj_num; i++)
            adj[i] += 1;
    
        adj_row[node_num]+=1;

  }

void convert_to_zero_index( int node_num, int adj_num, int adj_row[], int adj[])
  {

        for (int i = 0; i < node_num; i++)
        {
            adj_row[i] -= 1;
        }
        for(int i = 0; i < adj_num; i++)
            adj[i] -= 1;
    
        adj_row[node_num]-=1;
  }



void WriteAdj(const char *fname, int nvts, idx_t *xadj, idx_t *adjncy)
{

    ofstream fout;
    fout.open(fname);
    if (fout.is_open())
    {

        fout << nvts << endl;
        for (int i = 0; i <= nvts; i++)
            fout << xadj[i] << endl;

        for (int i = 0; i < xadj[nvts]; i++)
            fout << adjncy[i] << endl;

        fout.close();
    }
}

void WriteAIJ(const char *fname, int nvts, idx_t *xadj, idx_t *adjncy, int one_flag)
{

    ofstream fout;
    fout.open(fname);
    if (fout.is_open())
    {
        for (int i = 0; i < nvts; i++)
        {
            fout << "Adj. node " << i << "[" << xadj[i] << " - " << xadj[i + 1] - 1 << "]: ";
            for (int j = xadj[i]; j <= xadj[i + 1] - 1; j++)
            {
                int col = adjncy[j - one_flag] - one_flag;
                fout << col << " ";
            }
            fout << endl;
        }
        fout.close();
    }
}



void MeshToGraph(mesh_t *mesh, idx_t **xadj, idx_t **adjncy)
{
    int result;

    idx_t *ne = &mesh->n_elements;
    idx_t *nn = &mesh->n_nodes;
    idx_t numflag = 0;

    int ofs = mesh->offset[mesh->n_face_elements];
    int *eptr = new int[mesh->n_elements + 1];
    for (int i = mesh->n_face_elements, j = 0; i < mesh->offset.size(); i++, j++)
    {
        eptr[j] = mesh->offset[i] - ofs;
    }

    idx_t *eind = &mesh->conn[ofs];

    result = METIS_MeshToNodal(ne, nn, eptr, eind, &numflag, xadj, adjncy);

    if (result == METIS_OK)
    {
        
        int adj_size = (*xadj)[*nn]; 
        convert_to_one_index(*nn, adj_size, *xadj, *adjncy);

        cout << "   - Original Bandwidth: " << adj_bandwidth(mesh->n_nodes, (*xadj)[mesh->n_nodes], *xadj, *adjncy) << endl;

        convert_to_zero_index(*nn, adj_size, *xadj, *adjncy);
     
        cout << "Mesh to graph succesfully applied" << endl;
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

void ApplyReorderMesh(mesh_t *mesh, int *perm, int *iperm)
{
    vector<double> newCoord;
    newCoord.resize(mesh->coord.size());

    cout << "  Applying reordering..." << endl;

#pragma omp parallel for
    for (int i = 0; i < mesh->n_nodes; i++)
    {
        for (int j = 0; j < 3; j++)
            newCoord[(3 * i) + j] = mesh->coord[(3 * perm[i]) + j];
    }
    mesh->coord.swap(newCoord);
    newCoord.clear();

    vector<int> newConn;
    newConn.resize(mesh->conn.size());
    
#pragma omp parallel for
    for (int i = 0; i < mesh->conn.size(); i++)
    {
        newConn[i] = iperm[mesh->conn[i]];
    }
    mesh->conn.swap(newConn);
    newConn.clear();
}

int compare_idx(const void *a, const void *b)
{
    const idx_t *da = (const idx_t *)a;
    const idx_t *db = (const idx_t *)b;

    return (*da > *db);
}

void MeshToRCMGraph(mesh_t *mesh, idx_t **xadj, idx_t **adjncy)
{

    MeshToGraph(mesh, xadj, adjncy);

    idx_t *xadjA = *xadj;
    idx_t *adjncyA = *adjncy;

#ifdef DEBUG
    WriteAIJ("antes_rcm.txt", mesh->n_nodes, xadjA, adjncyA, 0);
#endif

#pragma omp parallel for
    for (int n = 0; n < mesh->n_nodes; n++)
    {
        int start = xadjA[n];
        int end   = xadjA[n + 1];
        qsort(&adjncyA[start], (end - start), sizeof(idx_t), compare_idx);
    }

    int n_adjncyA = xadjA[mesh->n_nodes];

#pragma omp parallel for
    for (int i = 0; i <= mesh->n_nodes; i++)
        xadjA[i] += 1;

#pragma omp parallel for
    for (int i = 0; i < n_adjncyA; i++)
        adjncyA[i] += 1;
}

void MeshReorderingRCM(mesh_t *mesh, idx_t *xadj, idx_t *adjncy, int *perm, int *iperm)
{
    cout << "  Applyng RCM reordering " << endl;

#ifdef DEBUG
    WriteAIJ("adj_rcm.txt", mesh->n_nodes, xadj, adjncy, 1);
#endif

    genrcm(mesh->n_nodes, xadj[mesh->n_nodes], xadj, adjncy, perm);
    // função responsável por retornar o iperm a partir do numero de elementos permutados e do perm
    perm_inverse3(mesh->n_nodes, perm, iperm);

    cout << "   - Final Bandwidth: " << adj_perm_bandwidth(mesh->n_nodes,xadj[mesh->n_nodes], xadj, adjncy,perm, iperm) << endl;

#pragma omp parallel for
    for (int i = 0; i < mesh->n_nodes; i++)
    {
        perm[i]--;
        iperm[i]--;
    }
}

void MeshReorderingMETIS(mesh_t *mesh, idx_t *xadj, idx_t *adjncy, int *perm, int *iperm)
{
    int result;
    idx_t *nn = &mesh->n_nodes;
    idx_t *vwgt = 0;
    idx_t options[METIS_NOPTIONS];

    cout << "  Applyng METIS_NodeND reordering " << endl;
    METIS_SetDefaultOptions(options);

    options[METIS_OPTION_NUMBERING] = 0;
    result = METIS_NodeND(nn, xadj, adjncy, vwgt, options, perm, iperm);

    if (result == METIS_OK)
    {
        cout << "METIS reordering succesfully applied" << endl;
        int adj_size = xadj[*nn];
        convert_to_one_index(*nn,adj_size,xadj, adjncy);
        for(int i = 0; i < mesh->n_nodes; i++)
        {
            perm[i]++;
            iperm[i]++;
        }
        cout << "  - Final Bandwidth: " << adj_perm_bandwidth(mesh->n_nodes,xadj[mesh->n_nodes], xadj, adjncy, perm, iperm) << endl;
        convert_to_zero_index(*nn,adj_size,xadj, adjncy);
                for(int i = 0; i < mesh->n_nodes; i++)
        {
            perm[i]--;
            iperm[i]--;
        }
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
}

void MeshReorderingFirstTouch(mesh_t *mesh, int *perm, int *iperm)
{

#pragma omp parallel for
    for (int i = 0; i < mesh->n_nodes; i++)
        perm[i] = -1;

    unsigned int counter = 0;


    for (int i = 0; i < mesh->n_elements; i++)
    {
        int iel = mesh->n_face_elements + i;

        for (int eno = mesh->offset[iel]; eno < mesh->offset[iel + 1]; eno++)
        {
            if (perm[mesh->conn[eno]] == -1)
            {
                perm[mesh->conn[eno]] = counter;
                iperm[counter] = mesh->conn[eno];
                counter++;
            }
        }
    }
}

void MeshReordering(mesh_t *mesh, reorder_t reorder = RCM)
{
    idx_t *xadj;
    idx_t *adjncy;

    std::unique_ptr<int[]> perm_ptr = make_unique<int[]>(mesh->n_nodes);
    std::unique_ptr<int[]> iperm_ptr = make_unique<int[]>(mesh->n_nodes);

    int *perm = perm_ptr.get();
    int *iperm = iperm_ptr.get();

    cout << "Starting mesh reordering... " << endl;
    switch (reorder)
    {
    case FF:
        MeshReorderingFirstTouch(mesh, perm, iperm);
        ApplyReorderMesh(mesh, perm, iperm);
        break;
    case METIS_ND:
        MeshToGraph(mesh, &xadj, &adjncy);
        MeshReorderingMETIS(mesh, xadj, adjncy, perm, iperm);
        ApplyReorderMesh(mesh, perm, iperm);
        break;
    default:
        MeshToRCMGraph(mesh, &xadj, &adjncy);
        MeshReorderingRCM(mesh, xadj, adjncy, perm, iperm);
        ApplyReorderMesh(mesh, perm, iperm);
        break;
    }
    cout << "Finished mesh reordering... " << endl;

    METIS_Free(xadj);
    METIS_Free(adjncy);
}