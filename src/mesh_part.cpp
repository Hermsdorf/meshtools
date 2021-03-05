#include <iostream>
#include <set>
#include <fstream>
#include <sstream>

#include "metis.h"
#include "mesh.h"
#include "mesh_part.h"

Mesh_partition_t::Mesh_partition_t() 
{
    this->n_partitions = 1;
    this->nodal_part = NULL;
    this->elem_part = NULL;
}

Mesh_partition_t::~Mesh_partition_t()
{
    delete [] this->elem_part;
    delete [] this->nodal_part;
}

int Mesh_partition_t::get_n_partitions()
{
    return this->n_partitions;
}
int* Mesh_partition_t::get_nodal_part()
{
    return this->nodal_part;
}
int* Mesh_partition_t::get_elem_part()
{
    return this->elem_part;
}
void Mesh_partition_t::set_n_partitions(int n_partitions)
{
    this->n_partitions = n_partitions;
}
void Mesh_partition_t::set_nodal_part(int* nodal_part)
{
    this->nodal_part = nodal_part;
}
void Mesh_partition_t::set_elem_part(int* elem_part)
{
    this->elem_part = elem_part;
}

void Mesh_partition_t::MeshPartitionerInternal(Mesh* mesh, int nparts)
{
    int nelem = (int)mesh->get_n_elements();
    int nnodes = (int)mesh->get_n_nodes();
    
    std::cout << "Partitioning in "<< nparts <<" parts\n";

    // Allocate a new mesh partition data info and initialize
    this->n_partitions = nparts;
    this->elem_part    = new int [nelem];
    this->nodal_part   = new int [nnodes];

    for(int i = 0 ; i < nelem ; i++)
        this->elem_part[i] = 0;
    for(int i = 0 ; i < nnodes ; i++)
        this->nodal_part[i] = 0;

    if(nparts <= 1)
    {
        std::cout << "Successfully partitioned\n";
    }
    else
    {
        int ofs          = mesh->getElementOffset(0)[0];
        int *eptr        = new int[nelem + 1];
        
        int* offset_aux = (int*)mesh->getElementOffset(0);
        for (int i = 0, j = 0; i <= nelem ; i++, j++)
        {
            eptr[j] = offset_aux[i] - ofs;
        }


        idx_t *ne       = &nelem;
        idx_t *nn       = &nnodes; 
        idx_t *eind     = (idx_t*)mesh->getElementConn(0);
        idx_t *vwgt     = 0;
        idx_t *vsize    = 0;
        real_t *tpwgts  = 0;
        idx_t options[METIS_NOPTIONS];
        idx_t  objval   = 0;

        METIS_SetDefaultOptions(options);

        //idx_t *epart, idx t *npart
        options[METIS_OPTION_PTYPE]     = METIS_PTYPE_KWAY;
        options[METIS_OPTION_OBJTYPE]   = METIS_OBJTYPE_CUT;
        options[METIS_OPTION_NUMBERING] = 0;

        int metis_return = METIS_PartMeshNodal(ne,nn,eptr,eind,vwgt,vsize, &nparts, tpwgts, options, &objval, this->elem_part, this->nodal_part);
        if (metis_return == METIS_OK)
        {
                std::cout << "Successfully partitioned\n";
        }
        else
        {
            if (metis_return == METIS_ERROR_INPUT)
            {
                std::cout << "Input error\n";
                exit(1);
            }
            else
            {
                if (metis_return == METIS_ERROR_MEMORY)
                {
                    std::cout << "Memory error\n";
                    exit(1);
                }
                else
                {
                    std::cout << "Another kind of error\n";
                    exit(1);
                }
            }
        }

        delete [] eptr;
    }
}


