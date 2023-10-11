
#include <set>
#include <algorithm>
#include <cassert>
#include "meshtools.h"
#include "equation_manager.h"
using namespace std;

EquationManager::EquationManager(ParallelMesh &mesh):
    _mesh(mesh),
    _ndof(0),
    _first_global_equation_index(0),
    _prepared_to_use(false)
    {

    }

EquationManager::~EquationManager()
{
    _equation_indices.clear();
    _boundaries.clear();
}

unsigned int EquationManager::first_global_equation_index()
{
    return this->_first_global_equation_index;
}

unsigned int EquationManager::n_local_equations()
{
    return this->_n_local_equations;
}

void EquationManager::global_indices(int id_dof, std::vector<unsigned int>& conn, std::vector<int>& global_equation)
{

    global_equation.resize(conn.size());
    for(int i = 0 ; i < conn.size() ; i++)
    {
        unsigned int node_id = conn[i];
        for(int j = 0 ; j < _ndof; j++)
        {
            global_equation[i*_ndof+j] = _equation_indices[node_id*_ndof + id_dof];
        }
    }

}


void EquationManager::local_indices(int id_dof, std::vector<unsigned int>& conn, std::vector<int>& local_equation)
{

    local_equation.resize(conn.size());
    for(int i = 0 ; i < conn.size() ; i++)
    {
        unsigned int node_id = conn[i];
        for(int j = 0 ; j < _ndof; j++)
        {
            local_equation[i*_ndof+j] = node_id*_ndof + id_dof;
        }
    }

}

void EquationManager::equation_indices(int id_dof, int conn_size, const unsigned int *conn_local, int *global_equation)
{
    std::vector<int> dof_required;

    // When dof id is -1 it means that we want to get the equation for every dof in the system
    if(id_dof == -1)
    {
        dof_required.resize(_ndof);
        for(int i = 0 ; i < _ndof ; i++)
            dof_required[i] = i;
    }   
    else
    {
        dof_required.resize(1);
        dof_required[0] = id_dof;
    }
    
    for(int i = 0 ; i < conn_size ; i++)
    {
        unsigned int node_id = conn_local[i];
        
        for(int j = 0 ; j < dof_required.size(); j++)
        {
            unsigned int dof_id = dof_required[j];
            global_equation[i*dof_required.size() + j] = _equation_indices[node_id*_ndof + dof_id];
        }
    }

    dof_required.clear();
}   

void EquationManager::add_dirichlet_boundary(DirichletBoundary &boundary)
{
    for(auto it = _boundaries.begin(); it != _boundaries.end(); ++it)
        if(*it == boundary) return;

    _boundaries.push_back(boundary);
}

