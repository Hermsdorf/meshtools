#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <sstream>
#include <cassert>

#include "meshtools.h"
#include "metis.h"
#include "mesh.h"
#include "rcm.hpp"

#include "hdf5_helper.h"

size_t get_mesh_type_size(MeshDataType type)
{
    switch(type)
    {
        case UInt8: return sizeof(unsigned short);
        case Int8: return sizeof(short);
        case UInt32: return sizeof(unsigned int);
        case Int32: return sizeof(int);
        case Float32: return sizeof(float);
        case Float64: return sizeof(double);
        default:
            return 0;
    }
}


bool is_mesh_data_type_valid(MeshDataType type)
{
    switch(type)
    {
        case UInt8:
        case Int8:
        case UInt32:
        case Int32:
        case Float32:
        case Float64:
            return 1;
        default:
            return 0;
    }
}

MeshIODataAppended::MeshIODataAppended()
{
    this->time = -1.0;
    this->time_step = -1;
}

void MeshIODataAppended::addPointDataInfo(const char* name, MeshDataType type, void *data_ptr)
{
    PointData tmp;
    assert(is_mesh_data_type_valid(type));
    tmp.name = name;
    tmp.data = data_ptr;
    tmp.type = type;
    this->list_point_data.push_back(tmp);
}

void MeshIODataAppended::addCellDataInfo(const char* name, MeshDataType type, void *data_ptr)
{
    CellData tmp;
    assert(is_mesh_data_type_valid(type));
    tmp.name = name;
    tmp.data = data_ptr;
    tmp.type = type;
    this->list_cell_data.push_back(tmp);
}

void MeshIODataAppended::addTimeDataInfo(double time, int timestep)
{
    this->time = time;
    this->time_step = timestep;
}

static int element_type[6] = {-1, 2, 3, 4, 4, 8};
static int element_dim[6]  = { 0, 1, 2, 2, 3, 3};

int getGmshElemNNodes(int type)
{
    switch (type)
    {
        case 1: return 2;
        case 2: return 3;
        case 3: return 4;
        case 4: return 4;
        case 5: return 8;
        case 15: return 1;
        default: return -1;
        break;
    }
}

int getGmshElemTypeDim(int type)
{
    switch (type)
    {
        case 1: return 1; 
        case 2: return 2; 
        case 3: return 2;
        case 4: return 3;
        case 5: return 3;
        case 15: return 0;
        default: return -1;
        break;
    }
}

int GmshToVTKType(int type)
{
    switch (type)
    {
        case 1: return 3; // EDGE2
        case 2: return 5; // TRI3
        case 3: return 9; // QUAD4
        case 4: return 10; // TET4
        case 5: return 12; // HEX8
        case 15: return 1;
        default: return -1;
        break;

    }
}


