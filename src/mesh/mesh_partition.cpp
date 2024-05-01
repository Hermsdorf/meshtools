#include <iostream>
#include <set>
#include <fstream>
#include <sstream>
#include <cassert>


#include "meshtools.h"
#include "metis.h"
#include "mesh.h"
#include "mesh_partition.h"
#include "parallel_mesh.h"

#ifdef USE_MPI
#include "mpi.h"
#endif

#define MAX_STR 128
#define CONST_BUFFER_SIZE 15

MeshPartition::MeshPartition()
{
    this->n_partitions = 1;
    this->applied  = false;
}

MeshPartition::~MeshPartition()
{

}


std::vector<int>& MeshPartition::get_nodal_part()
{
    return this->nodal_part;
}

std::vector<int>& MeshPartition::get_elem_part()
{
    return this->elem_part;
}
void MeshPartition::set_n_partitions(int n_partitions)
{
    this->n_partitions = n_partitions;
}



void MeshPartition::apply_metis_partition(std::unique_ptr<Mesh>& mesh, int nparts)
{
    // Only process 0 partitions the mesh and send to others processes
    if(MeshTools::processor_id() != 0 )
        return;

    unsigned int nelem  = mesh->get_n_elements();
    unsigned int nfe    = mesh->get_n_surface_elements();
    unsigned int nnodes = mesh->get_n_nodes();

    auto& mesh_conn            =  mesh->get_connectivity_vector();
    auto& mesh_offset          =  mesh->get_offset_vector();
    auto& mesh_fato_to_element = mesh->get_face_to_element_vector();

    std::cout << "Partitioning in " << nparts << " parts\n";

    // Allocate a new mesh partition data info and initialize
    this->elem_part.resize(nelem);
    this->nodal_part.resize(nnodes);
    this->face_part.resize(nfe);
    

    for (int i = 0; i < nfe; i++)
        this->face_part[i] = -1;
    for (int i = 0; i < nelem; i++)
        this->elem_part[i] = 0;
    for (int i = 0; i < nnodes; i++)
        this->nodal_part[i] = 0;

    if (nparts <= 1)
    {
        for (int i = 0; i < nfe; i++)
            this->face_part[i] = 0;
        std::cout << "Successfully partitioned\n";
    }
    else
    {
        idx_t *ne = (idx_t*) &nelem;
        idx_t *nn = (idx_t*) &nnodes;
        idx_t *eptr = new idx_t[nelem+1];
        idx_t *eind = 0;

        unsigned int offset_first_element = mesh_offset[nfe];

        eind    = (idx_t *) &mesh_conn[offset_first_element];
        eptr[0] = 0;
        unsigned int offset_start = mesh->get_offset_vector()[nfe];
        for (int i = 0; i < nelem; i++)
        {
            unsigned int offset_end   = mesh_offset[nfe+i+1];
            eptr[i+1] = eptr[i] + (offset_end - offset_start);
            offset_start = offset_end;
        }

        idx_t *vwgt    = 0;
        idx_t *vsize   = 0;
        real_t *tpwgts = 0;
        idx_t options[METIS_NOPTIONS];
        idx_t objval = 0;
        idx_t ncommon = 1;

        METIS_SetDefaultOptions(options);
        
        if(nparts < 8)
           options[METIS_OPTION_PTYPE]    = METIS_PTYPE_RB;
        else
            options[METIS_OPTION_PTYPE]    = METIS_PTYPE_KWAY;

        options[METIS_OPTION_NUMBERING] = 0;
        options[METIS_OPTION_CONTIG]    = 1;

        
        int metis_return = METIS_PartMeshDual(ne, nn, eptr, eind, vwgt, vsize, &ncommon, &nparts, tpwgts, options, &objval, this->elem_part.data(), this->nodal_part.data());

        if (metis_return == METIS_OK)
        {
            std::cout << "Successfully partitioned\n";
        }
        else
        {
            if (metis_return == METIS_ERROR_INPUT)
            {
                std::cout << "Input error\n";
                if(eptr != 0) delete [] eptr;
                exit(1);
            }
            else
            {
                if (metis_return == METIS_ERROR_MEMORY)
                {
                    std::cout << "Memory error\n";
                    if(eptr != 0) delete [] eptr;
                    exit(1);
                }
                else
                {
                    std::cout << "Another kind of error\n";
                    if(eptr != 0) delete [] eptr;
                    exit(1);
                }
            }
        }
        

        // partitioning face elements
        for(int iel = 0; iel < nfe; iel++)
        {
            unsigned int internal_element = mesh->get_face_to_element_vector()[iel];
            this->face_part[iel] = this->elem_part[internal_element];
        }
    }

    std::vector<int> count_face(this->n_partitions,0);
    std::vector<int> count_elem(this->n_partitions,0);

    for(int iel = 0; iel < nfe; iel++)
    {
        count_face[this->face_part[iel]]++;
    }
    for(int iel = 0; iel < nelem; iel++)
    {
        count_elem[this->elem_part[iel]]++;
    }

    if(MeshTools::processor_id() == 0)
    {
        for(int p = 0; p < this->n_partitions; p++)
        std::cout << "Partition " << p << " has " << count_face[p] << " face elements, " << count_elem[p] << " elements and \n";  
    }
        

    this->applied = true;
}



