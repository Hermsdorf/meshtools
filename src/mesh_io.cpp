#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <string>

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


void Mesh::MeshGmshReader(const char* filename)
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
                    delete this;
                    exit(1);
                }
                
                if(format)
                {
                    cout << "ERRO: FORMATO NAO SUPORTADO\n";
                    cout << "Formato suportado: ASCII \n";
                    in.close();
                    delete this;
                    exit(1);
                }
            }
            // Read and process the "PhysicalNames" section.
            else if (s.find("$PhysicalNames") == 0)
            {

                // Read in the number of physical groups to expect in the file.
                unsigned int num_physical_groups = 0;
                in >> num_physical_groups;

                for (unsigned int i=0; i<num_physical_groups; ++i)
                {                    
                    int phy_dim, phy_id;
                    string phy_name;

                    in >> phy_dim >> phy_id >> phy_name;

                    this->physical_map[phy_id] = std::make_pair(phy_dim, phy_name);
                    
                }
            }
            else if(s.find("$Nodes") == 0) 
            {
                unsigned int num_nodes = 0;
                in >> num_nodes;
                this->n_nodes = num_nodes;

                this->coord.resize(num_nodes*3);

                int node_id;
                double x,y,z;
                for(int i = 0; i < num_nodes; i++) {
                    in >> node_id >> x >> y >> z;
                    this->coord[(i*3)+0] = x;
                    this->coord[(i*3)+1] = y;
                    this->coord[(i*3)+2] = z;
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

                this->physical_tag.resize(num_elem);
                this->offset.resize(num_elem+1);
                this->type.resize(num_elem);
                this->offset[0] = 0;

                int iel = 0;
                for(int i = 0; i < num_elem; i++)
                {
                    int id, type, physical=1, elementary=1, nnodes=0, ntags, elem_dim;

                    in >> id >> type >> ntags;
#ifdef DEBUG_                   
                   cout << id << "  " << type << "  " << ntags << " ";
#endif

                    this->type[i] = GmshToVTKType(type);

                    nnodes   = getGmshElemNNodes(type);
                    elem_dim = getGmshElemTypeDim(type);

                    if(nnodes < 0 )
                    {
                        cout << "ERRO: TIPO DO ELEMENTO " << type << " INVALIDO";
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
                        cout << physical << " ";
#endif
                    }

                    for (unsigned int j=0; j<nnodes; j++)
                    {
                        in >> node_id;
                        this->conn.push_back(node_id-1);
#ifdef DEBUG_
                        cout << node_id << " ";
#endif
                    }

                    this->offset[i+1] =  this->offset[i] + nnodes;
#ifdef DEBUG_
                    cout << endl;
                    cout << "OFFSET: " << this->offset[i+1] << " TYPE: "<< this->type[i] << endl;
#endif                   
                }

                // read the $ENDELM delimiter
                std::getline(in, s);

            } // End Elem
        } // end if(in)   

    } // end while (true)
       
    if(dim_count[3] != 0)
    {
        this->n_elements = dim_count[3];
        this->n_face_elements = dim_count[2];
    } else if (dim_count[2] != 0)
    {
        this->n_elements      = dim_count[2];
        this->n_face_elements = dim_count[1];
    }
    
    this->mesh_coloring_internal = new int [this->n_elements];

    for(int i = 0 ; i < this->n_elements ; i++)
        this->mesh_coloring_internal[i] = -1;

    
    string str = filename;
    str.resize(str.length()-4);
    str = str.append(".vtu");

    this->filename = str;

    cout << " Num. Nodes: " << this->n_nodes << endl;
    cout << " Num. Elements: "          << this->n_elements << endl;
    cout << " Num. Boundary Elements: " << this->n_face_elements << endl;
    cout << " Connectivity size: " << this->conn.size() << endl;

    in.close();
}

