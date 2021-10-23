# MeshTools - PT/BR

Biblioteca que manipula uma malha de elementos finitos.

MeshTools é uma biblioteca de leitura de malha de elementos finitos, pré-processamento de malha com aplicação de técnicas de computação de alto desempenho e escrita em VTK para visualização científica dos resultados.

MeshTools aplica técnicas como Reordenação Nodal, Coloração de elementos e Particionamento de domínio.

## Requisitos
- CMake >= 3.0.0
- METIS >= 5.1.0
- OpenMP
- OpenMPI

## Como usar
1. Gerar uma pasta para build do projeto
2. Executar o CMake para criação do makefile com link aos arquivos necessários
3. Executar o comando `make`
4. Executar o MeshTools

Para explicação dos parâmetros MeshTools executar: `./meshtools -h`

### Malha a ser processada
Para indicar a malha a ser processada é necessário utilizar o parâmetro `-m` e em seguida o caminho para a malha especificada.
Exemplo:
`-m /usr/msh/test.msh`

### Reordenação Nodal
O MeshTools disponibiliza 3 algoritmos para Reordenação Nodal, sendo eles:
1. RCM (Reverse Cuthill-McKee) [default] - `rcm`
2. Nested-Dissection - `nd`
3. First Touch - `natural`

Com o parâmetro `-r` indique o algoritmo de reordenação desejado, exemplo:
`-r nd`

### Coloração de elementos
O MeshTools disponibiliza 3 algoritmos de Coloração da malha, sendo eles:
1. Greedy Serial [default] - `greedy` 
2. Blocked Serial - `blocked`
3. OpenMP Greedy - `rokos`

No caso da execução com a versão `Blocked` é possível inserir o tamanho do bloco desejado máximo de elementos por cor com o parâmetro `-b`, sendo padrão o valor 4096. Exemplo: `-b 2048`

Com o parâmetro `-c` indique o algoritmo de coloração desejado, exemplo:
`-c greedy`


### Escrita da malha
O MeshTools disponibiliza duas opções de escrita da malha VTK, em formato ASCII e binário:
1. ASCII - `ascii`
2. Binário - `binary`

Com o parâmetro `-w` indique a forma de escrita desejada, exemplo:
`-w binary`

### Exemplos de execução
```
./meshtools -m ../msh/example.msh -r rcm -c greedy -w ascii 

./meshtools -m ../msh/example.msh -r nd -c rokos -w binary

./meshtools -m ../msh/example.msh -c blocked -b 1024 -w ascii

./meshtools -m ../msh/example.msh -r natural -w binary
```

## Exemplo para criar executável
Para executar o MeshTools: 
1. Crie uma pasta build, execute `cd build`
2. Dentro da pasta build execute o CMake, `cmake ../.`
3. Execute `make`

Assim, o executável do MeshTools foi gerado e está pronto para usar.

## Ajuda MeshTools
```
Usage: ./meshtools <options>
	 -h                   : show help
	 -m <filename>        : where <filename> is the gmsh file name (gmsh ascii v.2.2)> 
	 -c [color algorithm] : where [color algotihm] is the coloring algorithm. The options are: 
		  greedy  : greedy serial version (default)
		  blocked : blocked serial version
		  rokos   : openmp greedy version 
	 -b <block size> : where <block size> is block size used in the the blocked version coloring algorithm.
	 -r <reordering algorithm> : where [reordering algotihm] is the nodal renumering algorithm. The options are: 
		  rcm       : apply rcm (default) 
		  nd        : apply nested disection algorithm 
		  natural   : first touch algorithm
		  none      : keep gmsh ordering 
	  -w <vtk write_mode> : where [vtk type] is the way to write the mesh in vtk file. The options are: 
		  ascii             : write ascii files  
		  binary            : write binary files 

```

## Erro 1
Caso após execução do comando `cmake` o processo não seja concluído e apareça: 
```
CMake Error at /usr/share/cmake-3.16/Modules/FindPackageHandleStandardArgs.cmake:146 (message):
  Could NOT find METIS (missing: METIS_LIBRARIES METIS_INCLUDE_DIRS)
Call Stack (most recent call first):
  /usr/share/cmake-3.16/Modules/FindPackageHandleStandardArgs.cmake:393 (_FPHSA_FAILURE_MESSAGE)
  cmake/modules/FindMETIS.cmake:168 (find_package_handle_standard_args)
  CMakeLists.txt:15 (find_package)
```
  
  É necessário indicar o diretório da biblioteca METIS com a flag -DMETIS_DIR.
  Exemplo: `cmake ../. -DMETIS_DIR=/usr/lib/metis-5.1.0`


# MeshTools - EN

Library that manipulates a finite element mesh.

MeshTools is a finite element mesh reading library, pre-processing with application of high performance computing techniques and VTK writing for scientific visualization of results.

MeshTools applies techniques such as Nodal Reordering, Element Coloring and Domain Partitioning.

## Requirements
- CMake >= 3.0.0
- METIS >= 5.1.0
- OpenMP
- OpenMPI

## How to use
1. Generate a project build folder
2. Run CMake to create the makefile with link to the necessary files
3. Run the `make` command
4. Run MeshTools

For explanation of MeshTools parameters run: `./meshtools -h`

### Mesh to be processed
To indicate the mesh to be processed it is necessary to use the `-m` parameter and then the path to the specified mesh.
Example:
`-m /usr/msh/test.msh`

### Nodal Reordering
MeshTools provides 3 algorithms for Nodal Reordering, namely:
1. RCM (Reverse Cuthill-McKee) [default] - `rcm`
2. Nested-Dissection - `nd`
3. First Touch - `natural`

With the `-r` parameter indicate the desired reordering algorithm, for example:
`-r na`

### Element coloring
MeshTools provides 3 Mesh Coloring algorithms, namely:
1. Greedy Serial [default] - `greedy`
2. Blocked Serial - `blocked`
3. OpenMP Greedy - `rokos`

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
