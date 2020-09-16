#include <iostream>
#include <algorithm>

#include "metis.h"
#include "../include/mesh.h"

void MeshToDualGraph(Mesh *mesh, idx_t **xadj, idx_t **adjncy)
{
    int nelem = mesh->get_n_elements();
    int nnodes = mesh->get_n_nodes();
    idx_t* eptr = new idx_t[mesh->get_n_elements() + 1];
    idx_t* eind = (idx_t*)mesh->getElementConn(0);
    idx_t* ne = &nelem;;
    idx_t* nn = &nnodes;
    idx_t numflag = 0;
    idx_t ncommon = 1;

    unsigned int ofs = mesh->getElementOffset(0)[0];

    int* offset_aux = (int*)mesh->getElementOffset(0);
    for (int i = 0, j = 0; i <= nelem ; i++, j++)
    {
        eptr[j] = offset_aux[i] - ofs;
    }

    int result = METIS_MeshToDual(ne, nn, eptr, eind, &ncommon, &numflag, xadj, adjncy);

    if (result == METIS_OK)
    {
        std::cout << "Mesh to Dual Graph sucessfully applied\n";
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

// TODO: implementar a alteração do vetor de tipos também
void UpdateMeshArrays(Mesh* mesh, unsigned int** new_conn, unsigned int** new_offset)
{
    unsigned int ne = mesh->get_n_elements();
    unsigned int nfe = mesh->get_n_face_elements();
    unsigned int n_colors = mesh->get_n_internal_colors();
    int* mesh_coloring = mesh->get_mesh_coloring_internal();

    unsigned int* start_elem_offset = mesh->getElementOffset(0);
    unsigned int* end_elem_offset = mesh->getElementOffset(ne);

    for(unsigned int i = nfe, j = 0 ; i <= nfe + ne ; i++, j++)
    {
        unsigned int value = (*new_offset)[j];
        mesh->setOffsetPosition(value, i);
    }
    for(unsigned int i = start_elem_offset[0], j = 0 ; i < end_elem_offset[0] ; i++, j++)
    {
        unsigned int value = (*new_conn)[j];
        mesh->setConnPosition(value, i);
    }

    int* mesh_coloringAux = new int [n_colors];
    
    for(unsigned int i = 0 ; i < n_colors ; i++)
        mesh_coloringAux[i] = 0;

    for(unsigned int i = 0 ; i < ne ; i++)
    {
        mesh_coloringAux[mesh_coloring[i]-1]++;
    }

    delete [] mesh->get_mesh_coloring_internal();
    mesh->set_mesh_coloring_internal(mesh_coloringAux);
}


void ReorderElements(Mesh* mesh, unsigned int* sort, unsigned int** new_conn, unsigned int** new_offset)
{
    unsigned int ne = mesh->get_n_elements();
    unsigned int* elem_offset = mesh->getElementOffset(0);
    std::vector<unsigned int> &connAux = mesh->getConn();

    *new_conn = new unsigned int [mesh->getOffset().back() - elem_offset[0]];

    *new_offset = new unsigned int [ne + 1];
    unsigned int count_conn = 0;
    unsigned int count_offset = 1;

    (*new_offset)[0] = elem_offset[0];

    for(unsigned int i = 0 ; i < ne ; i++)
    {
        unsigned int start = elem_offset[sort[i]];
        unsigned int end = elem_offset[sort[i] + 1];

        (*new_offset)[count_offset] = (*new_offset)[count_offset - 1] + (end-start);
        count_offset++;

        for(unsigned int j = start ; j < end ; j++)
        {
            (*new_conn)[count_conn] = connAux[j];
            count_conn++;
        }
    }
}

void CreateSort(Mesh* mesh, unsigned int* sort)
{
    unsigned int ne = mesh->get_n_elements();
    unsigned int aux_n_colors = mesh->get_n_internal_colors();
    int* mesh_coloring = mesh->get_mesh_coloring_internal();

    unsigned int count = 0;

    for(unsigned int i = 1 ; i <= aux_n_colors ; i++)
    {
        for(unsigned int j = 0 ; j < ne ; j++)
        {
            if(mesh_coloring[j] == i)
            {
                sort[count] = j;
                count++;
            }
        }
    }
}

struct greater
{
    template<class T>
    bool operator()(T const &a, T const &b) const { return a > b; }
};

void ReorderGraph(Mesh* mesh, idx_t** xadj, idx_t** adjncy, int** unsortpos)
{
    std::cout << "Reordering graph...\n";

    unsigned int ne = mesh->get_n_elements();
    unsigned int* n_adjsort = new unsigned int [ne];
    unsigned int* n_adj = new unsigned int [ne];
    idx_t* p_xadj = *xadj;
    idx_t* p_adjncy = *adjncy;
    int* p_unsortpos = *unsortpos;

    for(unsigned int i = 0 ; i < ne ; i++)
    {
        unsigned int dif = p_xadj[i+1] - p_xadj[i];
        n_adj[i] = n_adjsort[i] = dif;
    }

    std::sort(&n_adjsort[0], &n_adjsort[ne], greater());

    idx_t* xadj_aux = new idx_t [ne+1];
    idx_t* adjncy_aux = new idx_t [p_xadj[ne]];

    xadj_aux[0] = 0;
    unsigned int c_xadj = 1;
    unsigned int c_adjncy = 0;
    unsigned int c_unsortpos = 0;
    for(unsigned int i = 0 ; i < ne ; i++)
    {
        unsigned int value = n_adjsort[i];
        unsigned int pos;
        for(int k = 0 ; k < ne ; k++)
        {
            if(n_adj[k] == value)
            {
                pos = k;
                p_unsortpos[c_unsortpos] = pos;
                c_unsortpos++;
                break;
            }
        }
        
        xadj_aux[c_xadj] = xadj_aux[c_xadj - 1] + value;
        c_xadj++;

        unsigned int start = p_xadj[pos];
        unsigned int end = p_xadj[pos+1];

        for(unsigned int j = start ; j < end ; j++)
        {
            adjncy_aux[c_adjncy] = p_adjncy[j];
            c_adjncy++;
        }

        n_adj[pos] = 0; // elemento ja inserido no vetor
    }

    xadj_aux[ne] = p_xadj[ne];

    for(int i = 0 ; i < c_xadj ; i++)
        (*xadj)[i] = xadj_aux[i];

    for(int i = 0 ; i < c_adjncy ; i++)
        (*adjncy)[i] = adjncy_aux[i];

    delete [] n_adj;
    delete [] n_adjsort;
    delete [] xadj_aux;
    delete [] adjncy_aux;

    std::cout << "Graph reordered succesfully!\n";
}

unsigned int ColoringReordLimit(Mesh* mesh)
{
    idx_t* xadj;
    idx_t* adjncy;
    unsigned int ne = mesh->get_n_elements();
    int* elements_color = new int[ne];
    int* unsortpos = new int[ne];
    unsigned int n_colors = 1;                // Número total de cores da malha.
    unsigned int max_n_elements = 20000;       // Número máximo de elementos por cor.
    std::vector<unsigned int> n_elements_color(9999999, 0); // Vector responsável por contar quantos elementos tem por cor.

    MeshToDualGraph(mesh, &xadj, &adjncy);

    ReorderGraph(mesh, &xadj, &adjncy, &unsortpos);        

    std::fill(&elements_color[0], &elements_color[ne], -1);  // Flag para elemento sem cor.

    for(unsigned int i = 0 ; i < ne ; i++)
    {
        unsigned int start = xadj[i];
        unsigned int end = xadj[i+1];

        unsigned int color = 1;  // Possível cor para o elemento i.
        unsigned int j = start;

        while(j < end)
        {
            unsigned int elem_adj = adjncy[j];

            if(elements_color[elem_adj] == color || n_elements_color[color-1] >= max_n_elements)
            {
                color++;
                j = start;
            }
            else
                j++;

        } // Verificamos as cores dos elementos adjacentes ao elemento i, 
          // ao final color vai ter a coloração correta para o elemento i
          // sendo ela menor cor possível dentre as cores dos elementos adjacentes.

        elements_color[unsortpos[i]] = color;

        if(color > n_colors)
        {
            n_colors = color;
            //n_elements_color.resize(n_colors, 0);
        }
        
        n_elements_color[color-1]++;
    }

    n_elements_color.clear();
    delete [] mesh->get_mesh_coloring_internal(); // delete do new feito na função MeshGmshReader 
                                                  // onde inicializa todo o vetor mesh_coloring_internal com -1.
    mesh->set_mesh_coloring_internal(elements_color);

    METIS_Free(xadj);
    METIS_Free(adjncy);

    return n_colors;
}

unsigned int ColoringReord(Mesh* mesh)
{
    idx_t* xadj;
    idx_t* adjncy;

    unsigned int ne = mesh->get_n_elements();
    int* elements_color = new int[ne];
    unsigned int n_colors = 1;                // Número total de cores da malha.
    int* unsortpos = new int [ne];

    MeshToDualGraph(mesh, &xadj, &adjncy);

    ReorderGraph(mesh, &xadj, &adjncy, &unsortpos);        

    std::fill(&elements_color[0], &elements_color[ne], -1);  // Flag para elemento sem cor.

    for(unsigned int i = 0 ; i < ne ; i++)
    {
        unsigned int start = xadj[i];
        unsigned int end = xadj[i+1];

        unsigned int color = 1;  // Possível cor para o elemento i.
        unsigned int j = start;

        while(j < end)
        {
            unsigned int elem_adj = adjncy[j];

            if(elements_color[elem_adj] == color)
            {
                color++;
                j = start;
            }
            else
                j++;
        } // Verificamos as cores dos elementos adjacentes ao elemento i, 
          // ao final color vai ter a coloração correta para o elemento i
          // sendo ela menor cor possível dentre as cores dos elementos adjacentes.
        


        elements_color[unsortpos[i]] = color; // seguir a coloração com sentido à reordenação do grafo

        if(color > n_colors)
            n_colors = color;
    }

    delete [] unsortpos;
    delete [] mesh->get_mesh_coloring_internal(); // delete do new feito na função MeshGmshReader 
                                                  // onde inicializa todo o vetor mesh_coloring_internal com -1.
    mesh->set_mesh_coloring_internal(elements_color);

    METIS_Free(xadj);
    METIS_Free(adjncy);

    return n_colors;
}

unsigned int Coloring(Mesh* mesh)
{
    idx_t* xadj;
    idx_t* adjncy;

    unsigned int ne = mesh->get_n_elements();
    int* elements_color = new int[ne];
    unsigned int n_colors = 1;                // Número total de cores da malha.

    MeshToDualGraph(mesh, &xadj, &adjncy);      

    std::fill(&elements_color[0], &elements_color[ne], -1);  // Flag para elemento sem cor.

    for(unsigned int i = 0 ; i < ne ; i++)
    {
        unsigned int start = xadj[i];
        unsigned int end = xadj[i+1];

        unsigned int color = 1;  // Possível cor para o elemento i.
        unsigned int j = start;

        while(j < end)
        {
            unsigned int elem_adj = adjncy[j];

            if(elements_color[elem_adj] == color)
            {
                color++;
                j = start;
            }
            else
                j++;
        } // Verificamos as cores dos elementos adjacentes ao elemento i, 
          // ao final color vai ter a coloração correta para o elemento i
          // sendo ela menor cor possível dentre as cores dos elementos adjacentes.

        elements_color[i] = color; // seguir a coloração com sentido à reordenação do grafo

        if(color > n_colors)
            n_colors = color;
    }

    delete [] mesh->get_mesh_coloring_internal(); // delete do new feito na função MeshGmshReader 
                                                  // onde inicializa todo o vetor mesh_coloring_internal com -1.
    mesh->set_mesh_coloring_internal(elements_color);

    METIS_Free(xadj);
    METIS_Free(adjncy);

    return n_colors;
}

void Mesh::MeshColoring()
{
    std::cout << "Starting mesh coloring...\n";
    
    unsigned int* sort_internal = new unsigned int [n_elements];
    unsigned int* new_conn;
    unsigned int* new_offset;

    n_internal_colors = ColoringReordLimit(this);
    CreateSort(this, sort_internal);
    ReorderElements(this, sort_internal, &new_conn, &new_offset);
    UpdateMeshArrays(this, &new_conn, &new_offset);

    for(int i = 0 ; i < n_internal_colors ; i++)
        std::cout << mesh_coloring_internal[i] << " ";

    std::cout << "\nFinished mesh coloring...\n";

    delete [] sort_internal;
    delete [] new_conn;
    delete [] new_offset; 
}