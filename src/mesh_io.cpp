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

                    mesh->type[i] = GmshToVTKType(type);

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

    for(int i = 0 ; i < mesh->n_elements ; i++)
        mesh->mesh_coloring_internal[i] = -1;

    cout << " Num. Nodes: " << mesh->n_nodes << endl;
    cout << " Num. Elements: "          << mesh->n_elements << endl;
    cout << " Num. Boundary Elements: " << mesh->n_face_elements << endl;
    cout << " Connectivity size: " << mesh->conn.size() << endl;


    in.close();
    //return mesh;
}

void MeshVTKWriter(mesh_t* mesh, const char* filename, int timeStep, int *npart, int* epart, int* color, double* velocity, float* pressure)
{
    cout << "Writing VTK boundary and internal elements..." << endl;
    std::ofstream fout;

    if(timeStep)
    {
        string str = filename;
        str.insert(str.length() - 4, "_" + to_string(timeStep));

        fout.open(str);
    }
    else{
            fout.open(filename);
    }

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
                if(i % 18 == 0 && i != 0)
                    fout << endl << "\t\t\t\t\t";
                
                fout << npart[i] << " ";
            }
            fout << endl;
            fout << "\t\t\t\t </DataArray> " << endl;
        }
        if(velocity)
        {
            fout << "\t\t\t\t <DataArray type=\"Float64\" Name=\"velocity\" format=\"ascii\">" << endl;
            fout << "\t\t\t\t\t";
            for(int i = 0 ; i < mesh->n_nodes * 3 ; i++)
            {
                if(i % 18 == 0 && i != 0)
                    fout << endl << "\t\t\t\t\t";

                fout << velocity[i] << " ";
            }
            fout << endl;
            
            fout << "\t\t\t\t </DataArray> " << endl;
        }
        if(pressure)
        {
            fout << "\t\t\t\t <DataArray type=\"Float32\" Name=\"pressure\" format=\"ascii\">" << endl;
            fout << "\t\t\t\t\t";
            for(int i = 0 ; i < mesh->n_nodes ; i++)
            {
                if(i % 18 == 0 && i != 0)
                    fout << endl << "\t\t\t\t\t";

                fout << pressure[i] << " ";
            }
            fout << endl;
            fout << "\t\t\t\t </DataArray> " << endl;
        }
        fout << "\t\t\t</PointData>" << endl;
        fout << "\t\t\t<CellData>" << endl;
        if(epart)
        {
            fout << "\t\t\t\t <DataArray type=\"Int32\" Name=\"epart\" format=\"ascii\">" << endl;
            fout << "\t\t\t\t\t";
            for(int i = 0 ; i < nelem ; i++)
            {
                if(i % 18 == 0 && i != 0)
                    fout << endl << "\t\t\t\t\t";
                
                fout << epart[i] << " ";
            }
            fout << endl;
            fout << "\t\t\t\t </DataArray> " << endl;
        }
        if(color)
        {
            fout << "\t\t\t\t <DataArray type=\"Int32\" Name=\"Color\" format=\"ascii\">" << endl;
            fout << "\t\t\t\t\t";
            for(int i = 0 ; i < mesh->n_face_elements ; i++)
            {
                if(i % 18 == 0 && i != 0)
                    fout << endl << "\t\t\t\t\t";
                fout << -1 << " "; // cor dos elementos de superficie
            }
            for(int i = 0 ; i < mesh->n_internal_colors ; i++)
            {
                for(int j = 0 ; j < color[i] ; j++)
                {
                    if(j % 18 == 0 && j != 0)
                        fout << endl << "\t\t\t\t\t";

                    fout << i+1 << " ";
                }
            }
            fout << endl;
            fout << "\t\t\t\t </DataArray> " << endl;
        }
        fout << "\t\t\t</CellData>" << endl;
        fout << "\t\t\t<Points>" << endl;
        fout << "\t\t\t\t<DataArray type=\"Float64\" Name=\"Points\" NumberOfComponents=\"3\" format=\"ascii\">" << endl;
        fout << "\t\t\t\t\t";
        
        for(int i = 0 ; i < mesh->coord.size() ; i++)
        {
            if(i % 18 == 0 && i != 0)
                fout << endl << "\t\t\t\t\t";

            fout << mesh->coord[i] << " ";
        }
        fout << endl;
        fout << "\t\t\t\t</DataArray>" << endl;
        fout << "\t\t\t</Points>" << endl;
        fout << "\t\t\t<Cells>" << endl;
        fout << "\t\t\t\t<DataArray type=\"Int32\" Name=\"connectivity\" format=\"ascii\">" << endl;
        fout << "\t\t\t\t\t";
        
        for(int i = 0 ; i < mesh->conn.size() ; i++)
        {
            if(i % 18 == 0 && i != 0)
                fout << endl << "\t\t\t\t\t";

            fout << mesh->conn[i] << " ";
        }
        fout << endl;
        fout << "\t\t\t\t</DataArray>" << endl;
        fout << "\t\t\t\t<DataArray type=\"Int32\" Name=\"offsets\" format=\"ascii\">" << endl;
        fout << "\t\t\t\t\t";
        for(int i = 1 ; i < mesh->offset.size() ; i++)
        {
            if(i % 18 == 0 && i != 0)
                fout << endl << "\t\t\t\t\t";

            fout << mesh->offset[i] << " ";
        }
        fout << endl;
        fout << "\t\t\t\t</DataArray>" << endl;
        fout << "\t\t\t\t<DataArray type=\"Int32\" Name=\"types\" format=\"ascii\">" << endl;
        fout << "\t\t\t\t\t";
        
        for(int i = 0 ; i < mesh->type.size() ; i++)
        { 
            if(i % 18 == 0 && i != 0)
                fout << endl << "\t\t\t\t\t";

            fout << mesh->type[i] << " ";
        }
        fout << endl;
        fout << "\t\t\t\t</DataArray>" << endl;
        fout << "\t\t\t</Cells>" << endl;
        fout << "\t\t</Piece>" << endl;
        fout << "\t</UnstructuredGrid>" << endl;
        fout << "</VTKFile>" << endl;

        fout.close();
        cout << "Writing completed successfully" << endl;
    }
}


