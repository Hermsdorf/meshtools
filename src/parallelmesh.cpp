#include <iostream>
#include <fstream>
#include <sstream>

#include "mpi.h"
#include "parallelmesh.h"

ParallelMesh::ParallelMesh()
{
    this->n_elements = 0;
    this->n_nodes = 0;
    this->internal_mesh = false;
    this->mesh_coloring_internal = nullptr;
}

ParallelMesh::~ParallelMesh()
{
    local_to_global.clear();
    communication_map.clear();
}

SharedNodes::SharedNodes()
{
    this->n_shared_nodes = 0;
    this->id_processador_vizinho = 0;
}

SharedNodes::~SharedNodes()
{
    this->nodes.clear();
}

std::vector<unsigned int>& ParallelMesh::get_local_to_global()
{
    return this->local_to_global;
}

void ParallelMesh::set_local_to_global(std::vector<unsigned int> local_to_global)
{
    this->local_to_global = local_to_global;
}

int ParallelMesh::get_n_processadores_vizinhos()
{
    return this->n_processadores_vizinhos;
}

void ParallelMesh::set_n_processadores_vizinhos(int n_processadores_vizinhos)
{
    this->n_processadores_vizinhos = n_processadores_vizinhos;
}

std::vector<SharedNodes>& ParallelMesh::get_communication_map()
{
    return this->communication_map;
}

void ParallelMesh::set_communication_map(std::vector<SharedNodes> communication_map)
{
    this->communication_map = communication_map;
}

bool ParallelMesh::get_internal_mesh()
{
    return this->internal_mesh;
}

void ParallelMesh::set_internal_mesh(bool internal_mesh)
{
    this->internal_mesh = internal_mesh;
}

unsigned int SharedNodes::get_id_processador_vizinho()
{
    return this->id_processador_vizinho;
}

void SharedNodes::set_id_processador_vizinho(unsigned int id_processador_vizinho)
{
    this->id_processador_vizinho = id_processador_vizinho;
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

                this->n_processadores_vizinhos = commsize;

                for(int i = 0 ; i < commsize ; i++)
                {
                    unsigned int id_processador_vizinho_i, n_shared_nodes_i;
                    in >> id_processador_vizinho_i >> n_shared_nodes_i;

                    
                    this->communication_map[i].set_id_processador_vizinho(id_processador_vizinho_i);
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
                this->n_processadores_vizinhos = commsize;

                for(int i = 0 ; i < commsize ; i++)
                {
                    unsigned int id_processador_vizinho_i, n_shared_nodes_i;

                    in.read((char*) &id_processador_vizinho_i, sizeof(unsigned int));
                    in.read((char*) &n_shared_nodes_i, sizeof(unsigned int));
                    
                    this->communication_map[i].set_id_processador_vizinho(id_processador_vizinho_i);
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

void writePvtu(ParallelMesh* pmesh)
{
    std::cout << "Writing VTK parallel mesh...\n";
    std::ofstream fout;

    int size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    std::string str(pmesh->getFilename());
    str.insert(str.length(), ".pvtu"); // inserir "p" em ".vtu" -> ".pvtu"
    fout.open(str.c_str());
    
    std::string os;

    std::string str_aux(pmesh->getFilename());
    
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
        fout << "\t\t\t<PDataArray type=\"Int32\" Name=\"color\"/>\n";
        fout << "\t\t</PCellData>\n";
        fout << "\t\t<PPoints>\n";
        fout << "\t\t\t<PDataArray type=\"Float64\" NumberOfComponents=\"3\"/>\n";
        fout << "\t\t</PPoints>\n";
        fout << "\t\t<Piece Source=\"" << str_aux << "\"/>\n";

        str_aux.clear();
        os.clear();
        str_aux = pmesh->getFilename();
    }
    fout << "\t</PUnstructuredGrid>\n";
    fout << "</VTKFile>\n";

    fout.close();
    std::cout << "Writing pvtu completed successfully\n";
}

void ParallelMesh::writeParallelMesh()
{
    bool pmesh_is_internal = this->internal_mesh;
    int rank, size;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    int nnodes = this->get_n_nodes();
    int nelem = pmesh_is_internal ? this->get_n_elements() : (this->get_n_elements() + this->get_n_face_elements());
    int* npart = new int[nnodes];
    for(int i = 0 ; i < nnodes ; i++)
        npart[i] = rank;

    int* epart = new int[nelem];
    for(int i = 0 ; i < nelem ; i++)
        epart[i] = rank;

    if(pmesh_is_internal)
        MeshVTKWriterInternal(rank, npart, epart, this->mesh_coloring_internal, NULL, NULL);
    else
        MeshVTKWriter(rank, npart, epart, this->mesh_coloring_internal, NULL, NULL);

    if(rank == 0)
        writePvtu(this);

    delete [] npart;
    delete [] epart;
}

void ParallelMesh::writeParallelMeshBin()
{
    bool pmesh_is_internal = this->internal_mesh;
    int rank, size;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    int nnodes = this->get_n_nodes();
    int nelem = pmesh_is_internal ? this->get_n_elements() : (this->get_n_elements() + this->get_n_face_elements());
    int* npart = new int[nnodes];
    for(int i = 0 ; i < nnodes ; i++)
        npart[i] = rank;

    int* epart = new int[nelem];
    for(int i = 0 ; i < nelem ; i++)
        epart[i] = rank;


    if(pmesh_is_internal)
        MeshVTKWriterInternalBinAppended(rank, npart, epart, this->mesh_coloring_internal, NULL, NULL);
    else
        MeshVTKWriterBinAppended(rank, npart, epart, this->mesh_coloring_internal, NULL, NULL);

    if(rank == 0)
        writePvtu(this);

    delete [] npart;
    delete [] epart;
}