/**
 * @brief Obtem a listas de processos que que compartilham o no
 * 
 * @param mesh 
 * @param node_partition 
 */
void MeshPartition::get_node_partition(std::unique_ptr<Mesh>& mesh, std::map<unsigned int, std::set<unsigned int> > &node_partition)
{
    unsigned int ne  = mesh->get_n_elements();
    unsigned int nfe = mesh->get_n_surface_elements();

    auto& mesh_offset = mesh->get_offset_vector();
    auto& mesh_conn   = mesh->get_connectivity_vector();

    for (int iel = 0; iel < mesh->get_n_elements(); ++iel)
    {
        unsigned int offset_start = mesh_offset[nfe+iel];
        unsigned int offset_end   = mesh_offset[nfe+iel+1];
        for(int ino = offset_start; ino < offset_end; ino++)
                node_partition[mesh_conn[ino]].insert(this->elem_part[iel]);
    }
#ifdef _DEBUG
    for(auto it = node_partition.begin(); it != node_partition.end(); ++it){
        std::cout << it->first << " shared with ";
        for(auto p = it->second.begin(); p != it->second.end(); ++p)
            std::cout << *p << " ";
        std::cout << std::endl;
    }
#endif
}

void MeshPartition::get_and_send_local_data(
    std::unique_ptr<Mesh> &mesh, 
    int sendto,
    int array_sizes[],
    std::map<unsigned int, std::set<unsigned int> > &node_partition,
    std::vector<double>         & coord,
    std::vector<unsigned int>   & node_index, 
    std::vector<unsigned int>   & conn,
    std::vector<unsigned int>   & offset,
    std::vector<unsigned short> & type,
    std::vector<int>            & tag,
    std::vector<unsigned int>   & neighbors,
    std::vector<unsigned int>   & neighbors_offset,
    std::vector<unsigned int>   & neighbors_nodes,
    std::vector<unsigned int>            & face_to_element,
    bool                        enable_send
)
{
    
    // Getting values from mesh, which is going 
    // to be splitted among processors
    unsigned int nelem             = mesh->get_n_elements();
    unsigned int nface_elem        = mesh->get_n_surface_elements();
    unsigned int nnodes            = mesh->get_n_nodes();

    

    MPI_Request requests[11];
    MPI_Status  status[11];

    // Global to local node numbering
    std::map<unsigned int, unsigned int>           g2l;

    // Global to local element numbering 
    std::map<unsigned int, unsigned int>           g2l_elem;

    // Local to global node numbering
    std::vector<unsigned int>                     l2g;
    
    // Cleaning vectors which will be filled and
    // set to the parallel mesh
    coord.clear();
    conn.clear();
    offset.clear();
    type.clear();
    tag.clear();
    node_index.clear();
    neighbors.clear();
    neighbors_offset.clear();
    neighbors_nodes.clear();
    face_to_element.clear();

    // Getting the number of internal and face 
    // elements, number of nodes and the size of
    // local conn vector 
    unsigned int nelem_face_local = 0;
    unsigned int nelem_local      = 0;
    int nnode_local               = 0;
    int connSizeTotal             = 0;

    for (unsigned int iel = 0; iel < nelem; ++iel)
    {
        if (this->elem_part[iel] == sendto) // if the element that is being processed 
                                            // is going to be sent to process `sendto`
        {
            std::vector<unsigned int>          conn;
            mesh->get_element_connectivity(iel, conn);

            connSizeTotal += conn.size();

            for (int j = 0; j < conn.size(); j++)
            {
                unsigned int conn_node = conn[j];
                
                // verify if `conn_node` node was already inserted at g2l map
                auto it = g2l.find(conn_node);
                if (it == g2l.end())
                {
                    g2l[conn_node] = nnode_local;
                    l2g.push_back(conn_node);

                    nnode_local++;
                }
            }
            nelem_local++;
        }
    }

    for (unsigned int iel = 0; iel < nface_elem; ++iel)
    {
        if (this->face_part[iel] == sendto) // if the element that is being processed 
                                            // is going to be sent to process `sendto`
        {
            std::vector<unsigned int> conn;
            mesh->get_surface_element_connectivity(iel, conn);

            connSizeTotal += conn.size();

            for (int j = 0; j < conn.size(); j++)
            {
                unsigned int conn_node = conn[j];

                // verify if `conn_node` node was already inserted at g2l map
                auto it = g2l.find(conn_node);
                if (it == g2l.end())
                {
                    g2l[conn_node] = nnode_local;
                    l2g.push_back(conn_node);
                    nnode_local++;
                }
            }
            nelem_face_local++;
        }
    }

    // Fills nodes_per_processors vector which contains the local nodes present in each processor other than the one is going to be sent.
    // Its like the inversion of node_partition structure
    auto map_it     = node_partition.begin();
    std::vector< std::set <unsigned int> > nodes_per_processors(this->n_partitions);
    for( ; map_it !=  node_partition.end(); map_it++)
    {
        int node_id = map_it->first;
       
        if(map_it->second.count(sendto)>0)
        {
            for(auto list_processors = map_it->second.begin(); list_processors != map_it->second.end(); list_processors++)
            {
                unsigned int processor = *list_processors;
                if(processor != sendto)
                {
                    nodes_per_processors[processor].insert(node_id);
                }
            }
        }
    }

    unsigned int ofs = 0;
    neighbors_offset.push_back(ofs);
    for (int np = 0; np < this->n_partitions; np++)
    {
        if (np != sendto && nodes_per_processors[np].size() != 0)   
        {
            neighbors.push_back(np);
            neighbors_offset.push_back(ofs + nodes_per_processors[np].size());
            ofs      += nodes_per_processors[np].size();
            for(auto it_list = nodes_per_processors[np].begin(); it_list != nodes_per_processors[np].end(); it_list++)
                neighbors_nodes.push_back(g2l[*it_list]);
        }
    }

    array_sizes[0]  = nface_elem;
    array_sizes[1]  = nelem_face_local;
    array_sizes[2]  = nelem;
    array_sizes[3]  = nelem_local;
    array_sizes[4]  = nnodes;
    array_sizes[5]  = nnode_local;
    array_sizes[6]  = connSizeTotal;
    array_sizes[7]  = neighbors.size();
    array_sizes[8]  = neighbors_offset.size();
    array_sizes[9]  = neighbors_nodes.size();
    array_sizes[10] = mesh->get_physical_map().size();
    array_sizes[11] = mesh->get_surface_mesh_element_type();
    array_sizes[12] = mesh->get_mesh_element_type();


    if(enable_send)
        MPI_Isend(array_sizes , CONST_BUFFER_SIZE , MPI_INT, sendto, 0, MPI_COMM_WORLD, &requests[0]);

    // Gets local coordinates to send it
    coord.resize(nnode_local*3);
    node_index.resize(nnode_local);
    assert(l2g.size() == nnode_local);

    // Fills coord vector
    for (int n = 0; n < nnode_local; n++)
    {
        int ng           = l2g[n];
        node_index[n]    = mesh->get_node_index_vector()[ng];
        coord[3 * n]     = mesh->get_coordinate_vector()[ng * 3];
        coord[3 * n + 1] = mesh->get_coordinate_vector()[ng * 3 + 1];
        coord[3 * n + 2] = mesh->get_coordinate_vector()[ng * 3 + 2];
        
    }

    if(enable_send) {

        MPI_Wait(&requests[0], &status[0]);

        // Sending nodal data
        MPI_Isend(&coord[0]      , coord.size(), MPI_DOUBLE, sendto, 0, MPI_COMM_WORLD, &requests[0]);
        MPI_Isend(&node_index[0] , l2g.size(), MPI_UNSIGNED, sendto, 0, MPI_COMM_WORLD, &requests[1]);
    }

    // Gathering local data from element data
    conn.resize(connSizeTotal);
    offset.resize(nelem_face_local + nelem_local + 1);
    type.resize(nelem_face_local + nelem_local);
    tag.resize(nelem_face_local + nelem_local);
    face_to_element.resize(nelem_face_local);

    offset[0]       = 0;
    int nc          = 0;
    int iel_local   = 0;

    // Fills conn, offset vectors with surface elements
    for (unsigned int iel = 0; iel < nface_elem; ++iel)
    {
        if (this->face_part[iel] == sendto) // if the element that is being processed 
                                            // is going to be sent to process `sendto`
        {
            std::vector<unsigned int> face_conn;
            mesh->get_surface_element_connectivity(iel,face_conn);
            unsigned int connsize       = face_conn.size();
            offset[iel_local + 1] = offset[iel_local] + connsize;
            for (int i = 0; i < connsize; i++) 
                conn[nc++] = g2l[face_conn[i]];
                
            type[iel_local] =  mesh->get_surface_element_type(iel);
            tag[iel_local]  =  mesh->get_surface_element_physical_tag(iel);
            iel_local++;
        }
    }

    if(enable_send)
        MPI_Waitall(2, &requests[0], &status[0]);
    
    // Fills conn and offset vectors with internal elements
    for (int iel = 0; iel < nelem; ++iel)
    {
        if (this->elem_part[iel] == sendto) // if the element that is being processed 
                                            // is going to be sent to process `sendto`
        {
            std::vector<unsigned int> elem_conn;
            mesh->get_element_connectivity(iel,elem_conn);
            unsigned int connsize = elem_conn.size();

            offset[iel_local + 1] = offset[iel_local] + connsize;
            for (int i = 0; i < connsize; i++)
                   conn[nc++] = g2l[elem_conn[i]];

            type[iel_local] = mesh->get_element_type(iel);
            tag[iel_local]  = mesh->get_element_physical_tag(iel);
            g2l_elem[iel]   = iel_local-nelem_face_local;
            iel_local++;
        }
    }

    iel_local = 0;

    // Fills face_to_element vector
    for (unsigned int iel = 0; iel < nface_elem; ++iel)
    {
        if (this->face_part[iel] == sendto) // if the element that is being processed 
                                            // is going to be sent to process `sendto`
        {
            unsigned int global_elem_id         = mesh->get_element_with_surface_element(iel);
            face_to_element[iel_local] = g2l_elem[global_elem_id];
            iel_local++;
        }
    }

    if(enable_send) 
    {
        MPI_Isend(&conn[0]             , conn.size()             , MPI_UNSIGNED      , sendto, 0, MPI_COMM_WORLD, &requests[0]);
        MPI_Isend(&offset[0]           , offset.size()           , MPI_UNSIGNED      , sendto, 0, MPI_COMM_WORLD, &requests[1]);
        MPI_Isend(&type[0]             , type.size()             , MPI_UNSIGNED_SHORT, sendto, 0, MPI_COMM_WORLD, &requests[2]);
        MPI_Isend(&neighbors[0]        , neighbors.size()        , MPI_UNSIGNED      , sendto, 0, MPI_COMM_WORLD, &requests[3]);
        MPI_Isend(&neighbors_offset[0] , neighbors_offset.size() , MPI_UNSIGNED      , sendto, 0, MPI_COMM_WORLD, &requests[4]);
        MPI_Isend(&neighbors_nodes[0]  , neighbors_nodes.size()  , MPI_UNSIGNED      , sendto, 0, MPI_COMM_WORLD, &requests[5]);
        MPI_Isend(&tag[0]              , tag.size()              , MPI_UNSIGNED      , sendto, 0, MPI_COMM_WORLD, &requests[6]);
        MPI_Isend(&face_to_element[0]  , face_to_element.size()  , MPI_UNSIGNED      , sendto, 0, MPI_COMM_WORLD, &requests[7]);
        MPI_Waitall(8,&requests[0],&status[0]);
    }
}