void MeshVTKWriterInternal(mesh_t* mesh, const char* filename, int timeStep, int* npart, int* epart, int* color, double* velocity, float* pressure)
{
    cout << "Writing VTK internal elements..." << endl;
    std::ofstream fout;

    if(timeStep)
    {
        string str = filename;
        str.insert(str.length() - 4, "_" + to_string(timeStep));

        fout.open(str);
    }
    else{
            fout.open(filename);
    }

    if(fout.is_open())
    {

        fout << "<VTKFile type=\"UnstructuredGrid\" version=\"1.0\" byte_order=\"LittleEndian\" header_type=\"UInt64\">" << endl;
        fout << "\t<UnstructuredGrid>" << endl;
        fout << "\t\t<Piece NumberOfPoints=\"" << mesh->n_nodes <<"\" NumberOfCells=\""<< (mesh->n_elements) << "\">" << endl;
        fout << "\t\t\t<PointData>" << endl;
        if(npart)
        {
            fout << "\t\t\t\t <DataArray type=\"Int32\" Name=\"npart\" format=\"ascii\">" << endl;
            fout << "\t\t\t\t\t";
            for(int i = 0 ; i < mesh->n_nodes ; i++)
            {
                if(i % 18 == 0 && i != 0)
                    fout << endl << "\t\t\t\t\t";

                fout << npart[i] << " ";
            }
            fout << endl;
            fout << "\t\t\t\t </DataArray> " << endl;
        }
        if(velocity)
        {
            fout << "\t\t\t\t <DataArray type=\"Float64\" Name=\"velocity\" format=\"ascii\">" << endl;
            fout << "\t\t\t\t\t";
            for(int i = 0 ; i < mesh->n_nodes * 3 ; i++)
            {
                if(i % 18 == 0 && i != 0)
                    fout << endl << "\t\t\t\t\t";

                fout << velocity[i] << " ";
            }
            fout << endl;
            fout << "\t\t\t\t </DataArray> " << endl;
        }
        if(pressure)
        {
            fout << "\t\t\t\t <DataArray type=\"Float32\" Name=\"pressure\" format=\"ascii\">" << endl;
            fout << "\t\t\t\t\t";
            for(int i = 0 ; i < mesh->n_nodes ; i++)
            {
                if(i % 18 == 0 && i != 0)
                    fout << endl << "\t\t\t\t\t";

                fout << pressure[i] << " ";
            }
            fout << endl;
            fout << "\t\t\t\t </DataArray> " << endl;
        }
        fout << "\t\t\t</PointData>" << endl;
        fout << "\t\t\t<CellData>" << endl;
        if(epart)
        {
            fout << "\t\t\t\t <DataArray type=\"Int32\" Name=\"epart\" format=\"ascii\">" << endl;
            fout << "\t\t\t\t\t";
            for(int i = 0 ; i < mesh->n_elements ; i++)
            {
                if(i % 18 == 0 && i != 0)
                    fout << endl << "\t\t\t\t\t";
                
                fout << epart[i] << " ";
            }
            fout << endl;
            fout << "\t\t\t\t </DataArray> " << endl;
        }
        if(color)
        {
            fout << "\t\t\t\t <DataArray type=\"Int32\" Name=\"Color\" format=\"ascii\">" << endl;
            fout << "\t\t\t\t\t";
            for(int i = 0 ; i < mesh->n_internal_colors ; i++)
            {
                for(int j = 0 ; j < color[i] ; j++)
                {
                    if(j % 18 == 0 && j != 0)
                        fout << endl << "\t\t\t\t\t";

                    fout << i+1 << " ";
                }
            }
            fout << "\t\t\t\t </DataArray> " << endl;
        }
        fout << "\t\t\t</CellData>" << endl;
        fout << "\t\t\t<Points>" << endl;
        fout << "\t\t\t\t<DataArray type=\"Float64\" Name=\"Points\" NumberOfComponents=\"3\" format=\"ascii\">" << endl;
        fout << "\t\t\t\t\t";
        for(int i = 0 ; i < mesh->coord.size() ; i++)
        {
            if(i % 18 == 0 && i != 0)
                fout << endl << "\t\t\t\t\t";

            fout << mesh->coord[i] << " ";
        }
        fout << endl;
        fout << "\t\t\t\t</DataArray>" << endl;
        fout << "\t\t\t</Points>" << endl;
        fout << "\t\t\t<Cells>" << endl;
        fout << "\t\t\t\t<DataArray type=\"Int32\" Name=\"connectivity\" format=\"ascii\">" << endl;
        fout << "\t\t\t\t\t";
        
        int ofs = mesh->offset[mesh->n_face_elements];

        for(int i = ofs ; i < mesh->conn.size() ; i++)
        {
            if(i % 18 == 0 )
                fout << endl << "\t\t\t\t\t";

            fout << mesh->conn[i] << " ";
        }
        fout << endl;
        fout << "\t\t\t\t</DataArray>" << endl;
        fout << "\t\t\t\t<DataArray type=\"Int32\" Name=\"offsets\" format=\"ascii\">" << endl;
        fout << "\t\t\t\t\t";
        
        for(int i = mesh->n_face_elements ; i < mesh->offset.size()-1 ; i++)
        {
            if(i % 18 == 0 && i != 0)
                fout << endl << "\t\t\t\t\t";

            fout << mesh->offset[i+1]- ofs << " ";
        }
        fout << endl;
        fout << "\t\t\t\t</DataArray>" << endl;
        fout << "\t\t\t\t<DataArray type=\"Int32\" Name=\"types\" format=\"ascii\">" << endl;
        fout << "\t\t\t\t\t";
        
        for(int i = mesh->n_face_elements ; i < mesh->type.size() ; i++)
        { 
            if(i % 18 == 0 && i != 0)
                fout << endl << "\t\t\t\t\t";

            fout << mesh->type[i] << " ";
        }
        fout << endl;
        fout << "\t\t\t\t</DataArray>" << endl;
        fout << "\t\t\t</Cells>" << endl;
        fout << "\t\t</Piece>" << endl;
        fout << "\t</UnstructuredGrid>" << endl;
        fout << "</VTKFile>" << endl;

        fout.close();
        cout << "Writing completed successfully" << endl;
    }
}