void Mesh::MeshVTKWriter(int timeStep, int *npart, int* epart, int* color, double* velocity, float* pressure)
{
    cout << "Writing VTK boundary and internal elements..." << endl;
    std::ofstream fout;

    if(timeStep)
    {
        string str = this->getFilename();
        str.insert(str.length() - 4, "_" + to_string(timeStep));

        fout.open(str);
    }
    else{
            fout.open(this->getFilename());
    }

    if(fout.is_open())
    {
        int nnodes = this->n_nodes;
        int nelem = this->n_elements + this->n_face_elements;
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
            for(int i = 0 ; i < this->n_nodes * 3 ; i++)
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
            for(int i = 0 ; i < this->n_nodes ; i++)
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
            for(int i = 0 ; i < this->n_face_elements ; i++)
            {
                if(i % 18 == 0 && i != 0)
                    fout << endl << "\t\t\t\t\t";
                fout << -1 << " "; // cor dos elementos de superficie
            }
            for(int i = 0 ; i < this->n_internal_colors ; i++)
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
        
        for(int i = 0 ; i < this->coord.size() ; i++)
        {
            if(i % 18 == 0 && i != 0)
                fout << endl << "\t\t\t\t\t";

            fout << this->coord[i] << " ";
        }
        fout << endl;
        fout << "\t\t\t\t</DataArray>" << endl;
        fout << "\t\t\t</Points>" << endl;
        fout << "\t\t\t<Cells>" << endl;
        fout << "\t\t\t\t<DataArray type=\"Int32\" Name=\"connectivity\" format=\"ascii\">" << endl;
        fout << "\t\t\t\t\t";
        
        for(int i = 0 ; i < this->conn.size() ; i++)
        {
            if(i % 18 == 0 && i != 0)
                fout << endl << "\t\t\t\t\t";

            fout << this->conn[i] << " ";
        }
        fout << endl;
        fout << "\t\t\t\t</DataArray>" << endl;
        fout << "\t\t\t\t<DataArray type=\"Int32\" Name=\"offsets\" format=\"ascii\">" << endl;
        fout << "\t\t\t\t\t";
        for(int i = 1 ; i < this->offset.size() ; i++)
        {
            if(i % 18 == 0 && i != 0)
                fout << endl << "\t\t\t\t\t";

            fout << this->offset[i] << " ";
        }
        fout << endl;
        fout << "\t\t\t\t</DataArray>" << endl;
        fout << "\t\t\t\t<DataArray type=\"Int32\" Name=\"types\" format=\"ascii\">" << endl;
        fout << "\t\t\t\t\t";
        
        for(int i = 0 ; i < this->type.size() ; i++)
        { 
            if(i % 18 == 0 && i != 0)
                fout << endl << "\t\t\t\t\t";

            fout << this->type[i] << " ";
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


void Mesh::MeshVTKWriterInternal(int timeStep, int* npart, int* epart, int* color, double* velocity, float* pressure)
{
    cout << "Writing VTK internal elements..." << endl;
    std::ofstream fout;

    if(timeStep)
    {
        string str = this->getFilename();
        str.insert(str.length() - 4, "_" + to_string(timeStep));

        fout.open(str);
    }
    else{
            fout.open(this->getFilename());
    }

    if(fout.is_open())
    {

        fout << "<VTKFile type=\"UnstructuredGrid\" version=\"1.0\" byte_order=\"LittleEndian\" header_type=\"UInt64\">" << endl;
        fout << "\t<UnstructuredGrid>" << endl;
        fout << "\t\t<Piece NumberOfPoints=\"" << this->n_nodes <<"\" NumberOfCells=\""<< this->n_elements << "\">" << endl;
        fout << "\t\t\t<PointData>" << endl;
        if(npart)
        {
            fout << "\t\t\t\t <DataArray type=\"Int32\" Name=\"npart\" format=\"ascii\">" << endl;
            fout << "\t\t\t\t\t";
            for(int i = 0 ; i < this->n_nodes ; i++)
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
            for(int i = 0 ; i < this->n_nodes * 3 ; i++)
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
            for(int i = 0 ; i < this->n_nodes ; i++)
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
            for(int i = 0 ; i < this->n_elements ; i++)
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
            for(int i = 0 ; i < this->n_internal_colors ; i++)
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
        for(int i = 0 ; i < this->coord.size() ; i++)
        {
            if(i % 18 == 0 && i != 0)
                fout << endl << "\t\t\t\t\t";

            fout << this->coord[i] << " ";
        }
        fout << endl;
        fout << "\t\t\t\t</DataArray>" << endl;
        fout << "\t\t\t</Points>" << endl;
        fout << "\t\t\t<Cells>" << endl;
        fout << "\t\t\t\t<DataArray type=\"Int32\" Name=\"connectivity\" format=\"ascii\">" << endl;
        fout << "\t\t\t\t\t";
        
        int ofs = this->offset[this->n_face_elements];

        for(int i = ofs ; i < this->conn.size() ; i++)
        {
            if(i % 18 == 0 )
                fout << endl << "\t\t\t\t\t";

            fout << this->conn[i] << " ";
        }
        fout << endl;
        fout << "\t\t\t\t</DataArray>" << endl;
        fout << "\t\t\t\t<DataArray type=\"Int32\" Name=\"offsets\" format=\"ascii\">" << endl;
        fout << "\t\t\t\t\t";
        
        for(int i = this->n_face_elements ; i < this->offset.size()-1 ; i++)
        {
            if(i % 18 == 0 && i != 0)
                fout << endl << "\t\t\t\t\t";

            fout << this->offset[i+1]- ofs << " ";
        }
        fout << endl;
        fout << "\t\t\t\t</DataArray>" << endl;
        fout << "\t\t\t\t<DataArray type=\"Int32\" Name=\"types\" format=\"ascii\">" << endl;
        fout << "\t\t\t\t\t";
        
        for(int i = this->n_face_elements ; i < this->type.size() ; i++)
        { 
            if(i % 18 == 0 && i != 0)
                fout << endl << "\t\t\t\t\t";

            fout << this->type[i] << " ";
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

void Mesh::MeshVTKWriterInternalBinAppended(int timeStep, int* npart, int* epart, int* color, double* velocity, float* pressure)
{
    cout << "Writing VTK internal elements..." << endl;
    
    FILE*         fout;
    unsigned int boffset = 0; /* Offset into binary file */
    const char *byte_order = BinaryBigEndian() ? "BigEndian" : "LittleEndian";

    string str = this->getFilename();
    if(timeStep)
        str.insert(str.length() - 4, "_" + to_string(timeStep));

    fout = fopen(str.c_str(), "wb");

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
        sz = this->offset.size() - (this->n_face_elements + 1);
        boffset += sz*sizeof(int) + sizeof(unsigned long);

        fprintf(fout,"        <DataArray type=\"%s\" Name=\"%s\" NumberOfComponents=\"%d\" format=\"appended\" offset=\"%d\" />\n","UInt16","types",1, boffset);
        boffset += this->n_elements*sizeof(unsigned short) + sizeof(unsigned long);
        fprintf(fout, "   </Cells>\n");


        // Writting nodal attribute data
        fprintf(fout, "   <PointData>\n");
        if(npart)
        {
            fprintf(fout, "        <DataArray type=\"%s\" Name=\"%s\" NumberOfComponents=\"%d\" format=\"appended\" offset=\"%d\" />\n","Int32","npart",1,boffset);
            boffset += this->n_nodes*sizeof(int) + sizeof(unsigned long);
        }
        if(velocity)
        {
            fprintf(fout, "        <DataArray type=\"%s\" Name=\"%s\"  NumberOfComponents=\"%d\" format=\"appended\" offset=\"%d\" />\n","Float64","velocity",3,boffset);
            boffset += 3*this->n_nodes*sizeof(double) + sizeof(unsigned long);
        }
        if(pressure)
        {
            fprintf(fout, "        <DataArray type=\"%s\" Name=\"%s\"  NumberOfComponents=\"%d\" format=\"appended\" offset=\"%d\" />\n","Float32","pressure",1,boffset);
            boffset += this->n_nodes*sizeof(float) + sizeof(unsigned long);
        }

        fprintf(fout, "   </PointData>\n");
        fprintf(fout, "   <CellData>\n");
       
        if(epart)
        {
            fprintf(fout, "        <DataArray type=\"%s\" Name=\"%s\"  NumberOfComponents=\"%d\" format=\"appended\" offset=\"%d\" />\n","Int32","epart",1, boffset);
            boffset += this->n_elements*sizeof(int) + sizeof(unsigned long);
        }
        if(color)
        {
                  
            fprintf(fout, "        <DataArray type=\"%s\" Name=\"%s\"  NumberOfComponents=\"%d\" format=\"appended\" offset=\"%d\" />\n","Int32","Color",1, boffset);
            boffset += this->n_elements*sizeof(int) + sizeof(unsigned long);
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

        // Writing attribute data
        if(npart)
        {
            nbytes = sizeof(int)*this->n_nodes;
            fwrite((void*)&nbytes, sizeof(unsigned long),1,fout);
            fwrite((void*)npart,sizeof(int), this->n_nodes,fout);
        }
        if(velocity)
        {
            nbytes = sizeof(double)*3*this->n_nodes;
            fwrite((void*)&nbytes, sizeof(unsigned long),1,fout);
            fwrite((void*)velocity,sizeof(double), 3*this->n_nodes,fout);
        }
        if(pressure)
        {
            nbytes = sizeof(float)*this->n_nodes;
            fwrite((void*)&nbytes, sizeof(unsigned long),1,fout);
            fwrite((void*)pressure,sizeof(float), this->n_nodes,fout);
        }
        if(epart)
        {
            nbytes = sizeof(int)*this->n_elements;
            fwrite((void*)&nbytes, sizeof(unsigned long),1,fout);
            fwrite((void*)epart,sizeof(int), this->n_elements,fout);
        }
        if(color)
        {
            nbytes = sizeof(int)*this->n_elements;
            fwrite((void*)&nbytes, sizeof(unsigned long),1,fout);

            for(int i = 0 ; i < this->n_internal_colors ; i++)
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

void Mesh::MeshVTKWriterBinAppended(int timeStep, int* npart, int* epart, int* color, double* velocity, float* pressure)
{
    cout << "Writing VTK internal elements..." << endl;
    
    FILE*         fout;
    unsigned int boffset = 0; /* Offset into binary file */
    const char *byte_order = BinaryBigEndian() ? "BigEndian" : "LittleEndian";

    string str = this->getFilename();
    if(timeStep)
        str.insert(str.length() - 4, "_" + to_string(timeStep));

    fout = fopen(str.c_str(), "wb");

    if(fout)
    {
        // Writting header info
        fprintf(fout, "<VTKFile type=\"UnstructuredGrid\" version=\"1.0\" byte_order=\"%s\" header_type=\"UInt64\">\n", byte_order);
        fprintf(fout, " <UnstructuredGrid>\n");
        fprintf(fout, "  <Piece NumberOfPoints=\"%d\" NumberOfCells=\"%d\">\n", this->n_nodes, this->n_elements + this->n_face_elements);

        // writing mesh info
        fprintf(fout, "   <Points>\n");
        fprintf(fout, "        <DataArray type=\"%s\" Name=\"%s\" NumberOfComponents=\"%d\" format=\"appended\" offset=\"%d\" />\n","Float64","Points",3, boffset);
        boffset += 3*this->n_nodes*sizeof(double) + sizeof(unsigned long);
        
        fprintf(fout, "   </Points>\n") ;
        fprintf(fout, "   <Cells>\n");

        fprintf(fout, "        <DataArray type=\"%s\" Name=\"%s\" NumberOfComponents=\"%d\" format=\"appended\" offset=\"%d\" />\n","Int32","connectivity",1, boffset);
        boffset += this->conn.size()*sizeof(int) + sizeof(unsigned long);


        fprintf(fout, "        <DataArray type=\"%s\" Name=\"%s\" NumberOfComponents=\"%d\" format=\"appended\" offset=\"%d\" />\n","Int32","offsets",1, boffset);
        boffset += (this->offset.size()-1)*sizeof(int) + sizeof(unsigned long);

        fprintf(fout,"        <DataArray type=\"%s\" Name=\"%s\" NumberOfComponents=\"%d\" format=\"appended\" offset=\"%d\" />\n","UInt16","types",1, boffset);
        boffset += this->type.size()*sizeof(unsigned short) + sizeof(unsigned long);

        fprintf(fout, "   </Cells>\n");

        // Writting nodal attribute data
        fprintf(fout, "   <PointData>\n");
        if(npart)
        {
            fprintf(fout, "        <DataArray type=\"%s\" Name=\"%s\" NumberOfComponents=\"%d\" format=\"appended\" offset=\"%d\" />\n","Int32","npart",1,boffset);
            boffset += this->n_nodes*sizeof(int) + sizeof(unsigned long);
        }
        if(velocity)
        {
            fprintf(fout, "        <DataArray type=\"%s\" Name=\"%s\"  NumberOfComponents=\"%d\" format=\"appended\" offset=\"%d\" />\n","Float64","velocity",3,boffset);
            boffset += 3*this->n_nodes*sizeof(double) + sizeof(unsigned long);
        }
        if(pressure)
        {
            fprintf(fout, "        <DataArray type=\"%s\" Name=\"%s\"  NumberOfComponents=\"%d\" format=\"appended\" offset=\"%d\" />\n","Float32","pressure",1,boffset);
            boffset += this->n_nodes*sizeof(float) + sizeof(unsigned long);
        }

        fprintf(fout, "   </PointData>\n");
        fprintf(fout, "   <CellData>\n");
        
        // Writting element attribute data
        if(epart)
        {
            fprintf(fout, "        <DataArray type=\"%s\" Name=\"%s\"  NumberOfComponents=\"%d\" format=\"appended\" offset=\"%d\" />\n","Int32","epart",1, boffset);
            boffset += (this->n_elements+this->n_face_elements)*sizeof(int) + sizeof(unsigned long);
        }
        if(color)
        {
            fprintf(fout, "        <DataArray type=\"%s\" Name=\"%s\"  NumberOfComponents=\"%d\" format=\"appended\" offset=\"%d\" />\n","Int32","Color",1, boffset);
            boffset += (this->n_elements+this->n_face_elements)*sizeof(int) + sizeof(unsigned long);
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
        nbytes = sizeof(int)*this->conn.size();
        fwrite((void*)&nbytes, sizeof(unsigned long),1,fout);
        fwrite((void*)&this->conn[0], sizeof(int),this->conn.size(),fout);

        // writting element offsets
        nbytes = sizeof(int)*(this->offset.size()-1);
        fwrite((void*)&nbytes, sizeof(unsigned long),1,fout);
        fwrite((void*)&this->offset[1], sizeof(int),this->offset.size()-1,fout);

        // writting types
        nbytes = sizeof(unsigned short)*(this->type.size());
        fwrite((void*)&nbytes, sizeof(unsigned long),1,fout);
        fwrite((void*)&this->type[0], sizeof(unsigned short),this->type.size(),fout);

        // Writing attribute data
        if(npart)
        {
            nbytes = sizeof(int)*this->n_nodes;
            fwrite((void*)&nbytes, sizeof(unsigned long),1,fout);
            fwrite((void*)npart,sizeof(int), this->n_nodes,fout);
        }
        if(velocity)
        {
            nbytes = sizeof(double)*3*this->n_nodes;
            fwrite((void*)&nbytes, sizeof(unsigned long),1,fout);
            fwrite((void*)velocity,sizeof(double), 3*this->n_nodes,fout);
        }
        if(pressure)
        {
            nbytes = sizeof(float)*this->n_nodes;
            fwrite((void*)&nbytes, sizeof(unsigned long),1,fout);
            fwrite((void*)pressure,sizeof(float), this->n_nodes,fout);
        }
        if(epart)
        {
            nbytes = sizeof(int)*(this->n_face_elements + this->n_elements);
            fwrite((void*)&nbytes, sizeof(unsigned long),1,fout);
            fwrite((void*)epart,sizeof(int), this->n_face_elements + this->n_elements,fout);
        }
        if(color)
        {
            nbytes = sizeof(int)*(this->n_face_elements + this->n_elements);
            fwrite((void*)&nbytes, sizeof(unsigned long),1,fout);
            int colorAux = -1; // cor dos elementos de superfície

            for(int i = 0 ; i < this->n_face_elements ; i++)
            {
                fwrite((void*)&colorAux,sizeof(int),1, fout);
            }

            for(int i = 0 ; i < this->n_internal_colors ; i++)
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