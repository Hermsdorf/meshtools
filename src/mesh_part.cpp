//
#include <iostream>

#include "metis.h"

#include "mesh.h"


mesh_partition_t* MeshPartitioner(mesh_t* mesh, int nparts)
{

    int metis_return; 

    // Allocate a new mesh partition data info and initialize
    mesh_partition_t *mp = new mesh_partition_t();
    mp->n_partitions = nparts;
    mp->elem_part    = new int [mesh->n_elements];
    mp->nodal_part   = new int [mesh->n_nodes];

    int ofs          = mesh->offset[mesh->n_face_elements];
    int *eptr        = new int [mesh->n_elements+1];
    
#ifdef DEBUG_
    std::cout << "EPTR: " << endl;
#endif
    for(int i = mesh->n_face_elements, j = 0; i < mesh->offset.size() ; i++, j++)
    {
            eptr[j] = mesh->offset[i] - ofs;
#ifdef DEBUG_
            std::cout << eptr[j] << " ";
#endif
    }
    
#ifdef DEBUG_
    std::cout << std::endl;
#endif

    idx_t *ne = &mesh->n_elements;
    idx_t *nn = &mesh->n_nodes; 
    idx_t *eind = &mesh->conn[ofs]; // mesh->conn + ofs;
    idx_t *vwgt = 0;
    idx_t *vsize = 0;
    idx_t ncommon = 1;
    //idx_t *nparts
    real_t *tpwgts = 0;
    idx_t options[METIS_NOPTIONS];
    idx_t  objval = 0;

    METIS_SetDefaultOptions(options);

    //idx_t *epart, idx t *npart
    options[METIS_OPTION_PTYPE]     = METIS_PTYPE_KWAY;
    options[METIS_OPTION_OBJTYPE]   = METIS_OBJTYPE_CUT;
    options[METIS_OPTION_NUMBERING] = 0;


    std::cout << "nel: "    << *ne    << std::endl;
    std::cout << "nnodes: " << *nn << std::endl;

    metis_return = METIS_PartMeshDual(ne,nn,eptr,eind,vwgt,vsize, &ncommon, &mp->n_partitions, tpwgts, options, &objval, mp->elem_part, mp->nodal_part);

    std::cout << "Objval: " << objval << std::endl;

    delete [] eptr;

    return mp;

}



void MeshPartitionDestroy(mesh_partition_t* mp)
{

    if(mp) {
        if(mp->elem_part) delete [] mp->elem_part;
        if(mp->nodal_part) delete [] mp->nodal_part;
        delete mp;
    }

}







