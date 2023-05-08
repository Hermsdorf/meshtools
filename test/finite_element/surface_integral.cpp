#include "meshtools.h"
#include "mesh.h"
#include "mesh_part.h"
#include "parallel_mesh.h"
#include "boundary_fem_functions.h"
#include "numeric_vector.h"


int surface_integral(int argc, char *argv[], std::string mesh_path, std::string mesh_file)
{
    MeshPartition *parts = new MeshPartition();

    Mesh *mesh;          // serial mesh
    ParallelMesh *pmesh; // parallel mesh
    int processor_id, n_processors;

    MeshTools::Init(argc, argv);
    processor_id = MeshTools::processor_id();
    n_processors = MeshTools::n_processors();

    double local_integral = 0.0;
    double global_integral = 0.0;

    if (processor_id == 0)
    {
        // Rodando serial ou em paralelo o processo mestre
        // irá ler a malha.
        mesh = new Mesh(mesh_path + mesh_file);

        // Se houver mais um processo, o processo mestre irá
        // particionar a malha
        if (n_processors > 1)
        {
            parts->ApplyPartitioner(mesh, n_processors);
        }
    }

    pmesh = parts->DistributedMesh(mesh);

    QGauss qrule;
    BoundaryFEMFunction bfem;
        
    std::vector<double>   & phi = bfem.get_phi();
    std::vector<Gradient> & dphi= bfem.get_dphi();
    double                & JxW    = bfem.get_JxW();
    Point                 & qpoint = bfem.get_xyz();

    bool flag = true;
    unsigned short elem_type;
    
    // loop sobre os elementos da malha
    for (int iel = 0; iel < pmesh->get_n_face_elements() ; iel++)
    {
        SurfaceElement surface_elem;
        pmesh->getSurfaceElement(iel, surface_elem);
        if(surface_elem.region() == 6)
        {
            elem_type = surface_elem.type();
            int nnoel = surface_elem.n_nodes();

            // Obtem pontos de integração para elemento elem
            qrule.reset(surface_elem);

            // loop sobre os pontos de integração
            for (int q = 0; q < qrule.n_points(); q++)
            {
                // calculando a função de forma e suas derivadas para o ponto de integração q
                bfem.ComputeFunction(surface_elem, qrule.get(q));

                for(int n = 0 ; n < 2 ; n++)
                {
                    local_integral += (JxW * 1 * phi[n]);
                    std::cout << "JxW " << JxW << std::endl;
                    std::cout << "phi " << phi[q] << std::endl;
                }
            }
        }
    }

    MPI_Reduce(&local_integral, &global_integral, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    
    if (MeshTools::processor_id() == 0)
    {   
        double expected_value;
        // if EDGE2 elements, it means it is a 2D problem
        if(elem_type == 1)
            expected_value = 2*M_PI*0.25;
        else
            expected_value = 4*M_PI*0.25*0.25;
        std::cout << "Elem type " << elem_type << std::endl;
        std::cout << "Integral value " << global_integral << std::endl;
        std::cout << "Expected value " << expected_value << std::endl;
        delete mesh;
    }
    delete pmesh;
    delete parts;

    MeshTools::Finalize();

    return 0;
}

int main(int argc, char *argv[])
{
    std::string meshfile = std::string(argv[1]);
    surface_integral(argc, argv, "", meshfile);

    return 0;
}
