#include <iostream>
#include <fstream>
#include <sstream>

#include "meshtools.h"
#include "parallel_mesh.h"

ParallelMesh::ParallelMesh()
{
    this->n_elements                 = 0;
    this->n_nodes                    = 0;
    this->internal_mesh              = false;
    this->mesh_coloring_internal     = nullptr;
    this->processor_id               = 0;
    this->n_processors               = 1;
    this->n_global_elements          = 0;
    this->n_global_internal_elements = 0;
    this->n_global_nodes             = 0;
    shared_nodes_offset.push_back(0);
#ifdef USE_MPI
    MPI_Comm_size(MPI_COMM_WORLD, &this->n_processors);
    MPI_Comm_rank(MPI_COMM_WORLD, &this->processor_id);
#endif

}



ParallelMesh::~ParallelMesh()
{
    local_to_global.clear();
}

std::vector<unsigned int>& ParallelMesh::getLocal2Global()
{
    return this->local_to_global;
}

int ParallelMesh::get_n_neighbor_processors()
{
    return this->n_neighbor_processors;
}

void ParallelMesh::set_n_neighbor_processors(int n_neighbor_processors)
{
    this->n_neighbor_processors = n_neighbor_processors;
}

bool ParallelMesh::get_internal_mesh()
{
    return this->internal_mesh;
}

void ParallelMesh::set_internal_mesh(bool internal_mesh)
{
    this->internal_mesh = internal_mesh;
}

unsigned int SharedNodes::get_id_neighbor_process()
{
    return this->id_neighbor_process;
}

void SharedNodes::set_id_neighbor_process(unsigned int id_neighbor_process)
{
    this->id_neighbor_process = id_neighbor_process;
}

unsigned int SharedNodes::get_n_shared_nodes()
{
    return this->n_shared_nodes;
}

void SharedNodes::set_n_shared_nodes(unsigned int n_shared_nodes)
{
    this->n_shared_nodes = n_shared_nodes;
}

std::vector<unsigned int>& SharedNodes::get_nodes()
{
    return this->nodes;
}

