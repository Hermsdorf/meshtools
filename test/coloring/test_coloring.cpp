#include <iostream>
#include <memory>

#include "test_config.h"
#include "mesh.h"
#include "mesh_part.h"

using namespace std;

void WriteAdj(const char *fname, int nvts, unsigned int *xadj, unsigned int *adjncy)
{
    std::ofstream fout;
    fout.open(fname);
    if (fout.is_open())
    {

        fout << nvts << "\n";
        for (int i = 0; i <= nvts; i++)
            fout << xadj[i] << "\n";

        for (int i = 0; i < xadj[nvts]; i++)
            fout << adjncy[i] << "\n";

        fout.close();
    }
}

int main()
{       
    std::vector<std::string> meshes;
    
    meshes.push_back(TEST_MESH_DIR +std::string("/msh/arteria/arteria.msh"));
    //meshes.push_back(TEST_MESH_DIR +std::string("/msh/tesla/tesla.msh"));

    std::vector<std::string> vtk_filenames;
    vtk_filenames.push_back("arteria");
    vtk_filenames.push_back("tesla");
    for(int i = 0 ; i < meshes.size() ; i++)
    {
        Mesh* mesh = new Mesh(meshes[i]);

        mesh->MeshReordering(METIS_ND);
        std::string nd_adj_name = vtk_filenames[i]+"_metisnd.adj";
        WriteAdj(nd_adj_name.c_str(), mesh->get_n_nodes(), mesh->getOffset().data(), mesh->getConn().data());

        delete mesh;
        mesh = new Mesh(meshes[i]);

        mesh->MeshReordering(RCM);
        std::string rcm_adj_name = vtk_filenames[i]+"_rcm.adj";
        WriteAdj(rcm_adj_name.c_str(), mesh->get_n_nodes(), mesh->getOffset().data(), mesh->getConn().data());

        mesh->MeshColoring(COLOR_DEFAULT);
        std::vector<unsigned int> coloring_greedy = mesh->getColoring();
        cout << "# Elements per color (Greedy):";
        for (unsigned int i = 0; i < coloring_greedy.size(); i++)
            cout << " " << coloring_greedy[i];
        cout << endl;

        unsigned int color = 0;
        unsigned int iel = 0;
        std::vector<unsigned int> elements_color(mesh->get_n_elements());
        std:fill(elements_color.begin(), elements_color.end(), 0);
        while(color < coloring_greedy.size())
        {
            for (unsigned int i = 0; i < coloring_greedy[color]; i++){
                elements_color[iel] = color+1;
                iel++;
            }

            color++;
        }

        mesh->MeshColoring(COLOR_DEFAULT_BLOCK, 4096);
        std::vector<unsigned int> coloring_blocked = mesh->getColoring();
        cout << "# Elements per color (Blocked):";
        for (unsigned int i = 0; i < coloring_blocked.size(); i++)
            cout << " " << coloring_blocked[i];
        cout << endl;

        MeshIODataAppended* io = new MeshIODataAppended();
        
        MeshPartition *parts    = new MeshPartition();

        parts->ApplyPartitioner(mesh, 4);

        io->addCellDataInfo("partition_elem", Int32, parts->get_elem_part());
        //io->addPointDataInfo("partition_nodes", Int32, parts->get_nodal_part());
        
        io->addCellDataInfo("color", UInt32, elements_color.data());
        mesh->WriteVTK(vtk_filenames[i].c_str(), io);

        delete parts;
        delete mesh;
        delete io;
    }
    return 0;
}

// using namespace std;

// int main()
// {         
//     Mesh* mesh = new Mesh("quadrado.msh");
//     Mesh* mesh_trid = new Mesh("trid.msh");
    
//     cout << "Testing coloring algorithm in a bidimensional mesh...\n";
//     mesh->MeshReordering(RCM);
//     mesh->MeshColoring_test();

//     cout << "Testing coloring algorithm in a tridimensional mesh...\n";
//     mesh_trid->MeshReordering(RCM);
//     mesh_trid->MeshColoring_test();
    
//     delete mesh;
//     delete mesh_trid;
//     return 0;
// }
