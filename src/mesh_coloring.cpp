#include <iostream>
#include <algorithm>
#include <vector>
#include <map>
#include <cstring>

#include "metis.h"
#include "omp.h"
#include "mesh.h"

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

void CheckColoring(idx_t** xadj, idx_t** adjncy, int** elements_color, int ne, int* error=NULL)
{
    idx_t* xadj_aux = *xadj;
    idx_t* adjncy_aux = *adjncy;
    int* color = *elements_color;

    for(unsigned int i = 0 ; i < ne ; i++)
    {
        unsigned int start = xadj_aux[i];
        unsigned int end = xadj_aux[i+1];

        for(unsigned int j = start ; j < end ; j++)
        {
            if(color[i] == color[adjncy_aux[j]])
            {
                std::cout << "ERROR: ADJACENT ELEMENTS WITH THE SAME COLOR\n";
                *error = 1;
                break;
            }
        }
    }
}

void UpdateMeshArrays(Mesh* mesh, std::vector<unsigned int>& elememts_color, std::vector<unsigned int>& new_conn, std::vector<unsigned int>& new_offset, std::vector<unsigned short>& new_type)
{
    unsigned int ne       = mesh->get_n_elements();
    unsigned int nfe      = mesh->get_n_face_elements();
    unsigned int n_colors = mesh->get_n_colors();
    std::vector<unsigned int>& coloring    = mesh->getColoring();

    unsigned int* start_elem_offset = mesh->getElementOffset(0);
    unsigned int* end_elem_offset   = mesh->getElementOffset(ne);
    int loopsize = nfe + ne;

    for(unsigned int i = nfe, j = 0 ; i <= loopsize ; i++, j++)
    {
        unsigned int value_offset = new_offset[j];
        mesh->setOffsetPosition(value_offset, i);
    }

    for(unsigned int i = start_elem_offset[0], j = 0 ; i < end_elem_offset[0] ; i++, j++)
    {
        unsigned int value = new_conn[j];
        mesh->setConnPosition(value, i);
    }

    for(unsigned int i = nfe, j = 0 ; i <= loopsize ; i++, j++)
    {
        unsigned short value_type = new_type[j];
        mesh->setTypePosition(value_type, i);
    }


    coloring.resize(n_colors);
    std::fill(coloring.begin(), coloring.end(), 0);

    for(unsigned int i = 0 ; i < ne ; i++)
    {
        coloring[elememts_color[i]-1]++;
    }

}

void ReorderElements(Mesh* mesh, std::vector<unsigned int>& sort, std::vector<unsigned int>& new_conn, std::vector<unsigned int>& new_offset, std::vector<unsigned short>& new_type)
{
    unsigned int ne           = mesh->get_n_elements();
    unsigned int nfe          = mesh->get_n_face_elements();
    unsigned int* elem_offset = mesh->getElementOffset(0);
    std::vector<unsigned int>   &connAux = mesh->getConn();
    std::vector<unsigned short> &typeAux = mesh->getType();

    new_conn.resize(mesh->getOffset().back() - elem_offset[0]);
    new_offset.resize(ne + 1);
    new_type.resize(ne);

    unsigned int count_conn = 0;
    unsigned int count_type = 0;
    unsigned int count_offset = 1;

    new_offset[0] = elem_offset[0];

    for(unsigned int i = 0 ; i < ne ; i++)
    {
        unsigned int start = elem_offset[sort[i]];
        unsigned int end = elem_offset[sort[i] + 1];

        new_offset[count_offset] = new_offset[count_offset - 1] + (end-start);
        count_offset++;
        
        new_type[count_type]    = typeAux[nfe + sort[i]];
        count_type++;

        for(unsigned int j = start ; j < end ; j++)
        {
            new_conn[count_conn] = connAux[j];
            count_conn++;
        }
    }
}

