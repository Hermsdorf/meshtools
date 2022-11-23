#include <iostream>
#include <fstream>
#include <sstream>

#include "meshtools.h"
#include "parallel_mesh.h"
#include "mpi.h"


ParallelMesh::ParallelMesh()
{
    this->n_elements                 = 0;
    this->n_nodes                    = 0;
    this->internal_mesh              = false;
    //this->mesh_coloring_internal     = nullptr;
    this->processor_id               = MeshTools::processor_id();
    this->n_processors               = MeshTools::n_processors();
    this->n_global_elements          = 0;
    this->n_global_internal_elements = 0;
    this->n_global_nodes             = 0;
    shared_nodes_offset.push_back(0);
}



ParallelMesh::~ParallelMesh()
{
    //local_to_global.clear();
}


void ParallelMesh::setNeighborProcessors(std::vector<unsigned int> neighbors_processors)
{
    this->neighbor_processors = neighbor_processors;
}
void ParallelMesh::setSharedNodesOffset(std::vector<unsigned int> shared_nodes_offset)
{
    this->shared_nodes_offset = shared_nodes_offset;
}
void ParallelMesh::setSharedNodes(std::vector<unsigned int> shared_nodes)
{
    this->shared_nodes = shared_nodes;
}
/*
void ParallelMesh::setLocal2Global(std::vector<unsigned int> local2global)
{
    this->local_to_global = local2global;
}
*/

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

void ParallelMesh::set_n_global_face_elements(unsigned int n_global_face_elements)
{
    this->n_global_faces = n_global_face_elements;
}

void ParallelMesh::set_n_global_elements(unsigned int n_global_elements)
{
    this->n_global_elements = n_global_elements;
}

unsigned int ParallelMesh::get_n_local_nodes()
{
    return this->n_local_nodes;
}

void ParallelMesh::readParallelMesh(const char* filename)
{
    this->setFilename(filename);
    std::ifstream in(filename);
    std::string s;

    if(!in.is_open())
    {
        std::cout << "ERRO: Nao foi possivel abrir o arquivo: " << filename << "\n";
        exit(1);
    }

    unsigned int nelem, nnodes, connsize;


    in >> nelem >> nnodes >> connsize;
    this->n_elements = nelem;
    this->n_nodes = nnodes;

    while(!in.eof())
    {
        std::getline(in, s);
        if(in)
        {
            if(s.find("COORD_LOCAL: ") == 0)
            {
                for(int i = 0 ; i < nnodes ; i++)
                {
                    double x, y, z;
                    in >> x >> y >> z;

                    this->coord.push_back(x);
                    this->coord.push_back(y);
                    this->coord.push_back(z);
                }
            } 
            else if(s.find("CONN_LOCAL: ") == 0)
            {
                for(int i = 0 ; i < connsize ; i++)
                {
                    unsigned int conn_i;
                    in >> conn_i;

                    this->conn.push_back(conn_i);
                }
            }
            else if(s.find("OFFSET_LOCAL: ") == 0)
            {
                for(int i = 0 ; i < nelem+1 ; i++)
                {
                    unsigned int offset_i;
                    in >> offset_i;

                    this->offset.push_back(offset_i);
                }
            }
            else if(s.find("TYPE_LOCAL: ") == 0)
            {
                for(int i = 0 ; i < nelem ; i++)
                {
                    unsigned short type_i;
                    in >> type_i;

                    this->type.push_back(type_i);
                }
            }
            else if(s.find("LOCAL_TO_GLOBAL: ") == 0)
            {
                for(int i = 0 ; i < nnodes ; i++)
                {
                    unsigned int lglobal_i;
                    in >> lglobal_i;

                    this->node_index.push_back(lglobal_i);
                }
            }
            else if(s.find("SHARED NODES: ") == 0)
            {
                unsigned int commsize;
                in >> commsize;
                //this->communication_map.resize(commsize);

                this->n_neighbor_processors = commsize;

                for(int i = 0 ; i < commsize ; i++)
                {
                    unsigned int id_neighbor_process_i, n_shared_nodes_i;
                    in >> id_neighbor_process_i >> n_shared_nodes_i;

                    
                    //this->communication_map[i].set_id_neighbor_process(id_neighbor_process_i);
                    //this->communication_map[i].set_n_shared_nodes(n_shared_nodes_i);

                    for(int j = 0 ; j < n_shared_nodes_i ; j++)
                    {
                        unsigned int node_i;
                        in >> node_i;
                        //this->communication_map[i].nodes.push_back(node_i);
                    }
                }
            }
            else
            {
                std::cout << "ERRO: Formato do arquivo invalido\n";
            }
        }
    }
}

