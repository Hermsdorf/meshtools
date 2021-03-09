#include <iostream>
#include <fstream>

#include "parallelmesh.h"

ParallelMesh::ParallelMesh()
{
    this->set_n_elements(0);
    this->set_n_nodes(0);
}

ParallelMesh::~ParallelMesh()
{
    this->getConn().clear();
    this->getCoord().clear();
    this->getOffset().clear();
    this->getType().clear();
    this->get_physical_tag().clear();
    this->get_physical_map().clear();
    this->getFilename().clear();
    delete [] this->get_mesh_coloring_internal();
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

    std::vector<unsigned int>& conn = this->getConn();
    std::vector<double>& coord = this->getCoord();
    std::vector<unsigned int>& offset = this-> getOffset();
    std::vector<unsigned short>& type = this->getType();

    in >> nelem >> nnodes >> connsize;
    this->set_n_elements(nelem);
    this->set_n_nodes(nnodes);

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

                    coord.push_back(x);
                    coord.push_back(y);
                    coord.push_back(z);
                }
            } 
            else if(s.find("CONN_LOCAL: ") == 0)
            {
                for(int i = 0 ; i < connsize ; i++)
                {
                    unsigned int conn_i;
                    in >> conn_i;

                    conn.push_back(conn_i);
                }
            }
            else if(s.find("OFFSET_LOCAL: ") == 0)
            {
                for(int i = 0 ; i < nelem+1 ; i++)
                {
                    unsigned int offset_i;
                    in >> offset_i;

                    offset.push_back(offset_i);
                }
            }
            else if(s.find("TYPE_LOCAL: ") == 0)
            {
                for(int i = 0 ; i < nelem ; i++)
                {
                    unsigned short type_i;
                    in >> type_i;

                    type.push_back(type_i);
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
                        this->communication_map[i].get_nodes().push_back(node_i);
                    }
                }
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

    std::vector<unsigned int>& conn = this->getConn();
    std::vector<double>& coord = this->getCoord();
    std::vector<unsigned int>& offset = this-> getOffset();
    std::vector<unsigned short>& type = this->getType();

    in.read((char *) &nelem, sizeof(unsigned int));
    in.read((char *) &nnodes, sizeof(unsigned int));
    in.read((char *) &connsize, sizeof(unsigned int));
    
    this->set_n_elements(nelem);
    this->set_n_nodes(nnodes);
    conn.resize(connsize);
    coord.resize(nnodes*3);
    offset.resize(nelem+1);
    type.resize(nelem);
    local_to_global.resize(nnodes);

    while(!in.eof())
    {
        std::getline(in, s);
        if(in)
        {
            if(s.find("COORD_LOCAL: ") == 0)
            {
                for(int i = 0 ; i < nnodes*3 ; i++)
                    in.read((char*) &coord[i], sizeof(double));
            } 
            else if(s.find("CONN_LOCAL: ") == 0)
            {
                for(int i = 0 ; i < connsize ; i++)
                    in.read((char*) &conn[i], sizeof(unsigned int));
            }
            else if(s.find("OFFSET_LOCAL: ") == 0)
            {
                for(int i = 0 ; i < nelem+1 ; i++)
                    in.read((char*) &offset[i], sizeof(unsigned int));
            }
            else if(s.find("TYPE_LOCAL: ") == 0)
            {
                for(int i = 0 ; i < nelem ; i++)
                    in.read((char*) &type[i], sizeof(unsigned short));
            }
            else if(s.find("LOCAL_TO_GLOBAL: ") == 0)
            {
                for(int i = 0 ; i < nnodes ; i++)
                    in.read((char*) &local_to_global[i], sizeof(unsigned int));
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

                    for(int j = 0 ; j < n_shared_nodes_i ; j++)
                    {
                        unsigned int node_i;
                        in.read((char*) &node_i, sizeof(unsigned int));

                        this->communication_map[i].get_nodes().push_back(node_i);
                    }
                }
            }
        }
    }
}