/*
void CreateSort(Mesh* mesh,std::vector<unsigned int>& sort)
{
    unsigned int ne           = mesh->get_n_elements();
    unsigned int aux_n_colors = mesh->get_n_internal_colors();
    std::vector<unsigned int> = mesh->getColoring();

    unsigned int count = 0;

    for(unsigned int i = 1 ; i <= aux_n_colors ; i++)
    {
        for(unsigned int j = 0 ; j < ne ; j++)
        {
            if(coloring[j] == i)
            {
                sort[count] = j;
                count++;
            }
        }
    }
}
*/

void DecreasingAdj(Mesh* mesh, idx_t** xadj, idx_t** adjncy, int** sequence)
{
    std::cout << "  Calculating number of adjacencies...\n";

    unsigned int ne = mesh->get_n_elements();
    unsigned int* n_adj = new unsigned int [ne];
    idx_t* p_xadj = *xadj;
    idx_t* p_adjncy = *adjncy;
    int* p_sequence = *sequence;

    int greater_diff = 0;
    int small_diff = p_xadj[1] - p_xadj[0];
    for(int i = 0 ; i < ne ; i++)
    {
        n_adj[i] = p_xadj[i+1] - p_xadj[i];

        if(n_adj[i] > greater_diff)
            greater_diff = n_adj[i];

        if(n_adj[i] < small_diff)
            small_diff = n_adj[i];
    }

    int s_count = 0;
    for(int i = greater_diff ; i >= small_diff ; i--)
    {
        for(int j = 0 ; j < ne ; j++)
        {
            if(n_adj[j] == i)
            {
                p_sequence[s_count] = j;
                s_count++;
            }
        }    
    }
    
    delete [] n_adj;

    std::cout << "  Adjacencies calculated succesfully\n";
}

/*
unsigned int ColoringOpenMP_RokosOpt(Mesh* mesh)
{
    idx_t* xadj;
    idx_t* adjncy;

    unsigned int ne       = mesh->get_n_elements();
    int* elements_color   = mesh->get_mesh_coloring_internal();
    unsigned int n_colors = 1;  // Número total de cores da malha.

    std::fill(&elements_color[0], &elements_color[ne], -1);  // Flag para elemento sem cor.

    std::vector<unsigned int> U(ne); // vector com todos os vértices a serem coloridos
    std::vector<unsigned int> L;     // vector com o vértices que tem de ser recoloridos
    int n_threads = 1;

    // Criando grafo dual a partir da malha de elementos finitos
    MeshToDualGraph(mesh, &xadj, &adjncy);   

#pragma omp parallel shared(U, L)
{
#ifdef _OPENMP
    n_threads = omp_get_num_threads();
#endif

    #pragma omp for nowait schedule(dynamic)
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
        }

        elements_color[i] = color; // seguir a coloração com sentido à reordenação do grafo
    }
    
    if(n_threads > 1)
    {
        #pragma omp for schedule(dynamic)
        for(int i = 0 ; i < ne ; i++)
            U[i] = i;
        
        while(!U.empty())
        {
            std::vector<unsigned int>::iterator it;
            #pragma omp for schedule(dynamic)
            for(it = U.begin() ; it != U.end() ; it++)
            {
                unsigned int start = xadj[*it];
                unsigned int end = xadj[(*it) + 1];

                for(int j = start ; j < end ; j++)
                {
                    unsigned int elem_adj_it = adjncy[j];
                    if(elements_color[*it] == elements_color[elem_adj_it] && elem_adj_it > *it)
                    {
                        unsigned int z = start;
                        unsigned int color = 1;
                        while(z < end)
                        {
                            unsigned int elem_adj = adjncy[z];

                            if(color == elements_color[elem_adj])
                            {
                                color++;
                                z = start;
                            }
                            else
                                z++;
                        }

                        elements_color[*it] = color;

                        #pragma omp critical
                        {
                            L.push_back(*it); 
                        }
                    }
                }
            }

            #pragma omp single
            U.clear();
            
            #pragma omp barrier

            #pragma omp single
            {
                U.swap(L);
                L.clear();
            }
        }
    }

    #pragma omp for reduction(max : n_colors)
    for(int i = 0 ; i < ne ; i++)
    {
        if(n_colors < elements_color[i])
                n_colors = elements_color[i];
    }
}

    METIS_Free(xadj);
    METIS_Free(adjncy);

    return n_colors;
} 
*/