void Mesh::MeshGmshReader(const char* filename)
{

    int format=0, size=0;
    double version = 1.0;
    bool binary_file = false;
    std::string s;

    int dim_count[4] = {0};

    std::ifstream in(filename);

    if(!in.is_open())
    {
        std::cout << "ERRO: Nao foi possivel abrir o arquivo: " << filename << "\n";
        exit(1);
    }

    std::cout << "Reading file " << filename << "\n";

    while(!in.eof())
    {
        // Try to read something.  This may set EOF!
        std::getline(in, s);
        if(in)
        {
            // Process s...
            if(s.find("$MeshFormat") == 0)
            {
                in >> version >> format >> size;

                if(version != 2.2)
                {  
                    std::cout << "ERRO: VERSAO .MSH NAO SUPORTADA\n";
                    std::cout << "Versão suportada: 2.2\nFormato aberto: " << s << "\n";
                    in.close();
                    delete this;
                    exit(1);
                }

                if(format)
                {
                    int current_position = in.tellg(); // saving last position readed from in fstream
                    in.close();

                    binary_file = true;
                    in = std::ifstream(filename, std::ios::binary);
                    in.seekg(current_position); // continue from the same position that was reading fstream file
                }

            }
            // Read and process the "PhysicalNames" section.
            else if (s.find("$PhysicalNames") == 0)
            {
                // Read in the number of physical groups to expect in the file.
                unsigned int num_physical_groups = 0;
                in >> num_physical_groups;
                
                std::string phy_name;
                for (unsigned int i=0; i<num_physical_groups; ++i)
                {                    
                    int phy_dim, phy_id;

                    in >> phy_dim >> phy_id >> phy_name;

                    this->physical_map[phy_id] = std::make_pair(phy_dim, phy_name);
                }
            }
            else if(s.find("$Nodes") == 0) 
            {
                unsigned int num_nodes = 0;

                in >> num_nodes;
                this->n_nodes = num_nodes;

                this->node_index.resize(num_nodes);
                this->coord.resize(num_nodes*3);

                int node_id;

                if(binary_file)
                {   
                    double xyz[3];
                    int current_pos = in.tellg();
                    in.seekg(current_pos+1); // i dont know why but it is necessary to get one position ahead of the file to get the correct data

                    for(unsigned int i = 0; i < num_nodes; i++){
                        in.read((char*)&node_index[i], sizeof(int)); // reading node_id
                        in.read((char*)xyz, 3*sizeof(double));       // reading node_i_x, node_i_y and node_i_z

                        this->coord[(i*3)+0] = xyz[0];
                        this->coord[(i*3)+1] = xyz[1];
                        this->coord[(i*3)+2] = xyz[2];
                    }
                }
                else
                { 
                    double x, y, z;
                    for(unsigned int i = 0; i < num_nodes; i++) {
                        in >> node_id >> x >> y >> z;
                        node_index[i]  = node_id;
                        this->coord[(i*3)+0] = x;
                        this->coord[(i*3)+1] = y;
                        this->coord[(i*3)+2] = z;
                    }
                }

                // read the $ENDNOD delimiter
                std::getline(in, s);
            }

            else if (s.find("$Elements")==0)
            {
                int num_elem, node_id;
                in >> num_elem;
#ifdef DEBUG_
                std::cout << " Num. elementos: " << num_elem << "\n";
#endif

                this->physical_tag.resize(num_elem);
                this->offset.resize(num_elem+1);
                this->type.resize(num_elem);
                this->offset[0] = 0;

                if(binary_file)
                {
                    int current_pos = in.tellg();
                    in.seekg(current_pos+1); // i dont know why but it is necessary to get one position ahead of the file to get the correct data

                    int elem_count = 0;

                    // while is there element-header-binary
                    while(true)
                    {
                        int header[3]; // elm_type, num_elm_follow, num_tags
                        in.read((char*)header, 3*sizeof(int));

                        int elm_type = header[0];
                        int num_elm_follow = header[1];
                        int ntags = header[2];

                        for(unsigned int i = 0; i < num_elm_follow; i++)
                        {
                            this->type[elem_count] = GmshToVTKType(elm_type);
                            int nnodes             = getGmshElemNNodes(elm_type);
                            int elem_dim           = getGmshElemTypeDim(elm_type);

                            int arr_size = 3 + nnodes;
                            int data[arr_size]; // num_i, physical, elementary, node_i_1, ... node_i_x
                            in.read((char*)data, arr_size*sizeof(int));

                            if(nnodes < 0 )
                            {
                                std::cout << "\nERROR: ELEMENT TYPE " << elm_type << " INVALID\n";
                                in.close();
                                delete this;
                                exit(1);
                            }

                            dim_count[elem_dim]++;

                            this->physical_tag[elem_count] = data[1];

                            int arr_shift = 3; // shift num_i, physical and elementary values
                            for (unsigned int j=0; j<nnodes; j++)
                            {
                                this->conn.push_back(data[arr_shift + j] - 1);
#ifdef DEBUG_
                                std::cout << node_id << " ";
#endif
                            }

                            this->offset[elem_count+1] =  this->offset[elem_count] + nnodes;

                            elem_count++;
                        }

                        if(elem_count == num_elem)
                            break;
                    }
                }
                else
                {
                    for(int i = 0; i < num_elem; i++)
                    {
                        int id, type, physical=1, elementary=1, nnodes=0, ntags, elem_dim;

                        in >> id >> type >> ntags;
#ifdef DEBUG_                   
                        std::cout << id << "  " << type << "  " << ntags << " ";
#endif

                        this->type[i] = GmshToVTKType(type);

                        nnodes   = getGmshElemNNodes(type);
                        elem_dim = getGmshElemTypeDim(type);

                        if(nnodes < 0 )
                        {
                            std::cout << "\nERROR: ELEMENT TYPE " << type << " INVALID\n";
                            in.close();
                            delete this;
                            exit(1);
                        }

                        dim_count[elem_dim]++;

                        for(int j = 0; j < ntags; j++)
                        {
                            in >> physical;
                            if(j == 0)
                                this->physical_tag[i] = physical;
#ifdef DEBUG_
                            std::cout << physical << " ";
#endif
                        }

                        for (unsigned int j=0; j<nnodes; j++)
                        {
                            in >> node_id;
                            this->conn.push_back(node_id-1);
#ifdef DEBUG_
                            std::cout << node_id << " ";
#endif
                        }

                        this->offset[i+1] =  this->offset[i] + nnodes;
#ifdef DEBUG_
                        std::cout << "\n";
                        std::cout << "OFFSET: " << this->offset[i+1] << " TYPE: "<< this->type[i] << "\n";
#endif                   
                    }
                }

                // read the $ENDELM delimiter
                std::getline(in, s);

            } // End Elem
        } // end if(in)   

    } // end while (true)
       
    if(dim_count[3] != 0)
    {
        this->n_elements      = dim_count[3];
        this->n_face_elements = dim_count[2];
        this->dim = 3;
 

    } else if (dim_count[2] != 0)
    {
        this->n_elements      = dim_count[2];
        this->n_face_elements = dim_count[1];
        this->dim             = 2;
    } else
    {
        this->n_elements      = dim_count[1];
        this->dim = 1;
    }

    this->element_type          = this->type[this->n_face_elements];
    this->boundary_element_type = this->type[0];
    

    std::string str(filename);
    str.resize(str.length()-4);

    this->filename = str;

    std::cout << " Num. Nodes: " << this->n_nodes << "\n";
    std::cout << " Num. Elements: "          << this->n_elements << "\n";
    std::cout << " Num. Boundary Elements: " << this->n_face_elements << "\n";
    std::cout << " Connectivity size: " << this->conn.size() << "\n";

    in.close();
}

