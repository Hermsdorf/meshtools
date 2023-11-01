

#include "navier_stokes.h"


void assemble_lhs_2d(TransientImplicitSystem* system)
{
    auto pmesh = system->get_mesh();
    int  ndim  =  pmesh.getDim();

    // Gerencia as numerações das equações do sistema
    auto equation_manager = system->get_equation_manager();
    int dof  = 0;

    int n_elements = pmesh->get_n_elements();

    double *old_solution = system->get_old_solution_array();

    QGauss qrule;
    FEMFunction fem;

    // loop sobre os elementos da malha por cores
    for (int iel = 0; iel < n_elements; iel++)
    {
        Element elem;
        pmesh.getElement(iel,elem);

        int nnoel = elem.n_nodes();

        std::vector<int>        global_indices_u;
        std::vector<int>        local_indices_u;
        std::vector<int>        global_indices_v;
        std::vector<int>        local_indices_v;
        std::vector<int>        global_indices_p;
        std::vector<int>        local_indices_p;
        DenseMatrix<double>     Kuu(nnoel, nnoel);  // matriz de rigidez do elemento
        DenseMatrix<double>     Kuv(nnoel, nnoel);  // matriz de rigidez do elemento
        DenseMatrix<double>     Kup(nnoel, nnoel);  // matriz de rigidez do elemento
        DenseMatrix<double>     Kvv(nnoel, nnoel);  // matriz de rigidez do elemento
        DenseMatrix<double>     Kvu(nnoel, nnoel);  // matriz de rigidez do elemento
        DenseMatrix<double>     Kvp(nnoel, nnoel);  // matriz de rigidez do elemento
        DenseMatrix<double>     Kpp(nnoel, nnoel);  // matriz de rigidez do elemento
        DenseMatrix<double>     Kpu(nnoel, nnoel);  // matriz de rigidez do elemento
        DenseMatrix<double>     Kpv(nnoel, nnoel);  // matriz de rigidez do elemento



        std::vector<double>     Fu(nnoel);          // vetor de força do elemento
        std::vector<double>     Fv(nnoel);          // vetor de força do elemento
        std::vector<double>     Fp(nnoel);          // vetor de força do elemento

        std::vector<double>   & phi = fem.get_phi();
        std::vector<Gradient> & dphi= fem.get_dphi();
        double                & JxW = fem.get_JxW();
        RealVector            & g   = fem.get_g();
        RealTensor            & G   = fem.get_G();
        
        equation_manager.global_indices(0, elem.connectivity(), global_indices_u);
        equation_manager.global_indices(0, elem.connectivity(), global_indices_v);
        equation_manager.global_indices(0, elem.connectivity(), global_indices_p);
        equation_manager.local_indices(0,  elem.connectivity(), local_indices_u);
        equation_manager.local_indices(1,  elem.connectivity(), local_indices_v);
        equation_manager.local_indices(2,  elem.connectivity(), local_indices_p);

        // Obtem pontos de integração para elemento
        qrule.reset(elem);

        Gradient velocity;
        double k            = 1.0E-08;
        double sigma        = 0.0;
        double theta        = 0.5;
        double dt           = system->get_deltat();
        double dt_stab       = 0.1;

        // loop sobre os pontos de integração
        for(int qp = 0; qp < qrule.n_points(); qp++)
        {
            fem.ComputeFunction(elem,qrule.get(qp));

            double u_old = 0.0, v_old = 0.0;
            double u =0.0; v =0.0;
            RealVector grad_u;
            RealVector grad_v;

            RealVector grad_p;

            RealVector grad_old_u;
            RealVector grad_old_v;

            for(int ino = 0; i < elem.n_nodes(); ino++)
            {
                u_old   +=  old_solution[local_indices_u[i]]*phi[i];
                v_old   +=  old_solution[local_indices_v[i]]*phi[i];
               
                grad_u_old(0) +=  old_solution[local_indices_u[i]]*dphi[i](0);
                grad_u_old(1) +=  old_solution[local_indices_u[i]]*dphi[i](1);


                u           +=  solution[local_indices_u[i]]*phi[i];
                v           +=  solution[local_indices_v[i]]*phi[i];

                grad_u(0)     +=  solution[local_indices_u[i]]*dphi[i](0);
                grad_u(1)     +=  solution[local_indices_u[i]]*dphi[i](1);

                grad_v(0)     +=  solution[local_indices_v[i]]*dphi[i](0);
                grad_v(1)     +=  solution[local_indices_v[i]]*dphi[i](1);

                grad_p(0)     +=  solution[local_indices_p[i]]*dphi[i](0);
                grad_p(1)     +=  solution[local_indices_p[i]]*dphi[i](1);
            }

            RealVector velocity(u,v);
            RealVector source;
            // Compute the instability parameter
            const double tau = TAUStab(velocity, G, k, dt_stab, dt);

            double fxy = 0.0;
            // RBVMS residual
            double res_u = (u-u_old)/dt + velocity*grad_u + grad_p(0) - source(0);
            double res_v = (v-v_old)/dt + velocity*grad_v + grad_p(1) - source(1);
            double res_c = 
        

            for(int i = 0; i < elem.n_nodes(); i++)
            {
                Fu(i) += 

                for(int j = 0; j < elem.n_nodes(); j++)
                {

                }
            }



        }

        
    }
}

void assemble_rhs_2d(TransientImplicitSystem* system)
{

}


NavierStokes::NavierStokes(ParallelMesh* mesh)
{
    _system = std::make_unique<TransientImplicitSystem>(mesh,"NavierStokes"); 
    _system->add_variable("u");
    _system->add_variable("v");
    if(mesh->dimension() > 2)
        _system->add_variable("w");
    _system->add_variable("u");
}

NavierStokes::~NavierStokes()
{

}

void NavierStokes::init()
{
    _system->init();
}