//TODO: Implementar para novo formato
void ParallelMesh::readParallelMeshBin(const char* filename)
{
    this->setFilename(filename);
    std::ifstream in(filename, std::ios::binary);
    std::string s;

    if(!in.is_open())
    {
        std::cout << "ERRO: Nao foi possivel abrir o arquivo: " << filename << "\n";
        exit(1);
    }

    unsigned int nelem, nnodes, connsize;

    in.read((char *) &nelem, sizeof(unsigned int));
    in.read((char *) &nnodes, sizeof(unsigned int));
    in.read((char *) &connsize, sizeof(unsigned int));
    
    this->n_elements = nelem;
    this->n_nodes = nnodes;
    this->conn.resize(connsize);
    this->coord.resize(nnodes*3);
    this->offset.resize(nelem+1);
    this->type.resize(nelem);
    this->node_index.resize(nnodes);

    while(!in.eof())
    {
        std::getline(in, s);
        if(in)
        {
            if(s.find("COORD_LOCAL: ") == 0)
            {
                in.read((char*) &this->coord[0], nnodes*3*sizeof(double));
            } 
            else if(s.find("CONN_LOCAL: ") == 0)
            {
                in.read((char*) &this->conn[0], connsize*sizeof(unsigned int));
            }
            else if(s.find("OFFSET_LOCAL: ") == 0)
            {
                in.read((char*) &this->offset[0], (nelem+1)*sizeof(unsigned int));
            }
            else if(s.find("TYPE_LOCAL: ") == 0)
            {
                in.read((char*) &this->type[0], nelem*sizeof(unsigned short));
            }
            else if(s.find("LOCAL_TO_GLOBAL: ") == 0)
            {
                in.read((char*) &this->node_index[0], nnodes*sizeof(unsigned int));
            }
            else if(s.find("SHARED NODES: ") == 0)
            {
                unsigned int commsize;
                in.read((char*) &commsize, sizeof(unsigned int));
                //this->communication_map.resize(commsize);
                this->n_neighbor_processors = commsize;

                for(int i = 0 ; i < commsize ; i++)
                {
                    unsigned int id_neighbor_process_i, n_shared_nodes_i;

                    in.read((char*) &id_neighbor_process_i, sizeof(unsigned int));
                    in.read((char*) &n_shared_nodes_i, sizeof(unsigned int));
                    
                    //this->communication_map[i].set_id_neighbor_process(id_neighbor_process_i);
                    //this->communication_map[i].set_n_shared_nodes(n_shared_nodes_i);
                    ///in.read((char*) &this->communication_map[i].nodes[0], n_shared_nodes_i*sizeof(unsigned int));

                }
            }
            else
            {
                std::cout << "ERRO: Formato do arquivo invalido\n";
            }
        }
    }
}

