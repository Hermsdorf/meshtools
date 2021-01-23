
#include <iostream>
#include <fstream>
#include <memory>
#include <algorithm>

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
    std::ofstream fout;
    fout.open(fname);
    if (fout.is_open())
    {

        fout << nvts << "\n";
        for (int i = 0; i <= nvts; i++)
            fout << xadj[i] << "\n";

        for (int i = 0; i < xadj[nvts]; i++)
            fout << adjncy[i] << "\n";

        fout.close();
    }
}

void WriteAIJ(const char *fname, int nvts, idx_t *xadj, idx_t *adjncy, int one_flag)
{

    std::ofstream fout;
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
            fout << "\n";
        }
        fout.close();
    }
}

void MeshToGraph(Mesh *mesh, idx_t **xadj, idx_t **adjncy)
{
    int nelem = (int)mesh->get_n_elements();
    int nnodes = (int)mesh->get_n_nodes();

    idx_t *ne = &nelem;
    idx_t *nn = &nnodes;
    idx_t numflag = 0;

    unsigned int ofs = mesh->getElementOffset(0)[0];
    int *eptr = new int[mesh->get_n_elements() + 1];

    int* offset_aux = (int*)mesh->getElementOffset(0);
    for (int i = 0, j = 0; i <= nelem ; i++, j++)
    {
        eptr[j] = offset_aux[i] - ofs;
    }

    idx_t *eind = (idx_t*)mesh->getElementConn(0);

    int result = METIS_MeshToNodal(ne, nn, eptr, eind, &numflag, xadj, adjncy);

    if (result == METIS_OK)
    {     
        std::cout << "Mesh to Nodal Graph succesfully applied\n";
    }
    else
    {
        if (result == METIS_ERROR_INPUT)
        {
            std::cout << "Input error\n";
            exit(1);
        }
        else
        {
            if (result == METIS_ERROR_MEMORY)
            {
                std::cout << "Memory error\n";
                exit(1);
            }
            else
            {
                std::cout << "Another kind of error\n";
                exit(1);
            }
        }
    }

    delete[] eptr;
}

void ApplyReorderMesh(Mesh *mesh, int *perm, int *iperm)
{
    std::vector<double> newCoord;
    std::vector<double> &coordAux = mesh->getCoord();
    newCoord.resize(mesh->getCoord().size());
    std::cout << "  Applying reordering...\n";

#pragma omp parallel for
    for (unsigned int i = 0; i < mesh->get_n_nodes(); i++)
    {
            newCoord[3 * i] = coordAux[3 * perm[i]];
            newCoord[(3 * i) + 1] = coordAux[(3 * perm[i]) + 1];
            newCoord[(3 * i) + 2] = coordAux[(3 * perm[i]) + 2];
    }
    
    mesh->getCoord().swap(newCoord);
    newCoord.clear();

    std::vector<unsigned int> newConn;
    std::vector<unsigned int> &connAux = mesh->getConn();
    unsigned int connSize = mesh->getConn().size();
    newConn.resize(connSize);

#pragma omp parallel for
    for (unsigned int i = 0; i < connSize; i++)
    {
        newConn[i] = iperm[connAux[i]];
    }
    mesh->getConn().swap(newConn);
    newConn.clear();
}

void MeshToRCMGraph(Mesh *mesh, idx_t **xadj, idx_t **adjncy)
{
    MeshToGraph(mesh, xadj, adjncy);

    idx_t *xadjA = *xadj;
    idx_t *adjncyA = *adjncy;

    unsigned int nnodes = mesh->get_n_nodes();

#ifdef DEBUG
    WriteAIJ("antes_rcm.txt", nnodes, xadjA, adjncyA, 0);
#endif

#pragma omp parallel for
    for (unsigned int n = 0; n < nnodes; n++)
    {
        unsigned int start = xadjA[n];
        unsigned int end   = xadjA[n + 1];

        std::sort(&adjncyA[start], &adjncyA[end]);        
    }

    int n_adjncyA = xadjA[nnodes];

#pragma omp parallel for
    for (unsigned int i = 0; i <= nnodes; i++)
        xadjA[i] += 1;

#pragma omp parallel for
    for (int i = 0; i < n_adjncyA; i++)
        adjncyA[i] += 1;
}

