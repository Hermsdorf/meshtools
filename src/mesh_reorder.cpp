
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


void WriteAdj(const char * fname, int nvts,  idx_t* xadj, idx_t* adjncy)
{

    ofstream fout;
    fout.open(fname);
    if(fout.is_open())
    {

        fout << nvts << endl;
        for(int i = 0; i <= nvts; i++)
            fout << xadj[i] << endl;
        
        for(int i = 0; i < xadj[nvts]; i++)
            fout << adjncy[i] << endl;

        fout.close();
    }

}

void WriteAIJ(const char * fname, int nvts,  idx_t* xadj, idx_t* adjncy, int one_flag)
{

    ofstream fout;
    fout.open(fname);
    if(fout.is_open())
    {
        for(int i = 0; i < nvts; i++)
        {  
            fout << "Adj. node " << i << ": ";
            for(int j = xadj[i]; j <= xadj[i+1]-1; j++) {
                int col = adjncy[j - one_flag] - one_flag;
                fout << col << " ";
            }
            fout << endl;
        }
        fout.close();
    }

}

void MeshToGraph(mesh_t* mesh, idx_t** xadj, idx_t** adjncy)
{
    int result;

    idx_t *ne = &mesh->n_elements;
    idx_t *nn = &mesh->n_nodes;
    idx_t numflag = 0;

    int ofs           = mesh->offset[mesh->n_face_elements];
    int *eptr         = new int [mesh->n_elements+1];
    for(int i = mesh->n_face_elements, j = 0; i < mesh->offset.size() ; i++, j++)
    {
        eptr[j] = mesh->offset[i] - ofs;
    }

    idx_t *eind     = &mesh->conn[ofs];
 

    result = METIS_MeshToNodal(ne, nn, eptr, eind, &numflag, xadj, adjncy);
    
    if(result == METIS_OK)
    {
        cout << "Mesh to graph succesfully applied" << endl;
    }
    else
    {
        if(result == METIS_ERROR_INPUT)
        {
            cout << "Input error" << endl;
            exit(1);
        }
        else
        {
            if(result == METIS_ERROR_MEMORY)
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

    delete [] eptr;
}

void ReorderMesh(mesh_t* mesh, int* perm, int *iperm)
{
    vector<double> newCoord;
    newCoord.resize(mesh->coord.size());
    for(int i = 0 ; i < mesh->n_nodes ; i++)
    {
        for(int j = 0 ; j < 3 ; j++)
            newCoord[(3*i)+j] = mesh->coord[(3*perm[i])+j];
    }
    mesh->coord.swap(newCoord);
    newCoord.clear();

    vector<int> newConn;
    newConn.resize(mesh->conn.size());
    for(int i = 0 ; i < mesh->conn.size() ; i++)
    {
        newConn[i] = iperm[mesh->conn[i]];
    }
    mesh->conn.swap(newConn);
    newConn.clear();
}

int
compare_idx (const void *a, const void *b)
{
  const idx_t *da = (const idx_t *) a;
  const idx_t *db = (const idx_t *) b;

  return (*da > *db) ;
}

void MeshToRCMGraph(mesh_t* mesh, idx_t** xadj, idx_t** adjncy)
{

    MeshToGraph(mesh, xadj, adjncy);

    idx_t* xadjA   = *xadj;
    idx_t* adjncyA = *adjncy;

#ifdef DEBUG
    WriteAIJ("antes_rcm.txt",mesh->n_nodes,xadjA,adjncyA,1);
#endif

    for(int n = 0; n < mesh->n_nodes; n++)
    {
        int start = xadjA[n];
        int end   = xadjA[n+1];
        qsort(&adjncyA[start],(end-start),sizeof(idx_t), compare_idx);
    }

    int n_adjncyA = xadjA[mesh->n_nodes];

    for(int i = 0; i <= mesh->n_nodes; i++)
        xadjA[i] += 1;

    for(int i = 0; i < n_adjncyA; i++)
         adjncyA[i] += 1;
    
#ifdef DEBUG
    WriteAIJ("apos_rcm.txt",mesh->n_nodes,xadjA,adjncyA,1);
#endif


/*
    // Cria a estrutura auxiliar para o rcm
    int adj_max        = xadjA[mesh->n_nodes];
    int rcm_adj_size = 0;
    idx_t *rcm_adj_row =  new idx_t[mesh->n_nodes];
    idx_t *rcm_adj     =  new idx_t[adj_max];
    int jstart, jend;

    adj_set(mesh->n_nodes, adj_max, &rcm_adj_size, rcm_adj_row, rcm_adj, -1, -1); // inicialização dos vetores rcm_adj_row e rcm_adj

    // Gera a matriz de adjacencias usando a rotina adj_set.
    for(int n = 0; n < mesh->n_nodes; n++)
    {
        int irow = n;
        jstart = xadjA[n];
        jend   = xadjA[n+1];
        for(int j = jstart; j < jend; j++)
        {
            int jcol = adjncyA[j];

            //if(irow == jcol) continue;
            adj_set(mesh->n_nodes, adj_max, &rcm_adj_size, rcm_adj_row, rcm_adj, irow+1, jcol+1);
        }
    }

    *xadj = rcm_adj_row;
    *adjncy = rcm_adj;
    */

}


void MeshReorderingRCM(mesh_t *mesh, idx_t* xadj, idx_t* adjncy, int* perm, int* iperm)
{
    cout << "Applyng RCM reordering " << endl;

#ifdef DEBUG
    WriteAIJ("adj_rcm.txt",mesh->n_nodes,xadj,adjncy,1);
#endif

    genrcm(mesh->n_nodes, xadj[mesh->n_nodes], xadj, adjncy, perm);

    // função responsável por retornar o iperm a partir do numero de elementos permutados e do perm
    perm_inverse3(mesh->n_nodes, perm, iperm); 

    for(int i = 0 ; i < mesh->n_nodes ; i++){
        perm[i]--;
        iperm[i]--;
    }


    cout << "Node reordering succesfully applied" << endl;
}

void MeshReorderingMETIS(mesh_t* mesh, idx_t *xadj, idx_t *adjncy, int* perm, int* iperm) 
{
    int result;
    idx_t *nn = &mesh->n_nodes;
    idx_t *vwgt = 0;
    idx_t options[METIS_NOPTIONS]; 

    cout << "Applyng METIS ND reordering " << endl;
    METIS_SetDefaultOptions(options);

    options[METIS_OPTION_NUMBERING] = 0;

    result = METIS_NodeND(nn, xadj, adjncy, vwgt, options, perm, iperm);
 
 #ifdef DEBUG
    WriteAIJ("adj_metis.txt",mesh->n_nodes,xadj,adjncy,0);
#endif


    if(result == METIS_OK)
    {
        cout << "Node reordering succesfully applied" << endl;
    }
    else
    {
        if(result == METIS_ERROR_INPUT)
        {
            cout << "Input error" << endl;
            exit(1);
        }
        else
        {
            if(result == METIS_ERROR_MEMORY)
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



void MeshReorderingFirstTouch(mesh_t* mesh)
{
    std::vector<int> numbering;
    std::vector<int> mapping;
    
    numbering.resize(mesh->n_nodes);
    mapping.resize(mesh->n_nodes);

    for(int i = 0; i < mesh->n_nodes; i++)
        numbering[i] = -1;

    unsigned int counter = 0;
    
    for(int i = 0; i < mesh->n_elements; i++)
    {
        int iel = mesh->n_face_elements + i;

        for(int eno = mesh->offset[iel]; eno < mesh->offset[iel+1]; eno++)
        {
            if(numbering[mesh->conn[eno]] == -1)
            {
                numbering[mesh->conn[eno]] = counter;
                mapping[counter] = mesh->conn[eno];
                counter++; 
            }
        }
    }

    vector<double> newCoord;
    newCoord.resize(mesh->coord.size());
    for(int i = 0 ; i < mesh->n_nodes ; i++)
    {
        for(int j = 0 ; j < 3 ; j++)
            newCoord[(3*i)+j] = mesh->coord[(3*numbering[i])+j];
    }
    mesh->coord.swap(newCoord);
    newCoord.clear();

    vector<int> newConn;
    newConn.resize(mesh->conn.size());
    for(int i = 0 ; i < mesh->conn.size() ; i++)
    {
        newConn[i] = mapping[mesh->conn[i]];
    }
    mesh->conn.swap(newConn);
    newConn.clear();
}


void MeshReordering(mesh_t* mesh, reorder_t reorder)
{
    if(reorder == FF)
    {
        MeshReorderingFirstTouch(mesh);
    }
    else
    {
        idx_t *xadj;
        idx_t *adjncy; 

        std::unique_ptr<int[]> perm_ptr  = make_unique<int[]>(mesh->n_nodes); 
        std::unique_ptr<int[]> iperm_ptr = make_unique<int[]>(mesh->n_nodes); 

        int *perm  = perm_ptr.get();
        int *iperm = iperm_ptr.get();
        
        //idx_t *perm  = new idx_t[mesh->n_nodes]; 
        //idx_t *iperm = new idx_t[mesh->n_nodes];

        switch(reorder)
        {
            case METIS_ND:
                MeshToGraph(mesh, &xadj, &adjncy);
                MeshReorderingMETIS(mesh, xadj, adjncy, perm, iperm);
                ReorderMesh(mesh, perm, iperm);
                break;
            default:
                MeshToRCMGraph(mesh, &xadj, &adjncy);
                MeshReorderingRCM(mesh, xadj, adjncy, perm, iperm);
                ReorderMesh(mesh, perm, iperm);
                break;
        }

        METIS_Free(xadj);
        METIS_Free(adjncy);

    }
}