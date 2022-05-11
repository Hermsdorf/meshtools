#include <iostream>
#include <set>
#include <fstream>
#include <sstream>
#include <cassert>


#include "meshtools.h"
#include "metis.h"
#include "mesh.h"
#include "mesh_part.h"
#include "parallel_mesh.h"

#ifdef USE_MPI
#include "mpi.h"
#endif

#define MAX_STR 128

MeshPartition::MeshPartition()
{
    this->n_partitions = 1;
    this->nodal_part = nullptr;
    this->elem_part = nullptr;
    this->applied  = false;
}

MeshPartition::~MeshPartition()
{
    delete[] this->elem_part;
    delete[] this->nodal_part;
}

int MeshPartition::get_n_partitions()
{
    return this->n_partitions;
}
int *MeshPartition::get_nodal_part()
{
    return this->nodal_part;
}
int *MeshPartition::get_elem_part()
{
    return this->elem_part;
}
void MeshPartition::set_n_partitions(int n_partitions)
{
    this->n_partitions = n_partitions;
}
void MeshPartition::set_nodal_part(int *nodal_part)
{
    this->nodal_part = nodal_part;
}
void MeshPartition::set_elem_part(int *elem_part)
{
    this->elem_part = elem_part;
}

void MeshPartition::ApplyPartitioner(Mesh *mesh, int nparts)
{
    // Only process 0 partitions the mesh and send to others processes
    if(MeshTools::processor_id() != 0 )
        return;

    //this->_use_bnd_elements = use_bnd_elem;
    int nelem  = (int)mesh->get_n_elements();
    int nfe    = (int)mesh->get_n_face_elements();
    int nnodes = (int)mesh->get_n_nodes();

    std::cout << "Partitioning in " << nparts << " parts\n";

    // Allocate a new mesh partition data info and initialize
    this->n_partitions = nparts;
    this->elem_part    = new int[nelem];
    this->nodal_part   = new int[nnodes];
    this->face_part    = new int[nfe];

    for (int i = 0; i < nfe; i++)
        this->face_part[i] = 0;
    for (int i = 0; i < nelem; i++)
        this->elem_part[i] = 0;
    for (int i = 0; i < nnodes; i++)
        this->nodal_part[i] = 0;

    if (nparts <= 1)
    {
        std::cout << "Successfully partitioned\n";
    }
    else
    {
        idx_t *ne = &nelem;
        idx_t *nn = &nnodes;
        idx_t *eptr = new idx_t[nelem+1];
        idx_t *eind = 0;

        eind    = (idx_t *)mesh->getElementConn(0);
        eptr[0] = 0;
        for (int i = 0; i < nelem; i++)
        {
            eptr[i+1] = eptr[i] + mesh->getElementConnSize(i);
        }

        /*
        idx_t *vwgt = new idx_t[nelem+1];

        for (int i =0; i < nelem; i++)
        {
            vwgt[i] = mesh->getElementConnSize(i);
        }
        */
        idx_t *vwgt    = 0;
        idx_t *vsize   = 0;
        real_t *tpwgts = 0;
        idx_t options[METIS_NOPTIONS];
        idx_t objval = 0;
        idx_t ncommon = 1;

        METIS_SetDefaultOptions(options);
        
        //options[METIS_OPTION_OBJTYPE] = METIS_OBJTYPE_VOL;
        if(nparts < 8)
           options[METIS_OPTION_PTYPE]    = METIS_PTYPE_RB;
        else
            options[METIS_OPTION_PTYPE]    = METIS_PTYPE_KWAY;

        //options[METIS_OPTION_OBJTYPE]  = METIS_OBJTYPE_CUT;
        options[METIS_OPTION_NUMBERING] = 0;
        options[METIS_OPTION_CONTIG]    = 1;

        //int metis_return = METIS_PartMeshNodal(ne, nn, eptr, eind, vwgt, vsize, &nparts, tpwgts, options, &objval, this->elem_part, this->nodal_part);
        int metis_return = METIS_PartMeshDual(ne, nn, eptr, eind, vwgt, vsize, &ncommon, &nparts, tpwgts, options, &objval, this->elem_part, this->nodal_part);

        if (metis_return == METIS_OK)
        {
            std::cout << "Successfully partitioned\n";
        }
        else
        {
            if (metis_return == METIS_ERROR_INPUT)
            {
                std::cout << "Input error\n";
                if(eptr != 0) delete [] eptr;
                exit(1);
            }
            else
            {
                if (metis_return == METIS_ERROR_MEMORY)
                {
                    std::cout << "Memory error\n";
                    if(eptr != 0) delete [] eptr;
                    exit(1);
                }
                else
                {
                    std::cout << "Another kind of error\n";
                    if(eptr != 0) delete [] eptr;
                    exit(1);
                }
            }
        }

        std::vector<int> mapnode(nnodes);
        // searching for elements present in partition `p` and mapping it on mapnode vector
        for(int p = 0; p < this->n_partitions; ++p)
        {
            for(int iel = 0; iel < nelem; iel++)
            {
                if(this->elem_part[iel] == p) {
                     int connsz            = mesh->getElementConnSize(iel);
                     unsigned int *connptr = mesh->getElementConn(iel);
                     for(int ino = 0; ino < connsz; ino++)
                     {
                         mapnode[connptr[ino]] = p;
                     }

                }
               
            }
            // partitioning face elements
            for(int iel = 0; iel < nfe; iel++)
            {
                int conn_in_partition_p = 0;
                unsigned int connsz   = mesh->getSurfaceElementConnSize(iel);
                unsigned int *connptr = mesh->getSurfaceElementConn(iel);
                for(int ino =0; ino < connsz; ino++)
                {
                    if(mapnode[connptr[ino]] == p) conn_in_partition_p++;
                }

                // if all conectivities in surface element `iel` is a part of partition p, it will be considered of partition p
                if(conn_in_partition_p == connsz) this->face_part[iel] = p;
            }
        }
    }
    this->applied = true;
}

