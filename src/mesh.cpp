
#include <fstream>
#include <sstream>
#include <iostream>
using namespace std;

#include "mesh.h"

static int element_type[6] = {-1, 2, 3, 4, 4, 8};

mesh_t* MeshGMSHReader(const char* filename)
{

    int format=0, size=0;
    double version = 1.0;
    string s;

    mesh_t *mesh = new mesh_t();

    std::ifstream in(filename);

    while(true)
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
                std::getline(in, s);

                for (unsigned int i=0; i<num_physical_groups; ++i)
                {
                    // Read an entire line of the PhysicalNames section.
                    std::getline(in, s);

                    std::istringstream s_stream(s);
                    int phy_dim, phy_id;
                    string phy_name;
                    s_stream >> phy_dim >> phy_id >> phy_name;

                    mesh->physical_list[phy_dim] = std::make_pair(phy_dim, phy_name);


                    
                }
            }
            else if(s.find("$Nodes") == 0) 
            {
                unsigned int num_nodes = 0;
                in >> num_nodes;

                mesh->coord.resize(num_nodes*3);

                int node_id;
                double x,y,z;
                for(int i = 0; i < num_nodes; i++) {
                    in >> node_id >> x >> y >> z;
                    mesh->coord[i*3+0] = x;
                    mesh->coord[i*3+1] = y;
                    mesh->coord[i*3+2] = z;
                }

                // read the $ENDNOD delimiter
                std::getline(in, s);

            }

            else if (s.find("$Elements")==0)
            {
                int num_elem, node_id;
                in >> num_elem;

                mesh->physical_tag.resize(num_elem);
                mesh->offset.resize(num_elem+1);
                mesh->offset[0] = 0;

                for(int i = 0; i < num_elem; i++)
                {
                    unsigned int id, type, physical=1, elementary=1, nnodes=0, ntags;

                    in >> id >> type >> ntags;
                    nnodes = element_type[type];

                    if(nnodes < 0)
                    {
                        cout << "ERRO: TIPO DO ELEMENTO " << id << " INVALIDO";
                        in.close();
                        delete mesh;
                        exit(1);
                    }
                    for(int j = 0; j < ntags; j++)
                    {
                        in >> physical;
                        if(j == 0)
                            mesh->physical_tag[i] = physical;
                    }

                    for (unsigned int j=0; j<nnodes; j++)
                    {
                        in >> node_id;
                        mesh->conn.push_back(node_id-1);
                    }

                    mesh->offset[i+1] = nnodes;
                }

                // read the $ENDELM delimiter
                std::getline(in, s);

            } // End Elem
        } // end if(in)   

    } // end while (true)

    in.close();
    return mesh;
}



mesh_t* MeshReadGMSH(const char* filename)
{

    fstream leitura(filename);

    string str;
    mesh_t * mesh = new mesh_t();

    if(leitura.is_open())
    {
        while(!leitura.eof())
        {
            getline(leitura, str);
            getline(leitura, str, ' ');

            if(str != "2.2")
            {  
                cout << "ERRO: VERSAO .MSH NAO SUPORTADA\n";
                cout << "Formato suportado: 2.2\nFormato aberto: " << str << "\n";
                exit(1);
            }

            // leitura das grupos físicos
            while(str != "$Nodes")
            {
                getline(leitura, str);
            }
            getline(leitura, str);


            int numNodes = stoi(str);
        
            while(true)
            {
                getline(leitura, str, ' '); 
                int identNode = stoi(str);

                getline(leitura, str, ' '); // x
                mesh->coord.push_back(stod(str));
                getline(leitura, str, ' '); // y
                mesh->coord.push_back(stod(str));
                getline(leitura, str); // z
                mesh->coord.push_back(stod(str));

                if(identNode == numNodes)
                    break;
            }
            while(str != "$Elements")
            {
                getline(leitura, str);
            }
            getline(leitura, str);
            int numElements = stoi(str);

            mesh->offset.push_back(0); // primeiro elemento tem como padrao offset igual a 0
            while(true)
            {
                getline(leitura, str, ' '); // identElement
                int identElement = stoi(str);

                getline(leitura, str, ' '); // element type
                int numConn = element_type[stoi(str)];
                if(numConn == -1)
                {
                    cout << "ERRO: TIPO DO ELEMENTO " << identElement << " INVALIDO";
                    exit(1);
                }

                getline(leitura, str, ' '); // numero de tags
                int numTags = stoi(str);

                getline(leitura, str, ' '); // physical group
                mesh->physical_tag.push_back(stoi(str));

                int i = 1;
                while(i < numTags)
                {
                    getline(leitura, str, ' ');
                    i++;
                } // while para descartar as outras tags além do physical group

                i = 0;
                while(i < numConn)
                {
                    if(i == numConn-1)
                    {
                        getline(leitura, str);
                        mesh->conn.push_back((stoi(str))-1); // -1 pois como estamos trabalhando com um vetor, o primeiro indice igual é a 0
                                                            // a posicao dos nós no vetor está uma posicao anterior com relação ao arquivo .msh
                        break;
                    } // se for a ultima conn do elemento, usamos o getline dessa forma pra nao dar erro
                    getline(leitura, str, ' ');
                    mesh->conn.push_back(stoi(str)-1);

                    i++;
                }
                mesh->offset.push_back(mesh->offset.back() + numConn); // o offset do elemento x é dado por offset(x-1) + numConn(x)

                if(identElement == numElements)
                    break;
            }

            getline(leitura, str);
            getline(leitura, str); // para chegar ao final do arquivo
        }
    }
    else
    {
        cout << "ERRO: NAO FOI POSSIVEL ABRIR O ARQUIVO DE LEITURA";
    }

    leitura.close();
    
    return mesh;

}
