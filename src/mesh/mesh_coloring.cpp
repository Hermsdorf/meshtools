#include <iostream>
#include <algorithm>
#include <vector>
#include <map>
#include <cstring>

#include "metis.h"
#include "mesh.h"
#include "mesh_coloring.h"


void update_mesh_arrays(Mesh& mesh, std::vector<unsigned int>& elements_color, std::vector<unsigned int>& new_conn, std::vector<unsigned int>& new_offset, std::vector<unsigned short>& new_type, std::vector<int>& new_tag)
{
    unsigned int ne       = mesh.get_n_elements();
    unsigned int nfe      = mesh.get_n_surface_elements();
    unsigned int n_colors = mesh.get_n_colors();

    unsigned int start_elem_offset = mesh.get_offset_vector()[nfe];
    unsigned int end_elem_offset   = mesh.get_offset_vector()[nfe+ne];
    int loopsize = nfe + ne;

    for(unsigned int i = nfe, j = 0 ; i <= loopsize ; i++, j++)
    {
        mesh.get_offset_vector()[i] = new_offset[j];

    }


    for(unsigned int i = start_elem_offset, j = 0 ; i < end_elem_offset ; i++, j++)
    {
        mesh.get_connectivity_vector()[i] =  new_conn[j];
    }

    for(unsigned int i = nfe, j = 0 ; i < loopsize ; i++, j++)
    {
        mesh.get_element_type_vector()[i]           = new_type[j];
        mesh.get_element_physical_tag_vector()[i]   = new_tag[j];
    }

    mesh.get_colors_vector().resize(n_colors);
    std::fill(mesh.get_colors_vector().begin(), mesh.get_colors_vector().end(), 0);

    for(unsigned int i = 0 ; i < ne ; i++)
    {
        mesh.get_colors_vector()[elements_color[i]-1]++;
    }

}

void reorder_elements(Mesh& mesh, std::vector<unsigned int>& sort, std::vector<unsigned int>& new_conn, std::vector<unsigned int>& new_offset, std::vector<unsigned short>& new_type, std::vector<int>& new_tag)
{
    unsigned int ne           = mesh.get_n_elements();
    unsigned int nfe          = mesh.get_n_surface_elements();
    unsigned int start_offset = mesh.get_offset_vector()[nfe];
    unsigned int end_offset   = mesh.get_offset_vector()[nfe+ne];


    new_conn.resize(end_offset - start_offset);
    new_offset.resize(ne + 1);
    new_type.resize(ne);
    new_tag.resize(ne);

    unsigned int count_conn   = 0;
    unsigned int count        = 0;
    unsigned int count_offset = 1;

    new_offset[0] = start_offset;

    for(unsigned int i = 0 ; i < ne ; i++)
    {
        unsigned int start = mesh.get_offset_vector()[nfe + sort[i]    ];
        unsigned int end   = mesh.get_offset_vector()[nfe + sort[i] + 1];

        new_offset[count_offset] = new_offset[count_offset - 1] + (end-start);
        count_offset++;
        
        new_type[count]    = mesh.get_element_type_vector()[nfe + sort[i]];
        new_tag[count]     = mesh.get_element_physical_tag_vector()[nfe + sort[i]];
        count++;

        for(unsigned int j = start ; j < end ; j++)
        {
            new_conn[count_conn] = mesh.get_connectivity_vector()[j];
            count_conn++;
        }
    }
}

