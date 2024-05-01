
#include "metis.h"
#include "rcm.hpp"
#include "mesh_reordering.h"


void mesh_to_graph(std::unique_ptr<Mesh>& mesh, idx_t **xadj, idx_t **adjncy)
{
    unsigned int nelem  =  mesh->get_n_elements();
    unsigned int nnodes =  mesh->get_n_nodes();
    unsigned int nfe    =  mesh->get_n_surface_elements();

    idx_t *ne = (idx_t*) &nelem;
    idx_t *nn = (idx_t*) &nnodes;
    idx_t numflag = 0;

    unsigned int final_surface_offset = mesh->get_offset_vector()[nfe];
    idx_t *eptr = new idx_t[nelem + 1];

    for (int i = 0, j = 0; i  <= nelem ; i++, j++)
    {
        eptr[j] = mesh->get_offset_vector()[nfe+i] - final_surface_offset;
    }

    idx_t *eind = reinterpret_cast<idx_t*> (&mesh->get_connectivity_vector()[final_surface_offset]);

    int result = METIS_MeshToNodal(ne, nn, eptr, eind, &numflag, xadj, adjncy);
    if (result != METIS_OK)
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

void rcm_pre_process_graph(size_t size, idx_t *xadj, idx_t *adjncy)
{
    idx_t *xadjA   = xadj;
    idx_t *adjncyA = adjncy;

#pragma omp parallel for
    for (unsigned int n = 0; n < size; n++)
    {
        unsigned int start = xadjA[n];
        unsigned int end   = xadjA[n + 1];
        std::sort(&adjncyA[start], &adjncyA[end]);        
    }

    int n_adjncyA = xadjA[size];

#pragma omp parallel for
    for (unsigned int i = 0; i <= size; i++)
        xadjA[i] += 1;

#pragma omp parallel for
    for (int i = 0; i < n_adjncyA; i++)
        adjncyA[i] += 1;
}

void rcm_apply(size_t n, idx_t *xadj, idx_t *adjncy, int *perm, int *iperm)
{
    genrcm(n, xadj[n], xadj, adjncy, perm);
    // função responsável por retornar o iperm a partir do numero de elementos permutados e do perm
    perm_inverse3(n, perm, iperm);

#pragma omp parallel for
    for (unsigned int i = 0; i < n; i++)
    {
        perm[i]--;
        iperm[i]--;
    }
}

void metis_nd_apply(size_t n, idx_t *xadj, idx_t *adjncy, int *perm, int *iperm)
{
    idx_t *nn = (idx_t*) &n;
    idx_t *vwgt = 0;
    idx_t options[METIS_NOPTIONS];

    METIS_SetDefaultOptions(options);

    options[METIS_OPTION_NUMBERING] = 0;
    int result = METIS_NodeND(nn, xadj, adjncy, vwgt, options, perm, iperm);

    if (result != METIS_OK)
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

void MeshReordering::apply_reordering(std::unique_ptr<Mesh>& mesh, int *perm, int* iperm)
{
    std::vector<double>         new_coord(mesh->get_coordinate_vector().size());
    std::vector<double>&        orig_coord =  mesh->get_coordinate_vector() ;       
    std::vector<unsigned int>&  node_index =   mesh->get_node_index_vector();

#pragma ivdep
    for (unsigned int i = 0; i < mesh->get_n_nodes(); i++)
    {
            node_index[i]         = iperm[i];
            new_coord[3 * i      ] = orig_coord[3 * perm[i]];
            new_coord[(3 * i) + 1] = orig_coord[(3 * perm[i]) + 1];
            new_coord[(3 * i) + 2] = orig_coord[(3 * perm[i]) + 2];
    }
    
    mesh->set_coordinate_vector(new_coord);
    new_coord.clear();

    
    std::vector<unsigned int> &connAux = mesh->get_connectivity_vector();
    unsigned int connSize = connAux.size();
    std::vector<unsigned int> newConn(connSize);

#pragma ivdep
    for (unsigned int i = 0; i < connSize; i++)
    {
        newConn[i] = iperm[connAux[i]];
    }
    mesh->set_connectivity_vector(newConn);
    newConn.clear();
}


void MeshReordering::apply_rcm_ordering(std::unique_ptr<Mesh>& mesh)
{
    idx_t *xadj   = nullptr;
    idx_t *adjncy = nullptr;

    int n_nodes = mesh->get_n_nodes();

    int *perm  =  new int[n_nodes];
    int *iperm =  new int[n_nodes];

    mesh_to_graph(mesh, &xadj, &adjncy);
    rcm_pre_process_graph(n_nodes,xadj,adjncy);
    rcm_apply(n_nodes,xadj,adjncy,perm,iperm);

    apply_reordering(mesh,perm,iperm);

    delete [] perm;
    delete [] iperm;

    if(xadj)   METIS_Free(xadj);
    if(adjncy) METIS_Free(adjncy);

}


void MeshReordering::apply_nd_ordering(std::unique_ptr<Mesh>& mesh)
{
    idx_t *xadj   = nullptr;
    idx_t *adjncy = nullptr;

    int n_nodes = mesh->get_n_nodes();

    int *perm  =  new int[n_nodes];
    int *iperm =  new int[n_nodes];

    mesh_to_graph(mesh, &xadj, &adjncy);
    metis_nd_apply(n_nodes,xadj,adjncy,perm,iperm);
    apply_reordering(mesh,perm,iperm);

    delete [] perm;
    delete [] iperm;

    if(xadj)   METIS_Free(xadj);
    if(adjncy) METIS_Free(adjncy);

}

void MeshReordering::reordering(std::unique_ptr<Mesh>& mesh, ReorderingMode mode)
{
    switch(mode)
    {
        case ReorderingMode::RCM:
            apply_rcm_ordering(mesh);
            break;
        case ReorderingMode::ND:
            apply_nd_ordering(mesh);
            break;
        default:
            break;
    }
}




