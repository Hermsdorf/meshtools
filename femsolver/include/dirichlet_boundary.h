#ifndef DIRICHLET_BOUNDARY_H
#define DIRICHLET_BOUNDARY_H

class DirichletBoundary {
 public:
  DirichletBoundary(int boundary_id, int variable_id);
  DirichletBoundary(const DirichletBoundary& bnd);
  ~DirichletBoundary();

 private:
  int _boundary_surface;
  int _variable_id;
};

#endif /* DIRICHLET_BOUNDARY_H */
