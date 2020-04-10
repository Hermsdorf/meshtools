
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <algorithm>

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

void WriteAIJ(const char * fname, int nvts,  idx_t* xadj, idx_t* adjncy)
{

    ofstream fout;
    fout.open(fname);
    if(fout.is_open())
    {
        for(int i = 0; i < nvts; i++)
        {  
            fout << i << " "  << i << endl;
            for(int j = xadj[i]; j < xadj[i+1]; j++)
                fout << i << " " << adjncy[j] << endl;
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

void MeshToRCMGraph(mesh_t* mesh, idx_t** xadj, idx_t** adjncy)
{
    MeshToGraph(mesh,xadj,adjncy);
    // Gera a matriz de adjacencias usando a rotina adj_set.
    for(int n = 0; n < mesh->n_nodes; n++)
    {
        int irow = n;
        int jstart = (*xadj)[n];
        int jend   = (*xadj)[n+1];
        for(int j = jstart; j < jend; j++)
        {
            (*adjncy)[j] += 1;
        }
    }

}


void MeshReorderingRCM(mesh_t *mesh, idx_t* xadj,idx_t* adjncy, int * perm, int* iperm)
{


  cout << "Applyng RCM reordering " << endl;
/*
    int adjbandAntes, adjbandDepois;

    MeshToGraph(mesh, &xadj, &adjncy);
    //WriteAIJ("torusAntes.txt", mesh->n_nodes, xadj, adjncy);
    //adjbandAntes = adj_bandwidth(mesh->n_nodes, xadj[mesh->n_nodes], xadj, adjncy);
    //cout << "Bandwith antes: " << adjbandAntes;

    // Cria a estrutura auxiliar para o rcm
    int adj_max        = xadj[mesh->n_nodes];
    int rcm_adj_size = 0;
    idx_t *rcm_adj_row =  new idx_t[mesh->n_nodes];
    idx_t *rcm_adj     =  new idx_t[adj_max];
    int jstart, jend;

    adj_set(mesh->n_nodes, adj_max, &rcm_adj_size, rcm_adj_row, rcm_adj, -1, -1); // inicialização dos vetores rcm_adj_row e rcm_adj

    // Gera a matriz de adjacencias usando a rotina adj_set.
    for(int n = 0; n < mesh->n_nodes; n++)
    {
        int irow = n;
        jstart = xadj[n];
        jend   = xadj[n+1];
        for(int j = jstart; j < jend; j++)
        {
            int jcol =  adjncy[j];

            if(irow == jcol) continue;
            adj_set(mesh->n_nodes, adj_max, &rcm_adj_size, rcm_adj_row, rcm_adj, irow+1, jcol+1);
        }
    }

    
*/


    genrcm(mesh->n_nodes, xadj[mesh->n_nodes], xadj,adjncy, perm);
    // função responsável por retornar o iperm a partir do numero de elementos permutados e do perm
    perm_inverse3(mesh->n_nodes, perm, iperm); 



    //adjbandDepois = adj_perm_bandwidth(mesh->n_nodes, xadj[mesh->n_nodes], xadj, adjncy, perm, iperm);
    //cout << "Bandwith depois: " << adjbandDepois;

  
    //MeshToGraph(mesh, &xadj, &adjncy);
    //WriteAIJ("torusDepois.txt", mesh->n_nodes, xadj, adjncy);

    cout << "Node reordering succesfully applied" << endl;


}

void MeshReorderingMETIS(mesh_t* mesh, idx_t *xadj, idx_t *adjncy, idx_t* perm, idx_t* iperm) 
{
    int result;
    idx_t *nn = &mesh->n_nodes;
    idx_t *vwgt = 0;
    idx_t options[METIS_NOPTIONS]; 



    cout << "Applyng METIS ND reordering " << endl;
    METIS_SetDefaultOptions(options);

    options[METIS_OPTION_NUMBERING] = 0;

    result = METIS_NodeND(nn, xadj, adjncy, vwgt, options, perm, iperm);
 
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
    idx_t* xadj;
    idx_t* adjncy;
    std::vector<int> numbering;
    std::vector<int> mapping;
    
    MeshToGraph(mesh, &xadj, &adjncy);
    WriteAIJ("aijAntes.txt",mesh->n_nodes, xadj,adjncy);

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

    
    MeshToGraph(mesh, &xadj, &adjncy);


    //WriteAIJ("aijDepois.txt",mesh->n_nodes, xadj,adjncy);


}


void MeshReordering(mesh_t* mesh, reorder_t reorder)
{

    idx_t *xadj  ;
    idx_t *adjncy; 

    idx_t *perm  = new idx_t[mesh->n_nodes]; 
    idx_t *iperm = new idx_t[mesh->n_nodes];

    switch(reorder)
    {
        case METIS_ND:
            MeshToGraph(mesh, &xadj, &adjncy);
            //int adjbandDepois = adj_perm_bandwidth(mesh->n_nodes, xadj[mesh->n_nodes], xadj, adjncy, perm, iperm);
            //cout << "Bandwith depois METIS: " << adjbandDepois << endl;
            MeshReorderingMETIS(mesh,xadj,adjncy,perm,iperm);
            ReorderMesh(mesh,perm,iperm);
            break;
        case RCM:
            MeshToRCMGraph(mesh, &xadj, &adjncy);
            MeshReorderingRCM(mesh,xadj,adjncy,perm,iperm);
            ReorderMesh(mesh,perm,iperm);
            break;
        default:
            MeshReorderingFirstTouch(mesh);
            break;

    }

    if(xadj)   delete [] xadj;
    if(adjncy) delete [] adjncy;
    //if(perm)   delete [] perm;
    //if(iperm)  delete [] iperm;

    
}