void MeshPartition::WriteInternalPartition(Mesh *mesh)
{
    std::map<unsigned int, std::set<unsigned int>> node_partition; // < no, particoes que o no participa >
    std::vector<double> coord = mesh->getCoord();
    std::vector<unsigned int> interface_nodes;
    unsigned int nelem = mesh->get_n_elements();
    unsigned int nnodes = mesh->get_n_nodes();

    for (int i = 0; i < this->n_partitions; i++)
    {
        unsigned int elem_num = 0;
        while (elem_num < nelem)
        {
            if (this->elem_part[elem_num] == i) // se o elemento for da particao que estamos processando
            {
                unsigned int *conn = mesh->getElementConn(elem_num);
                unsigned int connsize = mesh->getElementConnSize(elem_num);

                for (int j = 0; j < connsize; j++)
                {
                    unsigned int conn_node = conn[j];
                    node_partition[conn_node].insert(i); // nó conn_node participa da particao i
                }
            }

            elem_num++;
        }
    }

    std::map<unsigned int, std::set<unsigned int>>::iterator it_node;
    for (it_node = node_partition.begin(); it_node != node_partition.end(); it_node++)
    {
        if (it_node->second.size() > 1)
        {
            interface_nodes.push_back(it_node->first);
        }
    }

    std::vector<unsigned int> global_to_local;

    for (int i = 0; i < this->n_partitions; i++)
    {
        global_to_local.resize(mesh->getConn().size());

        std::ofstream fout;
        // create an output string stream
        std::ostringstream os;

        os << i;

        std::string str = mesh->getFilename();
        str.insert(str.length() - 4, "_part" + os.str());

        fout.open(str.c_str());

        std::vector<double> coord_local;
        std::vector<unsigned int> conn_local;
        std::vector<unsigned int> offset_local;
        std::vector<unsigned short> type_local;
        std::vector<unsigned int> local_to_global;

        unsigned int local_node = 0;
        for (int j = 0; j < nnodes; j++)
        {
            if (node_partition[j].count(i))
            {
                coord_local.push_back(coord[(j * 3) + 0]); // x
                coord_local.push_back(coord[(j * 3) + 1]); // y
                coord_local.push_back(coord[(j * 3) + 2]); // z

                local_to_global.push_back(j);
                global_to_local.at(j) = local_node;
                local_node++;
            } // se o no esta presente na particao processada
        }

        unsigned int elem_num = 0;
        unsigned int nelem_part = 0;
        unsigned int offset = 0;
        while (elem_num < nelem)
        {
            if (this->elem_part[elem_num] == i) // se o elemento for da particao que estamos processando
            {
                unsigned int *conn = mesh->getElementConn(elem_num);
                unsigned int connsize = mesh->getElementConnSize(elem_num);

                for (int j = 0; j < connsize; j++)
                    conn_local.push_back(global_to_local[conn[j]]);

                offset_local.push_back(offset);
                type_local.push_back(mesh->getElementType(elem_num));
                offset += connsize;
                nelem_part++;
            }

            elem_num++;
        }
        offset_local.push_back(offset);

        std::map<unsigned int, std::set<unsigned int>> shared_nodes; // <particao p, lista de nos compartilhados entre particao p e particao i>
        std::vector<unsigned int>::iterator it_interface;
        std::set<unsigned int>::iterator it_node_set;
        for (it_interface = interface_nodes.begin(); it_interface != interface_nodes.end(); it_interface++)
        {
            unsigned int node = *it_interface;
            if (node_partition[node].count(i))
            {
                for (it_node_set = node_partition[node].begin(); it_node_set != node_partition[node].end(); it_node_set++)
                {
                    unsigned int partition = *it_node_set;
                    if (partition != i)
                        shared_nodes[partition].insert(node);
                }
            } // se o no, que ja eh de interface, for da particao que estamos processando
        }

        fout << nelem_part << std::endl;
        fout << local_node << std::endl;
        fout << conn_local.size() << std::endl;

        fout << "COORD_LOCAL: \n";
        std::vector<double>::iterator coord_it;
        for (coord_it = coord_local.begin(); coord_it != coord_local.end(); coord_it += 3)
        {
            fout << *coord_it << " " << *(coord_it + 1) << " " << *(coord_it + 2) << "\n";
        }

        fout << "\nCONN_LOCAL: \n";
        std::vector<unsigned int>::iterator conn_it;
        for (conn_it = conn_local.begin(); conn_it != conn_local.end(); conn_it++)
        {
            fout << *conn_it << " ";
        }
        fout << "\n";

        fout << "\nOFFSET_LOCAL: \n";
        std::vector<unsigned int>::iterator offset_it;
        for (offset_it = offset_local.begin(); offset_it != offset_local.end(); offset_it++)
        {
            fout << *offset_it << " ";
        }
        fout << "\n";

        fout << "\nTYPE_LOCAL: \n";
        std::vector<unsigned short>::iterator type_it;
        for (type_it = type_local.begin(); type_it != type_local.end(); type_it++)
        {
            fout << *type_it << " ";
        }
        fout << "\n";

        fout << "\nLOCAL_TO_GLOBAL: \n";
        std::vector<unsigned int>::iterator local_global_it;
        for (local_global_it = local_to_global.begin(); local_global_it != local_to_global.end(); local_global_it++)
        {
            fout << *local_global_it << " ";
        }
        fout << "\n";

        fout << "\nSHARED NODES: \n";
        fout << shared_nodes.size() << "\n";
        for (auto it = shared_nodes.begin(); it != shared_nodes.end(); it++)
        {
            unsigned int partition = it->first;
            if (partition != i)
            {
                fout << partition << " " << shared_nodes[partition].size() << " ";
                for (auto it2 = shared_nodes[partition].begin(); it2 != shared_nodes[partition].end(); it2++)
                    fout << global_to_local[*it2] << " ";
                fout << "\n";
            }
        } // <Particao compartilhada> <n_nodes shared> <list nodes -> local_to_global>

        coord_local.clear();
        conn_local.clear();
        offset_local.clear();
        type_local.clear();
        local_to_global.clear();
        global_to_local.clear();
        shared_nodes.clear();

        fout.close();
    }
}

void MeshPartition::WriteInternalPartitionBin(Mesh *mesh)
{
    std::map<unsigned int, std::set<unsigned int>> node_partition; // < no, particoes que o no participa >
    std::vector<double> &coord = mesh->getCoord();
    std::vector<unsigned int> interface_nodes;
    unsigned int nelem = mesh->get_n_elements();
    unsigned int nnodes = mesh->get_n_nodes();

    for (int i = 0; i < this->n_partitions; i++)
    {
        unsigned int elem_num = 0;
        while (elem_num < nelem)
        {
            if (this->elem_part[elem_num] == i) // se o elemento for da particao que estamos processando
            {
                unsigned int *conn = mesh->getElementConn(elem_num);
                unsigned int connsize = mesh->getElementConnSize(elem_num);

                for (int j = 0; j < connsize; j++)
                {
                    unsigned int conn_node = conn[j];
                    node_partition[conn_node].insert(i); // nó conn_node participa da particao i
                }
            }

            elem_num++;
        }
    }

    std::map<unsigned int, std::set<unsigned int>>::iterator it_node;
    for (it_node = node_partition.begin(); it_node != node_partition.end(); it_node++)
    {
        if (it_node->second.size() > 1)
        {
            interface_nodes.push_back(it_node->first);
        }
    }

    std::vector<unsigned int> global_to_local;

    for (int i = 0; i < this->n_partitions; i++)
    {
        global_to_local.resize(mesh->getConn().size());

        std::ofstream fout;
        // create an output string stream
        std::ostringstream os;

        os << i;

        std::string str = mesh->getFilename();
        str.insert(str.length() - 4, "_part" + os.str());

        fout.open(str.c_str(), std::ios::out | std::ios::binary);

        std::vector<double> coord_local;
        std::vector<unsigned int> conn_local;
        std::vector<unsigned int> offset_local;
        std::vector<unsigned short> type_local;
        std::vector<unsigned int> local_to_global;

        int local_node = 0;
        for (int j = 0; j < nnodes; j++)
        {
            if (node_partition[j].count(i))
            {
                coord_local.push_back(coord[(j * 3) + 0]); // x
                coord_local.push_back(coord[(j * 3) + 1]); // y
                coord_local.push_back(coord[(j * 3) + 2]); // z

                local_to_global.push_back(j);
                global_to_local.at(j) = local_node;
                local_node++;
            } // se o no esta presente na particao processada
        }

        unsigned int elem_num = 0;
        unsigned int offset = 0;
        unsigned int nelem_part = 0;
        while (elem_num < nelem)
        {
            if (this->elem_part[elem_num] == i) // se o elemento for da particao que estamos processando
            {
                unsigned int *conn = mesh->getElementConn(elem_num);
                unsigned int connsize = mesh->getElementConnSize(elem_num);

                for (int j = 0; j < connsize; j++)
                    conn_local.push_back(global_to_local[conn[j]]);

                offset_local.push_back(offset);
                type_local.push_back(mesh->getElementType(elem_num));
                offset += connsize;
                nelem_part++;
            }

            elem_num++;
        }
        offset_local.push_back(offset);

        std::map<unsigned int, std::set<unsigned int>> shared_nodes; // <particao p, lista de nos compartilhados entre particao p e particao i>
        std::vector<unsigned int>::iterator it_interface;
        std::set<unsigned int>::iterator it_node_set;
        for (it_interface = interface_nodes.begin(); it_interface != interface_nodes.end(); it_interface++)
        {
            unsigned int node = *it_interface;
            if (node_partition[node].count(i))
            {
                for (it_node_set = node_partition[node].begin(); it_node_set != node_partition[node].end(); it_node_set++)
                {
                    unsigned int partition = *it_node_set;
                    if (partition != i)
                        shared_nodes[partition].insert(node);
                }
            } // se o no, que ja eh de interface, for da particao que estamos processando
        }

        std::vector<unsigned int> shared_out;
        shared_out.push_back(shared_nodes.size()); // insere a quantidade de particoes que compartilham nos com a particao processada
        std::map<unsigned int, std::set<unsigned int>>::iterator it_shared;
        for (it_shared = shared_nodes.begin(); it_shared != shared_nodes.end(); it_shared++)
        {
            shared_out.push_back(it_shared->first);         // particao
            shared_out.push_back(it_shared->second.size()); // n_nodes compartilhados

            for (it_node_set = it_shared->second.begin(); it_node_set != it_shared->second.end(); it_node_set++)
                shared_out.push_back(global_to_local[*it_node_set]); // no compartilhado
        }

        unsigned int conn_local_size = conn_local.size();

        fout.write((char *)&nelem_part, sizeof(unsigned int));
        fout.write((char *)&local_node, sizeof(unsigned int));
        fout.write((char *)&conn_local_size, sizeof(unsigned int));

        fout << "COORD_LOCAL: \n";
        fout.write((char *)&coord_local[0], coord_local.size() * sizeof(double));

        fout << "\nCONN_LOCAL: \n";
        fout.write((char *)&conn_local[0], conn_local.size() * sizeof(unsigned int));

        fout << "\nOFFSET_LOCAL: \n";
        fout.write((char *)&offset_local[0], offset_local.size() * sizeof(unsigned int));

        fout << "\nTYPE_LOCAL: \n";
        fout.write((char *)&type_local[0], type_local.size() * sizeof(unsigned short));

        fout << "\nLOCAL_TO_GLOBAL: \n";
        fout.write((char *)&local_to_global[0], local_to_global.size() * sizeof(unsigned int));

        fout << "\nSHARED NODES: \n";
        fout.write((char *)&shared_out[0], shared_out.size() * sizeof(unsigned int));

        coord_local.clear();
        conn_local.clear();
        offset_local.clear();
        type_local.clear();
        local_to_global.clear();
        global_to_local.clear();
        shared_nodes.clear();

        fout.close();
    }
}

