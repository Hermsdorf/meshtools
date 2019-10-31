//
#include <iostream>

#include "metis.h"

#include "mesh.h"


void MeshPartitioner(mesh_t* mesh, int nparts, int* npart, int* epart)
{

    int metis_return; 

    int ofs = mesh->offset[mesh->n_face_elements];
    int *eptr = new int [mesh->n_elements+1];
    
    std::cout << "EPTR: " << endl;
    for(int i = mesh->n_face_elements, j = 0; i < mesh->offset.size() ; i++, j++)
    {
            eptr[j] = mesh->offset[i] - ofs;
            std::cout << eptr[j] << " ";
    }
    
    std::cout << std::endl;

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
    options[METIS_OPTION_PTYPE] = METIS_PTYPE_KWAY;
    options[METIS_OPTION_OBJTYPE] = METIS_OBJTYPE_CUT;
    options[METIS_OPTION_NUMBERING] = 0;


    std::cout << "nel: " << *ne << std::endl;
    std::cout << "nnodes: " << *nn << std::endl;

    metis_return = METIS_PartMeshDual(ne,nn,eptr,eind,vwgt,vsize, &ncommon, &nparts, tpwgts, options, &objval, epart, npart);


    std::cout << "Objval: " << objval << std::endl;

    delete [] eptr;

}







