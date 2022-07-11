#include <iostream>
#include <cmath>
#include <string>

#include "meshtools.h"
#include "mesh.h"
#include "mesh_part.h"
#include "parallel_mesh.h"
#include "GetPot.hpp"

#include "petsc.h"

using namespace std;




void TestKSPMatrix(ParallelMesh *pmesh)
{



}

void TestPETScMatrix(ParallelMesh *pmesh)
{

    Mat A;
    PetscInt Istart;
    PetscInt Iend;

    int n_local_eq = pmesh->get_n_local_nodes();

    // Preallocation Matrix
    std::vector<std::set<PetscInt>> vdiag(n_local_eq);
    std::vector<std::set<PetscInt>> voff(n_local_eq);

    auto &gindex = pmesh->getLocal2Global();
    unsigned int start = pmesh->get_start_global_index();
    unsigned int end = start + n_local_eq;

    // Getting the d_nnz e o_nnz vector needed to matrix preallocation
    for (int iel = 0; iel < pmesh->get_n_elements(); ++iel)
    {
        int connsz = pmesh->getElementConnSize(iel);
        unsigned int *conn = pmesh->getElementConn(iel);
        for (int i = 0; i < connsz; ++i)
        {
            unsigned int gi = gindex[conn[i]];
            unsigned int li = gi - start;
            if (gi >= start && gi < end)
            {
                for (int j = 0; j < connsz; ++j)
                {
                    unsigned int gj = gindex[conn[j]];
                    if (gj >= start && gj < end)
                        vdiag[li].insert(gj);
                    else
                        voff[li].insert(gj);
                }
            }
        }
    }

    std::vector<PetscInt> d_nnz(n_local_eq);
    std::vector<PetscInt> o_nnz(n_local_eq);
    for (int i = 0; i < n_local_eq; i++)
    {
        d_nnz[i] = vdiag[i].size();
        o_nnz[i] = voff[i].size();
    }

    MatCreate(MeshTools::Comm(), &A);
    MatSetSizes(A, n_local_eq, n_local_eq, PETSC_DETERMINE, PETSC_DETERMINE);
    MatSetFromOptions(A);
    MatMPIAIJSetPreallocation(A, PETSC_DEFAULT, &d_nnz[0], PETSC_DEFAULT, &o_nnz[0]);
    MatSeqAIJSetPreallocation(A, PETSC_DEFAULT, &d_nnz[0]);
    MatSetOption(A, MAT_NEW_NONZERO_LOCATION_ERR, PETSC_FALSE);

    MatGetOwnershipRange(A, &Istart, &Iend);

    // Check whether global range computed by meshtools is the same
    //  that computed by petsc
    assert(Istart == start);
    assert(Iend == end);

    // simulates a FE assembly
    for (int iel = 0; iel < pmesh->get_n_elements(); ++iel)
    {
        int connsz         = pmesh->getElementConnSize(iel);
        unsigned int *conn = pmesh->getElementConn(iel);

        PetscInt indices[connsz];
        PetscScalar mat[connsz][connsz];

        for (int i = 0; i < connsz; ++i)
        {
            indices[i] = gindex[conn[i]];
            for (int j = 0; j < connsz; ++j)
            {
                mat[i][j] = 1.0;
            }
        }

        MatSetValues(A, connsz, indices, connsz, indices, &mat[0][0], ADD_VALUES);
    }

    MatAssemblyBegin(A, MAT_FINAL_ASSEMBLY);
    MatAssemblyEnd(A, MAT_FINAL_ASSEMBLY);

    // Show the matrix
    MatView(A, PETSC_VIEWER_STDOUT_WORLD);

    MatDestroy(&A);
}