bool BinaryBigEndian(void) 
{
    long _v = 1; 
    return ((char*)&_v)[0] ? false : true;
}

void MeshVTKWriterInternalBinAppended(mesh_t* mesh, const char* filename, int timeStep, int* npart, int* epart, int* color, double* velocity, float* pressure)
{
    cout << "Writing VTK internal elements..." << endl;
    
    FILE*         fout;
    unsigned int boffset = 0; /* Offset into binary file */
    const char *byte_order = BinaryBigEndian() ? "BigEndian" : "LittleEndian";

    string str = filename;
    if(timeStep)
    {
        string str = filename;
        str.insert(str.length() - 4, "_" + to_string(timeStep));
    }

    fout = fopen(str.c_str(), "wb");

    if(fout)
    {

        // Writting header info
        fprintf(fout, "<VTKFile type=\"UnstructuredGrid\" version=\"1.0\" byte_order=\"%s\" header_type=\"UInt64\">\n", byte_order);
        fprintf(fout, " <UnstructuredGrid>\n");
        fprintf(fout, "  <Piece NumberOfPoints=\"%d\" NumberOfCells=\"%d\">\n", mesh->n_nodes, mesh->n_elements);


        // writing mesh info
        fprintf(fout, "   <Points>\n");
        fprintf(fout, "        <DataArray type=\"%s\" Name=\"%s\" NumberOfComponents=\"%d\" format=\"appended\" offset=\"%d\" />\n","Float64","Points",3, boffset);
        boffset += 3*mesh->n_nodes*sizeof(double) + sizeof(unsigned long);

        fprintf(fout, "   </Points>\n") ;
        fprintf(fout, "   <Cells>\n");

        fprintf(fout, "        <DataArray type=\"%s\" Name=\"%s\" NumberOfComponents=\"%d\" format=\"appended\" offset=\"%d\" />\n","Int32","connectivity",1, boffset);
        int ofs = mesh->offset[mesh->n_face_elements];
        int sz  = mesh->conn.size() - ofs;
        boffset += sz*sizeof(int) + sizeof(unsigned long);


        fprintf(fout, "        <DataArray type=\"%s\" Name=\"%s\" NumberOfComponents=\"%d\" format=\"appended\" offset=\"%d\" />\n","Int32","offsets",1, boffset);
        sz = mesh->offset.size() - (mesh->n_face_elements + 1);
        boffset += sz*sizeof(int) + sizeof(unsigned long);

        fprintf(fout,"        <DataArray type=\"%s\" Name=\"%s\" NumberOfComponents=\"%d\" format=\"appended\" offset=\"%d\" />\n","UInt16","types",1, boffset);
        boffset += mesh->n_elements*sizeof(unsigned short) + sizeof(unsigned long);
        fprintf(fout, "   </Cells>\n");


        // Writting nodal attribute data
        fprintf(fout, "   <PointData>\n");
        if(npart)
        {
            fprintf(fout, "        <DataArray type=\"%s\" Name=\"%s\" NumberOfComponents=\"%d\" format=\"appended\" offset=\"%d\" />\n","Int32","npart",1,boffset);
            boffset += mesh->n_nodes*sizeof(int) + sizeof(unsigned long);
        }
        if(velocity)
        {
            fprintf(fout, "        <DataArray type=\"%s\" Name=\"%s\"  NumberOfComponents=\"%d\" format=\"appended\" offset=\"%d\" />\n","Float64","velocity",3,boffset);
            boffset += 3*mesh->n_nodes*sizeof(double) + sizeof(unsigned long);
        }
        if(pressure)
        {
            fprintf(fout, "        <DataArray type=\"%s\" Name=\"%s\"  NumberOfComponents=\"%d\" format=\"appended\" offset=\"%d\" />\n","Float32","pressure",1,boffset);
            boffset += mesh->n_nodes*sizeof(float) + sizeof(unsigned long);
        }

        fprintf(fout, "   </PointData>\n");
        fprintf(fout, "   <CellData>\n");
       
        if(epart)
        {
            fprintf(fout, "        <DataArray type=\"%s\" Name=\"%s\"  NumberOfComponents=\"%d\" format=\"appended\" offset=\"%d\" />\n","Int32","epart",1, boffset);
            boffset += mesh->n_elements*sizeof(int) + sizeof(unsigned long);
        }
        if(color)
        {
                  
            fprintf(fout, "        <DataArray type=\"%s\" Name=\"%s\"  NumberOfComponents=\"%d\" format=\"appended\" offset=\"%d\" />\n","Int32","Color",1, boffset);
            boffset += mesh->n_elements*sizeof(int) + sizeof(unsigned long);
        }

        fprintf(fout, "   </CellData>\n");
        fprintf(fout, "  </Piece>\n");
        fprintf(fout, " </UnstructuredGrid>\n");
        fprintf(fout, "  <AppendedData encoding=\"raw\">\n");
        fprintf(fout, "_");

        // writting nodes coordinates
        unsigned long nbytes = sizeof(double)*mesh->n_nodes*3;
        fwrite((void*)&nbytes, sizeof(unsigned long),1,fout);
        fwrite((void*)&mesh->coord[0], sizeof(double),mesh->n_nodes*3,fout);
        
        // writting element connectivity
        int nfe = mesh->n_face_elements;
        int ne = mesh->n_elements;
        ofs = mesh->offset[nfe];

        nbytes = sizeof(int)*(mesh->conn.size() - ofs);
        fwrite((void*)&nbytes, sizeof(unsigned long),1,fout);
        fwrite((void*)&mesh->conn[ofs], sizeof(int),mesh->conn.size() - ofs,fout);

        // writting element offsets
        nbytes = sizeof(int)*(mesh->offset.size() - (nfe + 1));
        fwrite((void*)&nbytes, sizeof(unsigned long),1,fout);
        for(int i = nfe+1 ; i < mesh->offset.size() ; i++)
        {
            int offset = mesh->offset[i] - mesh->offset[nfe];
            fwrite((void*)&offset, sizeof(int), 1,fout);
        }

        // writting types
        nbytes = sizeof(unsigned short)*(ne);
        fwrite((void*)&nbytes, sizeof(unsigned long),1,fout);
        fwrite((void*)&mesh->type[nfe], sizeof(unsigned short),ne,fout);

        // Writing attribute data
        if(npart)
        {
            nbytes = sizeof(int)*mesh->n_nodes;
            fwrite((void*)&nbytes, sizeof(unsigned long),1,fout);
            fwrite((void*)npart,sizeof(int), mesh->n_nodes,fout);
        }
        if(velocity)
        {
            nbytes = sizeof(double)*3*mesh->n_nodes;
            fwrite((void*)&nbytes, sizeof(unsigned long),1,fout);
            fwrite((void*)velocity,sizeof(double), 3*mesh->n_nodes,fout);
        }
        if(pressure)
        {
            nbytes = sizeof(float)*mesh->n_nodes;
            fwrite((void*)&nbytes, sizeof(unsigned long),1,fout);
            fwrite((void*)pressure,sizeof(float), mesh->n_nodes,fout);
        }
        if(epart)
        {
            nbytes = sizeof(int)*mesh->n_elements;
            fwrite((void*)&nbytes, sizeof(unsigned long),1,fout);
            fwrite((void*)epart,sizeof(int), mesh->n_elements,fout);
        }
        if(color)
        {
            nbytes = sizeof(int)* mesh->n_elements;
            fwrite((void*)&nbytes, sizeof(unsigned long),1,fout);

            for(int i = 0 ; i < mesh->n_internal_colors ; i++)
            {
                for(int j = 0 ; j < color[i] ; j++)
                {
                    int colorAux = i+1;
                    fwrite((void*)&colorAux,sizeof(int),1, fout);
                }
            }
        }

    
        fprintf(fout,"\n </AppendedData>\n");
        fprintf(fout,"</VTKFile>");


        fclose(fout);

        cout << "Writing completed successfully" << endl;
    }
}