#ifdef USE_HDF5
void ParallelMesh::readParallelMeshHDF5(const char* filename)
{
    this->setFilename(filename);
    std::ifstream in(filename, std::ios::binary);
    std::string s;

    hid_t       file, filetype, memtype, space, dset;/* Handles */
    herr_t      status;
    hsize_t     dims[1];

    file = H5Fopen (FILE, H5F_ACC_RDONLY, H5P_DEFAULT);

    unsigned int nelem, nnodes, connsize;


    // readign attribute data

     dset = H5Dopen(file,"attributes");
     

    //in.read((char *) &nelem, sizeof(unsigned int));
    //in.read((char *) &nnodes, sizeof(unsigned int));
    //in.read((char *) &connsize, sizeof(unsigned int));
    dset = H5Dopen (file, "coordinates", H5P_DEFAULT);

    /*
     * Get the datatype and its dimensions.
     */
    filetype = H5Dget_type (dset);
    ndims    = H5Tget_array_dims (filetype, 3);

    /*
     * Get dataspace and allocate memory for read buffer.  This is a
     * three dimensional dataset when the array datatype is included so
     * the dynamic allocation must be done in steps.
     */
    space = H5Dget_space (dset);
    ndims = H5Sget_simple_extent_dims (space, dims, NULL);
    
    this->n_nodes = nnodes;



    this->conn.resize(connsize);
    this->coord.resize(nnodes*3);
    this->offset.resize(nelem+1);
    this->type.resize(nelem);
    this->local_to_global.resize(nnodes);

    while(!in.eof())
    {
        std::getline(in, s);
        if(in)
        {
            if(s.find("COORD_LOCAL: ") == 0)
            {
                in.read((char*) &this->coord[0], nnodes*3*sizeof(double));
            } 
            else if(s.find("CONN_LOCAL: ") == 0)
            {
                in.read((char*) &this->conn[0], connsize*sizeof(unsigned int));
            }
            else if(s.find("OFFSET_LOCAL: ") == 0)
            {
                in.read((char*) &this->offset[0], (nelem+1)*sizeof(unsigned int));
            }
            else if(s.find("TYPE_LOCAL: ") == 0)
            {
                in.read((char*) &this->type[0], nelem*sizeof(unsigned short));
            }
            else if(s.find("LOCAL_TO_GLOBAL: ") == 0)
            {
                in.read((char*) &this->local_to_global[0], nnodes*sizeof(unsigned int));
            }
            else if(s.find("SHARED NODES: ") == 0)
            {
                unsigned int commsize;
                in.read((char*) &commsize, sizeof(unsigned int));
                this->communication_map.resize(commsize);
                this->n_neighbor_processors = commsize;

                for(int i = 0 ; i < commsize ; i++)
                {
                    unsigned int id_neighbor_process_i, n_shared_nodes_i;

                    in.read((char*) &id_neighbor_process_i, sizeof(unsigned int));
                    in.read((char*) &n_shared_nodes_i, sizeof(unsigned int));
                    
                    this->communication_map[i].set_id_neighbor_process(id_neighbor_process_i);
                    this->communication_map[i].set_n_shared_nodes(n_shared_nodes_i);
                    in.read((char*) &this->communication_map[i].nodes[0], n_shared_nodes_i*sizeof(unsigned int));

                }
            }
            else
            {
                std::cout << "ERRO: Formato do arquivo invalido\n";
            }
        }
    }
}
#endif