void MeshPartition::WritePartition(Mesh *mesh)
{
    std::map<unsigned int, std::set<unsigned int>> node_partition; // < no, particoes que o no participa >
    std::vector<double> coord = mesh->getCoord();
    std::vector<unsigned int> interface_nodes;
    unsigned int nelem = mesh->get_n_elements();
    unsigned int nface_elem = mesh->get_n_face_elements();
    unsigned int nnodes = mesh->get_n_nodes();

    for (int i = 0; i < this->n_partitions; i++)
    {
        unsigned int elem_num = 0;

        while (elem_num < nface_elem)
        {
            if (this->elem_part[elem_num] == i) // se o elemento de superficie for da particao que estamos processando
            {
                unsigned int *conn = mesh->getSurfaceElementConn(elem_num);
                unsigned int connsize = mesh->getSurfaceElementConnSize(elem_num);

                for (int j = 0; j < connsize; j++)
                {
                    unsigned int conn_node = conn[j];
                    node_partition[conn_node].insert(i); // nó conn_node participa da particao i
                }
            }

            elem_num++;
        }

        elem_num = 0;

        while (elem_num < nelem)
        {
            if (this->elem_part[elem_num + nface_elem] == i) // se o elemento for da particao que estamos processando
            {
                unsigned int *conn = mesh->getElementConn(elem_num);
                unsigned int connsize = mesh->getElementConnSize(elem_num);

                for (int j = 0; j < connsize; j++)
                {
                    unsigned int conn_node = conn[j];
                    node_partition[conn_node].insert(i); // nó conn_node participa da particao i
                }
            }

            elem_num++;
        }
    }

    std::map<unsigned int, std::set<unsigned int>>::iterator it_node;
    for (it_node = node_partition.begin(); it_node != node_partition.end(); it_node++)
    {
        if (it_node->second.size() > 1)
        {
            interface_nodes.push_back(it_node->first);
        }
    }

    std::vector<unsigned int> global_to_local;

    for (int i = 0; i < this->n_partitions; i++)
    {
        global_to_local.resize(mesh->getConn().size());

        std::ofstream fout;
        // create an output string stream
        std::ostringstream os;

        os << i;

        std::string str = mesh->getFilename();
        str.insert(str.length() - 4, "_part" + os.str());

        fout.open(str.c_str());

        std::vector<double> coord_local;
        std::vector<unsigned int> conn_local;
        std::vector<unsigned int> offset_local;
        std::vector<unsigned short> type_local;
        std::vector<unsigned int> local_to_global;

        unsigned int local_node = 0;
        for (int j = 0; j < nnodes; j++)
        {
            if (node_partition[j].count(i))
            {
                coord_local.push_back(coord[(j * 3) + 0]); // x
                coord_local.push_back(coord[(j * 3) + 1]); // y
                coord_local.push_back(coord[(j * 3) + 2]); // z

                local_to_global.push_back(j);
                global_to_local.at(j) = local_node;
                local_node++;
            } // se o no esta presente na particao processada
        }

        unsigned int elem_num = 0;
        unsigned int nelem_part = 0;
        unsigned int offset = 0;

        while (elem_num < nface_elem)
        {
            if (this->elem_part[elem_num] == i) // se o elemento for da particao que estamos processando
            {
                unsigned int *conn = mesh->getSurfaceElementConn(elem_num);
                unsigned int connsize = mesh->getSurfaceElementConnSize(elem_num);

                for (int j = 0; j < connsize; j++)
                    conn_local.push_back(global_to_local[conn[j]]);

                offset_local.push_back(offset);
                type_local.push_back(mesh->getSurfaceElementType(elem_num));
                offset += connsize;
                nelem_part++;
            }

            elem_num++;
        }
        // offset_local.push_back(offset);

        elem_num = 0;

        while (elem_num < nelem)
        {
            if (this->elem_part[elem_num + nface_elem] == i) // se o elemento for da particao que estamos processando
            {
                unsigned int *conn = mesh->getElementConn(elem_num);
                unsigned int connsize = mesh->getElementConnSize(elem_num);

                for (int j = 0; j < connsize; j++)
                    conn_local.push_back(global_to_local[conn[j]]);

                offset_local.push_back(offset);
                type_local.push_back(mesh->getElementType(elem_num));
                offset += connsize;
                nelem_part++;
            }

            elem_num++;
        }
        offset_local.push_back(offset);

        std::map<unsigned int, std::set<unsigned int>> shared_nodes; // <particao p, lista de nos compartilhados entre particao p e particao i>
        std::vector<unsigned int>::iterator it_interface;
        std::set<unsigned int>::iterator it_node_set;
        for (it_interface = interface_nodes.begin(); it_interface != interface_nodes.end(); it_interface++)
        {
            unsigned int node = *it_interface;
            if (node_partition[node].count(i))
            {
                for (it_node_set = node_partition[node].begin(); it_node_set != node_partition[node].end(); it_node_set++)
                {
                    unsigned int partition = *it_node_set;
                    if (partition != i)
                        shared_nodes[partition].insert(node);
                }
            } // se o no, que ja eh de interface, for da particao que estamos processando
        }

        fout << nelem_part << std::endl;
        fout << local_node << std::endl;
        fout << conn_local.size() << std::endl;

        fout << "COORD_LOCAL: \n";
        std::vector<double>::iterator coord_it;
        for (coord_it = coord_local.begin(); coord_it != coord_local.end(); coord_it += 3)
        {
            fout << *coord_it << " " << *(coord_it + 1) << " " << *(coord_it + 2) << "\n";
        }

        fout << "\nCONN_LOCAL: \n";
        std::vector<unsigned int>::iterator conn_it;
        for (conn_it = conn_local.begin(); conn_it != conn_local.end(); conn_it++)
        {
            fout << *conn_it << " ";
        }
        fout << "\n";

        fout << "\nOFFSET_LOCAL: \n";
        std::vector<unsigned int>::iterator offset_it;
        for (offset_it = offset_local.begin(); offset_it != offset_local.end(); offset_it++)
        {
            fout << *offset_it << " ";
        }
        fout << "\n";

        fout << "\nTYPE_LOCAL: \n";
        std::vector<unsigned short>::iterator type_it;
        for (type_it = type_local.begin(); type_it != type_local.end(); type_it++)
        {
            fout << *type_it << " ";
        }
        fout << "\n";

        fout << "\nLOCAL_TO_GLOBAL: \n";
        std::vector<unsigned int>::iterator local_global_it;
        for (local_global_it = local_to_global.begin(); local_global_it != local_to_global.end(); local_global_it++)
        {
            fout << *local_global_it << " ";
        }
        fout << "\n";

        fout << "\nSHARED NODES: \n";
        fout << shared_nodes.size() << "\n";
        for (auto it = shared_nodes.begin(); it != shared_nodes.end(); it++)
        {
            unsigned int partition = it->first;
            if (partition != i)
            {
                fout << partition << " " << shared_nodes[partition].size() << " ";
                for (auto it2 = shared_nodes[partition].begin(); it2 != shared_nodes[partition].end(); it2++)
                    fout << global_to_local[*it2] << " ";
                fout << "\n";
            }
        } // <Particao compartilhada> <n_nodes shared> <list nodes -> local_to_global>

        coord_local.clear();
        conn_local.clear();
        offset_local.clear();
        type_local.clear();
        local_to_global.clear();
        global_to_local.clear();
        shared_nodes.clear();

        fout.close();
    }
}

