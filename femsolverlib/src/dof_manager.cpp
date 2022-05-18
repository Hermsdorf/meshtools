
#include <set>
#include <algorithm>

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
    MPI_Barrier(MPI_COMM_WORLD);
    cout << "DofManager::prepare_to_use()" << endl;

    //* 1. Defining nodes with boundary conditions
    int n_nodes             = _mesh.get_n_nodes();
    int n_boundary_elements = _mesh.get_n_face_elements();
    std::vector<int> &tags  = _mesh.getPhysicalTag();

    _dof_indices.resize(n_nodes*_ndof);

    cout << "processor[" << MeshTools::processor_id() << "]  - dof_indices.size() " << _dof_indices.size() << endl;

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

         cout << "processor[" << MeshTools::processor_id() << "] has " << boundary_nodes.size() << " nodes on boundary " << boundary_id<< endl;

        for(auto bnd_node_iter =  boundary_nodes.begin(); bnd_node_iter != boundary_nodes.end(); ++bnd_node_iter)
        {
            int node_id = *bnd_node_iter;
            // flag indicanting that there is no equation to this node because it is a boundary node (with its respective boundary condition)
            _dof_indices[node_id*_ndof + dof_id] = -1;
        }
    }

    //* 2. Marking nodes that doesnt belongs to the processor
    
    // Mark at _dof_indices, nodes that are belong to my master (which are process with id greater than mine)
    std::vector<MessageInformation>& recvfrom = _mesh.get_recvfrom_info();
    unsigned int max_buffer_size = 0;
    unsigned int n_dof_shared   = 0;
    for(int i = 0; i < recvfrom.size(); ++i)
    {
        unsigned int neighbor                   = recvfrom[i].processor_id;
        std::vector<unsigned int>& shared_nodes = recvfrom[i].nodes;
        unsigned int n_shared_nodes             = shared_nodes.size();
        
        if(n_shared_nodes > max_buffer_size) max_buffer_size = n_shared_nodes;
        for(int ino = 0; ino < n_shared_nodes ; ino++)
        {
            int node_id = shared_nodes[ino]; 

            for(int dof_id =0; dof_id < _ndof; ++dof_id) {
                // flag indicanting that this node belongs to my master, so the equation belongs to him
                _dof_indices[node_id*_ndof + dof_id] = -2;
                n_dof_shared++;
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

    cout << "processor[" << MeshTools::processor_id() << "] has " << n_local_equations << " local equations , n_shared_dof =" << n_dof_shared << endl;

    MPI_Barrier(MPI_COMM_WORLD);
    if(MeshTools::processor_id() == 0)
    {
        cout << "Before send messages..." << endl;
        for(int n =0; n < n_nodes; n++)
        {
            for(int dof_id =0; dof_id < _ndof; ++dof_id) {
                std::cout << "[" << MeshTools::processor_id() << "] node " << n << ", dof " << dof_id << ": " << _dof_indices[n*_ndof+dof_id] << " \n";
            }
        }
        
    }
    MPI_Barrier(MPI_COMM_WORLD);

    if(MeshTools::processor_id() == 1)
    {
        cout << "Before send messages..." << endl;
        for(int n =0; n < n_nodes; n++)
        {
            for(int dof_id =0; dof_id < _ndof; ++dof_id) {
                std::cout << "[" << MeshTools::processor_id() << "] node " << n << ", dof " << dof_id << ": " << _dof_indices[n*_ndof+dof_id] << " \n";
            }
        }
        std::cout << "\n\n====================================\n\n";
        
    }
    MPI_Barrier(MPI_COMM_WORLD);
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

    std::vector<MessageInformation>& sendto_neighbors_map   = _mesh.get_sendto_info();
    std::vector<MessageInformation>& recvfrom_neighbors_map = _mesh.get_recvfrom_info();

    unsigned int recv_n_shared_nodes = 0;
    for(int i = 0; i < recvfrom_neighbors_map.size(); ++i)
        recv_n_shared_nodes += recvfrom_neighbors_map[i].nodes.size();

    unsigned int sendto_n_shared_nodes = 0;
    for(int i = 0; i < sendto_neighbors_map.size(); ++i)
        sendto_n_shared_nodes += sendto_neighbors_map[i].nodes.size();

    std::vector<unsigned int> recvBuffer(recv_n_shared_nodes*_ndof);
    std::vector<unsigned int> sendBuffer(sendto_n_shared_nodes*_ndof);
    std::vector<MPI_Request>  requests(sendto_neighbors_map.size()+recvfrom_neighbors_map.size());
    std::vector<MPI_Status>   status(sendto_neighbors_map.size()+recvfrom_neighbors_map.size());
    
    // Exchange Data from
    unsigned int r = 0;
    int n_recvs = recvfrom_neighbors_map.size();
    for(int i =0; i < n_recvs; i++)
    {
        std::vector<unsigned int> neighbor_nodes  = recvfrom_neighbors_map[i].nodes;
        int recv_from                             = recvfrom_neighbors_map[i].processor_id;
        unsigned int n_shared_dof                 = neighbor_nodes.size()*_ndof;

        MPI_Irecv(&recvBuffer[neighbor_nodes[0]],n_shared_dof,MPI_UNSIGNED, recv_from,0,MPI_COMM_WORLD,&requests[r++]);
    }

    int n_sends = sendto_neighbors_map.size();
    for(int i =0; i < n_sends; i++)
    {
        std::vector<unsigned int> neighbor_nodes = sendto_neighbors_map[i].nodes;
        int sendto                               = sendto_neighbors_map[i].processor_id;
        unsigned int n_shared_dof                = neighbor_nodes.size()*_ndof;

        for(int ino =0; ino < neighbor_nodes.size(); ino++) 
        {
            int node = neighbor_nodes[ino];
            for(int dof_id =0; dof_id < _ndof; ++dof_id)
                sendBuffer[ino*_ndof + dof_id] = _dof_indices[node*_ndof + dof_id];
        }
        MPI_Isend(&sendBuffer[neighbor_nodes[0]],n_shared_dof,MPI_UNSIGNED,sendto,0,MPI_COMM_WORLD,&requests[r++]);
    }

    // FIXME: buffer de envio/recebimento nao correspondem 
    MPI_Waitall(r,&requests[0], &status[0]);

    for(int i =0; i < n_recvs; i++)
    {
        std::vector<unsigned int> neighbor_nodes  = recvfrom_neighbors_map[i].nodes;
        unsigned int n_shared_nodes               = neighbor_nodes.size();
        
        for(int ino = 0; ino < n_shared_nodes; ino++) 
        {
            int node = neighbor_nodes[ino];

            for(int dof_id =0; dof_id < _ndof; ++dof_id){
                int recv_value = recvBuffer[ino*_ndof + dof_id];
                if(recv_value >= 0)
                    _dof_indices[node*_ndof + dof_id] = recv_value;
            }
        }  
    }
    std::cout << "After sending messages..." << endl;
    MPI_Barrier(MPI_COMM_WORLD);
    if(MeshTools::processor_id() == 0)
    {
        cout << endl;
        for(int n =0; n < n_nodes; n++)
        {
            for(int dof_id =0; dof_id < _ndof; ++dof_id) {
                std::cout << "[" << MeshTools::processor_id() << "] node " << n << ", dof " << dof_id << ": " << _dof_indices[n*_ndof+dof_id] << " \n";
            }
        }
        std::cout << "\n\n====================================\n\n";
    }

    MPI_Barrier(MPI_COMM_WORLD);

    if(MeshTools::processor_id() == 1)
    {
        for(int n =0; n < n_nodes; n++)
        {
            for(int dof_id =0; dof_id < _ndof; ++dof_id) {
                std::cout << "[" << MeshTools::processor_id() << "] node " << n << ", dof " << dof_id << ": " << _dof_indices[n*_ndof+dof_id] << " \n";
            }
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    if(MeshTools::processor_id() == 2)
    {
        cout << endl;
        for(int n =0; n < n_nodes; n++)
        {
            for(int dof_id =0; dof_id < _ndof; ++dof_id) {
                std::cout << "[" << MeshTools::processor_id() << "] node " << n << ", dof " << dof_id << ": " << _dof_indices[n*_ndof+dof_id] << " \n";
            }
        }
        std::cout << "\n\n====================================\n\n";
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