void Mesh_partition_t::MeshPartitioner(Mesh* mesh, int nparts)
{
    int nelem = (int)mesh->get_n_elements();
    int nfe = (int)mesh->get_n_face_elements();
    int nnodes = (int)mesh->get_n_nodes();
    int ntelem = nelem + nfe;

    std::cout << "Partitioning in "<< nparts <<" parts\n";

    // Allocate a new mesh partition data info and initialize
    this->n_partitions = nparts;
    this->elem_part    = new int [ntelem];
    this->nodal_part   = new int [nnodes];

    for(int i = 0 ; i < ntelem ; i++)
        this->elem_part[i] = 0;
    for(int i = 0 ; i < nnodes ; i++)
        this->nodal_part[i] = 0;
    
    if(nparts <= 1)
    {
        std::cout << "Successfully partitioned\n";
    }
    else
    {
        idx_t *ne       = &ntelem;
        idx_t *nn       = &nnodes; 
        idx_t *eptr     = (idx_t*)mesh->getSurfaceElementOffset(0);
        idx_t *eind     = (idx_t*)mesh->getSurfaceElementConn(0);
        idx_t *vwgt     = 0;
        idx_t *vsize    = 0;
        real_t *tpwgts  = 0;
        idx_t options[METIS_NOPTIONS];
        idx_t  objval   = 0;

        METIS_SetDefaultOptions(options);

        options[METIS_OPTION_PTYPE]     = METIS_PTYPE_KWAY;
        options[METIS_OPTION_OBJTYPE]   = METIS_OBJTYPE_CUT;
        options[METIS_OPTION_NUMBERING] = 0;


        int metis_return = METIS_PartMeshNodal(ne,nn, eptr, eind,vwgt,vsize, &nparts, tpwgts, options, &objval, this->elem_part, this->nodal_part);

        if (metis_return == METIS_OK)
        {
                std::cout << "Successfully partitioned\n";
        }
        else
        {
            if (metis_return == METIS_ERROR_INPUT)
            {
                std::cout << "Input error\n";
                exit(1);
            }
            else
            {
                if (metis_return == METIS_ERROR_MEMORY)
                {
                    std::cout << "Memory error\n";
                    exit(1);
                }
                else
                {
                    std::cout << "Another kind of error\n";
                    exit(1);
                }
            }
        }

    }
}