void MeshPartition::WritePartitionBin(Mesh *mesh)
{
    std::map<unsigned int, std::set<unsigned int>> node_partition; // < no, particoes que o no participa >
    std::vector<double> &coord = mesh->getCoord();
    std::vector<unsigned int> interface_nodes;
    unsigned int nelem = mesh->get_n_elements();
    unsigned int nface_elem = mesh->get_n_face_elements();
    unsigned int nnodes = mesh->get_n_nodes();

    for (int i = 0; i < this->n_partitions; i++)
    {
        unsigned int elem_num = 0;

        while (elem_num < nface_elem)
        {
            if (this->elem_part[elem_num] == i) // se o elemento de superficie for da particao que estamos processando
            {
                unsigned int *conn = mesh->getSurfaceElementConn(elem_num);
                unsigned int connsize = mesh->getSurfaceElementConnSize(elem_num);

                for (int j = 0; j < connsize; j++)
                {
                    unsigned int conn_node = conn[j];
                    node_partition[conn_node].insert(i); // nó conn_node participa da particao i
                }
            }

            elem_num++;
        }

        elem_num = 0;

        while (elem_num < nelem)
        {
            if (this->elem_part[elem_num + nface_elem] == i) // se o elemento for da particao que estamos processando
            {
                unsigned int *conn = mesh->getElementConn(elem_num);
                unsigned int connsize = mesh->getElementConnSize(elem_num);

                for (int j = 0; j < connsize; j++)
                {
                    unsigned int conn_node = conn[j];
                    node_partition[conn_node].insert(i); // nó conn_node participa da particao i
                }
            }

            elem_num++;
        }
    }

    std::map<unsigned int, std::set<unsigned int>>::iterator it_node;
    for (it_node = node_partition.begin(); it_node != node_partition.end(); it_node++)
    {
        if (it_node->second.size() > 1)
        {
            interface_nodes.push_back(it_node->first);
        }
    }

    std::vector<unsigned int> global_to_local;

    for (int i = 0; i < this->n_partitions; i++)
    {
        global_to_local.resize(mesh->getConn().size());

        std::ofstream fout;
        // create an output string stream
        std::ostringstream os;

        os << i;

        std::string str = mesh->getFilename();
        str.insert(str.length() - 4, "_part" + os.str());

        fout.open(str.c_str(), std::ios::out | std::ios::binary);

        std::vector<double> coord_local;
        std::vector<unsigned int> conn_local;
        std::vector<unsigned int> offset_local;
        std::vector<unsigned short> type_local;
        std::vector<unsigned int> local_to_global;

        int local_node = 0;
        for (int j = 0; j < nnodes; j++)
        {
            if (node_partition[j].count(i))
            {
                coord_local.push_back(coord[(j * 3) + 0]); // x
                coord_local.push_back(coord[(j * 3) + 1]); // y
                coord_local.push_back(coord[(j * 3) + 2]); // z

                local_to_global.push_back(j);
                global_to_local.at(j) = local_node;
                local_node++;
            } // se o no esta presente na particao processada
        }

        unsigned int elem_num = 0;
        unsigned int offset = 0;
        unsigned int nelem_part = 0;
        while (elem_num < nface_elem)
        {
            if (this->elem_part[elem_num] == i) // se o elemento for da particao que estamos processando
            {
                unsigned int *conn = mesh->getSurfaceElementConn(elem_num);
                unsigned int connsize = mesh->getSurfaceElementConnSize(elem_num);

                for (int j = 0; j < connsize; j++)
                    conn_local.push_back(global_to_local[conn[j]]);

                offset_local.push_back(offset);
                type_local.push_back(mesh->getSurfaceElementType(elem_num));
                offset += connsize;
                nelem_part++;
            }

            elem_num++;
        }
        // offset_local.push_back(offset);

        elem_num = 0;

        while (elem_num < nelem)
        {
            if (this->elem_part[elem_num + nface_elem] == i) // se o elemento for da particao que estamos processando
            {
                unsigned int *conn = mesh->getElementConn(elem_num);
                unsigned int connsize = mesh->getElementConnSize(elem_num);

                for (int j = 0; j < connsize; j++)
                    conn_local.push_back(global_to_local[conn[j]]);

                offset_local.push_back(offset);
                type_local.push_back(mesh->getElementType(elem_num));
                offset += connsize;
                nelem_part++;
            }

            elem_num++;
        }
        offset_local.push_back(offset);

        std::map<unsigned int, std::set<unsigned int>> shared_nodes; // <particao p, lista de nos compartilhados entre particao p e particao i>
        std::vector<unsigned int>::iterator it_interface;
        std::set<unsigned int>::iterator it_node_set;
        for (it_interface = interface_nodes.begin(); it_interface != interface_nodes.end(); it_interface++)
        {
            unsigned int node = *it_interface;
            if (node_partition[node].count(i))
            {
                for (it_node_set = node_partition[node].begin(); it_node_set != node_partition[node].end(); it_node_set++)
                {
                    unsigned int partition = *it_node_set;
                    if (partition != i)
                        shared_nodes[partition].insert(node);
                }
            } // se o no, que ja eh de interface, for da particao que estamos processando
        }

        std::vector<unsigned int> shared_out;
        shared_out.push_back(shared_nodes.size()); // insere a quantidade de particoes que compartilham nos com a particao processada
        std::map<unsigned int, std::set<unsigned int>>::iterator it_shared;
        for (it_shared = shared_nodes.begin(); it_shared != shared_nodes.end(); it_shared++)
        {
            shared_out.push_back(it_shared->first);         // particao
            shared_out.push_back(it_shared->second.size()); // n_nodes compartilhados

            for (it_node_set = it_shared->second.begin(); it_node_set != it_shared->second.end(); it_node_set++)
                shared_out.push_back(global_to_local[*it_node_set]); // no compartilhado
        }

        unsigned int conn_local_size = conn_local.size();

        fout.write((char *)&nelem_part, sizeof(unsigned int));
        fout.write((char *)&local_node, sizeof(unsigned int));
        fout.write((char *)&conn_local_size, sizeof(unsigned int));

        fout << "COORD_LOCAL: \n";
        fout.write((char *)&coord_local[0], coord_local.size() * sizeof(double));

        fout << "\nCONN_LOCAL: \n";
        fout.write((char *)&conn_local[0], conn_local.size() * sizeof(unsigned int));

        fout << "\nOFFSET_LOCAL: \n";
        fout.write((char *)&offset_local[0], offset_local.size() * sizeof(unsigned int));

        fout << "\nTYPE_LOCAL: \n";
        fout.write((char *)&type_local[0], type_local.size() * sizeof(unsigned short));

        fout << "\nLOCAL_TO_GLOBAL: \n";
        fout.write((char *)&local_to_global[0], local_to_global.size() * sizeof(unsigned int));

        fout << "\nSHARED NODES: \n";
        fout.write((char *)&shared_out[0], shared_out.size() * sizeof(unsigned int));

        coord_local.clear();
        conn_local.clear();
        offset_local.clear();
        type_local.clear();
        local_to_global.clear();
        global_to_local.clear();
        shared_nodes.clear();

        fout.close();
    }
}