void ParallelMesh::writePVTK(const char* fname, MeshIODataAppended* info)
{   
    this->WriteVTK(fname, info);

    if(MeshTools::processor_id() != 0) return;

    int timestep = info->getTimeStep();

    char filename[256];

    if (timestep != -1)
        sprintf(filename,"%s_%d_%04d.pvtu", fname, MeshTools::n_processors(), timestep);
    else
        sprintf(filename,"%s_%d.pvtu", fname, MeshTools::n_processors());

    std::ofstream fout;

    fout.open(filename);
    
    std::string os;

    std::string str_aux(this->getFilename());
    
    fout << "<VTKFile type=\"PUnstructuredGrid\" version=\"1.0\" byte_order=\"LittleEndian\" header_type=\"UInt64\">\n";
    fout << " <PUnstructuredGrid>\n";
    fout << "  <PPoints>\n";
    fout << "    <PDataArray type=\"Float64\" NumberOfComponents=\"3\"/>\n";
    fout << "  </PPoints>\n";
    fout << "  <PCells>\n"; 
    fout << "   <PDataArray type=\"Int32\" Name=\"connectivity\" NumberOfComponents=\"1\"/>\n"; 
    fout << "   <PDataArray type=\"Int32\" Name=\"offsets\"      NumberOfComponents=\"1\"/>\n"; 
    fout << "   <PDataArray type=\"UInt16\" Name=\"types\"       NumberOfComponents=\"1\"/>\n"; 
    fout << "  </PCells>\n";
    fout << "  <PPointData>\n";
    fout << "   <PDataArray type=\"UInt32\" Name=\"node-id\"/>\n";
   
    if(info != nullptr)
    {
        auto & point_data = info->getPointDataInfo();
            
        if(point_data.size()!= 0 )
        {
            for(int i = 0; i < point_data.size(); ++i)
                fout << "   <PDataArray type=\""<<MeshDataTypeSTR[point_data[i].type]<<"\" Name=\""<<point_data[i].name<<"\"/>\n";
                
        }
    }
    fout << "  </PPointData>\n";
    fout << "  <PCellData>\n";
    fout << "   <PDataArray type=\"UInt32\" Name=\"tag-id\"/>\n";
    if(info != nullptr)
    {
        auto & cell_data  = info->getCellDataInfo();
        if(cell_data.size()!= 0 )
        {
            
            for(int i = 0; i < cell_data.size(); ++i)
                fout << "   <PDataArray type=\""<<MeshDataTypeSTR[cell_data[i].type]<<"\" Name=\""<<cell_data[i].name<<"\"/>\n";               
           
        }
    }
    fout << "  </PCellData>\n";
    
    for(int p = 0 ; p < MeshTools::n_processors() ; p++)
    {
        char  str_aux[256];

        if (timestep != -1)
            sprintf(str_aux,"%s_%d_%d_%04d.vtu", fname,  MeshTools::n_processors(), p, timestep);
        else
            sprintf(str_aux,"%s_%d_%d.vtu", fname, MeshTools::n_processors(), p);
            
        fout << "  <Piece Source=\"" << str_aux << "\"/>\n";

    }

    fout << " </PUnstructuredGrid>\n";
    fout << "</VTKFile>\n";

    fout.close();
    std::cout << "Writing pvtu completed successfully\n";
}

void  ParallelMesh::getGhostNodesIds(std::vector<unsigned int>& local_ghosts_nodes, std::vector<unsigned int>& global_ghosts_nodes)
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
            local_ghosts_nodes.push_back(ino);
            global_ghosts_nodes.push_back(this->node_index[ino]);
        }

}

void  ParallelMesh::add_neighbor_shared_nodes(unsigned int p, unsigned int n_shared_nodes, const unsigned *node_list)
{
    this->neighbor_processors.push_back(p);
    unsigned int ofs_prev = this->shared_nodes_offset.back();
    for(int i = 0; i < n_shared_nodes; ++i)
        this->shared_nodes.push_back(node_list[i]);
    this->shared_nodes_offset.push_back(ofs_prev+n_shared_nodes);
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

std::vector<unsigned int>& ParallelMesh::getNeighborsProcessors()
{
    return this->neighbor_processors;
}
        
std::vector<unsigned int>&  ParallelMesh::getSharedNodesOffset()
{
    return this->shared_nodes_offset;
}
        
std::vector<unsigned int>&  ParallelMesh::getSharedNodes()
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

    if(MeshTools::processor_id() == 0)
        std::cout << "Building communication map\n";

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
                    info.nodes.push_back(node_id);
                }
            }
            if(info.nodes.size() > 0)
                this->recvfrom_info.push_back(info);
        } else if(this->processor_id > neighbor) // processor_id is master of p
        {
            MessageInformation info;
            info.processor_id = neighbor;
            unsigned int start     = this->shared_nodes_offset[i];
            unsigned int end       = this->shared_nodes_offset[i+1];
            for(int ino = start; ino < end; ino++){   
                int node_id = this->shared_nodes[ino];
                if(greather_neighbor_process[node_id] == this->processor_id) {
                    info.nodes.push_back(node_id);
                }
                //info.nodes.push_back(node_id);
            }
            if(info.nodes.size() > 0)
                this->sendto_info.push_back(info);
        }
    }
    //MPI_Barrier(MeshTools::Comm());
    //cout << "Processor " << MeshTools::processor_id() << ": " << this->sendto_info.size() << " sendto_info\n";
    //cout << "Processor " << MeshTools::processor_id() << ": " << this->recvfrom_info.size() << " recvfrom_info\n";
    if(MeshTools::processor_id() == 0)
        std::cout << "fininshing communication map\n";
}