void Mesh_partition_t::WritePartitionInternal(Mesh* mesh)
{
    // TODO: ESCREVER TIPO DOS ELEMENTOS
    
    std::map<unsigned int, std::set<unsigned int>> node_partition; // < no, particoes que o no participa >
    std::vector<double> coord = mesh->getCoord();
    std::vector<unsigned int> interface_nodes;
    unsigned int nelem = mesh->get_n_elements();
    unsigned int nnodes = mesh->get_n_nodes();
    
    for(int i = 0 ; i < this->n_partitions ; i++)
    {
        unsigned int elem_num = 0;
        while(elem_num < nelem)
        {
            if(this->elem_part[elem_num] == i)  // se o elemento for da particao que estamos processando
            {
                unsigned int* conn = mesh->getElementConn(elem_num);
                unsigned int connsize = mesh->getElementConnSize(elem_num);

                for(int j = 0 ; j < connsize ; j++)
                {
                    unsigned int conn_node = conn[j];
                    node_partition[conn_node].insert(i); // nó conn_node participa da particao i
                }
            }

            elem_num++;
        }
    }
    
    std::map<unsigned int, std::set<unsigned int>>::iterator it_node;
    for(it_node = node_partition.begin() ; it_node != node_partition.end() ; it_node++)
    {
        if(it_node->second.size() > 1)
        {
            interface_nodes.push_back(it_node->first);
        }
    }    

    std::vector<unsigned int> global_to_local;
    

    for(int i = 0 ; i < this->n_partitions ; i++)
    {
        global_to_local.resize(mesh->getConn().size());

        std::ofstream fout;
        //create an output string stream
        std::ostringstream os;

        os << i;

        std::string str = mesh->getFilename();
        str.insert(str.length() - 4, "_part" + os.str());

        fout.open(str.c_str());

        std::vector<unsigned int> coord_local;
        std::vector<unsigned int> conn_local;
        std::vector<unsigned int> offset_local;
        std::vector<unsigned int> local_to_global;

        unsigned int local_node = 0;
        for(int j = 0 ; j < nnodes ; j++)
        {
            if(node_partition[j].count(i))
            {
                coord_local.push_back(coord[(j*3) + 0]); // x
                coord_local.push_back(coord[(j*3) + 1]); // y
                coord_local.push_back(coord[(j*3) + 2]); // z

                local_to_global.push_back(j);
                global_to_local.at(j) = local_node;
                local_node++;
            } // se o no esta presente na particao processada
        } 

        unsigned int elem_num = 0;
        unsigned int offset = 0;
        while(elem_num < nelem)
        {
            if(this->elem_part[elem_num] == i)  // se o elemento for da particao que estamos processando
            {
                unsigned int* conn = mesh->getElementConn(elem_num);
                unsigned int connsize = mesh->getElementConnSize(elem_num);

                for(int j = 0 ; j < connsize ; j++)
                    conn_local.push_back(global_to_local[conn[j]]);
                
                offset_local.push_back(offset);
                offset += connsize;
            }

            elem_num++;
        }
        offset_local.push_back(offset);

        std::map<unsigned int, std::set<unsigned int>> shared_nodes; // <particao p, lista de nos compartilhados entre particao p e particao i>
        std::vector<unsigned int>::iterator it_interface;
        std::set<unsigned int>::iterator it_node_set;
        for(it_interface = interface_nodes.begin() ; it_interface != interface_nodes.end() ; it_interface++)
        {
            unsigned int node = *it_interface;
            if(node_partition[node].count(i))
            {
                for(it_node_set = node_partition[node].begin() ; it_node_set != node_partition[node].end() ; it_node_set++)
                {   
                    unsigned int partition = *it_node_set;
                    if(partition != i)
                        shared_nodes[partition].insert(node);     
                }
            } // se o no, que ja eh de interface, for da particao que estamos processando
        }

        fout << elem_num   << std::endl;
        fout << local_node << std::endl;
        fout << conn_local.size()   << std::endl;

        fout << "COORD_LOCAL: \n";
        std::vector<unsigned int>::iterator coord_it;
        for(coord_it = coord_local.begin() ; coord_it != coord_local.end() ; coord_it+=3)
        {
            fout << *coord_it << " " << *(coord_it + 1) << " " << *(coord_it + 2) << "\n";
        }

        fout << "\nCONN_LOCAL: \n";
        std::vector<unsigned int>::iterator conn_it;
        for(conn_it = conn_local.begin() ; conn_it != conn_local.end() ; conn_it++)
        {
            fout << *conn_it << " ";
        }
        fout << "\n";

        fout << "\nOFFSET_LOCAL: \n";
        std::vector<unsigned int>::iterator offset_it;
        for(offset_it = offset_local.begin() ; offset_it != offset_local.end() ; offset_it++)
        {
            fout << *offset_it << " ";
        }
        fout << "\n";

        fout << "\nLOCAL_TO_GLOBAL: \n";
        std::vector<unsigned int>::iterator local_global_it;
        for(local_global_it = local_to_global.begin() ; local_global_it != local_to_global.end() ; local_global_it++)
        {
            fout << *local_global_it << " ";
        }
        fout << "\n";

        fout << "\nSHARED NODES: \n";
        fout << shared_nodes.size() << "\n";
        for(auto it = shared_nodes.begin() ; it != shared_nodes.end() ; it++)
        {
            unsigned int partition = it->first;
            if(partition != i)
            {
                fout << partition << " " << shared_nodes[partition].size() << " ";
                for(auto it2 = shared_nodes[partition].begin() ; it2 != shared_nodes[partition].end() ; it2++)
                    fout << global_to_local[*it2] << " ";
                fout << "\n";
            }
        } // <Particao compartilhada> <n_nodes shared> <list nodes -> local_to_global>
        
        coord_local.clear();
        conn_local.clear();
        offset_local.clear();
        local_to_global.clear();
        global_to_local.clear();
        shared_nodes.clear();

        fout.close();
    }
}



