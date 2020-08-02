#include <fstream>
#include <iostream>

#include "metis.h"
#include "mesh.h"
#include "mesh_part.h"

using namespace std;

Mesh_partition_t::Mesh_partition_t() 
{
    this->n_partitions = 1;
    this->nodal_part = NULL;
    this->elem_part = NULL;
}

Mesh_partition_t::~Mesh_partition_t()
{
    delete [] this->elem_part;
    delete [] this->nodal_part;
}

int Mesh_partition_t::get_n_partitions()
{
    return this->n_partitions;
}
int* Mesh_partition_t::get_nodal_part()
{
    return this->nodal_part;
}
int* Mesh_partition_t::get_elem_part()
{
    return this->elem_part;
}
void Mesh_partition_t::set_n_partitions(int n_partitions)
{
    this->n_partitions = n_partitions;
}
void Mesh_partition_t::set_nodal_part(int* nodal_part)
{
    this->nodal_part = nodal_part;
}
void Mesh_partition_t::set_elem_part(int* elem_part)
{
    this->elem_part = elem_part;
}

void Mesh_partition_t::MeshPartitionerInternal(Mesh* mesh, int nparts)
{
    int metis_return; 
    int nelem = (int)mesh->get_n_elements();
    int nnodes = (int)mesh->get_n_nodes();
    
    std::cout << "Partitioning in "<< nparts <<" parts" << std::endl;

    // Allocate a new mesh partition data info and initialize
    this->n_partitions = nparts;
    this->elem_part    = new int [nelem];
    this->nodal_part   = new int [nnodes];

    for(int i = 0 ; i < nelem ; i++)
        this->elem_part[i] = 0;
    for(int i = 0 ; i < nnodes ; i++)
        this->nodal_part[i] = 0;

    if(nparts <= 1)
    {
        cout << "Successfully partitioned" << endl;
    }
    else
    {
        int ofs          = mesh->getElementOffset(0)[0];
        int *eptr        = new int[nelem + 1];
        
        int* offset_aux = (int*)mesh->getElementOffset(0);
        for (int i = 0, j = 0; i <= nelem ; i++, j++)
        {
            eptr[j] = offset_aux[i] - ofs;
        }


        idx_t *ne       = &nelem;
        idx_t *nn       = &nnodes; 
        idx_t *eind     = (idx_t*)mesh->getElementConn(0);
        idx_t *vwgt     = 0;
        idx_t *vsize    = 0;
        idx_t ncommon   = 1;
        real_t *tpwgts  = 0;
        idx_t options[METIS_NOPTIONS];
        idx_t  objval   = 0;

        METIS_SetDefaultOptions(options);

        //idx_t *epart, idx t *npart
        options[METIS_OPTION_PTYPE]     = METIS_PTYPE_KWAY;
        options[METIS_OPTION_OBJTYPE]   = METIS_OBJTYPE_CUT;
        options[METIS_OPTION_NUMBERING] = 0;

        metis_return = METIS_PartMeshNodal(ne,nn,eptr,eind,vwgt,vsize, &nparts, tpwgts, options, &objval, this->elem_part, this->nodal_part);

        delete [] eptr;
        cout << "Successfully partitioned" << endl;
    }
}


void Mesh_partition_t::MeshPartitioner(Mesh* mesh, int nparts)
{

    int metis_return; 
    int nelem = (int)mesh->get_n_elements();
    int nfe = (int)mesh->get_n_face_elements();
    int nnodes = (int)mesh->get_n_nodes();
    int ntelem = nelem + nfe;

    std::cout << "Partitioning in "<< nparts <<" parts" << std::endl;

    // Allocate a new mesh partition data info and initialize
    this->n_partitions = nparts;
    this->elem_part    = new int [ntelem];
    this->nodal_part   = new int [nnodes];

    for(int i = 0 ; i < ntelem ; i++)
        this->elem_part[i] = 0;
    for(int i = 0 ; i < nnodes ; i++)
        this->nodal_part[i] = 0;
    
    if(nparts <= 1)
    {
        cout << "Successfully partitioned" << endl;
    }
    else
    {
        idx_t *ne       = &ntelem;
        idx_t *nn       = &nnodes; 
        idx_t *eptr     = (idx_t*)mesh->getSurfaceElementOffset(0);
        idx_t *eind     = (idx_t*)mesh->getSurfaceElementConn(0);
        idx_t *vwgt     = 0;
        idx_t *vsize    = 0;
        idx_t ncommon   = 1;
        real_t *tpwgts  = 0;
        idx_t options[METIS_NOPTIONS];
        idx_t  objval   = 0;

        METIS_SetDefaultOptions(options);

        options[METIS_OPTION_PTYPE]     = METIS_PTYPE_KWAY;
        options[METIS_OPTION_OBJTYPE]   = METIS_OBJTYPE_CUT;
        options[METIS_OPTION_NUMBERING] = 0;


        metis_return = METIS_PartMeshNodal(ne,nn, eptr, eind,vwgt,vsize, &nparts, tpwgts, options, &objval, this->elem_part, this->nodal_part);
        cout << "Successfully partitioned" << endl;
    }
}