bool BinaryBigEndian(void) 
{
    long _v = 1; 
    return ((char*)&_v)[0] ? false : true;
}


void Mesh::WriteVTK(const char* fname, MeshIODataAppended* info )
{
    if(MeshTools::processor_id() == 0)
        std::cout << "Writing VTK ...\n";
    
    std::FILE*         fout;
    char filename[256];

    unsigned int boffset = 0; /* Offset into binary file */
    const char *byte_order = BinaryBigEndian() ? "BigEndian" : "LittleEndian";

    int timestep = info->getTimeStep();

    if (timestep != -1)
        sprintf(filename,"%s_%d_%d_%04d.vtu",fname,MeshTools::n_processors(),MeshTools::processor_id(), timestep);
    else
        sprintf(filename,"%s_%d_%d.vtu",fname,MeshTools::n_processors(),MeshTools::processor_id());
    
    fout = fopen(filename, "wb");

    if(fout)
    {

        // Writting header info
        fprintf(fout, "<VTKFile type=\"UnstructuredGrid\" version=\"1.0\" byte_order=\"%s\" header_type=\"UInt64\">\n", byte_order);
        fprintf(fout, " <UnstructuredGrid>\n");
        fprintf(fout, "  <Piece NumberOfPoints=\"%d\" NumberOfCells=\"%d\">\n", this->n_nodes, this->n_elements);


        // writing mesh info
        fprintf(fout, "   <Points>\n");
        fprintf(fout, "        <DataArray type=\"%s\" Name=\"%s\" NumberOfComponents=\"%d\" format=\"appended\" offset=\"%d\" />\n","Float64","Points",3, boffset);
        boffset += 3*this->n_nodes*sizeof(double) + sizeof(unsigned long);

        fprintf(fout, "   </Points>\n") ;
        fprintf(fout, "   <Cells>\n");

        fprintf(fout, "        <DataArray type=\"%s\" Name=\"%s\" NumberOfComponents=\"%d\" format=\"appended\" offset=\"%d\" />\n","Int32","connectivity",1, boffset);
        int ofs = this->offset[this->n_face_elements];
        int sz  = this->conn.size() - ofs;
        boffset += sz*sizeof(int) + sizeof(unsigned long);


        fprintf(fout, "        <DataArray type=\"%s\" Name=\"%s\" NumberOfComponents=\"%d\" format=\"appended\" offset=\"%d\" />\n","Int32","offsets",1, boffset);
        sz       = this->offset.size() - (this->n_face_elements + 1);
        boffset += sz*sizeof(int) + sizeof(unsigned long);

        fprintf(fout,"        <DataArray type=\"%s\" Name=\"%s\" NumberOfComponents=\"%d\" format=\"appended\" offset=\"%d\" />\n","UInt16","types",1, boffset);
        boffset += this->n_elements*sizeof(unsigned short) + sizeof(unsigned long);
        fprintf(fout, "   </Cells>\n");
        fprintf(fout, "   <PointData>\n");
        fprintf(fout, "        <DataArray type=\"%s\" Name=\"%s\" NumberOfComponents=\"%d\" format=\"appended\" offset=\"%d\" />\n","UInt32", "node-id",1,boffset);
        boffset += this->n_nodes*sizeof(unsigned int) + sizeof(unsigned long);
        if(info != nullptr)
        {
            auto& point_data = info->getPointDataInfo();
            if( point_data.size() != 0)
            {
                // Writting nodal attribute data
                for(int i= 0; i < point_data.size(); ++i)
                {
                   fprintf(fout, "        <DataArray type=\"%s\" Name=\"%s\" NumberOfComponents=\"%d\" format=\"appended\" offset=\"%d\" />\n",MeshDataTypeSTR[point_data[i].type].c_str(), point_data[i].name.c_str(),1,boffset);
                   boffset += this->n_nodes*get_mesh_type_size(point_data[i].type) + sizeof(unsigned long);
                }
                
            }
        }
        fprintf(fout, "   </PointData>\n");
        fprintf(fout, "   <CellData>\n");
        fprintf(fout, "        <DataArray type=\"%s\" Name=\"%s\" NumberOfComponents=\"%d\" format=\"appended\" offset=\"%d\" />\n","UInt32", "tag-id",1,boffset);
        boffset += this->n_elements*sizeof(unsigned int) + sizeof(unsigned long);
        if(info != nullptr)
        {
            auto& cell_data = info->getCellDataInfo();
            if( cell_data.size() != 0)
            {
                // Writting nodal attribute data
                for(int i= 0; i < cell_data.size(); ++i)
                {
                   fprintf(fout, "        <DataArray type=\"%s\" Name=\"%s\" NumberOfComponents=\"%d\" format=\"appended\" offset=\"%d\" />\n",MeshDataTypeSTR[cell_data[i].type].c_str(),cell_data[i].name.c_str(),1,boffset);
                   boffset += this->n_elements*get_mesh_type_size(cell_data[i].type) + sizeof(unsigned long);
                }
               
            }
        }
        fprintf(fout, "   </CellData>\n");
        fprintf(fout, "  </Piece>\n");
        fprintf(fout, " </UnstructuredGrid>\n");
        fprintf(fout, "  <AppendedData encoding=\"raw\">\n");
        fprintf(fout, "_");

        // writting nodes coordinates
        unsigned long nbytes = sizeof(double)*this->n_nodes*3;
        fwrite((void*)&nbytes, sizeof(unsigned long),1,fout);
        fwrite((void*)&this->coord[0], sizeof(double),this->n_nodes*3,fout);
        
        // writting element connectivity
        int nfe = this->n_face_elements;
        int ne = this->n_elements;
        ofs = this->offset[nfe];

        nbytes = sizeof(int)*(this->conn.size() - ofs);
        fwrite((void*)&nbytes, sizeof(unsigned long),1,fout);
        fwrite((void*)&this->conn[ofs], sizeof(int),this->conn.size() - ofs,fout);

        // writting element offsets
        nbytes = sizeof(int)*(this->offset.size() - (nfe + 1));
        fwrite((void*)&nbytes, sizeof(unsigned long),1,fout);
        for(int i = nfe+1 ; i < this->offset.size() ; i++)
        {
            int offset = this->offset[i] - this->offset[nfe];
            fwrite((void*)&offset, sizeof(int), 1,fout);
        }

        // writting types
        nbytes = sizeof(unsigned short)*(ne);
        fwrite((void*)&nbytes, sizeof(unsigned long),1,fout);
        fwrite((void*)&this->type[nfe], sizeof(unsigned short),ne,fout);
        // writing node indices
        nbytes = sizeof(unsigned int)*this->n_nodes;
        fwrite((void*)&nbytes, sizeof(unsigned long),1,fout);
        fwrite((void*)&this->node_index[0],sizeof(unsigned int),this->n_nodes,fout);

        if(info != nullptr)
        {
            auto& point_data = info->getPointDataInfo();
            
            if( point_data.size() != 0)
            {
                
                for(int i= 0; i < point_data.size(); ++i)
                {
                    nbytes = get_mesh_type_size(point_data[i].type)*this->n_nodes;
                    fwrite((void*)&nbytes, sizeof(unsigned long),1,fout);
                    fwrite((void*)point_data[i].data,get_mesh_type_size(point_data[i].type),this->n_nodes,fout);
                }
            }
        }
        fwrite((void*)&this->physical_tag[n_face_elements],sizeof(unsigned int),this->n_elements,fout);

        if(info != nullptr)
        {
            auto& cell_data   = info->getCellDataInfo();
            if( cell_data.size() != 0)
            {
                
                for(int i= 0; i < cell_data.size(); ++i)
                {
                    nbytes = get_mesh_type_size(cell_data[i].type)*this->n_elements;
                    fwrite((void*)&nbytes, sizeof(unsigned long),1,fout);
                    fwrite((void*)cell_data[i].data,get_mesh_type_size(cell_data[i].type),this->n_elements,fout);
                }
            }
        }
    
        fprintf(fout,"\n </AppendedData>\n");
        fprintf(fout,"</VTKFile>");
        fclose(fout);

        if(MeshTools::processor_id() == 0)
            std::cout << "Writing completed successfully\n";
    }
}

