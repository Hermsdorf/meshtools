#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>

#include "meshtools.h"
#include "parallel_mesh.h"
#include "mpi.h"


// std::unique_ptr<ParallelMesh> ParallelMesh::New()
// {
//     return std::make_unique<ParallelMesh>(new ParallelMesh());
// }
        
ParallelMesh::ParallelMesh()
{
    this->n_elements                 = 0;
    this->n_nodes                    = 0;
    this->processor_id               = MeshTools::processor_id();
    this->n_processors               = MeshTools::n_processors();
    this->n_global_elements          = 0;
    this->n_global_internal_elements = 0;
    this->n_global_nodes             = 0;
    shared_nodes_offset.emplace_back(0);
}

ParallelMesh::~ParallelMesh() { }

void ParallelMesh::set_neighbor_processors_vector(std::vector<unsigned int>& neighbors_processors)
{
    this->neighbor_processors.swap(neighbor_processors);
}

void ParallelMesh::set_shared_nodes_offset_vector(std::vector<unsigned int>& shared_nodes_offset)
{
    this->shared_nodes_offset.swap(shared_nodes_offset);
}

void ParallelMesh::set_shared_nodes_vector(std::vector<unsigned int>& shared_nodes)
{
    this->shared_nodes.swap(shared_nodes);
}

void ParallelMesh::set_n_local_nodes(unsigned int n_local_nodes)
{
    this->n_local_nodes = n_local_nodes;
}

int ParallelMesh::get_n_neighbor_processors()
{
    return this->n_neighbor_processors;
}

void ParallelMesh::set_n_neighbor_processors(int n_neighbor_processors)
{
    this->n_neighbor_processors = n_neighbor_processors;
}

unsigned int ParallelMesh::get_n_global_nodes()
{
    return this->n_global_nodes;
}

void ParallelMesh::set_n_global_nodes(unsigned int n_global_nodes)
{
    this->n_global_nodes = n_global_nodes;
}

unsigned int ParallelMesh::get_n_global_elements()
{
    return this->n_global_elements;
}

void ParallelMesh::set_n_global_surface_elements(unsigned int n_global_face_elements)
{
    this->n_global_surface_elements = n_global_face_elements;
}

void ParallelMesh::set_n_global_elements(unsigned int n_global_elements)
{
    this->n_global_elements = n_global_elements;
}

unsigned int ParallelMesh::get_n_local_nodes()
{
    return this->n_local_nodes;
}



void  ParallelMesh::get_ghost_nodes_ids(std::vector<unsigned int>& local_ghosts_nodes, std::vector<unsigned int>& global_ghosts_nodes)
{
    // Indicates local node, what means that it is not shared with other process
    std::vector<unsigned short> mask_node(this->n_nodes);
    local_ghosts_nodes.clear();
    global_ghosts_nodes.clear();

    // Mark at mask_nodes, nodes that are belong to my master (which are process with id greater than mine)
    for(int i = 0; i < this->neighbor_processors.size(); ++i)
    {
        unsigned int neighbor  = this->neighbor_processors[i];
        if(neighbor > this->processor_id)
        {
            unsigned int start     = this->shared_nodes_offset[i];
            unsigned int end       = this->shared_nodes_offset[i+1];

            for(int ino = start; ino < end; ino++)
            {
                int node_id = this->shared_nodes[ino]; 
                mask_node[node_id]=1;
            }
        }
    }

    for(int ino = 0; ino < this->n_nodes; ++ino)
        if(mask_node[ino]){
            local_ghosts_nodes.emplace_back(ino);
            global_ghosts_nodes.emplace_back(this->node_index[ino]);
        }

}

void  ParallelMesh::add_neighbor_shared_nodes(unsigned int p, unsigned int n_shared_nodes, const unsigned *node_list)
{
    this->neighbor_processors.emplace_back(p);
    unsigned int ofs_prev = this->shared_nodes_offset.back();
    for(int i = 0; i < n_shared_nodes; ++i)
        this->shared_nodes.emplace_back(node_list[i]);
    this->shared_nodes_offset.emplace_back(ofs_prev+n_shared_nodes);
}

unsigned int ParallelMesh::n_neighbor_shared_nodes(unsigned int p)
{
    return this->shared_nodes_offset[p+1] - this->shared_nodes_offset[p];
}