void ParallelMesh::update()
{
    // Indicates local node, what means that it is not shared with other process
    std::vector<unsigned short> mask_node(this->n_nodes);

    for(int i = 0; i < this->n_nodes; ++i)
         mask_node[i] = 0;
   
    unsigned int n_nodes_offset;

   // Mark nodes that are belong to my master (which are process with id greater than mine)
    std::vector<MessageInformation>& recvfrom = this->get_recvfrom_info();

    unsigned int max_buffer_size = 0;
    unsigned int n_nodes_shared   = 0;
    for(int i = 0; i < recvfrom.size(); ++i)
    {
        unsigned int neighbor                   = recvfrom[i].processor_id;
        std::vector<unsigned int>& shared_nodes = recvfrom[i].nodes;
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


    // Count local nodes, which arent from other process (my master)
    this->n_local_nodes = 0;
    for(int i=0; i < this->n_nodes; i++)
    {
        if(mask_node[i]==0) {
            this->node_index[i] = this->n_local_nodes;
            this->n_local_nodes++;
        } 
    }

    if(MeshTools::n_processors() == 1)
    {
         this->start_node_index = 0;
         return;
    }
    // Sends from predecessor process the value of `n_nodes_local` to `n_nodes_offset` variable`
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

    std::vector<MessageInformation>& sendto_neighbors_map   =  this->get_sendto_info();
    std::vector<MessageInformation>& recvfrom_neighbors_map =  this->get_recvfrom_info();

    unsigned int recv_n_shared_nodes = 0;
    for(int i = 0; i < recvfrom_neighbors_map.size(); ++i)
        recv_n_shared_nodes += recvfrom_neighbors_map[i].nodes.size();

    unsigned int sendto_n_shared_nodes = 0;
    for(int i = 0; i < sendto_neighbors_map.size(); ++i)
        sendto_n_shared_nodes += sendto_neighbors_map[i].nodes.size();

    std::vector<unsigned int> recvBuffer(recv_n_shared_nodes);
    std::vector<unsigned int> sendBuffer(sendto_n_shared_nodes);
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
            int node = neighbor_nodes[ino];
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
            int node = neighbor_nodes[ino];
            unsigned int recv_value = recvBuffer[offset+ino];
            node_index[node] = recv_value;
        } 
        offset += n_shared_nodes; 
    }

    process_face_to_element();
}


