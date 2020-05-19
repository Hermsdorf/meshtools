#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <algorithm>

using namespace std;

#include "metis.h"
#include "mesh.h"
#include "rcm.hpp"

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
        case 1: return 3;
        case 2: return 5;
        case 3: return 9;
        case 4: return 10;
        case 5: return 12;
        case 15: return 1;
    default: return -1;
        break;
    }
}


void MeshGmshReader(mesh_t* mesh, const char* filename)
{

    int format=0, size=0;
    double version = 1.0;
    string s;

    int dim_count[4] = {0};

  

    std::ifstream in(filename);

    if(!in.is_open())
    {
        cout << "ERRO: Nao foi possivel abrir o arquivo: " << filename << endl;
        exit(1);
    }

    cout << "Reading file " << filename << endl;

    //mesh_t *mesh = new mesh_t();

    while(!in.eof())
    {
        // Try to read something.  This may set EOF!
        std::getline(in, s);
        if (in)
        {
            // Process s...
            if(s.find("$MeshFormat") == 0)
            {
                in >> version >> format >> size;
                if(version != 2.2)
                {  
                    cout << "ERRO: VERSAO .MSH NAO SUPORTADA\n";
                    cout << "Versão suportada: 2.2\nFormato aberto: " << s << "\n";
                    in.close();
                    delete mesh;
                    exit(1);
                }

                if(format)
                {
                    cout << "ERRO: FORMATO NAO SUPORTADO\n";
                    cout << "Formato suportado: ASCII \n";
                    in.close();
                    delete mesh;
                    exit(1);
                }
            }
            // Read and process the "PhysicalNames" section.
            else if (s.find("$PhysicalNames") == 0)
            {

                // Read in the number of physical groups to expect in the file.
                unsigned int num_physical_groups = 0;
                in >> num_physical_groups;

                // Read rest of line including newline character.
                //std::getline(in, s);

                for (unsigned int i=0; i<num_physical_groups; ++i)
                {
                    // Read an entire line of the PhysicalNames section.
                    //std::getline(in, s);
                    
                    //std::istringstream s_stream(s);
                    int phy_dim, phy_id;
                    string phy_name;
                    //s_stream >> phy_dim >> phy_id >> phy_name;
                    in >> phy_dim >> phy_id >> phy_name;

                    mesh->physical_map[phy_id] = std::make_pair(phy_dim, phy_name);
                    
                }
            }
            else if(s.find("$Nodes") == 0) 
            {
                unsigned int num_nodes = 0;
                in >> num_nodes;
                mesh->n_nodes = num_nodes;

                mesh->coord.resize(num_nodes*3);

                int node_id;
                double x,y,z;
                for(int i = 0; i < num_nodes; i++) {
                    in >> node_id >> x >> y >> z;
                    mesh->coord[(i*3)+0] = x;
                    mesh->coord[(i*3)+1] = y;
                    mesh->coord[(i*3)+2] = z;
                }

                // read the $ENDNOD delimiter
                std::getline(in, s);

            }

            else if (s.find("$Elements")==0)
            {
                int num_elem, node_id;
                in >> num_elem;
#ifdef DEBUG_
                cout << " Num. elementos: " << num_elem << endl;
#endif

                mesh->physical_tag.resize(num_elem);
                mesh->offset.resize(num_elem+1);
                mesh->type.resize(num_elem);
                mesh->offset[0] = 0;

                int iel = 0;
                for(int i = 0; i < num_elem; i++)
                {
                    int id, type, physical=1, elementary=1, nnodes=0, ntags, elem_dim;

                    in >> id >> type >> ntags;
#ifdef DEBUG_                   
                   cout << id << "  " << type << "  " << ntags << " ";
#endif

                    mesh->type[i] = type;

                    nnodes   = getGmshElemNNodes(type);
                    elem_dim = getGmshElemTypeDim(type);

                    if(nnodes < 0 )
                    {
                        cout << "ERRO: TIPO DO ELEMENTO " << type << " INVALIDO";
                        in.close();
                        delete mesh;
                        exit(1);
                    }

                    dim_count[elem_dim]++;

                    for(int j = 0; j < ntags; j++)
                    {
                        in >> physical;
                        if(j == 0)
                            mesh->physical_tag[i] = physical;
#ifdef DEBUG_
                        cout << physical << " ";
#endif
                    }

                    for (unsigned int j=0; j<nnodes; j++)
                    {
                        in >> node_id;
                        mesh->conn.push_back(node_id-1);
#ifdef DEBUG_
                        cout << node_id << " ";
#endif
                    }

                    mesh->offset[i+1] =  mesh->offset[i] + nnodes;
#ifdef DEBUG_
                    cout << endl;
                    cout << "OFFSET: " << mesh->offset[i+1] << " TYPE: "<< mesh->type[i] << endl;
#endif                   
                }

                // read the $ENDELM delimiter
                std::getline(in, s);

            } // End Elem
        } // end if(in)   

    } // end while (true)
       
    if(dim_count[3] != 0)
    {
        mesh->n_elements = dim_count[3];
        mesh->n_face_elements = dim_count[2];
    } else if (dim_count[2] != 0)
    {
        mesh->n_elements      = dim_count[2];
        mesh->n_face_elements = dim_count[1];
    }
    
    mesh->mesh_coloring_internal = new int [mesh->n_elements];
    mesh->mesh_coloring_bound = new int [mesh->n_face_elements];

    for(int i = 0 ; i < mesh->n_elements ; i++)
        mesh->mesh_coloring_internal[i] = -1;
    for(int i = 0 ; i < mesh->n_face_elements ; i++)
        mesh->mesh_coloring_bound[i] = -1;

    cout << " Num. Nodes: " << mesh->n_nodes << endl;
    cout << " Num. Elements: "          << mesh->n_elements << endl;
    cout << " Num. Boundary Elements: " << mesh->n_face_elements << endl;
    cout << " Connectivity size: " << mesh->conn.size() << endl;


    in.close();
    //return mesh;
}


