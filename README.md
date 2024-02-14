# MeshTools

Library that manipulates and solves a finite element mesh in parallel applying HPC techniques.

It is divided in two disjunct componentes: Mesh Manipulation and FEM Solver.

The user can use these two modules and obtain it results indepedently. With Meshtools it is possible to improve a mesh to apply it in other FEM software. Also, the user can improve its mesh and solve it in MeshTools.

## Mesh Manipulation
MeshTools is a finite element library that provides tools for apply high performance computing techniques in order to improve the mesh solutions. These techniques provides the possibility of solving the Finite Element Method (FEM) in parallel on shared-memory system and/or distributed-memory system. With MeshTools is also possible to improve the data locality which improves the solution performance.

With MeshTools, the user can apply the following techniques in his mesh
  1. Nodal Reordering: in this technique, the mesh nodes are renumbered focusing in get elemental connectivities numerically closer. It improves the solution's performance of the FEM method by improving the data locality. It increases the cache hit, decreasing the necessity of search the data in the main memory.
  2. Element Coloring: in order to decrease the system assembly time, it is possible to parallelize it in shared-memory with OpenMP. A safe parallelization is possible only if the data racing is avoided. The element coloring technique groups non-adjacent elements which are elements that does not share any nodes. These elements that belongs to the same group can be parallelized in the assembly step safely.
  3. Domain Partitioning: through [METIS library](http://glaros.dtc.umn.edu/gkhome/metis/metis/overview), it decomposes the mesh among MPI processors. Each MPI process solves its local mesh.

MeshTools is also linked with [ParaView Catalyst](https://www.paraview.org/Wiki/ParaView/Catalyst/Overview) which provides the co-processing. With the co-processing is possible to generate visualizations for the mesh with no necessity of write raw data in VTK format. By writing in disk directly images, tables and etc. it decreases the disk requirements.

## FEM Solver
PETSc library is coupled in MeshTools in order to solve the linear system arising from the Finite Element Method. Thus, MeshTools wraps some PETSc structures such as Mat, Vec, KSP and etc. to be able to solve the FEM problem. At the moment, MeshTools supports only linear elements. For bidimensional elements, it supports three-nodes triangular elements (TRI3) and four-nodes quadrangular elements (QUAD4). For tridimensional elements, MeshTools supports four-nodes tetrahedral elements (TET4) and six-nodes hexahedral elements (HEX6). 

## Requirements
- CMake >= 3.0.0
- METIS >= 5.1.0
- PETSc >= 3.10.0 
- Paraview Catalyst
- OpenMP
- MPI

## How to use
1. Generate a project build folder - `$ mkdir build`
2. Change directory to the new `build` folder - `$ cd build`
3. Run CMake to create the makefile with link to the necessary files - `$ cmake ../.`
   1. If the necessary packages weren't installed by the OS package manager, or if any package wasn't found, it is necessary to indicate it path as parameter. Example: `$ cmake ../. -DMETIS_DIR=/path/to/metis`
4. Run the `make` command
5. Run MeshTools

For explanation of MeshTools parameters run: `./meshtools -h`

### Mesh Manipulation
MeshTools only supports GMSH meshes version 2.0
To indicate the mesh to be processed it is necessary to use the `-m` parameter and then the path to the specified mesh.
Example:
`-m /usr/msh/test.msh`

### Nodal Reordering
MeshTools provides 3 algorithms for Nodal Reordering, namely:
1. RCM (Reverse Cuthill-McKee) [default] - `rcm`
2. Nested-Dissection - `nd`

With the `-r` parameter indicate the desired reordering algorithm, for example:
`-r na`

### Element coloring
MeshTools provides 3 Mesh Coloring algorithms, namely:
1. Greedy Serial [default] - `greedy`
2. Blocked Serial - `blocked`

In the case of execution with the `Blocked` version, it is possible to enter the maximum desired block size of elements per color with the parameter `-b`, defaulting to 4096. Example: `-b 2048`

With the `-c` parameter indicate the desired coloring algorithm, for example:
`-c greedy`


### Mesh Writing
MeshTools provides two VTK mesh writing options, in ASCII and binary format:
1. ASCII - `ascii`
2. Binary - `binary`

With the `-w` parameter indicate the desired writing form, for example:
`-w binary`

### Execution Examples
```
./meshtools -m ../msh/example.msh -r rcm -c greedy -w ascii

./meshtools -m ../msh/example.msh -r nd -c rokos -w binary

./meshtools -m ../msh/example.msh -c blocked -b 1024 -w ascii

./meshtools -m ../msh/example.msh -r natural -w binary
```

## Example to create executable
To run MeshTools:
1. Create a build folder, run `cd build`
2. Inside the build folder run CMake, `cmake ../.`
3. Run `make`

Thus, the MeshTools executable has been generated and is ready to use.

## MeshTools Help
```
Usage: ./meshtools <options>
-h: show help
-m <filename> : where <filename> is the gmsh file name (gmsh ascii v.2.2)>
-c [color algorithm] : where [color algotihm] is the coloring algorithm. The options are:
greedy : greedy serial version (default)
blocked : blocked serial version
rokos: openmp greedy version
-b <block size> : where <block size> is block size used in the blocked version coloring algorithm.
-r <reordering algorithm> : where [reordering algorithm] is the nodal renumering algorithm. The options are:
rcm : apply rcm (default)
nd : apply nested section algorithm
natural: first touch algorithm
none : keep gmsh ordering
-w <vtk write_mode> : where [vtk type] is the way to write the mesh in vtk file. The options are:
ascii : write ascii files
binary : write binary files

```

## Error 1
If after executing the `cmake` command the process is not completed and appears:
```
CMake Error at /usr/share/cmake-3.16/Modules/FindPackageHandleStandardArgs.cmake:146 (message):
  Could NOT find METIS (missing: METIS_LIBRARIES METIS_INCLUDE_DIRS)
Call Stack (most recent call first):
  /usr/share/cmake-3.16/Modules/FindPackageHandleStandardArgs.cmake:393 (_FPHSA_FAILURE_MESSAGE)
  cmake/modules/FindMETIS.cmake:168 (find_package_handle_standard_args)
  CMakeLists.txt:15 (find_package)
```
  
  It is necessary to indicate the METIS library directory with the -DMETIS_DIR flag.
  Example: `cmake ../. -DMETIS_DIR=/usr/lib/metis-5.1.0` 