void ParallelMesh::WritePMeshMTS(const char *fname)
{
    char filename[256];
    sprintf(filename,"%s_%04d_%04d.mts",fname,MeshTools::n_processors(),MeshTools::processor_id());
    FILE* fout = fopen(filename,"w");
    if(!fout) return ;


    fprintf(fout, "# MeshTools File \n");
    fprintf(fout, "1.0  0  1 \n");
    fprintf(fout, "%8d  # num. faces   \n", this->n_face_elements);
    fprintf(fout, "%8d  # num. elements\n", this->n_elements);
    fprintf(fout, "%8d  # num. nodes \n",   this->n_nodes);
    fprintf(fout, "%8ld # num. physical region\n", this->physical_map.size());
    fprintf(fout, "$BEGIN_PHYSICAL_DATA\n");
    for(auto it = this->physical_map.begin(); it != this->physical_map.end(); it++)
        fprintf(fout, "%d %d %s\n", it->first, it->second.first, it->second.second.c_str());
    fprintf(fout, "$END_PHYSICAL_DATA\n"); 
    fprintf(fout, "$BEGIN_NODE_DATA\n");
    for(int n = 0; n < this->n_nodes; n++)
        fprintf(fout,"%-4d %8.8e %8.8e %8.8e\n",n,coord[n*3],coord[n*3+1], coord[n*3+2]);
    fprintf(fout, "$ENDNODE_DATA\n");
    fprintf(fout,"$BEGIN_BOUNDARY_DATA\n");
    for(int iel = 0; iel <  this->n_face_elements; iel++)
    {
        fprintf(fout,"%-4d %-4d", iel, this->physical_tag[iel]);
        unsigned int connsize = this->getSurfaceElementConnSize(iel);
        unsigned int *conn    = this->getSurfaceElementConn(iel);
        for(int i = 0; i < connsize; ++i)
            fprintf(fout, "%-4d ", conn[i]);
        fprintf(fout,"\n");
    }
    fprintf(fout,"$END_BOUNDARY_DATA\n");
    fprintf(fout,"$BEGIN_ELEMENT_DATA\n");
    for(int iel = 0; iel < this->n_elements; iel++)
    {
        fprintf(fout,"%-4d %-4d ", iel, this->physical_tag[iel+this->n_face_elements]);
        unsigned int connsize = this->getElementConnSize(iel);
        unsigned int *conn    = this->getElementConn(iel);
        for(int i = 0; i < connsize; ++i)
            fprintf(fout, "%-4d ", conn[i]);
        fprintf(fout,"\n");
    }
    fprintf(fout,"$END_ELEMENT_DATA\n");
    fprintf(fout,"$BEGIN_GLOBAL_NODE_IDS\n");
    for(int i = 0; i < this->n_nodes; ++i) {
        fprintf(fout, "%-4d ", this->node_index[i]);
        if((i+1)%5 == 0) fprintf(fout,"\n");
    }
    fprintf(fout,"\n$END_GLOBAL_NODE_IDS\n");
    fprintf(fout ,"$BEGIN_PARALLEL_DATA\n");
    fprintf(fout, "%ld  # number of neighbor processors \n", neighbor_processors.size());
    for(int i=0; i < this->neighbor_processors.size(); i++)
            fprintf(fout,"%d ", neighbor_processors[i]);
    fprintf(fout, "  # neighbor processors\n");
    for(int i=0; i < this->shared_nodes_offset.size(); i++)
        fprintf(fout,"%d ", shared_nodes_offset[i]);
    fprintf(fout, " # node offset\n");
    for(int i=0; i < shared_nodes.size(); i++)
        fprintf(fout,"%d ", shared_nodes[i]);
    fprintf(fout, "\n");
    fprintf(fout ,"$END_PARALLEL_DATA\n");
    fclose(fout);
}

void ParallelMesh::set_start_node_index(unsigned int start_node_index)
{
    this->start_node_index = start_node_index;
}

void ParallelMesh::set_n_processors(int n_processors)
{
    this->n_processors = n_processors;
}

void ParallelMesh::set_sendto_info(std::vector<MessageInformation>  info)
{
    this->sendto_info = info;
}

void ParallelMesh::set_recvfrom_info(std::vector<MessageInformation>  info)
{
    this->recvfrom_info = info;
}

bool ParallelMesh::get_internal_mesh()
{
    return this->internal_mesh;
}

void ParallelMesh::set_internal_mesh(bool internal_mesh)
{
    this->internal_mesh = internal_mesh;
}