# MeshTools

Biblioteca que manipula uma malha de elementos finitos.

MeshTools é uma biblioteca de leitura de malha de elementos finitos, pré-processamento de malha com aplicação de técnicas de computação de alto desempenho e escrita em VTK para visualização científica dos resultados.

## Rotinas
#### Leitura: 
Para a leitura é disponibilizada da função *MeshGmshReader* a qual armazena as informações da malha a partir de um arquivo de entrada indicado no argumento da execução do programa. Essa biblioteca só é compatível com formato msh versão 2.2.

#### Técnicas de computação de alto desempenho: 
São disponibilizados algoritmos de particionamento, coloração e reordenação nodal. 
- Para os algoritmos de particionamento utilizamos da biblioteca METIS. Pode ser feito o particionamento da malha somente dos elementos internos, com a rotina *MeshPartitionerInternal* ou também particionamento completo levando em consideração tanto os elementos internos quanto os de superfície com a rotina *MeshPartitioner*.
- Na técnica de coloração utilizamos do algoritmo First-Fit Coloring o qual insere a menor cor possível para cada elemento da malha. Essas cores dos elementos podem ser calculadas com a função *MeshColoring* a qual, em cada posição do array mesh_coloring_internal, armazena a quantidade de elementos por cor.
- Para a reordenação nodal é possível utilizar de dois algoritmos, o METIS_ND o qual utiliza o algoritmo Multilevel Nested Dissection, ou o Reverse Cuthill McKee (RCM). É recomendado o algoritmo RCM pelo mesmo ter um melhor resultado gerando uma menor largura de banda. Para aplicar a técnica de reordenação deve ser utilizada da rotina *MeshReordering* passando por argumento o algoritmo desejado (RCM ou METIS_ND).

#### Escrita:
É possível escrever sua malha em formato VTK em quatro diferentes funções, a *MeshVTKWriter* a qual escreve em ASCII todos os elementos da malha, ou seja, tanto os elementos internos quanto de superfície, ou ainda usando da rotina *MeshVTKWriterInternal* a qual escreve também em ASCII a malha levando em consideração somente os elementos internos ou também as versões dessas duas funções com escrita em binário, respectivamente *MeshVTKWriterBinAppended* e *MeshVTKWriterInternalBinAppended*.

## Observação
Informações como parâmetros, tipos de retorno e o objetivo de cada função das classes são encontrados nos headers de cada arquivo.