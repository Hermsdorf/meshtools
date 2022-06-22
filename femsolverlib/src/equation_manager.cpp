
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

void EquationManager::equation_indices(int id_dof, unsigned int *local_equation, unsigned int *global_equation)
{}

void EquationManager::add_dirichlet_boundary(DirichletBoundary &boundary)
{
    for(auto it = _boundaries.begin(); it != _boundaries.end(); ++it)
        if(*it == boundary) return;

    _boundaries.push_back(boundary);
}

void EquationManager::prepare_to_use()
{

#ifdef DEBUG    
    MPI_Barrier(MPI_COMM_WORLD);
    cout << "EquationManager::prepare_to_use()" << endl;
#endif

    //* 1. Defining nodes with boundary conditions
    int n_nodes             = _mesh.get_n_nodes();
    int n_boundary_elements = _mesh.get_n_face_elements();
    std::vector<int> &tags  = _mesh.getPhysicalTag();

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
            _equation_indices[node_id*_ndof + dof_id] = -1;
            _boundary_nodes_map[bnd_id][ibcno++] = node_id;
        }
    }

    //* 2. Marking nodes that doesnt belongs to the processor
    
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
                _equation_indices[node_id*_ndof + dof_id] = -2;
                cout << "[ " << MeshTools::processor_id() << " ] " << "node_id: " << node_id << " belongs to processor " << neighbor << endl;
                n_dof_shared++;
            }
        }
        
    }

    //* 3. Defining the number of the equations that were not marked with the previous -1 and -2 flags
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

    //* 4. Calculating offset
        
    // Sends from predecessor process the value of `n_local_equations` to calculate `n_equations_offset` variable
    MPI_Scan(&n_local_equations,&n_equations_offset,1,MPI_UNSIGNED,MPI_SUM,MPI_COMM_WORLD);
    n_equations_offset -= n_local_equations;

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
                if(recv_value >= 0)
                    _equation_indices[node*_ndof + dof_id] = recv_value;
            }
        } 
        offset += n_shared_nodes; 
    }

    _prepared_to_use = true;
}

void EquationManager::calculate_dnnz_onnz(std::vector<unsigned int> &dnnz, std::vector<unsigned int> &onnz)
{
    /* FIXME: mesh dez.msh:
        [ 1 ] _equation_indices: 3 4 5 6 7 8 9
        [ 0 ] _equation_indices: 4 8 6 0 1 2 7 

        processor 0 should have 4 indices with value -2 which belongs to processor 1
        -1 is propagating but -2 isn't
    */
    cout << " [ " << MeshTools::processor_id() << " ] _equation_indices: ";
    for(int i = 0 ; i < _equation_indices.size(); i++)
    {
        cout << _equation_indices[i] << " ";
    }
    cout << "\n\n";

    // Preallocation Matrix
    int n_nodes = _mesh.get_n_nodes();
    cout << "[ " << MeshTools::processor_id() << " ] nnodes = " << n_nodes << "\n";
    std::vector< std::set<int> > vdiag;
    std::vector< std::set<int>  > voff ;
    
    dnnz.resize(_n_local_equations);
    onnz.resize(_n_local_equations);

    cout << "[ " << MeshTools::processor_id() << " ] n_local_equations = " << _n_local_equations << "\n";
    int start = _first_global_equation_index;
    int end   = start + _n_local_equations;
    cout << "EquationManager::calculate_dnnz_onnz()" << endl;
    cout << "  [ " << MeshTools::processor_id() << " ] first_global_equation_index = " << start << "\n";
    cout << "  [ " << MeshTools::processor_id() << " ] last_global_equation_index = " << end << "\n";
    // Getting the d_nnz e o_nnz vector needed to matrix preallocation
    for (int iel = 0; iel < _mesh.get_n_elements(); ++iel)
    {
        int         connsz = _mesh.getElementConnSize(iel);
        unsigned int *conn = _mesh.getElementConn(iel);

        // TODO: Criar uma rotina para obter as equacoes do elemento
        std::vector<int> equations;

        for (int i = 0; i < equations.size(); ++i)
        {
            int eqI = equations[i];
             if(eqI >= 0 ) {
                   
                    if(eqI >= start && eqI < end)
                    {
                         for(int j = 0; j < equations.size(); j++)
                         {
                                int eqJ = equations[j];
                                if(eqJ >= start && eqJ < end)
                                {
                                        vdiag[eqI-start].insert(eqJ));
                                }
                                else
                                {
                                        voff[eqI-start].insert(eqJ);
                                }

                         }
                    }
             }
        }

    }


    // FIXME: for iterating n_nodes*_ndof which is greather than the number of equations (_n_local_equations)
    MPI_Barrier(MPI_COMM_WORLD);
    cout << "  setting dnnz and onnz" << endl;
    for(int i = 0; i < _n_local_equations; i++)
    {
        dnnz[i] = vdiag[i].size();
        onnz[i] = voff[i].size();
    }

}