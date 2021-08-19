#ifndef A3BC3293_2C3C_4587_9FB5_B432880C488C
#define A3BC3293_2C3C_4587_9FB5_B432880C488C


namespace FiniteElementKernels
{
 
double* AllocateFiniteElementMatrix(int n_elements, int number_of_element_nodes);
void Assembly(Mesh& mesh, double *A_EBE, double *b);
void AssemblyOpenMP(Mesh& mesh);

void run(Mesh& mesh);
}



#endif /* A3BC3293_2C3C_4587_9FB5_B432880C488C */