void apply_coloring_aux(Mesh& mesh, std::vector<unsigned int> &elements_color,std::vector<unsigned int>& sort)
{ 
    int n_nodes                    = mesh.get_n_nodes();
    unsigned int ne                = mesh.get_n_elements();
    unsigned int nfe               = mesh.get_n_surface_elements();
    
    elements_color.resize(ne);

    std::vector<unsigned short> conn_proibido(n_nodes);

    int nelem_colored       = 0;

    std::memset(elements_color.data(),0,ne*sizeof(unsigned int));

    int color = 1;
    std::memset(conn_proibido.data(),0, n_nodes*sizeof(unsigned short));

    int istart = 0;
    while(nelem_colored < ne) 
    {
        for(int iel = istart; iel < ne ; ++iel)
        {
            if(elements_color[iel] == 0)
            {
                unsigned int start = mesh.get_offset_vector()[nfe+iel];
                unsigned int end   = mesh.get_offset_vector()[nfe+iel+1];
                unsigned int sum = 0;
                for(int i = start ; i < end ; ++i)
                    sum += conn_proibido[mesh.get_connectivity_vector()[i]];

                if(sum == 0)
                {
                    elements_color[iel] = color;
                    sort[nelem_colored] = iel;
                    nelem_colored++;
                    if(iel == istart)
                        istart++;
                    for(int i = start ; i < end ; ++i)
                        conn_proibido[mesh.get_connectivity_vector()[i]] = 1; // conectividade proibida
                } // nenhuma conectividade proibida, logo colore o elemento
            } // se o elemento nao estiver colorido, tenta colorir
        }
        
        color++;
        std::memset(conn_proibido.data(),0, n_nodes*sizeof(unsigned short));

    }

    mesh.set_n_colors(color-1);
   // return color-1;
}


void apply_blocked_coloring_aux(Mesh& mesh, std::vector<unsigned int> &elements_color,std::vector<unsigned int>& sort, int block_size) 
{ 
    
    int n_nodes                  = mesh.get_n_nodes();
    unsigned int ne              = mesh.get_n_elements();
    unsigned int nfe             = mesh.get_n_surface_elements();
    elements_color.resize(ne);

    int nelem_colored            = 0;
    int color                    = 1;
    unsigned int nelem_thiscolor = 0;
    unsigned int max_nelem       = block_size; // numero maximo de elementos por cor
    std::vector<unsigned short> conn_proibido(n_nodes);

    std::fill(elements_color.begin(), elements_color.end(), 0);
    std::fill(conn_proibido.begin(), conn_proibido.end(), 0);

    int istart = 0;
    while(nelem_colored < ne) 
    {
        bool nelem_reachlimit = false;
        for(int iel = istart ; iel < ne ; ++iel)
        {
            // se o elemento nao estiver colorido, tenta colorir
            if(elements_color[iel] == 0)
            {
                unsigned int start = mesh.get_offset_vector()[nfe+iel];
                unsigned int end   = mesh.get_offset_vector()[nfe+iel+1];

                unsigned int sum = 0;
                for(int i = start ; i < end ; ++i)
                    sum += conn_proibido[mesh.get_connectivity_vector()[i]];

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

                   for(int i = start ; i < end ; ++i)
                        conn_proibido[mesh.get_connectivity_vector()[i]] = 1; // conectividade proibida
                } 
            } 
        }
        
        if(!nelem_reachlimit)
            color++;

        std::fill(conn_proibido.begin(), conn_proibido.end(), 0);
    }

    mesh.set_n_colors(color-1);
}




void MeshColoring::apply_coloring(Mesh& mesh)
{
    int n_elements = mesh.get_n_elements();

    std::vector<unsigned int>   sort(n_elements);
    std::vector<unsigned int>   elements_color(n_elements);
    std::vector<unsigned int>   new_conn;
    std::vector<unsigned int>   new_offset;
    std::vector<unsigned short> new_type;
    std::vector<int> new_tag;

    apply_coloring_aux(mesh,elements_color,sort);
    reorder_elements(mesh, sort, new_conn, new_offset, new_type, new_tag);
    update_mesh_arrays(mesh, sort, new_conn, new_offset, new_type, new_tag);


}

void MeshColoring::apply_blocked_coloring(Mesh& mesh, unsigned int block_size)
{
    int n_elements = mesh.get_n_elements();

    std::vector<unsigned int>   sort(n_elements);
    std::vector<unsigned int>   elements_color(n_elements);
    std::vector<unsigned int>   new_conn;
    std::vector<unsigned int>   new_offset;
    std::vector<unsigned short> new_type;
    std::vector<int> new_tag;

    apply_blocked_coloring_aux(mesh,elements_color,sort,block_size);
    reorder_elements(mesh, sort, new_conn, new_offset, new_type, new_tag);
    update_mesh_arrays(mesh, sort, new_conn, new_offset, new_type, new_tag);
}


