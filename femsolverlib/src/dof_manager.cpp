
#include <set>

#include "meshtools.h"
#include "dof_manager.h"

DofManager::DofManager(ParallelMesh &mesh):
    _mesh(mesh),
    _ndof(0),
    _first_global_dof_index(0),
    _last_global_dof_index(0),
    _prepared_to_use(false)
    {

    }

DofManager::~DofManager()
{
    _dofs.clear();
    _dof_indices.clear();
    _boundaries.clear();
    if(&_mesh)
        delete &_mesh;
}
void DofManager::add_dirichlet_boundary(DirichletBoundary &boundary)
{
    for(auto it = _boundaries.begin(); it != _boundaries.end(); ++it)
        if(*it == boundary) return;

    _boundaries.push_back(boundary);
}


void DofManager::prepare_to_use()
{
    //* 1. Defining nodes with boundary conditions
    int n_nodes             = _mesh.get_n_nodes();
    int n_boundary_elements = _mesh.get_n_face_elements();
    std::vector<int> &tags  = _mesh.getPhysicalTag();
    std::vector<unsigned int>&  neighbor_processors = _mesh.getNeigborsProcessors();
    std::vector<unsigned int>&  shared_nodes_offset = _mesh.getSharedNodesOffset();
    std::vector<unsigned int>&  shared_nodes = _mesh.getSharedNodes();

    _dof_indices.resize(n_nodes*_ndof);

    for(auto it = _boundaries.begin(); it != _boundaries.end(); ++it)
    {
        // Get each node from its `it` boundary 
        int boundary_id = it->get_boundary_id(); // Gmsh id
        int dof_id      = it->get_dof_id();      // Meshtools id

        std::set<int>  boundary_nodes;

        for(int iel = 0; iel < n_boundary_elements; iel++)
        {
            if(tags[iel] == boundary_id)
            {
                unsigned int connsize = _mesh.getSurfaceElementConnSize(iel);
                unsigned int *conn    = _mesh.getSurfaceElementConn(iel);
                for(int ino = 0; ino < connsize; ++ino)
                    boundary_nodes.insert(conn[ino]);
            }
        }

        for(auto bnd_node_iter =  boundary_nodes.begin(); bnd_node_iter != boundary_nodes.end(); ++bnd_node_iter)
        {
            int node_id = *bnd_node_iter;
            // flag indicanting that there is no equation to this node because it is a boundary node (with its respective boundary condition)
            _dof_indices[node_id*_ndof + dof_id] = -1;
        }
    }

    //* 2. Marking nodes that doesnt belongs to the processor
    
    // Indicates local node, what means that it is not shared with other process
    std::vector<unsigned short> mask_node(n_nodes);

    for(int i = 0; i < n_nodes; ++i)
         mask_node[i] = 0;

    // Mark at _dof_indices, nodes that are belong to my master (which are process with id greater than mine)
    int max_buffer_size = 0;
    for(int i = 0; i < neighbor_processors.size(); ++i)
    {
        unsigned int neighbor  = neighbor_processors[i];
        if(neighbor > MeshTools::processor_id())
        {
            unsigned int start     = shared_nodes_offset[i];
            unsigned int end       = shared_nodes_offset[i+1];
            if((end-start) > max_buffer_size) max_buffer_size = (end-start);
            for(int ino = start; ino < end; ino++)
            {
                int node_id = shared_nodes[ino]; 

                for(int dof_id =0; dof_id < _ndof; ++dof_id)
                    // flag indicanting that this node belongs to my master, so the equation belongs to him
                    _dof_indices[node_id*_ndof + dof_id] = -2;
            }
        }
    }

    //* 3. Defining the number of the equations that were not marked with the previous -1 and -2 flags
    unsigned int n_equations_offset = 0;
    int n_local_equations = 0;
    for(int i = 0; i < _dof_indices.size(); ++i)
    {
        if(_dof_indices[i] >= 0)
        {
            _dof_indices[i] = n_local_equations;
            n_local_equations++;
        }
    }

    //* 4. Calculating offset
        
    // Sends from predecessor process the value of `n_local_equations` to calculate `n_equations_offset` variable
    MPI_Scan(&n_local_equations,&n_equations_offset,1,MPI_UNSIGNED,MPI_SUM,MPI_COMM_WORLD);
    n_equations_offset -= n_local_equations;

    this->_first_global_dof_index = n_equations_offset; 

    for(int i=0; i < _dof_indices.size(); i++)
    {
        if(_dof_indices[i] >= 0)
            _dof_indices[i] += n_equations_offset;
    }

    //* 5. Communicating dof of interface nodes
    std::vector<unsigned int> sendto_neighbors_map;
    std::vector<unsigned int> recvfrom_neighbors_map;
    for(int i = 0; i < neighbor_processors.size(); ++i)
    {
        unsigned int p     = neighbor_processors[i];
        
        if(MeshTools::processor_id() < p) // processor_id is slave of p
            recvfrom_neighbors_map.push_back(i);

        else if(MeshTools::processor_id() > p) // processor id is master of  p
            sendto_neighbors_map.push_back(i);
    }

    std::vector<unsigned int> recvBuffer(shared_nodes.size()*_ndof);
    std::vector<unsigned int> sendBuffer(shared_nodes.size()*_ndof);
    std::vector<MPI_Request>  requests(sendto_neighbors_map.size()+recvfrom_neighbors_map.size());
    std::vector<MPI_Status>   status(sendto_neighbors_map.size()+recvfrom_neighbors_map.size());
    
    // Exchange Data from
    unsigned int r = 0;
    int n_recvs = recvfrom_neighbors_map.size();
    for(int i =0; i < n_recvs; i++)
    {
        int neighbor_idx       = recvfrom_neighbors_map[i];
        int recv_from          = neighbor_processors[neighbor_idx];
        unsigned int start     = shared_nodes_offset[i];
        unsigned int end       = shared_nodes_offset[i+1];
        unsigned int n_shared_dof = (end-start)*_ndof;
        MPI_Irecv(&recvBuffer[start],n_shared_dof,MPI_UNSIGNED, recv_from,0,MPI_COMM_WORLD,&requests[r++]);
    }
    int n_sends = sendto_neighbors_map.size();
    for(int i =0; i < n_sends; i++)
    {
        int neighbor_idx       = sendto_neighbors_map[i];
        int sendto             = neighbor_processors[neighbor_idx];
        unsigned int start     = shared_nodes_offset[i];
        unsigned int end       = shared_nodes_offset[i+1];
        unsigned int n_shared_dof = (end-start)*_ndof;

        for(int ino =start; ino < end; ino++) 
        {
            int node        = shared_nodes[ino];
            for(int dof_id =0; dof_id < _ndof; ++dof_id)
                sendBuffer[ino*_ndof + dof_id] = _dof_indices[node*_ndof + dof_id];
        }
        MPI_Isend(&sendBuffer[start],n_shared_dof,MPI_UNSIGNED,sendto,0,MPI_COMM_WORLD,&requests[r++]);
    }

    MPI_Waitall(r,&requests[0], &status[0]);

    for(int i =0; i < n_recvs; i++)
    {
        int neighbor_idx = recvfrom_neighbors_map[i];
        int recv_from    = neighbor_processors[neighbor_idx];
        unsigned int start     = shared_nodes_offset[i];
        unsigned int end       = shared_nodes_offset[i+1];
        unsigned int n_shared_nodes = (end-start);
        for(int ino =start; ino < end; ino++) {
            int node              = shared_nodes[ino];

            for(int dof_id =0; dof_id < _ndof; ++dof_id){
                int recv_value = recvBuffer[ino*_ndof + dof_id];
                if(recv_value >= 0)
                    _dof_indices[node*_ndof + dof_id] = recv_value;
            }
        }  
    }

    _prepared_to_use = true;
}