void MeshPartition::GetNodePartition(Mesh* mesh, std::map<unsigned int, std::set<unsigned int> > &node_partition)
{
    for (int iel = 0; iel < mesh->get_n_elements(); ++iel)
    {
            unsigned int *conn    = mesh->getElementConn(iel);
            unsigned int connsize = mesh->getElementConnSize(iel);
            for(int ino = 0; ino < connsize; ino++)
                node_partition[conn[ino]].insert(this->elem_part[iel]);
    }
//#define _DEBUG
#ifdef _DEBUG
    for(auto it = node_partition.begin(); it != node_partition.end(); ++it){
        std::cout << it->first << " shared with ";
        for(auto p = it->second.begin(); p != it->second.end(); ++p)
            std::cout << *p << " ";
        std::cout << std::endl;
    }
#endif
}

void MeshPartition::GetAndSendLocalData(Mesh *mesh, int sendto,
    int *array_sizes,
    std::map<unsigned int, std::set<unsigned int> > &node_partition,
    std::vector<double>         & coord,
    std::vector<unsigned int>   & l2g, 
    std::vector<unsigned int>   & conn,
    std::vector<unsigned int>   & offset,
    std::vector<unsigned short> & type,
    std::vector<int>            & tag,
    std::vector<unsigned int>   & neighbors,
    std::vector<unsigned int>   & neighbors_offset,
    std::vector<unsigned int>   & neighbors_nodes,
    bool                        enable_send
)
{
  
    unsigned int nelem             = mesh->get_n_elements();
    unsigned int nface_elem        = mesh->get_n_face_elements();
    unsigned int nnodes            = mesh->get_n_nodes();
    auto&    coord_orig = mesh->getCoord();
    auto&    tag_orig   = mesh->getPhysicalTag();
    auto&    map        = mesh->getPhysicalMap();
    

    MPI_Request requests[8];
    MPI_Status  status[8];

    std::map<unsigned int, unsigned int>           g2l;

    coord.clear();
    l2g.clear();
    conn.clear();
    offset.clear();
    type.clear();
    tag.clear();
    neighbors.clear();
    neighbors_offset.clear();
    neighbors_nodes.clear();

    unsigned int iel              = 0;
    unsigned int nelem_face_local = 0;
    unsigned int nelem_local      = 0;
    int connSizeTotal             = 0;
    int nnode_local               = 0;


    for (iel = 0; iel < nelem; ++iel)
    {
        if (this->elem_part[iel] == sendto) // if the element that is being processed 
                                            // is going to be sent to process `sendto`
        {
            unsigned int *conn    = mesh->getElementConn(iel);
            unsigned int connsize = mesh->getElementConnSize(iel);
            connSizeTotal += connsize;
            for (int j = 0; j < connsize; j++)
            {
                unsigned int conn_node = conn[j];
                
                // verify if `conn_node` node was already inserted at g2l map
                auto it = g2l.find(conn_node);
                if (it == g2l.end())
                {
                    g2l[conn_node] = nnode_local;
                    l2g.push_back(conn_node);

                    nnode_local++;
                }
            }
            nelem_local++;
        }
    }

    for (iel = 0; iel < nface_elem; ++iel)
    {
        if (this->face_part[iel] == sendto) // if the element that is being processed 
                                            // is going to be sent to process `sendto`
        {
            unsigned int *conn    = mesh->getSurfaceElementConn(iel);
            unsigned int connsize = mesh->getSurfaceElementConnSize(iel);
            connSizeTotal += connsize;

            for (int j = 0; j < connsize; j++)
            {
                unsigned int conn_node = conn[j];

                // verify if `conn_node` node was already inserted at g2l map
                auto it = g2l.find(conn_node);
                if (it == g2l.end())
                {
                    g2l[conn_node] = nnode_local;
                    l2g.push_back(conn_node);

                    nnode_local++;
                }
            }
            nelem_face_local++;
        }
    }

    // Fills nodes_per_processors vector which contains the local nodes present in each processor other than the one is going to be sent
    // its like the inversion of node_partition structure
    auto map_it     = node_partition.begin();
    std::vector< std::set <unsigned int> > nodes_per_processors(this->n_partitions);
    for( ; map_it !=  node_partition.end(); map_it++)
    {
        int node_id          = map_it->first;
        //int local_node_id    = g2l[node_id];
        
        if(map_it->second.count(sendto)>0)
        {
            // obter id do maior processo que contém o nó
            // auto end = map_it->second.end();
            // unsigned int maior_processor = *(--end);

            for(auto list_processors = map_it->second.begin(); list_processors != map_it->second.end(); list_processors++)
            {
                unsigned int processor = *list_processors;
                if(processor != sendto)
                {
                    nodes_per_processors[processor].insert(node_id);
                }
            }
        }
    }

    unsigned int ofs = 0;
    neighbors_offset.push_back(ofs);
    for (int np = 0; np < this->n_partitions; np++)
    {
        if (np != sendto && nodes_per_processors[np].size() != 0)
        {
                neighbors.push_back(np);
                neighbors_offset.push_back(ofs + nodes_per_processors[np].size());
                ofs      += nodes_per_processors[np].size();
                for(auto it_list = nodes_per_processors[np].begin(); it_list != nodes_per_processors[np].end(); it_list++)
                    neighbors_nodes.push_back(g2l[*it_list]);
        }
    }

    array_sizes[0] = nface_elem;
    array_sizes[1] = nelem_face_local;
    array_sizes[2] = nelem;
    array_sizes[3] = nelem_local;
    array_sizes[4] = nnodes;
    array_sizes[5] = nnode_local;
    array_sizes[6] = connSizeTotal;
    array_sizes[7] = neighbors.size();
    array_sizes[8] = neighbors_offset.size();
    array_sizes[9] = neighbors_nodes.size();
    //array_sizes[10] = map.size();

    if(enable_send)
        MPI_Isend(array_sizes, 11, MPI_INT, sendto, 0, MPI_COMM_WORLD, &requests[0]);

    // Gets local coordinates to send it
    coord.resize(nnode_local*3);
    assert(l2g.size() == nnode_local);

    // Fills coord vector
    for (int n = 0; n < nnode_local; n++)
    {
        int ng           = l2g[n];
        coord[3 * n]     = coord_orig[ng * 3];
        coord[3 * n + 1] = coord_orig[ng * 3 + 1];
        coord[3 * n + 2] = coord_orig[ng * 3 + 2];
        
    }

    if(enable_send) {

        MPI_Wait(&requests[0], &status[0]);

        // Sending nodal data
        MPI_Isend(&coord[0], coord.size(), MPI_DOUBLE, sendto, 0, MPI_COMM_WORLD, &requests[0]);
        MPI_Isend(&l2g[0]  , l2g.size(), MPI_UNSIGNED, sendto, 0, MPI_COMM_WORLD, &requests[1]);
    }

    // Gathering local data from element data
    conn.resize(connSizeTotal);
    offset.resize(nelem_face_local + nelem_local + 1);
    type.resize(nelem_face_local + nelem_local);
    tag.resize(nelem_face_local + nelem_local);

    offset[0]       = 0;
    int nc          = 0;
    int iel_local   = 0;

    // Fills conn and offset vectors with surface elements
    for (iel = 0; iel < nface_elem; ++iel)
    {
        if (this->face_part[iel] == sendto) // if the element that is being processed 
                                            // is going to be sent to process `sendto`
        {
            unsigned int *conn_ptr      = mesh->getSurfaceElementConn(iel);
            unsigned int connsize       = mesh->getSurfaceElementConnSize(iel);
            offset[iel_local + 1] = offset[iel_local] + connsize;
            for (int i = 0; i < connsize; i++) 
                conn[nc++] = g2l[conn_ptr[i]];
                
            type[iel_local] = mesh->getSurfaceElementType(iel);
            tag[iel_local] = tag_orig[iel];
            iel_local++;
        }
    }

    if(enable_send)
        MPI_Waitall(2, &requests[0], &status[0]);

    // Fills conn and offset vectors with internal elements
    for (int iel = 0; iel < nelem; ++iel)
    {
        if (this->elem_part[iel] == sendto) // if the element that is being processed 
                                            // is going to be sent to process `sendto`
        {
            unsigned int *conn_ptr = mesh->getElementConn(iel);
            unsigned int connsize  = mesh->getElementConnSize(iel);
            offset[iel_local + 1] = offset[iel_local] + connsize;
            for (int i = 0; i < connsize; i++)
                   conn[nc++] = g2l[conn_ptr[i]];
            type[iel_local] = mesh->getElementType(iel);
            tag[iel_local]  = tag_orig[iel+nface_elem];
            iel_local++;
        }
    }

    if(enable_send) 
    {
        MPI_Isend(&conn[0]             , conn.size()             , MPI_UNSIGNED      , sendto, 0, MPI_COMM_WORLD, &requests[0]);
        MPI_Isend(&offset[0]           , offset.size()           , MPI_UNSIGNED      , sendto, 0, MPI_COMM_WORLD, &requests[1]);
        MPI_Isend(&type[0]             , type.size()             , MPI_UNSIGNED_SHORT, sendto, 0, MPI_COMM_WORLD, &requests[2]);
        MPI_Isend(&neighbors[0]        , neighbors.size()        , MPI_UNSIGNED      , sendto, 0, MPI_COMM_WORLD, &requests[3]);
        MPI_Isend(&neighbors_offset[0] , neighbors_offset.size() , MPI_UNSIGNED      , sendto, 0, MPI_COMM_WORLD, &requests[4]);
        MPI_Isend(&neighbors_nodes[0]  , neighbors_nodes.size()  , MPI_UNSIGNED      , sendto, 0, MPI_COMM_WORLD, &requests[5]);
        MPI_Isend(&tag[0]              , tag.size()              , MPI_UNSIGNED      , sendto, 0, MPI_COMM_WORLD, &requests[6]);
        MPI_Waitall(7,&requests[0],&status[0]);
    }


}