void EquationManager::prepare_to_use()
{

    //* 1. Defining nodes with boundary conditions
    int n_nodes                      = _mesh.get_n_nodes();
    int n_boundary_elements          = _mesh.get_n_face_elements();
    std::vector<unsigned int> &  l2g = _mesh.getNodeIndexes();
    std::vector<int> &tags           = _mesh.getPhysicalTag();

    _equation_indices.resize(n_nodes*_ndof);
    _boundary_nodes_map.resize(_boundaries.size());

    int bnd_id = 0;
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

       
        _boundary_nodes_map[bnd_id].resize(boundary_nodes.size());

        unsigned int ibcno = 0;
        for(auto bnd_node_iter =  boundary_nodes.begin(); bnd_node_iter != boundary_nodes.end(); ++bnd_node_iter)
        {
                int node_id = *bnd_node_iter;
                // flag indicanting that there is no equation to this node because it is a boundary node (with its respective boundary condition)
                //_equation_indices[node_id*_ndof + dof_id] = -1;
                _boundary_nodes_map[bnd_id][ibcno++] = node_id;
        }
        bnd_id++;

    }


    for(int ino = 0; ino < n_nodes; ino++)
    {
        for(int idof = 0; idof < _ndof; idof++)
        {
            _equation_indices[ino*_ndof + idof] = l2g[ino]*_ndof + idof;
        }
    }

    this->_first_global_equation_index =  _mesh.get_start_global_index()*_ndof;
    this->_n_local_equations = _mesh.get_n_local_nodes()*_ndof;
    
    
    /*
    // 2. Marking nodes that doesnt belongs to the processor
    
    // Mark at _equation_indices, nodes that are belong to my master (which are process with id greater than mine)
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
                _equation_indices[node_id*_ndof + dof_id] = -1;
                n_dof_shared++;
            }
        }
        
    }

    // 3. Defining the number of the equations that were not marked with the previous -1 and -2 flags
    unsigned int n_equations_offset = 0;
    int n_local_equations = 0;
    for(int i = 0; i < _equation_indices.size(); ++i)
    {
        if(_equation_indices[i] >= 0)
        {
            _equation_indices[i] = n_local_equations;
            n_local_equations++;
        }
    }

    this->_n_local_equations = n_local_equations;
    this->_first_global_equation_index = 0;


    //if(MeshTools::n_processors() == 1)
    //{
    //    _prepared_to_use = true;
    //    return;
    //}

    // 4. Calculating offset
        
    // Sends from predecessor process the value of `n_local_equations` to calculate `n_equations_offset` variable
    MPI_Scan(&n_local_equations,&n_equations_offset,1,MPI_UNSIGNED,MPI_SUM,MPI_COMM_WORLD);
    n_equations_offset -= n_local_equations;

    
    if(MeshTools::processor_id() == 1) std::cout <<  "n_equations_offset = " << n_equations_offset << std::endl;

    this->_first_global_equation_index = n_equations_offset; 

    for(int i=0; i < _equation_indices.size(); i++)
    {
        if(_equation_indices[i] >= 0)
            _equation_indices[i] += n_equations_offset;
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
    unsigned int offset = 0;
    int n_recvs = recvfrom_neighbors_map.size();
    for(int i =0; i < n_recvs; i++)
    {
        std::vector<unsigned int> neighbor_nodes  = recvfrom_neighbors_map[i].nodes;
        int recv_from                             = recvfrom_neighbors_map[i].processor_id;
        unsigned int n_shared_dof                 = neighbor_nodes.size()*_ndof;

        MPI_Irecv(&recvBuffer[offset],n_shared_dof,MPI_UNSIGNED, recv_from,0,MPI_COMM_WORLD,&requests[r++]);

        offset += n_shared_dof;

    }

    int n_sends = sendto_neighbors_map.size();
    offset = 0;
    for(int i =0; i < n_sends; i++)
    {
        std::vector<unsigned int> neighbor_nodes = sendto_neighbors_map[i].nodes;
        int sendto                               = sendto_neighbors_map[i].processor_id;
        unsigned int n_shared_dof                = neighbor_nodes.size()*_ndof;

        for(int ino =0; ino < neighbor_nodes.size(); ino++) 
        {
            int node = neighbor_nodes[ino];
            for(int dof_id =0; dof_id < _ndof; ++dof_id) {
                unsigned int idx = ino*_ndof + dof_id;
                sendBuffer[offset+idx] = _equation_indices[node*_ndof + dof_id];
            }
        }
        
        MPI_Isend(&sendBuffer[offset],n_shared_dof,MPI_UNSIGNED,sendto,0,MPI_COMM_WORLD,&requests[r++]);
        offset += n_shared_dof;
    }

    MPI_Waitall(r,&requests[0], &status[0]);

    offset = 0;
    for(int i =0; i < n_recvs; i++)
    {
        std::vector<unsigned int> neighbor_nodes  = recvfrom_neighbors_map[i].nodes;
        unsigned int n_shared_nodes               = neighbor_nodes.size()*_ndof;
        
        for(int ino = 0; ino <  neighbor_nodes.size(); ino++) 
        {
            int node = neighbor_nodes[ino];

            for(int dof_id =0; dof_id < _ndof; ++dof_id){
                int recv_value = recvBuffer[offset+ino*_ndof + dof_id];
                //if(recv_value >= 0)
                    _equation_indices[node*_ndof + dof_id] = recv_value;
            }
        } 
        offset += n_shared_nodes; 
    }

    
    if(MeshTools::processor_id() == 0)
    {
        cout << "Equation indices: " << endl;
        for(int i = 0; i < _equation_indices.size(); ++i)
            cout << _equation_indices[i] << " ";
    }
    */

    _prepared_to_use = true;
}

void EquationManager::calculate_dnnz_onnz(std::vector<unsigned int> &dnnz, std::vector<unsigned int> &onnz)
{
    // Preallocation Matrix
    int n_nodes = _mesh.get_n_nodes();
    std::vector< std::set<int> > vdiag(_n_local_equations);
    std::vector< std::set<int> > voff(_n_local_equations);
    
    dnnz.resize(_n_local_equations);
    onnz.resize(_n_local_equations);

    int start = _first_global_equation_index;
    int end   = start + _n_local_equations;

    // Getting the d_nnz e o_nnz vector needed to matrix preallocation
    for (int iel = 0; iel < _mesh.get_n_elements(); ++iel)
    {
        int         connsz = _mesh.getElementConnSize(iel);
        const unsigned int *conn = _mesh.getElementConn(iel);

        int n_equations = connsz*_ndof;
        int* equations = new int[n_equations];
        equation_indices(-1, connsz, conn, equations);

        for(int i = 0; i < n_equations; i++)
        {
            int eqI = equations[i];
            if(eqI >= 0)
            {
                if(eqI >= start && eqI < end)
                {
                    for(int j = 0; j < n_equations; j++)
                    {
                        int eqJ = equations[j];
                        if(eqJ >= start && eqJ < end)
                        {
                            vdiag[eqI-start].insert(eqJ);
                        }
                        else
                        {
                            voff[eqI-start].insert(eqJ);
                        }
                    }
                }
             }
        }

        delete [] equations;
    }

    for(int i = 0; i < _n_local_equations; i++)
    {
        dnnz[i] = vdiag[i].size();
        onnz[i] = voff[i].size();
    }
}