void DofManager::calculate_onnz_dnnz(unsigned int *onnz, unsigned int *dnnz)
{
    int n_local_dof = _mesh.get_n_local_nodes()*_ndof;

    // Preallocation Matrix
    std::vector<std::set<PetscInt>> vdiag(n_local_dof);
    std::vector<std::set<PetscInt>> voff(n_local_dof);

    auto &gindex = _dof_indices;
    unsigned int start = _first_global_dof_index;
    unsigned int end = start + n_local_dof;

    // Getting the d_nnz e o_nnz vector needed to matrix preallocation
    for (int iel = 0; iel < _mesh.get_n_elements(); ++iel)
    {
        int connsz = _mesh.getElementConnSize(iel);
        unsigned int *conn = _mesh.getElementConn(iel);
        for (int i = 0; i < connsz; ++i)
        {
            unsigned int gi = gindex[conn[i]];
            unsigned int li = gi - start;
            if (gi >= start && gi < end)
            {
                for (int j = 0; j < connsz; ++j)
                {
                    unsigned int gj = gindex[conn[j]];
                    if (gj >= start && gj < end)
                        vdiag[li].insert(gj);
                    else
                        voff[li].insert(gj);
                }
            }
        }
    }

    unsigned int* d_nnz = new unsigned int[n_local_dof];
    unsigned int* o_nnz = new unsigned int[n_local_dof];
    for (int i = 0; i < n_local_dof; i++)
    {
        d_nnz[i] = vdiag[i].size();
        o_nnz[i] = voff[i].size();
    }

    onnz = o_nnz;
    dnnz = d_nnz;
}