ParallelMesh* MeshPartition::RecvLocalDataFromMaster()
{
    int          array_sizes[12];
    MPI_Status   status;

    ParallelMesh* pmesh = new ParallelMesh();

    std::cout <<"Processor " << MeshTools::processor_id() <<" receving data form 0" << std::endl;
    MPI_Recv(array_sizes, 11, MPI_INT,0, 0, MPI_COMM_WORLD, &status);

    int n_faces_global    = array_sizes[0];
    int n_faces_local     = array_sizes[1];
    int n_elements_global = array_sizes[2];
    int n_elements_local  = array_sizes[3]; 
    int n_nodes           = array_sizes[4];
    int n_nodes_local     = array_sizes[5];
    int connsize          = array_sizes[6];
    int n_neigbors        = array_sizes[7];
    int neihg_ofs         = array_sizes[8];
    int neig_nodes        = array_sizes[9];
    int n_physical        = array_sizes[10];

    pmesh->set_n_face_elements(n_faces_local);
    pmesh->set_n_elements(n_elements_local);
    pmesh->set_n_nodes(n_nodes_local);
    pmesh->set_n_global_elements(n_elements_global);
    pmesh->set_n_global_face_elements(n_faces_global);
    pmesh->set_n_global_nodes(n_nodes);
    pmesh->set_n_neighbor_processors(n_neigbors);

    auto & _coords = pmesh->getCoord();
    auto & _conn   = pmesh->getConn();
    auto & _l2g    = pmesh->getLocal2Global();
    auto & _type   = pmesh->getType();
    auto & _tag    = pmesh->getPhysicalTag(); 
    auto & _offset = pmesh->getOffset();
    auto & _neighbors           = pmesh->getNeigborsProcessors();
    auto & _shared_nodes_offset = pmesh->getSharedNodesOffset();
    auto & _shared_nodes        = pmesh->getSharedNodes();

    int n_total_elements = n_elements_local+n_faces_local;

    _coords.resize(n_nodes_local*3);
    _l2g.resize(n_nodes_local);
    _conn.resize(connsize);
    _offset.resize(n_total_elements+1);
    _type.resize(n_total_elements);
    _tag.resize(n_total_elements);
    _neighbors.resize(n_neigbors);
    _shared_nodes_offset.resize(neihg_ofs);
    _shared_nodes.resize(neig_nodes);

    MPI_Recv(&_coords[0], _coords.size(), MPI_DOUBLE  , 0, 0, MPI_COMM_WORLD, &status);
    MPI_Recv(&_l2g[0]   , _l2g.size()   , MPI_UNSIGNED, 0, 0, MPI_COMM_WORLD, &status);

    MPI_Recv(&_conn[0]        , _conn.size()  , MPI_UNSIGNED      , 0, 0, MPI_COMM_WORLD, &status);
    MPI_Recv(&_offset[0]      , _offset.size(), MPI_UNSIGNED      , 0, 0, MPI_COMM_WORLD, &status);
    MPI_Recv(&_type[0]        , _type.size()  , MPI_UNSIGNED_SHORT, 0, 0, MPI_COMM_WORLD, &status);
    MPI_Recv(&_neighbors[0]   , _neighbors.size()    , MPI_UNSIGNED      , 0, 0, MPI_COMM_WORLD, &status);
    MPI_Recv(&_shared_nodes_offset[0] , _shared_nodes_offset.size() , MPI_UNSIGNED      , 0, 0, MPI_COMM_WORLD, &status);
    MPI_Recv(&_shared_nodes[0]  , _shared_nodes.size()     , MPI_UNSIGNED      , 0, 0, MPI_COMM_WORLD, &status);
    MPI_Recv(&_tag[0]  , _tag.size()     , MPI_UNSIGNED      , 0, 0, MPI_COMM_WORLD, &status);

    return pmesh;

}