void MeshVTKWriter(mesh_t* mesh, const char* filename, int *npart=NULL, int* epart=NULL)
{
    std::ofstream fout;

    fout.open(filename);

    if(fout.is_open())
    {

        int nnodes = mesh->n_nodes;
        int nelem = (mesh->n_elements + mesh->n_face_elements);
        fout << "<VTKFile type=\"UnstructuredGrid\" version=\"1.0\" byte_order=\"LittleEndian\" header_type=\"UInt64\">" << endl;
        fout << "\t<UnstructuredGrid>" << endl;
        fout << "\t\t<Piece NumberOfPoints=\"" << nnodes  <<"\" NumberOfCells=\""<< nelem << "\">" << endl;
        fout << "\t\t\t<PointData>" << endl;
        if(npart)
        {
            fout << "\t\t\t\t <DataArray type=\"Int32\" Name=\"npart\" format=\"ascii\" >" << endl;
            fout << "\t\t\t\t\t";
            for(int i = 0 ; i < nnodes ; i++)
            {
                if(i % 6 == 0 && i != 0)
                    fout << endl << "\t\t\t\t\t";
                
                fout << npart[i] << " ";
            }
            fout << endl;
            fout << "\t\t\t\t </DataArray> " << endl;
        }
        fout << "\t\t\t</PointData>" << endl;
        fout << "\t\t\t<CellData>" << endl;
        if(epart)
        {
            fout << "\t\t\t\t <DataArray type=\"Int32\" Name=\"epart\" format=\"ascii\" >" << endl;
            fout << "\t\t\t\t\t";
            for(int i = 0 ; i < nelem ; i++)
            {
                if(i % 5 == 0 && i != 0)
                    fout << endl << "\t\t\t\t\t";
                
                fout << epart[i] << " ";
            }
            fout << endl;
            fout << "\t\t\t\t </DataArray> " << endl;
        }
        fout << "\t\t\t</CellData>" << endl;
        fout << "\t\t\t<Points>" << endl;
        fout << "\t\t\t\t<DataArray type=\"Float32\" Name=\"Points\" NumberOfComponents=\"3\" format=\"ascii\">" << endl;
        fout << "\t\t\t\t\t";
        
        for(int i = 0 ; i < mesh->coord.size() ; i++)
        {
            if(i % 6 == 0 && i != 0)
                fout << endl << "\t\t\t\t\t";

            fout << mesh->coord[i] << " ";
        }
        fout << endl;
        fout << "\t\t\t\t</DataArray>" << endl;
        fout << "\t\t\t</Points>" << endl;
        fout << "\t\t\t<Cells>" << endl;
        fout << "\t\t\t\t<DataArray type=\"Int64\" Name=\"connectivity\" format=\"ascii\">" << endl;
        fout << "\t\t\t\t\t";
        
        for(int i = 0 ; i < mesh->conn.size() ; i++)
        {
            if(i % 5 == 0 && i != 0)
                fout << endl << "\t\t\t\t\t";

            fout << mesh->conn[i] << " ";
        }
        fout << endl;
        fout << "\t\t\t\t</DataArray>" << endl;
        fout << "\t\t\t\t<DataArray type=\"Int64\" Name=\"offsets\" format=\"ascii\">" << endl;
        fout << "\t\t\t\t\t";
        for(int i = 1 ; i < mesh->offset.size() ; i++)
        {
            if(i % 6 == 0 && i != 0)
                fout << endl << "\t\t\t\t\t";

            fout << mesh->offset[i] << " ";
        }
        fout << endl;
        fout << "\t\t\t\t</DataArray>" << endl;
        fout << "\t\t\t\t<DataArray type=\"UInt8\" Name=\"types\" format=\"ascii\">" << endl;
        fout << "\t\t\t\t\t";
        
        for(int i = 0 ; i < mesh->type.size() ; i++)
        { 
            if(i % 6 == 0 && i != 0)
                fout << endl << "\t\t\t\t\t";

            fout << GmshToVTKType(mesh->type[i]) << " ";
        }
        fout << endl;
        fout << "\t\t\t\t</DataArray>" << endl;
        fout << "\t\t\t</Cells>" << endl;
        fout << "\t\t</Piece>" << endl;
        fout << "\t</UnstructuredGrid>" << endl;
        fout << "</VTKFile>" << endl;

        fout.close();
    }
}


