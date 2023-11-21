#ifndef CATALYST_ADAPTOR_H
#define CATALYST_ADAPTOR_H

#include "meshtools_config.h"

#ifdef CATALYST_ENABLE
#include <catalyst.hpp>
#endif

#include "implicit_system.h"

#include <iostream>
#include <string>

/**
 * The namespace hold wrappers for the three main functions of the catalyst API
 * - catalyst_initialize
 * - catalyst_execute
 * - catalyst_finalize
 * Although not required it often helps with regards to complexity to collect
 * catalyst calls under a class /namespace.
 */
namespace CatalystAdaptor
{

static bool initialized = false;

/**
 * Initialize the catalyst pipeline.
 * @param argc number of arguments
 * @param argv array of arguments
 */
void Initialize(int argc, char* argv[])
{
  //if(argc < 1) return;

#ifdef CATALYST_ENABLE
  
  conduit_cpp::Node node;

  if(argc < 2)
  {
    node["catalyst/scripts/script/filename"].set_string("catalyst_simple.py");
  }
  else
    node["catalyst/scripts/script/filename"].set_string(argv[1]);
  for (int cc = 2; cc < argc; ++cc)
  {
    conduit_cpp::Node list_entry = node["catalyst/scripts/script/args"].append();
    list_entry.set(argv[cc]);
  }

  node["catalyst_load/implementation"] = "paraview";
  node["catalyst_load/search_paths/paraview"] = PARAVIEW_IMPL_DIR;
  catalyst_status err = catalyst_initialize(conduit_cpp::c_node(&node));
  if (err != catalyst_status_ok)
  {
    std::cerr << "Failed to initialize Catalyst: " << err << std::endl;
  }

#endif

  initialized = true;
}

void Execute(int cycle, double time, ImplicitSystem *system)
{
  
#ifdef CATALYST_ENABLE
  conduit_cpp::Node exec_params;

  double *solution = system->get_local_solution_array();
  // State: Information about the current iteration. All parameters are
  // optional for catalyst but downstream filters may need them to execute
  // correctly.

  // add time/cycle information
  auto state = exec_params["catalyst/state"];
  state["timestep"].set(cycle);
  state["time"].set(time);
  state["multiblock"].set(1);

  // Channels: Named data-sources that link the data of the simulation to the
  // analysis pipeline in other words we map the simulation datastructures to
  // the ones expected by ParaView.  In this example we use the Mesh Blueprint
  // to describe data see also bellow.

  // Add channels.
  // We only have 1 channel here. Let's name it 'grid'.
  auto channel = exec_params["catalyst/channels/grid"];

  // Since this example is using Conduit Mesh Blueprint to define the mesh,
  // we set the channel's type to "mesh".
  channel["type"].set("mesh");

  // now create the mesh.
  auto mesh = channel["data"];

  ParallelMesh& meshtools_mesh = system->get_mesh();

  double *coods_ptr   = meshtools_mesh.getCoordinatesData();
  unsigned int nnodes = meshtools_mesh.get_n_nodes();

  mesh["coordsets/coords/type"].set("explicit");
  mesh["coordsets/coords/values/x"].set_external(coods_ptr, nnodes, 0               , 3 * sizeof(double));
  mesh["coordsets/coords/values/y"].set_external(coods_ptr, nnodes,   sizeof(double), 3 * sizeof(double));
  mesh["coordsets/coords/values/z"].set_external(coods_ptr, nnodes, 2*sizeof(double), 3 * sizeof(double));

  // Next, add topology
  mesh["topologies/mesh/type"].set("unstructured");
  mesh["topologies/mesh/coordset"].set("coords");

  auto ElemType = meshtools_mesh.get_mesh_element_type();

  int     nnoel = 0;
  switch(ElemType)
  {
    case QUAD4:
      mesh["topologies/mesh/elements/shape"].set("quad");
      nnoel = 4;
      break;
    case TRI3:
        mesh["topologies/mesh/elements/shape"].set("tri");
        nnoel = 3;
        break;
    case HEX8:
        mesh["topologies/mesh/elements/shape"].set("hex");
        nnoel = 8;
        break;
    case TET4:
        mesh["topologies/mesh/elements/shape"].set("tet");
        nnoel = 4;
        break;
  }

  unsigned int *connectivity_ptr = meshtools_mesh.getElementConnectivityData();
  unsigned int ncells            = meshtools_mesh.get_n_elements();
  
  mesh["topologies/mesh/elements/connectivity"].set_external(connectivity_ptr, ncells, 0, nnoel * sizeof(unsigned int));
#if DEBUG_CATALYST
  mesh.print();
#endif

  auto fields = mesh["fields"];

  int nvar = system->get_equation_manager().get_n_dofs();

  for(int nv = 0; nv < nvar; ++nv)
  { 
    std::string var_name = system->get_variable_name(nv);
    std::cout << var_name + "/association" << std::endl;
    fields[var_name + "/association"].set("vertex");
    fields[var_name + "/topology"].set("mesh");
    fields[var_name + "/volume_dependent"].set("false");
    fields[var_name + "/values"].set_external(solution, nnodes, nv*sizeof(double), nvar * sizeof(double));
  }
 
  catalyst_status err = catalyst_execute(conduit_cpp::c_node(&exec_params));
  if (err != catalyst_status_ok)
  {
    std::cerr << "Failed to execute Catalyst: " << err << std::endl;
  }
  system->restore_local_solution_array(&solution);

#endif

}

// Although no arguments are passed for catalyst_finalize  it is required in
// order to release any resources the ParaViewCatalyst implementation has
// allocated.
void Finalize()
{

#ifdef CATALYST_ENABLE
  conduit_cpp::Node node;
  catalyst_status err = catalyst_finalize(conduit_cpp::c_node(&node));
  if (err != catalyst_status_ok)
  {
    std::cerr << "Failed to finalize Catalyst: " << err << std::endl;
  }
#endif 
}
}




#endif /* CATALYST_ADAPTOR_H */