void MeshPartition::WriteDistributedMesh(Mesh* mesh, int processor, int n_processors, const char* basename)
{
    int array_sizes[12];
    std::vector<double>          coords;
    std::vector<unsigned int>    l2g;
    std::vector<unsigned int>    conn;
    std::vector<unsigned int>    offset;
    std::vector<unsigned short>  type;
    std::vector<int>             tag;
    std::vector<unsigned int>    neighbors;
    std::vector<unsigned int>    neighbors_offset;
    std::vector<unsigned int>    neighbors_nodes;
    std::map<unsigned int, std::set<unsigned int> > node_partition;
    this->GetNodePartition(mesh,node_partition);
    for(int p = 0; p < n_processors; p++)
    {

        this->GetAndSendLocalData(mesh,p,array_sizes,node_partition,coords,l2g,conn,offset,type,tag,neighbors,neighbors_offset,neighbors_nodes,false);
        this->WritePartionData(basename,p,array_sizes,coords,l2g,conn,offset,type,tag,neighbors,neighbors_offset,neighbors_nodes);
    }
}
void MeshPartition::WriteAscii(Mesh* mesh, int n_processors, const char *fname)
{
    //std::vector<double> &coord = mesh->getCoord();
    char filename[50];
    sprintf(filename,"%s_%04d.dat",fname,n_processors);
    FILE* fout = fopen(filename,"w");
    if(!fout) return ;

    int n_faces = mesh->get_n_face_elements();
    std::vector<double>&     coord = mesh->getCoord();
    std::vector<int>   &     tag   = mesh->getPhysicalTag();
    auto& map                      = mesh->getPhysicalMap();

    fprintf(fout, "# Mesh Tools File \n");
    fprintf(fout, "1.0  0  0 # [version] [0:ascii - 1:binary] [0:serial - 1:parallel]\n");
    fprintf(fout, "%d # num. faces   \n", mesh->get_n_face_elements());
    fprintf(fout, "%d # num. elements\n", mesh->get_n_elements());
    fprintf(fout, "%d # num. nodes \n",   mesh->get_n_nodes());
    fprintf(fout, "%ld # num. physical region\n", map.size());
    fprintf(fout, "$BEGIN_PHYSICAL_DATA");
    for(auto it = map.begin(); it != map.end(); it++)
        fprintf(fout, "%d %d %s\n", it->first, it->second.first, it->second.second.c_str());
    fprintf(fout, "$END_PHYSICAL_DATA\n"); 
    fprintf(fout, "$BEGIN_NODE_DATA\n");
    for(int n = 0; n < mesh->get_n_nodes(); n++)
        fprintf(fout,"%-4d %-4d %8.8e %8.8e %8.8e\n",n, this->nodal_part[n],coord[n*3],coord[n*3+1], coord[n*3+2]);
    fprintf(fout, "$ENDNODE_DATA\n");
    fprintf(fout,"$BEGIN_BOUNDARY_DATA\n");
    for(int iel = 0; iel <  mesh->get_n_face_elements(); iel++)
    {
        fprintf(fout,"%-4d %-4d", iel, tag[iel]);
        unsigned int connsize = mesh->getSurfaceElementConnSize(iel);
        unsigned int *conn    = mesh->getSurfaceElementConn(iel);
        for(int i = 0; i < connsize; ++i)
            fprintf(fout, "%-4d ", conn[i]);
        fprintf(fout,"\n");
    }
    fprintf(fout,"$END_BOUNDARY_DATA\n");
    fprintf(fout,"$BEGIN_ELEMENT DATA\n");
    for(int iel = 0; iel < mesh->get_n_elements(); iel++)
    {
        fprintf(fout,"%-4d %-4d ", iel, tag[iel+n_faces]);
        unsigned int connsize = mesh->getElementConnSize(iel);
        unsigned int *conn    = mesh->getElementConn(iel);
        for(int i = 0; i < connsize; ++i)
            fprintf(fout, "%-4d ", conn[i]);
        fprintf(fout,"\n");
    }
    fprintf(fout,"$END_ELEMENT DATA\n");
    fclose(fout);
}

void MeshPartition::WritePartionData(
    const char* basename,
    int processor,
    int *array_sizes,
    std::vector<double>         & coord,
    std::vector<unsigned int>   & l2g, 
    std::vector<unsigned int>   & conn,
    std::vector<unsigned int>   & offset,
    std::vector<unsigned short> & type,
    std::vector<int>            & tag,
    std::vector<unsigned int>   & neighbors,
    std::vector<unsigned int>   & neighbors_offset,
    std::vector<unsigned int>   & neighbors_nodes)
{
    char filename[50];
    sprintf(filename,"%s_%04d.dat",basename,processor);
    FILE* fout = fopen(filename,"w");
    if(fout) {
        int n_faces_global    = array_sizes[0];
        int n_faces_local     = array_sizes[1];
        int n_elements_global = array_sizes[2];
        int n_elements_local  = array_sizes[3]; 
        int n_nodes           = array_sizes[4];
        int n_nodes_local     = array_sizes[5];
        int connsize          = array_sizes[6];
        int n_neigbors        = array_sizes[7];
        int neihg_ofs         = array_sizes[8];
        int neig_nodes        = array_sizes[9];
    
        fprintf(fout, "# Mesh File version 0.1\n");
        fprintf(fout, "%d %d # num. faces global e local\n", n_faces_global, n_faces_local);
        fprintf(fout, "%d %d # num. elements global e local\n",n_elements_global, n_elements_local);
        fprintf(fout, "%d %d # num. nodes global e local\n", n_nodes, n_nodes_local);
        fprintf(fout, "# NODE DATA\n");
        for(int n = 0; n < n_nodes_local; n++)
            fprintf(fout,"%-4d %-4d %-4d %8.8e %8.8e %8.8e\n",n, l2g[n], this->nodal_part[l2g[n]],coord[n*3],coord[n*3+1], coord[n*3+2]);
        fprintf(fout,"# FACE DATA\n");
        for(int iel = 0; iel <  n_faces_local; iel++)
        {
            fprintf(fout,"%-4d %-4d", iel, tag[iel]);
            for(int ios = offset[iel]; ios < offset[iel+1]; ios++)
                fprintf(fout, "%-4d ", conn[ios]);
            fprintf(fout,"\n");
        }
        fprintf(fout,"# ELEMENT DATA \n");
        for(int iel = 0; iel <  n_elements_local; iel++)
        {
            fprintf(fout,"%-4d %-4d ", iel, tag[iel+n_faces_local]);
            for(int ios = offset[iel+n_faces_local]; ios < offset[n_faces_local+iel+1]; ios++)
                fprintf(fout, "%-4d ", conn[ios]);
            fprintf(fout,"\n");
        }
        fprintf(fout ,"PARALLEL DATA\n");
        fprintf(fout, "%ld  # number of neighbor processors \n", neighbors.size());
        for(int i=0; i < neighbors.size(); i++)
            fprintf(fout,"%d ", neighbors[i]);
        fprintf(fout, "  # neighbor processors\n");
        for(int i=0; i < neighbors_offset.size(); i++)
            fprintf(fout,"%d ", neighbors_offset[i]);
        fprintf(fout, " # node offset\n");
        for(int i=0; i < neighbors_nodes.size(); i++)
            fprintf(fout,"%d ", neighbors_nodes[i]);
        fprintf(fout, "\n");

        fclose(fout);
    }
}