const unsigned int *    ParallelMesh::get_neighbor_shared_nodes(unsigned int p)
{
    unsigned int start       = this->shared_nodes_offset[p];
    return   &this->shared_nodes[start];
}

std::vector<unsigned int>& ParallelMesh::get_neighbors_processors_vector()
{
    return this->neighbor_processors;
}
        
std::vector<unsigned int>&  ParallelMesh::get_shared_nodes_offset_vector()
{
    return this->shared_nodes_offset;
}
        
std::vector<unsigned int>&  ParallelMesh::get_shared_nodes_vector()
{
    return this->shared_nodes;
}

unsigned int ParallelMesh::get_start_global_index()
{
    return this->start_node_index;
}

std::vector<MessageInformation>& ParallelMesh::get_sendto_info()
{
    return this->sendto_info;
}
std::vector<MessageInformation>& ParallelMesh::get_recvfrom_info()
{
    return this->recvfrom_info;
}

void ParallelMesh::build_communication_map()
{

    this->sendto_info.clear();
    this->recvfrom_info.clear();

    MeshTools::PrintDebug("Building communication map\n");

    int greather_neighbor_process[this->n_nodes];

    for(int i = 0; i < this->n_nodes; ++i)
        greather_neighbor_process[i] = processor_id;
    
    for(int i = 0; i < this->neighbor_processors.size(); ++i)
    {
        int neighbor           = this->neighbor_processors[i];
        unsigned int start     = this->shared_nodes_offset[i];
        unsigned int end       = this->shared_nodes_offset[i+1];
        for(int ino = start; ino < end; ino++)
        {
            int node_id = this->shared_nodes[ino]; 
            if(neighbor > greather_neighbor_process[node_id])
                greather_neighbor_process[node_id] = neighbor;
        }
    }

    for(int i = 0; i < this->neighbor_processors.size(); ++i)
    {
        unsigned int neighbor  = this->neighbor_processors[i];

        // Construção do recvnodes
        if(this->processor_id < neighbor) // processor_id is slave of p
        {
            MessageInformation info;
            info.processor_id = neighbor;
            unsigned int start     = this->shared_nodes_offset[i];
            unsigned int end       = this->shared_nodes_offset[i+1];
            for(int ino = start; ino < end; ino++)
            {   
                int node_id = this->shared_nodes[ino]; 
                if(greather_neighbor_process[node_id] == neighbor) {
                    info.nodes.emplace_back(node_id);
                }
            }
            if(info.nodes.size() > 0)
                this->recvfrom_info.emplace_back(info);
        } else if(this->processor_id > neighbor) // processor_id is master of p
        {
            MessageInformation info;
            info.processor_id = neighbor;
            unsigned int start     = this->shared_nodes_offset[i];
            unsigned int end       = this->shared_nodes_offset[i+1];
            for(int ino = start; ino < end; ino++){   
                int node_id = this->shared_nodes[ino];
                if(greather_neighbor_process[node_id] == this->processor_id) {
                    info.nodes.emplace_back(node_id);
                }
                //info.nodes.push_back(node_id);
            }
            if(info.nodes.size() > 0)
                this->sendto_info.emplace_back(info);
        }
    }

}