std::unique_ptr<ParallelMesh> MeshPartition::recv_local_data_from_master()
{
    int          array_sizes[CONST_BUFFER_SIZE];
    MPI_Status   status;

    std::unique_ptr<ParallelMesh> pmesh = std::make_unique<ParallelMesh>();

    //std::cout <<"Processor " << MeshTools::processor_id() <<" receving data form 0" << std::endl;
    MPI_Recv(array_sizes,CONST_BUFFER_SIZE , MPI_INT,0, 0, MPI_COMM_WORLD, &status);

    int n_faces_global    = array_sizes[0];
    int n_faces_local     = array_sizes[1];
    int n_elements_global = array_sizes[2];
    int n_elements_local  = array_sizes[3]; 
    int n_nodes           = array_sizes[4];
    int n_nodes_local     = array_sizes[5];
    int connsize                   = array_sizes[6];
    int n_neigbors                 = array_sizes[7];
    int neigh_ofs                  = array_sizes[8];
    int neigh_nodes                = array_sizes[9];
    int n_physical                 = array_sizes[10];
    int mesh_boundary_element_type = array_sizes[11];
    int mesh_element_type          = array_sizes[12];

    pmesh->set_n_surface_elements(n_faces_local);
    pmesh->set_n_elements(n_elements_local);
    pmesh->set_n_nodes(n_nodes_local);
    pmesh->set_n_global_elements(n_elements_global);
    pmesh->set_n_global_surface_elements(n_faces_global);
    pmesh->set_n_global_nodes(n_nodes);
    pmesh->set_n_neighbor_processors(n_neigbors);
    pmesh->set_surface_mesh_element_type(mesh_boundary_element_type);
    pmesh->set_mesh_element_type(mesh_element_type);

    auto & _coords              = pmesh->get_coordinate_vector();
    auto & _conn                = pmesh->get_connectivity_vector();
    auto & _l2g                 = pmesh->get_node_index_vector();
    auto & _type                = pmesh->get_element_type_vector();
    auto & _tag                 = pmesh->get_element_physical_tag_vector(); 
    auto & _offset              = pmesh->get_offset_vector();
    auto & _neighbors           = pmesh->get_neighbors_processors_vector();
    auto & _shared_nodes_offset = pmesh->get_shared_nodes_offset_vector();
    auto & _shared_nodes        = pmesh->get_shared_nodes_vector();
    auto & _face_to_element     = pmesh->get_face_to_element_vector();

    int n_total_elements = n_elements_local+n_faces_local;

    _coords.resize(n_nodes_local*3);
    _l2g.resize(n_nodes_local);
    _conn.resize(connsize);
    _offset.resize(n_total_elements+1);
    _type.resize(n_total_elements);
    _tag.resize(n_total_elements);
    _neighbors.resize(n_neigbors);
    _shared_nodes_offset.resize(neigh_ofs);
    _shared_nodes.resize(neigh_nodes);
    _face_to_element.resize(n_faces_local);

    MPI_Recv(&_coords[0]              , _coords.size(), MPI_DOUBLE  , 0, 0, MPI_COMM_WORLD, &status);
    MPI_Recv(&_l2g[0]                 , _l2g.size()   , MPI_UNSIGNED, 0, 0, MPI_COMM_WORLD, &status);
    MPI_Recv(&_conn[0]                , _conn.size()  , MPI_UNSIGNED      , 0, 0, MPI_COMM_WORLD, &status);
    MPI_Recv(&_offset[0]              , _offset.size(), MPI_UNSIGNED      , 0, 0, MPI_COMM_WORLD, &status);
    MPI_Recv(&_type[0]                , _type.size()  , MPI_UNSIGNED_SHORT, 0, 0, MPI_COMM_WORLD, &status);
    MPI_Recv(&_neighbors[0]           , _neighbors.size()           , MPI_UNSIGNED      , 0, 0, MPI_COMM_WORLD, &status);
    MPI_Recv(&_shared_nodes_offset[0] , _shared_nodes_offset.size() , MPI_UNSIGNED      , 0, 0, MPI_COMM_WORLD, &status);
    MPI_Recv(&_shared_nodes[0]        , _shared_nodes.size()        , MPI_UNSIGNED      , 0, 0, MPI_COMM_WORLD, &status);
    MPI_Recv(&_tag[0]                 , _tag.size()                 , MPI_INT           , 0, 0, MPI_COMM_WORLD, &status);
    MPI_Recv(&_face_to_element[0]     , _face_to_element.size()     , MPI_UNSIGNED      , 0, 0, MPI_COMM_WORLD, &status);
    
    return pmesh;
}