unsigned int Coloring(Mesh* mesh, std::vector<unsigned int> &elements_color,std::vector<unsigned int>& sort)
{ 
    int n_nodes                           = mesh->get_n_nodes();
    unsigned int nelem                    = mesh->get_n_elements();
    unsigned int nsurf_elem               = mesh->get_n_face_elements();
    elements_color.resize(nelem);
    std::vector<unsigned short> conn_proibido(n_nodes);

    int nelem_colored       = 0;

    std::memset(elements_color.data(),0,nelem*sizeof(int));

    int color = 1;
    std::memset(conn_proibido.data(),0, n_nodes*sizeof(unsigned short));

    int istart = 0;
    while(nelem_colored < nelem) 
    {
        for(int iel = istart; iel < nelem ; ++iel)
        {
            if(elements_color[iel] == 0)
            {
                unsigned int* conn_elem = mesh->getElementConn(iel);
                unsigned int connsize = mesh->getElementConnSize(iel);

                unsigned int sum = 0;
                for(int i = 0 ; i < connsize ; ++i)
                    sum += conn_proibido[conn_elem[i]];

                if(sum == 0)
                {
                    elements_color[iel] = color;
                    sort[nelem_colored] = iel;
                    nelem_colored++;
                    if(iel == istart)
                        istart++;
                    for(int i = 0 ; i < connsize ; ++i)
                        conn_proibido[conn_elem[i]] = 1; // conectividade proibida
                } // nenhuma conectividade proibida, logo colore o elemento
            } // se o elemento nao estiver colorido, tenta colorir
        }
        
        color++;
        std::memset(conn_proibido.data(),0, n_nodes*sizeof(unsigned short));

    }

    return color-1;
}

/*
unsigned int ColoringAutoral(Mesh* mesh, unsigned int* sort_internal)
{ 

    
    unsigned int nelem      = mesh->get_n_elements();
    int n_nodes             = mesh->get_n_nodes();
    int* elements_color     = mesh->get_mesh_coloring_internal();
    int* conn_proibido      = new int [n_nodes];
    int nelem_colored       = 0;
     int color              = 1;


    std::memset(elements_color,-1, nelem*sizeof(int));
    std::memset(conn_proibido,0, n_nodes*sizeof(int));

    while(nelem_colored < nelem) 
    {
        for(int iel = 0; iel < nelem ; ++iel)
        {
            if(elements_color[iel] == -1)
            {
                unsigned int* conn_elem = mesh->getElementConn(iel);
                unsigned int connsize = mesh->getElementConnSize(iel);

                unsigned int sum = 0;
                for(int i = 0 ; i < connsize ; ++i)
                    sum += conn_proibido[conn_elem[i]];

                if(sum == 0)
                {
                    elements_color[iel] = color;
                    sort_internal[nelem_colored] = iel;
                    nelem_colored++;

                    for(int i = 0 ; i < connsize ; i++)
                        conn_proibido[conn_elem[i]] = 1; // conectividade proibida
                } // nenhuma conectividade proibida, logo colore o elemento
            } // se o elemento nao estiver colorido, tenta colorir
        }
        
        color++;

        std::memset(conn_proibido,0, n_nodes*sizeof(int));
    }

    delete [] conn_proibido;
    return color-1;
}
*/