/*
*  This function updates the local node_index variable, which is the local
*  to global node numbering of the mesh. This function is called after the
*  mesh is partitioned and the communication map is built.
*/
void ParallelMesh::fill_node_index()
{
    // Indicates local node, what means that it is not shared with other process
    std::vector<unsigned short> mask_node(this->n_nodes);
    this->node_index.resize(this->n_nodes);

    for(int i = 0; i < this->n_nodes; ++i)
         mask_node[i] = 0;
   
    unsigned int n_nodes_offset;

    //std::vector<MessageInformation>& recvfrom = this->get_recvfrom_info();
        
    std::vector<MessageInformation>& sendto_neighbors_map   =  this->get_sendto_info();
    std::vector<MessageInformation>& recvfrom_neighbors_map =  this->get_recvfrom_info();

    unsigned int max_buffer_size  = 0;
    unsigned int n_nodes_shared   = 0;

    // Mark nodes that belongs to my master (which are process with id greater than mine)
    for(int i = 0; i < recvfrom_neighbors_map.size(); ++i)
    {
        unsigned int neighbor                   = recvfrom_neighbors_map[i].processor_id;
        std::vector<unsigned int>& shared_nodes = recvfrom_neighbors_map[i].nodes;
        unsigned int n_shared_nodes             = shared_nodes.size();
        
        if(n_shared_nodes > max_buffer_size) max_buffer_size = n_shared_nodes;
        for(int ino = 0; ino < n_shared_nodes ; ino++)
        {
            int node_id = shared_nodes[ino]; 
             // flag indicanting that this node belongs to my master, so the equation belongs to him
            mask_node[node_id] = 1;
            n_nodes_shared++;
        }
    }


    // Count local nodes, in other words, the ones that arent from other process (my master)
    this->n_local_nodes = 0;
    for(int i=0; i < this->n_nodes; i++)
    {
        if(mask_node[i]==0) {
            this->node_index[i] = this->n_local_nodes;
            this->n_local_nodes++;
        } 
    }

    if(MeshTools::n_processors() == 1)
         return;

    // Sends from predecessor process the value of `n_nodes_local` to `n_nodes_offset` variable`
    // Accumulating n_local_nodes from all predecessor processes (processes with smaller rank)
    // in n_nodes_offset
    MPI_Scan(&n_local_nodes,&n_nodes_offset,1,MPI_UNSIGNED,MPI_SUM,MPI_COMM_WORLD);
    n_nodes_offset -= this->n_local_nodes;
    
    this->start_node_index = n_nodes_offset;
   
    // Builds the local node index
    for(int i=0; i < this->n_nodes; i++)
    {
        if(mask_node[i]==0) {
            this->node_index[i] += n_nodes_offset;
        } 
    }



    unsigned int recv_n_shared_nodes = 0;
    // Number of shared nodes that will be received from my master
    for(int i = 0; i < recvfrom_neighbors_map.size(); ++i)
        recv_n_shared_nodes += recvfrom_neighbors_map[i].nodes.size();

    // Number of shared nodes that I'm going to send to my slaves
    unsigned int sendto_n_shared_nodes = 0;
    for(int i = 0; i < sendto_neighbors_map.size(); ++i)
        sendto_n_shared_nodes += sendto_neighbors_map[i].nodes.size();

    std::vector<unsigned int> recvBuffer(recv_n_shared_nodes);
    std::vector<unsigned int> sendBuffer(sendto_n_shared_nodes);
    std::vector<MPI_Request>  requests(sendto_neighbors_map.size()+recvfrom_neighbors_map.size());
    std::vector<MPI_Status>   status(sendto_neighbors_map.size()+recvfrom_neighbors_map.size());
    
    // Exchange Data from. From here to the end of this method 
    // we fill the node_index positions of the shared nodes
    unsigned int r = 0;
    unsigned int offset = 0;
    int n_recvs = recvfrom_neighbors_map.size();
    for(int i =0; i < n_recvs; i++)
    {
        std::vector<unsigned int> neighbor_nodes  = recvfrom_neighbors_map[i].nodes;
        int recv_from                             = recvfrom_neighbors_map[i].processor_id;
        unsigned int n_shared_nodes               = neighbor_nodes.size();

        MPI_Irecv(&recvBuffer[offset],n_shared_nodes,MPI_UNSIGNED, recv_from,0,MPI_COMM_WORLD,&requests[r++]);

        offset += n_shared_nodes;

    }

    int n_sends = sendto_neighbors_map.size();
    offset = 0;
    for(int i =0; i < n_sends; i++)
    {
        std::vector<unsigned int> neighbor_nodes = sendto_neighbors_map[i].nodes;
        int sendto                               = sendto_neighbors_map[i].processor_id;
        unsigned int n_shared_nodes              = neighbor_nodes.size();

        for(int ino =0; ino < neighbor_nodes.size(); ino++) 
        {
            int node               = neighbor_nodes[ino];
            sendBuffer[offset+ino] = node_index[node];
        }
        
        MPI_Isend(&sendBuffer[offset],n_shared_nodes,MPI_UNSIGNED,sendto,0,MPI_COMM_WORLD,&requests[r++]);
        offset += n_shared_nodes;
    }

    MPI_Waitall(r,&requests[0], &status[0]);

    offset = 0;
    for(int i =0; i < n_recvs; i++)
    {
        std::vector<unsigned int> neighbor_nodes  = recvfrom_neighbors_map[i].nodes;
        unsigned int n_shared_nodes               = neighbor_nodes.size();
        
        for(int ino = 0; ino <  neighbor_nodes.size(); ino++) 
        {
            int node                = neighbor_nodes[ino];
            unsigned int recv_value = recvBuffer[offset+ino];
            node_index[node]        = recv_value; 
        } 
        offset += n_shared_nodes; 
    }

    unsigned int sendbuffer[3];
    unsigned int recvbuffer[3];
    sendbuffer[0] = this->n_local_nodes;
    sendbuffer[1] = this->n_elements;
    sendbuffer[2] = this->n_face_elements;

    MPI_Allreduce(sendbuffer, recvbuffer,3,MPI_UNSIGNED,MPI_SUM,MeshTools::Comm());

    this->n_global_nodes    = recvbuffer[0];
    this->n_global_elements = recvbuffer[1];
    this->n_global_surface_elements = recvbuffer[2];

}