std::unique_ptr<ParallelMesh> MeshPartition::distributed_mesh(std::unique_ptr<Mesh> &mesh)
{
    int processor_id  = MeshTools::processor_id();
    int n_processors  = MeshTools::n_processors(); 
        
    std::unique_ptr<ParallelMesh> pmesh;

    if(!this->applied)
        this->apply_metis_partition(mesh,n_processors);

    if(n_processors > 1)
    {
        int array_sizes[CONST_BUFFER_SIZE];

        // process 0 is responsible to generate local arrays and send it to each processor
        if (processor_id == 0)
        {
            std::vector<double>          coords;
            std::vector<unsigned int>    node;
            std::vector<unsigned int>    conn;
            std::vector<unsigned int>    offset;
            std::vector<unsigned short>  type;
            std::vector<int>             tag;
            std::vector<unsigned int>    node_index;
            std::vector<unsigned int>    neighbors;
            std::vector<unsigned int>    neighbors_offset;
            std::vector<unsigned int>    neighbors_nodes;
            std::vector<unsigned int>    face_to_element;

            // A map indicanting each process that the node i is present
            //
            //            |         |
            //       P0   |    P1   |   P1
            //            |         |
            //    -------o1---------o2------
            //            |         |
            //       P2   |    P3   |   P3
            //            |         |
            //
            // These nodes has node_partition[1] = {P0, P1, P2, P3} and node_partition[2] = {P1, P3}
            std::map<unsigned int, std::set<unsigned int> > node_partition;
        

        
            // Fills node_partition structure
            this->get_node_partition(mesh,node_partition);

            // Processing and sending local arrays and variables to each `p` processor other than 0
            for (int p = 1; p < this->n_partitions; p++)
            {
                //std::cout << "Sending data to processor " << p << endl;
                this->get_and_send_local_data(mesh,p,array_sizes,node_partition,coords,node_index,conn,offset,type,tag,neighbors,neighbors_offset,neighbors_nodes,face_to_element,true);
            }


            // Processing and filling local arrays and variables to process 0
            pmesh = std::make_unique<ParallelMesh>();

            auto & _coords = pmesh->get_coordinate_vector();
            auto & _conn   = pmesh->get_connectivity_vector();
            auto & _type   = pmesh->get_element_type_vector();
            auto & _tag    = pmesh->get_element_physical_tag_vector(); 
            auto & _offset = pmesh->get_offset_vector();
            auto & _node_index = pmesh->get_node_index_vector();
            auto & _neighbors           = pmesh->get_neighbors_processors_vector();
            auto & _shared_nodes_offset = pmesh->get_shared_nodes_offset_vector();
            auto & _shared_nodes        = pmesh->get_shared_nodes_vector();
            auto & _face_to_element     = pmesh->get_face_to_element_vector();

            this->get_and_send_local_data(mesh,0,array_sizes,node_partition,_coords,_node_index,_conn,_offset,_type,_tag,_neighbors,_shared_nodes_offset,_shared_nodes, _face_to_element, false);
            
            int n_faces_global    = array_sizes[0];
            int n_faces_local     = array_sizes[1];
            int n_elements_global = array_sizes[2];
            int n_elements_local  = array_sizes[3]; 
            int n_nodes           = array_sizes[4];
            int n_nodes_local     = array_sizes[5];
            int connsize          = array_sizes[6];
            int n_neigbors        = array_sizes[7];
            int neigh_ofs         = array_sizes[8];
            int neigh_nodes       = array_sizes[9];
            int n_physical        = array_sizes[10];
            int mesh_boundary_elem_type = array_sizes[11];
            int mesh_elem_type          = array_sizes[12];

            pmesh->set_n_surface_elements(n_faces_local);
            pmesh->set_n_elements(n_elements_local);
            pmesh->set_n_nodes(n_nodes_local);
            pmesh->set_n_global_elements(n_elements_global);
            pmesh->set_n_global_surface_elements(n_faces_global);
            pmesh->set_n_global_nodes(n_nodes);
            pmesh->set_n_neighbor_processors(n_neigbors);
            pmesh->set_surface_mesh_element_type(mesh_boundary_elem_type);
            pmesh->set_mesh_element_type(mesh_elem_type);

            auto &map = pmesh->get_physical_map();

            // Broadcasting Physical Groups
            n_physical = map.size();

            std::vector<int>  map_ids(2*n_physical);
            std::vector<char> map_names(n_physical*MAX_STR);
    
            auto iter = map.begin();
        
            for(int i = 0; iter != map.end(); iter++, i++)
            {
                map_ids[2*i  ]   = iter->first;           // id do grupo fisico
                map_ids[2*i+1]   = iter->second.first;    // dimensao
                strncpy(&map_names[i*MAX_STR],iter->second.second.c_str(),MAX_STR);
                std::cout << iter->first << " ";
                std::cout << iter->second.first << " ";
                std::cout << iter->second.second << std::endl;
            }

            MPI_Bcast(&n_physical,1, MPI_INT, 0, MPI_COMM_WORLD);
            MPI_Bcast(&map_ids[0],  n_physical*2, MPI_INT, 0, MPI_COMM_WORLD);
            MPI_Bcast(&map_names[0],n_physical*MAX_STR, MPI_CHAR, 0, MPI_COMM_WORLD);

            auto& pmap = pmesh->get_physical_map();
            pmap.clear();
            for(int i = 0; i < n_physical; i++)
            {
                std::pair<int,PhysicalData> data;
                char buffer[MAX_STR+1];
                data.first         = map_ids[2*i];
                data.second.first  = map_ids[2*i+1];
                strncpy(buffer,&map_names[i*MAX_STR],MAX_STR-1);
                buffer[MAX_STR] = '\0';
                data.second.second = buffer;
                pmap.insert(data);
            }
        }   
        else
        {
            pmesh = this->recv_local_data_from_master();

            int n_physical;

            // Broadcast Physical Groups
            MPI_Bcast(&n_physical,1, MPI_INT, 0, MPI_COMM_WORLD);

            //std::cout <<  "Receiving n_physical = " << n_physical <<std::endl;

            std::vector<int>  map_ids(2*n_physical);
            std::vector<char> map_names(n_physical*MAX_STR);

            MPI_Bcast(&map_ids[0], n_physical*2, MPI_INT, 0, MPI_COMM_WORLD);
            MPI_Bcast(&map_names[0],n_physical*MAX_STR, MPI_CHAR, 0, MPI_COMM_WORLD);
            
            auto& pmap = pmesh->get_physical_map();
            pmap.clear();
            for(int i = 0; i < n_physical; i++)
            {
                std::pair<int,PhysicalData> data;
                char buffer[MAX_STR+1];
                data.first         = map_ids[2*i];
                data.second.first  = map_ids[2*i+1];
                strncpy(buffer,&map_names[i*MAX_STR],MAX_STR-1);
                buffer[MAX_STR] = '\0';
                data.second.second = buffer;
                pmap.insert(data);
            }
        }

        pmesh->build_communication_map();

    }
    else
    {
        // pmesh is actually a serial mesh
        pmesh = std::make_unique<ParallelMesh>();

        pmesh->set_n_processors(1);
        pmesh->set_n_neighbor_processors(0);

        pmesh->set_n_elements(mesh->get_n_elements());
        pmesh->set_n_global_elements(mesh->get_n_elements());
        pmesh->set_n_surface_elements(mesh->get_n_surface_elements());
        pmesh->set_n_global_surface_elements(mesh->get_n_surface_elements());

        pmesh->set_n_nodes(mesh->get_n_nodes());
        pmesh->set_n_local_nodes(mesh->get_n_nodes());
        pmesh->set_n_global_nodes(mesh->get_n_nodes());

        pmesh->set_coordinate_vector(mesh->get_coordinate_vector());
        pmesh->set_connectivity_vector(mesh->get_connectivity_vector());
        pmesh->set_offset_vector(mesh->get_offset_vector());
        pmesh->set_element_type_vector(mesh->get_element_type_vector());
        pmesh->set_physical_map(mesh->get_physical_map());
        pmesh->set_element_physical_tag_vector(mesh->get_element_physical_tag_vector());
        pmesh->set_face_to_element_vector(mesh->get_face_to_element_vector());
        pmesh->set_mesh_element_type(mesh->get_mesh_element_type());
        pmesh->set_surface_mesh_element_type(mesh->get_surface_mesh_element_type());
        pmesh->set_node_index_vector(mesh->get_node_index_vector());
        pmesh->set_mesh_dimension(mesh->get_mesh_dimension());
        
        // Empty vectors because it doesn't exists any neighbors
        // std::vector<unsigned int> empty_vector;
        
        // pmesh->setNeighborProcessors(empty_vector);
        // pmesh->setSharedNodesOffset(empty_vector);
        // pmesh->setSharedNodes(empty_vector);

        // std::vector<MessageInformation> empty_vector_message;
        // pmesh->set_sendto_info(empty_vector_message);
        // pmesh->set_recvfrom_info(empty_vector_message);

        pmesh->set_start_node_index(0);
    }   

    pmesh->fill_node_index();
    return pmesh;
}


// void MeshPartition::WriteVTK(Mesh* mesh, const char* fname)
// {
//     //mesh->setFilename(fname);
//     //mesh->MeshVTKWriterInternal(0,&this->nodal_part[0],&this->elem_part[0]);
// }