void SharedNodes::set_nodes(std::vector<unsigned int> nodes)
{
    this->nodes = nodes;
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

unsigned int ParallelMesh::get_n_global_internal_elements()
{
    return this->n_global_internal_elements;
}

void ParallelMesh::set_n_global_internal_elements(unsigned int n_global_internal_elements)
{
    this->n_global_internal_elements = n_global_internal_elements;
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

                    this->local_to_global.push_back(lglobal_i);
                }
            }
            else if(s.find("SHARED NODES: ") == 0)
            {
                unsigned int commsize;
                in >> commsize;
                this->communication_map.resize(commsize);

                this->n_neighbor_processors = commsize;

                for(int i = 0 ; i < commsize ; i++)
                {
                    unsigned int id_neighbor_process_i, n_shared_nodes_i;
                    in >> id_neighbor_process_i >> n_shared_nodes_i;

                    
                    this->communication_map[i].set_id_neighbor_process(id_neighbor_process_i);
                    this->communication_map[i].set_n_shared_nodes(n_shared_nodes_i);

                    for(int j = 0 ; j < n_shared_nodes_i ; j++)
                    {
                        unsigned int node_i;
                        in >> node_i;
                        this->communication_map[i].nodes.push_back(node_i);
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

void ParallelMesh::writePvtu()
{
    std::cout << "Writing VTK parallel mesh...\n";
    std::ofstream fout;

    int size = n_processors;
    
    std::string str(this->getFilename());
    str.insert(str.length(), ".pvtu"); // inserir "p" em ".vtu" -> ".pvtu"
    fout.open(str.c_str());
    
    std::string os;

    std::string str_aux(this->getFilename());
    
    fout << "<VTKFile type=\"PUnstructuredGrid\" version=\"1.0\" byte_order=\"LittleEndian\" header_type=\"UInt64\">\n";
    fout << "\t<PUnstructuredGrid>\n";
    for(int i = 0 ; i < size ; i++)
    {
        os = std::to_string(i);
        int pos = str_aux.find_last_of('/'); // caso a malha esteja em outro diretorio, deixar somente o nome da malha
        str_aux.erase(0, pos+1);
        str_aux.insert(str_aux.length(), "_" + os + ".vtu");

        fout << "\t\t<PPointData>\n";
        fout << "\t\t\t<PDataArray type=\"Int32\" Name=\"npart\"/>\n";
        fout << "\t\t</PPointData>\n";
        fout << "\t\t<PCellData>\n";
        fout << "\t\t\t<PDataArray type=\"Int32\" Name=\"epart\"/>\n";
        if(this->n_internal_colors != 0) 
            fout << "\t\t\t<PDataArray type=\"Int32\" Name=\"color\"/>\n";
        fout << "\t\t</PCellData>\n";
        fout << "\t\t<PPoints>\n";
        fout << "\t\t\t<PDataArray type=\"Float64\" NumberOfComponents=\"3\"/>\n";
        fout << "\t\t</PPoints>\n";
        fout << "\t\t<Piece Source=\"" << str_aux << "\"/>\n";

        str_aux.clear();
        os.clear();
        str_aux = this->getFilename();
    }
    fout << "\t</PUnstructuredGrid>\n";
    fout << "</VTKFile>\n";

    fout.close();
    std::cout << "Writing pvtu completed successfully\n";
}

void ParallelMesh::writeParallelMesh()
{
    int rank, size;
    rank = processor_id;
    size = n_processors;

    int nnodes = this->get_n_nodes();
    int nelem  = this->get_n_elements();
    int* npart = new int[nnodes];
    for(int i = 0 ; i < nnodes ; i++)
        npart[i] = rank;

    int* epart = new int[nelem];
    for(int i = 0 ; i < nelem ; i++)
        epart[i] = rank;

    MeshVTKWriterInternal(rank, npart, epart, this->mesh_coloring_internal, NULL, NULL);
   
    if(rank == 0)
        this->writePvtu();

    delete [] npart;
    delete [] epart;
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

std::vector<unsigned int>& ParallelMesh::getNeigborsProcessors()
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

void ParallelMesh::BuildCommunicationMap()
{
  
    for(int i = 0; i < this->neighbor_processors.size(); ++i)
    {
        unsigned int p     = this->neighbor_processors[i];
        
        if(this->processor_id < p) // processor_id is slave of p
            this->recvfrom_neighbors_map.push_back(i);
        
        if(this->processor_id > p) // processor id is master of  p
            this->sendto_neighbors_map.push_back(i);
    }
}

void ParallelMesh::renumbering()
{
    // indicar nos locais ou seja não é compartilhando com 
    // nenhum outro processo
    std::vector<unsigned short> mask_node(this->n_nodes);

    std::fill(mask_node.begin(), mask_node.end(),0);

    // marcar em mask_nodes, nodes que pertencem ao meus mestres
    int max_buffer_size = 0;
    for(int i = 0; i < this->neighbor_processors.size(); ++i)
    {
        unsigned int neighbor  = this->neighbor_processors[i];
        if(neighbor > this->processor_id)
        {
            unsigned int start     = this->shared_nodes_offset[i];
            unsigned int end       = this->shared_nodes_offset[i+1];
            if((end-start) > max_buffer_size) max_buffer_size = (end-start);
            for(int ino = start; ino < end; ino++)
            {
                int node_id = this->shared_nodes[node_id]; 
                mask_node[node_id]=1;
            }
        }
    }

    // contabilizar nos que são do processor local sem pertecer
    // a nenhum mestre
    unsigned int n_nodes_local = 0;
    for(int i=0; i < this->n_nodes; i++)
    {
        if(mask_node[i]==0) {
            local_to_global[i] = n_nodes_local;
            n_nodes_local++;
        } 
    }
    unsigned int n_nodes_offset;
    MPI_Scan(&n_nodes_local,&n_nodes_offset,1,MPI_UNSIGNED,MPI_SUM,MPI_COMM_WORLD);

    for(int i=0; i < this->n_nodes; i++)
    {
        if(mask_node[i]==0) {
            local_to_global[i] += n_nodes_offset;
        } 
    }
    
    std::vector<unsigned int> recvBuffer(shared_nodes.size());
    std::vector<unsigned int> sendBuffer(shared_nodes.size());
    std::vector<MPI_Request>  requests(this->sendto_neighbors_map.size()+this->recvfrom_neighbors_map.size());
    std::vector<MPI_Status>   status(this->sendto_neighbors_map.size()+this->recvfrom_neighbors_map.size());
    // Exchange Data from
    unsigned int r = 0;
    int n_recvs = this->recvfrom_neighbors_map.size();
    for(int i =0; i < n_recvs; i++)
    {
        int neighbor_idx = this->recvfrom_neighbors_map[i];
        int recv_from    = this->neighbor_processors[neighbor_idx];
        unsigned int start     = this->shared_nodes_offset[i];
        unsigned int end       = this->shared_nodes_offset[i+1];
        unsigned int n_shared_nodes = (end-start);
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
            sendBuffer[ino] = local_to_global[node];
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