void TestPETScISVector(ParallelMesh *pmesh)
{

    Vec xlocal;  // size n - nodes
    Vec x; // size n - eq
    IS is_local;
    IS is_global;
    VecScatter scatter;
    std::vector<unsigned int> local_ghosts_nodes;
    std::vector<unsigned int> global_ghosts_nodes;

    int n_nodes = pmesh->get_n_nodes();
    int n_local = pmesh->get_n_local_nodes(); // n_nodes - n_ghost_nodes
    auto &gindex = pmesh->getLocal2Global();

    pmesh->getGhostNodesIds(local_ghosts_nodes, global_ghosts_nodes);

    ISCreateGeneral(MeshTools::Comm(), local_ghosts_nodes.size(), (PetscInt *)&local_ghosts_nodes[0], PETSC_COPY_VALUES, &is_local);
    ISCreateGeneral(MeshTools::Comm(), global_ghosts_nodes.size(), (PetscInt *)&global_ghosts_nodes[0], PETSC_COPY_VALUES, &is_global);

    VecCreateSeq(PETSC_COMM_SELF, n_nodes, &xlocal);
    VecCreateMPI(MeshTools::Comm(), n_local, PETSC_DETERMINE, &x);

    VecScatterCreate(x, is_global, xlocal, is_local, &scatter);
    PetscScalar *values;
    PetscInt array_local_size;
    VecGetLocalSize(x, &array_local_size);
    VecGetArray(x, &values);

    for (int i = 0; i < array_local_size; ++i)
        values[i] = MeshTools::processor_id();

    VecRestoreArray(x, &values);

    VecScatterBegin(scatter, x, xlocal, INSERT_VALUES, SCATTER_FORWARD);
    VecScatterEnd(scatter, x, xlocal, INSERT_VALUES, SCATTER_FORWARD);

    VecView(xlocal, PETSC_VIEWER_STDOUT_SELF);

    ISDestroy(&is_local);
    ISDestroy(&is_global);
    VecScatterDestroy(&scatter);
    VecDestroy(&xlocal);
    VecDestroy(&x);
}

void PrintMenu(std::string exec_name)
{
    std::cout << "\nUsage: ./" << exec_name << " [options] \n"
              << "  -h             : print help menu    \n"
              << "  -m [gmsh file] : read gmsh file     \n";
}

int main(int argc, char *argv[])
{
    MeshTools::Init(argc, argv);
    GetPot cl(argc, argv);

    Mesh *mesh = nullptr;
    ParallelMesh *pmesh = nullptr;
    MeshPartition *partitioner = new MeshPartition();

    if (cl.size() == 1 || cl.search("-h"))
    {
        if (MeshTools::processor_id() == 0)
            PrintMenu(cl[0]);

        MeshTools::Finalize();
        return 0;
    }

    if (!cl.search("-m"))
    {
        if (MeshTools::processor_id() == 0)
            PrintMenu(argv[0]);

        MeshTools::Finalize();
        return 0;
    }

    const string filename = cl.next(std::string(""));

    if (MeshTools::processor_id() == 0)
    {
        mesh = new Mesh();
        mesh->MeshGmshReader(filename.c_str());
        std::vector<unsigned int> gindex(mesh->get_n_nodes());
        for (int i = 0; i < gindex.size(); i++)
            gindex[i] = i;
        MeshIODataAppended info;
        info.addPointDataInfo("Index", UInt32, (void *)&gindex[0]);
        mesh->WriteVTK("serial", &info);
    }

    pmesh = partitioner->DistributedMesh(mesh);
    {
        MeshIODataAppended info;
        auto &l2g = pmesh->getLocal2Global();
        info.addPointDataInfo("Index", UInt32, (void *)&l2g[0]);
        pmesh->update();
        pmesh->writePVTK("parallel", &info);
        pmesh->WritePMesh("mesh");
    }

    TestPETScMatrix(pmesh);

    TestPETScISVector(pmesh);

    if (mesh)
        delete mesh;
    if (partitioner)
        delete partitioner;
    if (pmesh)
        delete pmesh;

    MeshTools::Finalize();
    return 0;
}