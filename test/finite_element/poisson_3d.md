# Benchmark: 3D Poisson
Simple tridiomensional Poisson solution. In a cube form domain with boundary conditions homogenous it is applied a force in the center of the domain.

## Methodology
- Stationary problem
- Equation type: Poisson equation
  $\quad \quad$ $
    \begin{cases}
    -\kappa \nabla^2 u & = s   \text{ in } \Omega \\
    u  & = u_D   \text{ in } \Gamma_D \\
    \mathbf{n} \cdot \kappa \nabla u & = h \text{ in } \Gamma_N
    \end{cases}
  $

- Problem dimension: 3D