#ifndef TRANSIENT_IMPLICIT_SYSTEM_H
#define TRANSIENT_IMPLICIT_SYSTEM_H

#include "implicit_system.h"
#include "nonlinear_implicit_system.h"
#include "fparser.hh"

class InitialCondition {
	public:
		InitialCondition(int region_id, int dof_id, std::string function, std::string vars):
            _region_id(region_id), _dof_id(dof_id)
            {
                this->_function_parser.Parse(function, vars);
            };
		InitialCondition(const InitialCondition& bnd):
            _region_id(bnd._region_id), _dof_id(bnd._dof_id), _function_parser(bnd._function_parser) { };
		~InitialCondition(){};
		bool operator==(const InitialCondition& db)
        {
            return (_region_id == db._region_id) && (_dof_id == db._dof_id);
        };
		int    get_region_id() { return _region_id; };
		int    get_dof_id()    { return _dof_id; }; 
		double get_value(double x, double y, double z=0, double t=0){
            const double vars[] = {x,y,z,t};
            return _function_parser.Eval(vars);
        };

	private:
		int            _region_id;
		int            _dof_id;
		FunctionParser _function_parser;
};


class TransientImplicitSystem: public NonLinearImplicitSystem
{
    public:
        TransientImplicitSystem(ParallelMesh &mesh, std::string name);
        void    set_deltat(double dt) { _dt = dt; }
        double  get_deltat() {return _dt; };
        int     get_time_step() { return timestep; }
        void    set_time(double t) { _t = t; }
        double  get_time() { return _t; }
        void    set_final_time(double t) { _final_time = t; }
        double  get_final_time() { return _final_time; }
        void    add_initial_condition(InitialCondition ic);
        void    solve_time_step();
        void    update_deltat();
        void    write_result(string filename);
        void    init();
        double* get_old_solution_array();
        double* get_older_solution_array();
        void    restore_old_solution_array(double** solution_array);
        void    restore_older_solution_array(double** solution_array);
        void    attach_assemble(void _assemble_function(TransientImplicitSystem *));
        void    attach_init_function(void _init(TransientImplicitSystem *));
        void    solve_nonlinear_system();

    private:
        double       _dt;             // timestep size
        double       _t;              // current time
        double       _final_time;     // final time
        unsigned int timestep;        // current timestep
        Vec _old_solution_local;      // solution at t-dt
        Vec _older_solution_local;    //  solution at t - 2*dt (needed fot BFD2 time solver)
        std::vector<InitialCondition> _initial_conditions;
        unsigned int _n_write;

        void apply_initial_conditions();
        void (* _assemble_function)(TransientImplicitSystem*  _system);
        void (* _init_function)(TransientImplicitSystem*  _system);

};

#endif /* TRANSIENT_IMPLICIT_SYSTEM_H */