void Mesh::WriteMTS(const char *fname)
{
    char filename[256];
    sprintf(filename,"%s_%04d.mts",fname,MeshTools::processor_id());
    FILE* fout = fopen(filename,"w");
    if(!fout) return ;


    fprintf(fout, "# Mesh Tools File \n");
    fprintf(fout, "1.0  0  0 # [version] [0:ascii - 1:binary] [0:serial - 1:parallel]\n");
    fprintf(fout, "%d # num. faces   \n", this->n_face_elements);
    fprintf(fout, "%d # num. elements\n", this->n_elements);
    fprintf(fout, "%d # num. nodes \n",   this->n_nodes);
    fprintf(fout, "%ld # num. physical region\n", this->physical_map.size());
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
    fprintf(fout,"$BEGIN_ELEMENT DATA\n");
    for(int iel = 0; iel < this->n_elements; iel++)
    {
        fprintf(fout,"%-4d %-4d ", iel, this->physical_tag[iel+this->n_face_elements]);
        unsigned int connsize = this->getElementConnSize(iel);
        unsigned int *conn    = this->getElementConn(iel);
        for(int i = 0; i < connsize; ++i)
            fprintf(fout, "%-4d ", conn[i]);
        fprintf(fout,"\n");
    }
    fprintf(fout,"$END_ELEMENT DATA\n");
    fclose(fout);
}