void MeshVTKWriterBinAppended(mesh_t* mesh, const char* filename, int timeStep, int* npart, int* epart, int* color, double* velocity, float* pressure)
{
    cout << "Writing VTK internal elements..." << endl;
    
    FILE*         fout;
    unsigned int boffset = 0; /* Offset into binary file */
    const char *byte_order = BinaryBigEndian() ? "BigEndian" : "LittleEndian";

    string str = filename;
    if(timeStep)
    {
        string str = filename;
        str.insert(str.length() - 4, "_" + to_string(timeStep));
    }

    fout = fopen(str.c_str(), "wb");

    if(fout)
    {
        // Writting header info
        fprintf(fout, "<VTKFile type=\"UnstructuredGrid\" version=\"1.0\" byte_order=\"%s\" header_type=\"UInt64\">\n", byte_order);
        fprintf(fout, " <UnstructuredGrid>\n");
        fprintf(fout, "  <Piece NumberOfPoints=\"%d\" NumberOfCells=\"%d\">\n", mesh->n_nodes, mesh->n_elements + mesh->n_face_elements);

        // writing mesh info
        fprintf(fout, "   <Points>\n");
        fprintf(fout, "        <DataArray type=\"%s\" Name=\"%s\" NumberOfComponents=\"%d\" format=\"appended\" offset=\"%d\" />\n","Float64","Points",3, boffset);
        boffset += 3*mesh->n_nodes*sizeof(double) + sizeof(unsigned long);
        
        fprintf(fout, "   </Points>\n") ;
        fprintf(fout, "   <Cells>\n");

        fprintf(fout, "        <DataArray type=\"%s\" Name=\"%s\" NumberOfComponents=\"%d\" format=\"appended\" offset=\"%d\" />\n","Int32","connectivity",1, boffset);
        boffset += mesh->conn.size()*sizeof(int) + sizeof(unsigned long);


        fprintf(fout, "        <DataArray type=\"%s\" Name=\"%s\" NumberOfComponents=\"%d\" format=\"appended\" offset=\"%d\" />\n","Int32","offsets",1, boffset);
        boffset += (mesh->offset.size()-1)*sizeof(int) + sizeof(unsigned long);

        fprintf(fout,"        <DataArray type=\"%s\" Name=\"%s\" NumberOfComponents=\"%d\" format=\"appended\" offset=\"%d\" />\n","UInt16","types",1, boffset);
        boffset += mesh->type.size()*sizeof(unsigned short) + sizeof(unsigned long);

        fprintf(fout, "   </Cells>\n");

        // Writting nodal attribute data
        fprintf(fout, "   <PointData>\n");
        if(npart)
        {
            fprintf(fout, "        <DataArray type=\"%s\" Name=\"%s\" NumberOfComponents=\"%d\" format=\"appended\" offset=\"%d\" />\n","Int32","npart",1,boffset);
            boffset += mesh->n_nodes*sizeof(int) + sizeof(unsigned long);
        }
        if(velocity)
        {
            fprintf(fout, "        <DataArray type=\"%s\" Name=\"%s\"  NumberOfComponents=\"%d\" format=\"appended\" offset=\"%d\" />\n","Float64","velocity",3,boffset);
            boffset += 3*mesh->n_nodes*sizeof(double) + sizeof(unsigned long);
        }
        if(pressure)
        {
            fprintf(fout, "        <DataArray type=\"%s\" Name=\"%s\"  NumberOfComponents=\"%d\" format=\"appended\" offset=\"%d\" />\n","Float32","pressure",1,boffset);
            boffset += mesh->n_nodes*sizeof(float) + sizeof(unsigned long);
        }

        fprintf(fout, "   </PointData>\n");
        fprintf(fout, "   <CellData>\n");
        
        // Writting element attribute data
        if(epart)
        {
            fprintf(fout, "        <DataArray type=\"%s\" Name=\"%s\"  NumberOfComponents=\"%d\" format=\"appended\" offset=\"%d\" />\n","Int32","epart",1, boffset);
            boffset += mesh->n_elements*sizeof(int) + sizeof(unsigned long);
        }
        if(color)
        {
            fprintf(fout, "        <DataArray type=\"%s\" Name=\"%s\"  NumberOfComponents=\"%d\" format=\"appended\" offset=\"%d\" />\n","Int32","Color",1, boffset);
            boffset += mesh->n_elements*sizeof(int) + sizeof(unsigned long);
        }

        fprintf(fout, "   </CellData>\n");
        fprintf(fout, "  </Piece>\n");
        fprintf(fout, " </UnstructuredGrid>\n");
        fprintf(fout, "  <AppendedData encoding=\"raw\">\n");
        fprintf(fout, "_");

        // writting nodes coordinates
        unsigned long nbytes = sizeof(double)*mesh->n_nodes*3;
        fwrite((void*)&nbytes, sizeof(unsigned long),1,fout);
        fwrite((void*)&mesh->coord[0], sizeof(double),mesh->n_nodes*3,fout);
        
        // writting element connectivity
        nbytes = sizeof(int)*mesh->conn.size();
        fwrite((void*)&nbytes, sizeof(unsigned long),1,fout);
        fwrite((void*)&mesh->conn[0], sizeof(int),mesh->conn.size(),fout);

        // writting element offsets
        nbytes = sizeof(int)*(mesh->offset.size()-1);
        fwrite((void*)&nbytes, sizeof(unsigned long),1,fout);
        fwrite((void*)&mesh->offset[1], sizeof(int),mesh->offset.size()-1,fout);

        // writting types
        nbytes = sizeof(unsigned short)*(mesh->type.size());
        fwrite((void*)&nbytes, sizeof(unsigned long),1,fout);
        fwrite((void*)&mesh->type[0], sizeof(unsigned short),mesh->type.size(),fout);

        // Writing attribute data
        if(npart)
        {
            nbytes = sizeof(int)*mesh->n_nodes;
            fwrite((void*)&nbytes, sizeof(unsigned long),1,fout);
            fwrite((void*)npart,sizeof(int), mesh->n_nodes,fout);
        }
        if(velocity)
        {
            nbytes = sizeof(double)*3*mesh->n_nodes;
            fwrite((void*)&nbytes, sizeof(unsigned long),1,fout);
            fwrite((void*)velocity,sizeof(double), 3*mesh->n_nodes,fout);
        }
        if(pressure)
        {
            nbytes = sizeof(float)*mesh->n_nodes;
            fwrite((void*)&nbytes, sizeof(unsigned long),1,fout);
            fwrite((void*)pressure,sizeof(float), mesh->n_nodes,fout);
        }
        if(epart)
        {
            nbytes = sizeof(int)*(mesh->n_face_elements + mesh->n_elements);
            fwrite((void*)&nbytes, sizeof(unsigned long),1,fout);
            fwrite((void*)epart,sizeof(int), mesh->n_face_elements + mesh->n_elements,fout);
        }
        if(color)
        {
            nbytes = sizeof(int)*(mesh->n_face_elements + mesh->n_elements);
            fwrite((void*)&nbytes, sizeof(unsigned long),1,fout);
            int colorAux = -1; // cor dos elementos de superfície

            for(int i = 0 ; i < mesh->n_face_elements ; i++)
            {
                fwrite((void*)&colorAux,sizeof(int),1, fout);
            }

            for(int i = 0 ; i < mesh->n_internal_colors ; i++)
            {
                for(int j = 0 ; j < color[i] ; j++)
                {
                    colorAux = i+1;
                    fwrite((void*)&colorAux,sizeof(int),1, fout);
                }
            }
        }

    
        fprintf(fout,"\n </AppendedData>\n");
        fprintf(fout,"</VTKFile>");


        fclose(fout);

        cout << "Writing completed successfully" << endl;
    }
}

