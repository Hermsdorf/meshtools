
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

void DofManager::add_dirichlet_boundary(DirichletBoundary &boundary)
{
    for(auto it = this->_boundaries.begin(); it != this->_boundaries.end(); ++it)
        if(*it == boundary) return;

    this->_boundaries.push_back(boundary);
}


void DofManager::prepare_to_use()
{
    // 1. Definir os nos com condições de contorno

    int n_nodes             = _mesh.get_n_nodes();
    int n_boundary_elements = _mesh.get_n_face_elements();
    std::vector<int> &tags  = _mesh.getPhysicalTag();
    std::vector<unsigned int>&  neighbor_processors = _mesh.getNeigborsProcessors();
    std::vector<unsigned int>&  shared_nodes_offset = _mesh.getSharedNodesOffset();
    std::vector<unsigned int>&  shared_nodes = _mesh.getSharedNodes();

    _dof_indices.resize(n_nodes*_ndof);

    for(auto it = this->_boundaries.begin(); it != this->_boundaries.end(); ++it)
    {
        // Extrai para cada contorno os nós 
        int boundary_id = it->get_boundary_id();
        int dof_id      = it->get_dof_id();

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

        int n_nodes_boundary = boundary_nodes.size();
        for(auto bnd_node_iter =  boundary_nodes.begin(); bnd_node_iter != boundary_nodes.end(); ++bnd_node_iter)
        {
            int node_id = *bnd_node_iter;
            _dof_indices[node_id*_ndof + dof_id] = -1;
            
        }
        
    }

    // 2. Marcar os nós que nao pertecem os processador
    
    // Indicates local node, what means that it is not shared with other process
    std::vector<unsigned short> mask_node(n_nodes);

    for(int i = 0; i < n_nodes; ++i)
         mask_node[i] = 0;
   
    
    

    // Mark at mask_nodes, nodes that are belong to my master (which are process with id greater than mine)
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
                    _dof_indices[node_id*_ndof + dof_id] = -2;
                
            }
        }
    }




    // 3. Definir os nós com dof's
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

    // 4; Calcular Offset

        
    // Sends from predecessor process the value of `n_nodes_local` to `n_nodes_offset` variable`
    MPI_Scan(&n_local_equations,&n_equations_offset,1,MPI_UNSIGNED,MPI_SUM,MPI_COMM_WORLD);
    n_equations_offset -= n_local_equations;

    for(int i=0; i < _dof_indices.size(); i++)
    {
        if(_dof_indices[i] >= 0)
            _dof_indices[i] += n_equations_offset;
        } 
    }

    // 5. Comunicar os dof dos nos de interface

    std::vector<unsigned int> recvBuffer(shared_nodes.size()*_ndof);
    std::vector<unsigned int> sendBuffer(shared_nodes.size()*_ndof);
    std::vector<MPI_Request>  requests(this->sendto_neighbors_map.size()+this->recvfrom_neighbors_map.size());
    std::vector<MPI_Status>   status(this->sendto_neighbors_map.size()+this->recvfrom_neighbors_map.size());
    
    // Exchange Data from
    unsigned int r = 0;
    int n_recvs = this->recvfrom_neighbors_map.size();
    for(int i =0; i < n_recvs; i++)
    {
        int neighbor_idx       = this->recvfrom_neighbors_map[i];
        int recv_from          = this->neighbor_processors[neighbor_idx];
        unsigned int start     = this->shared_nodes_offset[i];
        unsigned int end       = this->shared_nodes_offset[i+1];
        unsigned int n_shared_nodes = (end-start)*_ndof;
        MPI_Irecv(&recvBuffer[start],n_shared_nodes,MPI_UNSIGNED, recv_from,0,MPI_COMM_WORLD,&requests[r++]);
    }
    int n_sends = this->sendto_neighbors_map.size();
    for(int i =0; i < n_sends; i++)
    {
        int neighbor_idx       = this->sendto_neighbors_map[i];
        int sendto             = this->neighbor_processors[neighbor_idx];
        unsigned int start     = this->shared_nodes_offset[i];
        unsigned int end       = this->shared_nodes_offset[i+1];
        unsigned int n_shared_nodes = (end-start);

        for(int ino =start; ino < end; ino++) {
            int node        = this->shared_nodes[ino];

            sendBuffer[ino*_ndof +dof_id] = dof_indices[node];
        }
        MPI_Isend(&sendBuffer[start],n_shared_nodes,MPI_UNSIGNED,sendto,0,MPI_COMM_WORLD,&requests[r++]);
    }

    MPI_Waitall(r,&requests[0], &status[0]);

    for(int i =0; i < n_recvs; i++)
    {
        int neighbor_idx = this->recvfrom_neighbors_map[i];
        int recv_from    = this->neighbor_processors[neighbor_idx];
        unsigned int start     = this->shared_nodes_offset[i];
        unsigned int end       = this->shared_nodes_offset[i+1];
        unsigned int n_shared_nodes = (end-start);
        for(int ino =start; ino < end; ino++) {
            int node              = this->shared_nodes[ino];
            local_to_global[node] = recvBuffer[ino];
        }
        
    }








}
    