void MeshVTKWriterInternal(mesh_t* mesh, const char* filename, int* npart, int* epart, int* color)
{
    std::ofstream fout;

    fout.open(filename);

    if(fout.is_open())
    {

        fout << "<VTKFile type=\"UnstructuredGrid\" version=\"1.0\" byte_order=\"LittleEndian\" header_type=\"UInt64\">" << endl;
        fout << "\t<UnstructuredGrid>" << endl;
        fout << "\t\t<Piece NumberOfPoints=\"" << mesh->n_nodes <<"\" NumberOfCells=\""<< (mesh->n_elements) << "\">" << endl;
        fout << "\t\t\t<PointData>" << endl;
        fout << "\t\t\t\t <DataArray type=\"Int32\" Name=\"npart\" format=\"ascii\" >" << endl;
        fout << "\t\t\t\t\t";
        for(int i = 0 ; i < mesh->n_nodes ; i++)
        {
            if(i % 6 == 0 && i != 0)
                fout << endl << "\t\t\t\t\t";

            fout << npart[i] << " ";
        }
        fout << endl;
        fout << "\t\t\t\t </DataArray> " << endl;

        fout << "\t\t\t</PointData>" << endl;
        fout << "\t\t\t<CellData>" << endl;
        fout << "\t\t\t\t <DataArray type=\"Int32\" Name=\"epart\" format=\"ascii\" >" << endl;
        fout << "\t\t\t\t\t";
        for(int i = 0 ; i < mesh->n_elements ; i++)
        {
            if(i % 5 == 0 && i != 0)
                fout << endl << "\t\t\t\t\t";
            
            fout << epart[i] << " ";
        }
        fout << endl;
        fout << "\t\t\t\t </DataArray> " << endl;
                fout << "\t\t\t\t <DataArray type=\"Int32\" Name=\"Color\" format=\"ascii\" >" << endl;
        fout << "\t\t\t\t\t";
        for(int i = 0 ; i < mesh->n_internal_colors ; i++)
        {
            for(int j = 0 ; j < color[i] ; j++)
            {
                if(j % 5 == 0 && i != 0)
                    fout << endl << "\t\t\t\t\t";

                fout << i+1 << " ";
            }
        }
        fout << endl;
        fout << "\t\t\t\t </DataArray> " << endl;
        fout << "\t\t\t</CellData>" << endl;
        fout << "\t\t\t<Points>" << endl;
        fout << "\t\t\t\t<DataArray type=\"Float32\" Name=\"Points\" NumberOfComponents=\"3\" format=\"ascii\">" << endl;
        fout << "\t\t\t\t\t";
        for(int i = 0 ; i < mesh->coord.size() ; i++)
        {
            if(i % 6 == 0 && i != 0)
                fout << endl << "\t\t\t\t\t";

            fout << mesh->coord[i] << " ";
        }
        fout << endl;
        fout << "\t\t\t\t</DataArray>" << endl;
        fout << "\t\t\t</Points>" << endl;
        fout << "\t\t\t<Cells>" << endl;
        fout << "\t\t\t\t<DataArray type=\"Int64\" Name=\"connectivity\" format=\"ascii\">" << endl;
        fout << "\t\t\t\t\t";
        
        int ofs = mesh->offset[mesh->n_face_elements];

        //cout << "OFFSET: " << ofs << endl;

        for(int i = ofs ; i < mesh->conn.size() ; i++)
        {
            if(i % 5 == 0 )
                fout << endl << "\t\t\t\t\t";

            fout << mesh->conn[i] << " ";
        }
        fout << endl;
        fout << "\t\t\t\t</DataArray>" << endl;
        fout << "\t\t\t\t<DataArray type=\"Int64\" Name=\"offsets\" format=\"ascii\">" << endl;
        fout << "\t\t\t\t\t";
        
        for(int i = mesh->n_face_elements ; i < mesh->offset.size()-1 ; i++)
        {
            if(i % 6 == 0 && i != 0)
                fout << endl << "\t\t\t\t\t";

            fout << mesh->offset[i+1]- ofs << " ";
        }
        fout << endl;
        fout << "\t\t\t\t</DataArray>" << endl;
        fout << "\t\t\t\t<DataArray type=\"UInt8\" Name=\"types\" format=\"ascii\">" << endl;
        fout << "\t\t\t\t\t";
        
        for(int i = mesh->n_face_elements ; i < mesh->type.size() ; i++)
        { 
            if(i % 6 == 0 && i != 0)
                fout << endl << "\t\t\t\t\t";

            fout << GmshToVTKType(mesh->type[i]) << " ";
        }
        fout << endl;
        fout << "\t\t\t\t</DataArray>" << endl;
        fout << "\t\t\t</Cells>" << endl;
        fout << "\t\t</Piece>" << endl;
        fout << "\t</UnstructuredGrid>" << endl;
        fout << "</VTKFile>" << endl;

        fout.close();
    }
}