/*
void MeshVTKWriterBin(mesh_t* mesh, const char* filename, int timeStep, int *npart, int* epart, int* color, double* velocity, float* pressure)
{
    cout << "Writing VTK boundary and internal elements..." << endl;
    std::ofstream fout;

    if(timeStep)
    {
        string str = filename;
        str.insert(str.length() - 4, "_" + to_string(timeStep));

        fout.open(str);
    }
    else{
            fout.open(filename, ios::binary);
    }

    if(fout.is_open())
    {

        int nnodes = mesh->n_nodes;
        int nelem = mesh->n_elements + mesh->n_face_elements;
        fout << "<VTKFile type=\"UnstructuredGrid\" version=\"1.0\" byte_order=\"LittleEndian\" header_type=\"UInt64\">" << endl;
        fout << "\t<UnstructuredGrid>" << endl;
        fout << "\t\t<Piece NumberOfPoints=\"" << mesh->n_nodes <<"\" NumberOfCells=\""<< mesh->n_elements << "\">" << endl;
        fout << "\t\t\t<PointData>" << endl;
        if(npart)
        {
            fout << "\t\t\t\t <DataArray type=\"Int32\" Name=\"npart\" format=\"binary\">" << endl;
            for(int i = 0 ; i < mesh->n_nodes ; i++)
                fout.write((char*)&npart[i], sizeof(double));
            fout << endl;
            fout << "\t\t\t\t </DataArray> " << endl;
        }
        if(velocity)
        {
            fout << "\t\t\t\t <DataArray type=\"Float64\" Name=\"velocity\" format=\"binary\">" << endl;
            for(int i = 0 ; i < mesh->n_nodes*3 ; i++)
                fout.write((char*)&velocity[i], sizeof(double));
            fout << endl;
            fout << "\t\t\t\t </DataArray> " << endl;
        }
        if(pressure)
        {
            fout << "\t\t\t\t <DataArray type=\"Float32\" Name=\"pressure\" format=\"binary\">" << endl;
            for(int i = 0 ; i < mesh->n_nodes ; i++)
                fout.write((char*)&pressure[i], sizeof(float));
            fout << endl;
            fout << "\t\t\t\t </DataArray> " << endl;
        }
        fout << "\t\t\t</PointData>" << endl;
        fout << "\t\t\t<CellData>" << endl;
        if(epart)
        {
            fout << "\t\t\t\t <DataArray type=\"Int32\" Name=\"epart\" format=\"binary\">" << endl;
            for(int i = 0 ; i < mesh->n_elements + mesh->n_face_elements ; i++)
                fout.write((char*)&epart[i], sizeof(int));
            fout << endl;
            fout << "\t\t\t\t </DataArray> " << endl;
        }
        if(color)
        {
            fout << "\t\t\t\t <DataArray type=\"Int32\" Name=\"Color\" format=\"binary\">" << endl;
            for(int i = 0 ; i < mesh->n_internal_colors ; i++)
            {
                for(int j = 0 ; j < color[i] ; j++)
                {
                    int colorAux = i+1;
                    fout.write((char*)&colorAux, sizeof(int));
                }
            }
            fout << endl;
            fout << "\t\t\t\t </DataArray> " << endl;
        }
        fout << "\t\t\t</CellData>" << endl;
        fout << "\t\t\t<Points>" << endl;

        fout << "\t\t\t\t<DataArray type=\"Float64\" Name=\"Points\" NumberOfComponents=\"3\" format=\"binary\">" << endl;
        for(int i = 0 ; i < mesh->n_nodes*3 ; i++)
            fout.write((char*)&mesh->coord[i], sizeof(double));
        fout << endl;
        fout << "\t\t\t\t</DataArray>" << endl;

        fout << "\t\t\t</Points>" << endl;
        fout << "\t\t\t<Cells>" << endl;

        fout << "\t\t\t\t<DataArray type=\"Int32\" Name=\"connectivity\" format=\"binary\">" << endl;
        for(int i = 0 ; i < mesh->conn.size() ; i++)
            fout.write((char*)&mesh->conn[i], sizeof(int));
        fout << endl;
        fout << "\t\t\t\t</DataArray>" << endl;

        fout << "\t\t\t\t<DataArray type=\"Int32\" Name=\"offsets\" format=\"binary\">" << endl;
        for(int i = 1 ; i < mesh->offset.size() ; i++)
            fout.write((char*)&mesh->offset[i], sizeof(int));
        fout << endl;
        fout << "\t\t\t\t</DataArray>" << endl;

        fout << "\t\t\t\t<DataArray type=\"Int32\" Name=\"types\" format=\"binary\">" << endl; 
        for(int i = 0 ; i < mesh->type.size() ; i++)       
            fout.write((char*)&mesh->type[i], sizeof(int));
        fout << endl;
        fout << "\t\t\t\t</DataArray>" << endl;

        fout << "\t\t\t</Cells>" << endl;
        fout << "\t\t</Piece>" << endl;
        fout << "\t</UnstructuredGrid>" << endl;
        fout << "</VTKFile>" << endl;

        fout.close();
        cout << "Writing completed successfully" << endl;
    }
}


void MeshVTKWriterInternalBin(mesh_t* mesh, const char* filename, int timeStep, int* npart, int* epart, int* color, double* velocity, float* pressure)
{
    cout << "Writing VTK internal elements..." << endl;
    std::ofstream fout;

    if(timeStep)
    {
        string str = filename;
        str.insert(str.length() - 4, "_" + to_string(timeStep));

        fout.open(str);
    }
    else{
            fout.open(filename);
    }

    if(fout.is_open())
    {

        fout << "<VTKFile type=\"UnstructuredGrid\" version=\"1.0\" byte_order=\"LittleEndian\" header_type=\"UInt64\">" << endl;
        fout << "\t<UnstructuredGrid>" << endl;
        fout << "\t\t<Piece NumberOfPoints=\"" << mesh->n_nodes <<"\" NumberOfCells=\""<< mesh->n_elements << "\">" << endl;
        fout << "\t\t\t<PointData>" << endl;
        if(npart)
        {
            fout << "\t\t\t\t <DataArray type=\"Int32\" Name=\"npart\" format=\"binary\">" << endl;
            for(int i = 0 ; i < mesh->n_nodes ; i++)
                fout.write((char*)&npart[i], sizeof(int));
            fout << endl;
            fout << "\t\t\t\t </DataArray> " << endl;
        }
        if(velocity)
        {
            fout << "\t\t\t\t <DataArray type=\"Float64\" Name=\"velocity\" format=\"binary\">" << endl;
            for(int i = 0 ; i < mesh->n_nodes*3 ; i++)
                fout.write((char*)&velocity[i], sizeof(double));
            fout << endl;
            fout << "\t\t\t\t </DataArray> " << endl;
        }
        if(pressure)
        {
            fout << "\t\t\t\t <DataArray type=\"Float32\" Name=\"pressure\" format=\"binary\">" << endl;
            for(int i = 0 ; i < mesh->n_nodes ; i++)
                fout.write((char*)&pressure[i], sizeof(float));
            fout << endl;
            fout << "\t\t\t\t </DataArray> " << endl;
        }
        fout << "\t\t\t</PointData>" << endl;
        fout << "\t\t\t<CellData>" << endl;
        if(epart)
        {
            fout << "\t\t\t\t <DataArray type=\"Int32\" Name=\"epart\" format=\"binary\">" << endl;
            for(int i = 0 ; i < mesh->n_elements; i++)
                fout.write((char*)&epart[i], mesh->n_elements*sizeof(int));
            fout << endl;
            fout << "\t\t\t\t </DataArray> " << endl;
        }
        if(color)
        {
            fout << "\t\t\t\t <DataArray type=\"Int32\" Name=\"Color\" format=\"binary\">" << endl;
            for(int i = 0 ; i < mesh->n_internal_colors ; i++)
            {
                for(int j = 0 ; j < color[i] ; j++)
                {
                    int colorAux = i+1;
                    fout.write((char*)&colorAux, sizeof(int));
                }
            }
            fout << endl;
            fout << "\t\t\t\t </DataArray> " << endl;
        }
        fout << "\t\t\t</CellData>" << endl;
        fout << "\t\t\t<Points>" << endl;

        fout << "\t\t\t\t<DataArray type=\"Float64\" Name=\"Points\" NumberOfComponents=\"3\" format=\"binary\">" << endl;
        for(int i = 0 ; i < mesh->n_nodes*3 ; i++)
            fout.write((char*)&mesh->coord[i], sizeof(double));
        fout << endl;
        fout << "\t\t\t\t</DataArray>" << endl;

        fout << "\t\t\t</Points>" << endl;
        fout << "\t\t\t<Cells>" << endl;

        fout << "\t\t\t\t<DataArray type=\"Int32\" Name=\"connectivity\" format=\"binary\">" << endl;    
        int ofs = mesh->offset[mesh->n_face_elements];
        fout.write((char*)&mesh->conn[ofs], (mesh->conn.size() - ofs)*sizeof(int));
        fout << endl;
        fout << "\t\t\t\t</DataArray>" << endl;

        fout << "\t\t\t\t<DataArray type=\"Int32\" Name=\"offsets\" format=\"binary\">" << endl;       
        fout.write((char*)&mesh->offset[mesh->n_face_elements], (mesh->offset.size() - ofs)*sizeof(int));
        fout << endl;
        fout << "\t\t\t\t</DataArray>" << endl;

        fout << "\t\t\t\t<DataArray type=\"Int32\" Name=\"types\" format=\"binary\">" << endl;
        fout.write((char*)&mesh->type[mesh->n_face_elements], (mesh->type.size() - mesh->n_face_elements)*sizeof(int));
        fout << endl;
        fout << "\t\t\t\t</DataArray>" << endl;

        fout << "\t\t\t</Cells>" << endl;
        fout << "\t\t</Piece>" << endl;
        fout << "\t</UnstructuredGrid>" << endl;
        fout << "</VTKFile>" << endl;

        fout.close();

        cout << "Writing completed successfully" << endl;
    }
}
*/