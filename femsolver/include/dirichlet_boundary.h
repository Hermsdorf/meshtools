#ifndef DIRICHLET_BOUNDARY_H
#define DIRICHLET_BOUNDARY_H

#include "fparser.hh"

class DirichletBoundary {
 public:
   DirichletBoundary(int boundary_id, int dof_id, std::string function);
   DirichletBoundary(const DirichletBoundary& bnd);
  ~DirichletBoundary();
  bool operator==(const DirichletBoundary& db);
  int  get_boundary_id() { return _boundary_surface; };
  int  get_dof_id() { return _dof_id; }; 

 private:
  int _boundary_surface;
  int _dof_id;
  FunctionParser function_parser;
};

#endif /* DIRICHLET_BOUNDARY_H */