void MeshReorderingRCM(Mesh *mesh, idx_t *xadj, idx_t *adjncy, int *perm, int *iperm)
{
    std::cout << "  Applyng RCM reordering\n";
    unsigned int nnodes = mesh->get_n_nodes();
#ifdef DEBUG
    WriteAIJ("adj_rcm.txt", nnodes, xadj, adjncy, 1);
#endif

    genrcm(nnodes, xadj[nnodes], xadj, adjncy, perm);
    // função responsável por retornar o iperm a partir do numero de elementos permutados e do perm
    perm_inverse3(nnodes, perm, iperm);

#pragma omp parallel for
    for (unsigned int i = 0; i < nnodes; i++)
    {
        perm[i]--;
        iperm[i]--;
    }
}

void MeshReorderingMETIS(Mesh *mesh, idx_t *xadj, idx_t *adjncy, int *perm, int *iperm)
{
    int nnodes = (int)mesh->get_n_nodes();

    idx_t *nn = &nnodes;
    idx_t *vwgt = 0;
    idx_t options[METIS_NOPTIONS];

    std::cout << "  Applyng METIS_NodeND reordering\n";
    METIS_SetDefaultOptions(options);

    options[METIS_OPTION_NUMBERING] = 0;
    int result = METIS_NodeND(nn, xadj, adjncy, vwgt, options, perm, iperm);

    if (result == METIS_OK)
    {
        std::cout << "METIS reordering succesfully applied\n";
    }
    else
    {
        if (result == METIS_ERROR_INPUT)
        {
            std::cout << "Input error\n";
            exit(1);
        }
        else
        {
            if (result == METIS_ERROR_MEMORY)
            {
                std::cout << "Memory error\n";
                exit(1);
            }
            else
            {
                std::cout << "Another kind of error\n";
                exit(1);
            }
        }
    }
}

void MeshReorderingFirstTouch(Mesh *mesh, int *perm, int *iperm)
{
    std::vector<unsigned int> &connAux = mesh->getConn();
#pragma omp parallel for
    for (unsigned int i = 0; i < mesh->get_n_nodes(); i++)
        perm[i] = -1;

    unsigned int counter = 0;


    for (unsigned int i = 0; i < mesh->get_n_elements(); i++)
    {
        unsigned int iel = mesh->get_n_face_elements() + i;

        for (unsigned int eno = mesh->getOffset()[iel]; eno < mesh->getOffset()[iel + 1]; eno++)
        {
            if (perm[connAux[eno]] == -1)
            {
                perm[connAux[eno]] = counter;
                iperm[counter] = connAux[eno];
                counter++;
            }
        }
    }
}

void Mesh::MeshReordering(reorder_t reorder = RCM)
{
    idx_t *xadj;
    idx_t *adjncy;


    //std::unique_ptr<int[]> perm_ptr = std::make_unique<int[]>(this->n_nodes);
    //std::unique_ptr<int[]> iperm_ptr = std::make_unique<int[]>(this->n_nodes);

    //int *perm = perm_ptr.get();
    //int *iperm = iperm_ptr.get();

    int *perm  =  new int[this->n_nodes];
    int *iperm =  new int[this->n_nodes];

    std::cout << "Starting mesh reordering...\n";
    switch (reorder)
    {
    case FF:
        MeshReorderingFirstTouch(this, perm, iperm);
        ApplyReorderMesh(this, perm, iperm);
        break;
    case METIS_ND:
        MeshToGraph(this, &xadj, &adjncy);
        MeshReorderingMETIS(this, xadj, adjncy, perm, iperm);
        ApplyReorderMesh(this, perm, iperm);
        break;
    default:
        MeshToRCMGraph(this, &xadj, &adjncy);
        MeshReorderingRCM(this, xadj, adjncy, perm, iperm);
        ApplyReorderMesh(this, perm, iperm);
        break;
    }
    std::cout << "Finished mesh reordering...\n";

    METIS_Free(xadj);
    METIS_Free(adjncy);

    delete [] perm;
    delete [] iperm;
}


