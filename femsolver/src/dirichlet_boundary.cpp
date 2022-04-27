
#include "dirichlet_boundary.h"

DirichletBoundary::DirichletBoundary(int boundary_id, int dof_id, std::string function, std::string vars)
{
    this->_boundary_surface = boundary_id;
    this->_dof_id = dof_id;
    this->function_parser.Parse(function, vars);
}

DirichletBoundary::DirichletBoundary(const DirichletBoundary& bnd)
{
    this->_boundary_surface = bnd._boundary_surface;
    this->_dof_id = bnd._dof_id;
    this->function_parser = bnd.function_parser;
}

DirichletBoundary::~DirichletBoundary() { }