unsigned int BlockedColoring(Mesh* mesh, std::vector<unsigned int> &elements_color,std::vector<unsigned int>& sort, int block_size) 
{ 
    
    int n_nodes             = mesh->get_n_nodes();
    unsigned int nelem      = mesh->get_n_elements();
    unsigned int nsurf_elem = mesh->get_n_face_elements();
    elements_color.resize(nelem);

    int nelem_colored            = 0;
    int color                    = 1;
    unsigned int nelem_thiscolor = 0;
    unsigned int max_nelem       = block_size; // numero maximo de elementos por cor
    std::vector<unsigned short> conn_proibido(n_nodes);

    std::fill(elements_color.begin(), elements_color.end(), 0);
    std::fill(conn_proibido.begin(), conn_proibido.end(), 0);

    int istart = 0;
    while(nelem_colored < nelem) 
    {
        bool nelem_reachlimit = false;
        for(int iel = istart ; iel < nelem ; ++iel)
        {
            // se o elemento nao estiver colorido, tenta colorir
            if(elements_color[iel] == 0)
            {
                unsigned int* conn_elem = mesh->getElementConn(iel);
                unsigned int connsize   = mesh->getElementConnSize(iel);

                unsigned int sum = 0;
                for(int i = 0 ; i < connsize ; ++i)
                    sum += conn_proibido[conn_elem[i]];

                // nenhuma conectividade proibida, logo colore o elemento
                if(sum == 0)
                {
                    elements_color[iel]          = color;
                    sort[nelem_colored]          = iel;

                    //cont++;
                    nelem_colored++;
                    nelem_thiscolor++;
                    if(iel == istart) istart++;

                    if(nelem_thiscolor == max_nelem)
                    {
                        color++;
                        nelem_thiscolor = 0;
                        nelem_reachlimit = true;
                    }

                    for(int i = 0 ; i < connsize ; i++)
                        conn_proibido[conn_elem[i]] = 1; // conectividade proibida
                } 
            } 
        }
        
        if(!nelem_reachlimit)
            color++;

        std::fill(conn_proibido.begin(), conn_proibido.end(), 0);
    }

    return color-1;
}

void Mesh::MeshColoring(color_mode_t color_mode, int block_size)
{
    std::cout << "Starting mesh coloring...\n";
    
    std::vector<unsigned int>   sort(n_elements);
    std::vector<unsigned int>   elements_color(n_elements);
    std::vector<unsigned int>   new_conn;
    std::vector<unsigned int>   new_offset;
    std::vector<unsigned short> new_type;

    switch (color_mode)
    {
        case COLOR_DEFAULT:
            std::cout << "  Applying Greedy algorithm...\n";
            this->n_colors = Coloring(this, elements_color, sort);
            break;
        default:
            std::cout << "  Applying Blocked algorithm with "<< block_size << " elements per color...\n";
            this->n_colors = BlockedColoring(this, elements_color, sort, block_size);
            break;
    }

    ReorderElements(this, sort, new_conn, new_offset, new_type);
    UpdateMeshArrays(this, elements_color, new_conn, new_offset, new_type);

#ifdef DEBUG
    std::cout << "  # elements per color: \n";
    for(int i = 0 ; i < n_internal_colors ; i++) {
        std::cout.width(8);
        std::cout << mesh_coloring_internal[i];
        if((i+1)%12==0) std::cout << std::endl;
    }
#endif
    std::cout << "\n  # n colors: " << n_colors << "\n";
    std::cout << "\nFinished mesh coloring...\n";

}

/*
void Mesh::MeshColoring_test()
{
    idx_t* xadj;
    idx_t* adjncy;
    int error = 0;

    n_internal_colors = ColoringOpenMP_RokosOpt(this);
    MeshToDualGraph(this, &xadj, &adjncy);
    CheckColoring(&xadj, &adjncy, &this->mesh_coloring_internal, this->n_elements, &error);
    
    if(error != 1)
    {
        std::cout << "Coloring test mesh completed\n";
    }
}
*/