void Mesh_partition_t::WritePartitionInternalBin(Mesh* mesh)
{
    std::map<unsigned int, std::set<unsigned int>> node_partition; // < no, particoes que o no participa >
    std::vector<double> coord = mesh->getCoord();
    std::vector<unsigned int> interface_nodes;
    unsigned int nelem = mesh->get_n_elements();
    unsigned int nnodes = mesh->get_n_nodes();
    
    for(int i = 0 ; i < this->n_partitions ; i++)
    {
        unsigned int elem_num = 0;
        while(elem_num < nelem)
        {
            if(this->elem_part[elem_num] == i)  // se o elemento for da particao que estamos processando
            {
                unsigned int* conn = mesh->getElementConn(elem_num);
                unsigned int connsize = mesh->getElementConnSize(elem_num);

                for(int j = 0 ; j < connsize ; j++)
                {
                    unsigned int conn_node = conn[j];
                    node_partition[conn_node].insert(i); // nó conn_node participa da particao i
                }
            }

            elem_num++;
        }
    }
    
    std::map<unsigned int, std::set<unsigned int>>::iterator it_node;
    for(it_node = node_partition.begin() ; it_node != node_partition.end() ; it_node++)
    {
        if(it_node->second.size() > 1)
        {
            interface_nodes.push_back(it_node->first);
        }
    }
    

    std::vector<unsigned int> global_to_local;

    for(int i = 0 ; i < this->n_partitions ; i++)
    {
        global_to_local.resize(mesh->getConn().size());

        std::ofstream fout;
        //create an output string stream
        std::ostringstream os;

        os << i;

        std::string str = mesh->getFilename();
        str.insert(str.length() - 4, "_part" + os.str());

        fout.open(str.c_str(), std::ios::out | std::ios::binary);

        std::vector<unsigned int> coord_local;
        std::vector<unsigned int> conn_local;
        std::vector<unsigned int> offset_local;
        std::vector<unsigned int> local_to_global;


        int local_node = 0;
        for(int j = 0 ; j < nnodes ; j++)
        {
            if(node_partition[j].count(i))
            {
                coord_local.push_back(coord[(j*3) + 0]); // x
                coord_local.push_back(coord[(j*3) + 1]); // y
                coord_local.push_back(coord[(j*3) + 2]); // z

                local_to_global.push_back(j);
                global_to_local.at(j) = local_node;
                local_node++;
            } // se o no esta presente na particao processada
        } 

        unsigned int elem_num = 0;
        unsigned int offset = 0;
        while(elem_num < nelem)
        {
            if(this->elem_part[elem_num] == i)  // se o elemento for da particao que estamos processando
            {
                unsigned int* conn = mesh->getElementConn(elem_num);
                unsigned int connsize = mesh->getElementConnSize(elem_num);

                for(int j = 0 ; j < connsize ; j++)
                    conn_local.push_back(global_to_local[conn[j]]);
                
                offset_local.push_back(offset);
                offset += connsize;
            }

            elem_num++;
        }
        offset_local.push_back(offset);

        std::map<unsigned int, std::set<unsigned int>> shared_nodes; // <particao p, lista de nos compartilhados entre particao p e particao i>
        std::vector<unsigned int>::iterator it_interface;
        std::set<unsigned int>::iterator it_node_set;
        for(it_interface = interface_nodes.begin() ; it_interface != interface_nodes.end() ; it_interface++)
        {
            unsigned int node = *it_interface;
            if(node_partition[node].count(i))
            {
                for(it_node_set = node_partition[node].begin() ; it_node_set != node_partition[node].end() ; it_node_set++)
                {   
                    unsigned int partition = *it_node_set;
                    if(partition != i)
                        shared_nodes[partition].insert(node);     
                }
            } // se o no, que ja eh de interface, for da particao que estamos processando
        }

        std::vector<unsigned int> shared_out;
        shared_out.push_back(shared_nodes.size()); // insere a quantidade de particoes que compartilham nos com a particao processada
        std::map<unsigned int, std::set<unsigned int>>::iterator it_shared;
        for(it_shared = shared_nodes.begin() ; it_shared != shared_nodes.end() ; it_shared++)
        {
            shared_out.push_back(it_shared->first); // particao
            shared_out.push_back(it_shared->second.size()); // n_nodes compartilhados

            for(it_node_set = it_shared->second.begin() ; it_node_set != it_shared->second.end() ; it_node_set++)
                shared_out.push_back(global_to_local[*it_node_set]); // no compartilhado
        }

        unsigned int conn_local_size = conn_local.size();

        fout.write((char*)&elem_num,sizeof(unsigned int));
        fout.write((char*)&local_node,sizeof(unsigned int));
        fout.write((char*)&conn_local_size,sizeof(unsigned int));

        fout << "COORD_LOCAL: \n";
        fout.write((char*)&coord_local[0],coord_local.size()*sizeof(double));

        fout << "\nCONN_LOCAL: \n";
        fout.write((char*)&conn_local[0],conn_local.size()*sizeof(unsigned int));

        fout << "\nOFFSET_LOCAL: \n";
        fout.write((char*)&offset_local[0],offset_local.size()*sizeof(unsigned int));

        fout << "\nLOCAL_TO_GLOBAL: \n";
        fout.write((char*)&local_to_global[0],local_to_global.size()*sizeof(unsigned int));

        fout << "\nSHARED NODES: \n";
        fout.write((char*)&shared_out[0],shared_out.size()*sizeof(unsigned int));

        coord_local.clear();
        conn_local.clear();
        offset_local.clear();
        local_to_global.clear();
        global_to_local.clear();
        shared_nodes.clear();

        fout.close();
    }


}







