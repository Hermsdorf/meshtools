
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

bool DirichletBoundary::operator==(const DirichletBoundary& db)
{
    return (this->_boundary_surface == db._boundary_surface) && 
           (this->_dof_id           == db._dof_id); 
}


double DirichletBoundary::get_value(double x, double y, double z, double t)
{
    const double vars[] = {x,y,z,t};
    return this->function_parser.Eval(vars);
}


DirichletBoundary::~DirichletBoundary() { }