void ParallelMesh::set_start_node_index(unsigned int start_node_index)
{
    this->start_node_index = start_node_index;
}


void ParallelMesh::set_n_processors(int n_processors)
{
    this->n_processors = n_processors;
}

void ParallelMesh::set_sendto_info(std::vector<MessageInformation>&  info)
{
    this->sendto_info = info;
}

void ParallelMesh::set_recvfrom_info(std::vector<MessageInformation>&  info)
{
    this->recvfrom_info = info;
}

void ParallelMesh::print_info(bool debug_mode)
{
    Mesh::print_info(debug_mode);
    FILE *fout = !debug_mode ? stdout : MeshTools::DebugOutput();
    fprintf(fout, "Number of Global Elements: %d\n", this->n_global_elements);
    fprintf(fout, "Number of Global Nodes: %d\n", this->n_global_nodes);
    fprintf(fout, "Number of Global Surface Elements: %d\n", this->n_global_surface_elements);
    fprintf(fout, "Number of Local Nodes: %d\n", this->n_local_nodes);
    fprintf(fout, "Number of Neighbors: %ld\n", neighbor_processors.size());
    fprintf(fout, "Neighbors Processors: ");
    for(int i = 0; i < this->neighbor_processors.size(); i++) {
        fprintf(fout, "%d\n", this->neighbor_processors[i]);

        for(int j = this->shared_nodes_offset[i]; j < this->shared_nodes_offset[i+1]; j++){
            unsigned int node = this->shared_nodes[j];
            unsigned int global_node = this->node_index[node];
            fprintf(fout, "[%d, %d]: ", node, global_node);
            fprintf(fout, "(%f, %f, %f)\n", this->coord[node*3], this->coord[node*3+1], this->coord[node*3+2]);
        }
    }
    fprintf(fout, "Send to Info: \n");
    for(int i = 0; i < this->sendto_info.size(); i++)
    {
        fprintf(fout, "Processor ID: %d\n", this->sendto_info[i].processor_id);
        for(int j = 0; j < this->sendto_info[i].nodes.size(); j++)
        {
            unsigned int node        = this->sendto_info[i].nodes[j];
            unsigned int global_node = this->node_index[node];
            fprintf(fout, "[%d, %d]: ", node, global_node);
            fprintf(fout, "(%f, %f, %f)\n", this->coord[node*3], this->coord[node*3+1], this->coord[node*3+2]);
        }
    }
    fprintf(fout, "Recv from Info: \n");
    for(int i = 0; i < this->recvfrom_info.size(); i++)
    {
        fprintf(fout, "Processor ID: %d\n", this->recvfrom_info[i].processor_id);
        for(int j = 0; j < this->recvfrom_info[i].nodes.size(); j++)
        {
            unsigned int node        = this->recvfrom_info[i].nodes[j];
            unsigned int global_node = this->node_index[node];
            fprintf(fout, "[%d, %d]: ", node, global_node);
            fprintf(fout, "(%f, %f, %f)\n", this->coord[node*3], this->coord[node*3+1], this->coord[node*3+2]);
        }
    }
    fprintf(fout,"-----------------------------------------------\n");
}