ParallelMesh *MeshPartition::DistributedMesh(Mesh *mesh)
{

    int processor_id = MeshTools::processor_id();
    int n_procesors  = MeshTools::n_processors(); 

#ifdef USE_MPI
    int array_sizes[12];

   
    ParallelMesh *pmesh ; 

    // process 0 is responsible to generate local arrays and send it to each processor
    if (processor_id == 0)
    {
         if(!this->applied)
            this->ApplyPartitioner(mesh,n_procesors);

        std::vector<double>          coords;
        std::vector<unsigned int>    l2g;
        std::vector<unsigned int>    conn;
        std::vector<unsigned int>    offset;
        std::vector<unsigned short>  type;
        std::vector<int>             tag;
        std::vector<unsigned int>    neighbors;
        std::vector<unsigned int>    neighbors_offset;
        std::vector<unsigned int>    neighbors_nodes;

        // A map indicanting each process that the node i is present
        //
        //            |
        //       P0   |         P1
        //            |
        //    -------o1---------o2------
        //            |
        //       P2   |         P3
        //            |
        //
        // This node has node_partition[1] = {P0, P1, P2, P3} and node_partition is node_partition[2] = {P1, P3}
        std::map<unsigned int, std::set<unsigned int> > node_partition;
    
        auto & map      = mesh->getPhysicalMap();
        array_sizes[10] = map.size();
    
        // Fills node_partition structure
        this->GetNodePartition(mesh,node_partition);

        // Processing and sending local arrays and variables to each `p` processor other than 0
        for (int p = 1; p < this->n_partitions; p++)
        {
            std::cout << "Sending data to processor " << p << endl;
            this->GetAndSendLocalData(mesh,p,array_sizes,node_partition,coords,l2g,conn,offset,type,tag,neighbors,neighbors_offset,neighbors_nodes,true);
        }

        // Processing and filling local arrays and variables to process 0
        pmesh = new ParallelMesh();

        auto & _coords = pmesh->getCoord();
        auto & _conn   = pmesh->getConn();
        auto & _l2g    = pmesh->getLocal2Global();
        auto & _type   = pmesh->getType();
        auto & _tag    = pmesh->getPhysicalTag(); 
        auto & _offset = pmesh->getOffset();
        auto & _neighbors           = pmesh->getNeigborsProcessors();
        auto & _shared_nodes_offset = pmesh->getSharedNodesOffset();
        auto & _shared_nodes        = pmesh->getSharedNodes();

        this->GetAndSendLocalData(mesh,0,array_sizes,node_partition,_coords,_l2g,_conn,_offset,_type,_tag,_neighbors,_shared_nodes_offset,_shared_nodes,false);
        
        int n_faces_global    = array_sizes[0];
        int n_faces_local     = array_sizes[1];
        int n_elements_global = array_sizes[2];
        int n_elements_local  = array_sizes[3]; 
        int n_nodes           = array_sizes[4];
        int n_nodes_local     = array_sizes[5];
        int connsize          = array_sizes[6];
        int n_neigbors        = array_sizes[7];
        int neihg_ofs         = array_sizes[8];
        int neig_nodes        = array_sizes[9];
        int n_physical        = array_sizes[10];

        pmesh->set_n_face_elements(n_faces_local);
        pmesh->set_n_elements(n_elements_local);
        pmesh->set_n_nodes(n_nodes_local);
        pmesh->set_n_global_elements(n_elements_global);
        pmesh->set_n_global_face_elements(n_faces_global);
        pmesh->set_n_global_nodes(n_nodes);
        pmesh->set_n_neighbor_processors(n_neigbors);

        // Broadcasting Physical Groups
        n_physical = map.size();

        std::vector<int>  map_ids(2*n_physical);
        std::vector<char> map_names(n_physical*MAX_STR);
  
        auto iter = map.begin();
    
        for(int i = 0; iter != map.end(); iter++, i++)
        {
            map_ids[2*i  ]   = iter->first;           // id do grupo fisico
            map_ids[2*i+1]   = iter->second.first;    // dimensao
            strncpy(&map_names[i*MAX_STR],iter->second.second.c_str(),MAX_STR);
            std::cout << iter->first << " ";
            std::cout << iter->second.first << " ";
            std::cout << iter->second.second << std::endl;
            
        }

        MPI_Bcast(&n_physical,1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&map_ids[0],  n_physical*2, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&map_names[0],n_physical*MAX_STR, MPI_CHAR, 0, MPI_COMM_WORLD);

        auto& pmap = pmesh->getPhysicalMap();
        pmap.clear();
        for(int i = 0; i < n_physical; i++)
        {
            std::pair<int,physical_data_t> data;
            char buffer[MAX_STR+1];
            data.first         = map_ids[2*i];
            data.second.first  = map_ids[2*i+1];
            strncpy(buffer,&map_names[i*MAX_STR],MAX_STR-1);
            buffer[MAX_STR] = '\0';
            data.second.second = buffer;
            pmap.insert(data);
        }


    }   
    else
    {
        pmesh = this->RecvLocalDataFromMaster();

        int n_physical;

         // Broadcast Physical Groups
        MPI_Bcast(&n_physical,1, MPI_INT, 0, MPI_COMM_WORLD);

        std::cout <<  "Recevendo n_physical = " << n_physical <<std::endl;

        std::vector<int>  map_ids(2*n_physical);
        std::vector<char> map_names(n_physical*MAX_STR);

        MPI_Bcast(&map_ids[0], n_physical*2, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&map_names[0],n_physical*MAX_STR, MPI_CHAR, 0, MPI_COMM_WORLD);
        
        auto& pmap = pmesh->getPhysicalMap();
        pmap.clear();
        for(int i = 0; i < n_physical; i++)
        {
            std::pair<int,physical_data_t> data;
            char buffer[MAX_STR+1];
            data.first         = map_ids[2*i];
            data.second.first  = map_ids[2*i+1];
            strncpy(buffer,&map_names[i*MAX_STR],MAX_STR-1);
            buffer[MAX_STR] = '\0';
            data.second.second = buffer;
            pmap.insert(data);
        }

        
    }

    return pmesh;
#else
    return nullptr;
#endif
}


void MeshPartition::WriteVTK(Mesh* mesh, const char* fname)
{
    mesh->setFilename(fname);
    mesh->MeshVTKWriterInternal(0,&this->nodal_part[0],&this->elem_part[0]);
}
