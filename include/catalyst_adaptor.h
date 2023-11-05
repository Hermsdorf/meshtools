#ifndef CATALYST_ADAPTOR_H
#define CATALYST_ADAPTOR_H

#include <catalyst.hpp>

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
  if(argc < 0) return;

  // Populate the catalyst_initialize argument based on the "initialize" protocol [1].
  // [1] https://docs.paraview.org/en/latest/Catalyst/blueprints.html#protocol-initialize
  conduit_cpp::Node node;

  // Using the arguments given to the driver set the filename for the catalyst
  // script and pass the rest of the arguments as arguments of the script
  // itself. To retrieve these  arguments from the script  use the `get_args()`
  // method of the paraview catalyst module [2]
  // [2] https://kitware.github.io/paraview-docs/latest/python/paraview.catalyst.html

  

  node["catalyst/scripts/script/filename"].set_string(argv[1]);
  for (int cc = 2; cc < argc; ++cc)
  {
    conduit_cpp::Node list_entry = node["catalyst/scripts/script/args"].append();
    list_entry.set(argv[cc]);
  }

  // For this example we hardcode the implementation name to "paraview" and
  // define the "PARAVIEW_IMPL_DIR" during compilation time (see the
  // accompanying CMakeLists.txt). We could however defined them via
  // environmental variables  see [1].
  node["catalyst_load/implementation"] = "paraview";
  node["catalyst_load/search_paths/paraview"] = PARAVIEW_IMPL_DIR;
  catalyst_status err = catalyst_initialize(conduit_cpp::c_node(&node));
  if (err != catalyst_status_ok)
  {
    std::cerr << "Failed to initialize Catalyst: " << err << std::endl;
  }
  initialized = true;
}

void Execute(int cycle, double time, ImplicitSystem *system)
{
  // Populate the catalyst_execute argument based on the "execute" protocol [3].
  // [3] https://docs.paraview.org/en/latest/Catalyst/blueprints.html#protocol-execute

  conduit_cpp::Node exec_params;

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
  auto conduit_mesh = channel["data"];

  auto mesh = system->get_mesh();

  // populate the data node following the Mesh Blueprint [4]
  // [4] https://llnl-conduit.readthedocs.io/en/latest/blueprint_mesh.html

  // start with coordsets (of course, the sequence is not important, just make
  // it easier to think in this order).
  conduit_mesh["coordsets/coords/type"].set("explicit");

  // .set_external passes just the pointer  to the analysis pipeline allowing thus for zero-copy
  // data conversion see https://llnl-conduit.readthedocs.io/en/latest/tutorial_cpp_ownership.html
  conduit_mesh["coordsets/coords/values/x"].set_external(
    mesh.getCoordinatesData(), mesh.get_n_nodes(), /*offset=*/0, /*stride=*/ 3 * sizeof(double));
  conduit_mesh["coordsets/coords/values/y"].set_external(mesh.getCoordinatesData(), mesh.get_n_nodes(),
    /*offset=*/sizeof(double), /*stride=*/ 3 * sizeof(double));
  conduit_mesh["coordsets/coords/values/z"].set_external(mesh.getCoordinatesData(), mesh.get_n_nodes(),
    /*offset=*/2 * sizeof(double), /*stride=*/3 * sizeof(double));

  // Next, add topology
  conduit_mesh["topologies/mesh/type"].set("unstructured");
  conduit_mesh["topologies/mesh/coordset"].set("coords");

  auto ElemType = mesh.get_mesh_element_type();
  int     nnodes = 0;
  switch(elemType)
  {
    case QUAD4:
      conduit_mesh["topologies/mesh/elements/shape"].set("quad");
      nnodes = 4;
      break;
    case TRI3:
        conduit_mesh["topologies/mesh/elements/shape"].set("tri");
        nnodes = 3;
        break;
    case HEX8:
        conduit_mesh["topologies/mesh/elements/shape"].set("hex");
        nnodes = 8;
        break;
    case TET4:
        conduit_mesh["topologies/mesh/elements/shape"].set("tet");
        nnodes = 4;
        break;
  }

  //conduit_mesh["topologies/mesh/elements/shape"].set("hex");

  conduit_mesh["topologies/mesh/elements/connectivity"].set_external(
    mesh.getElementConnectivityData(), mesh.get_n_elements(), /*offset=*/0, /*stride=*/ nnodes * sizeof(unsigned int));

  // Finally, add fields.

  // First component of the path is the name of the field . The rest are described
  // in https://llnl-conduit.readthedocs.io/en/latest/blueprint_mesh.html#fields
  // under the Material-Independent Fields section.
  auto fields = mesh["fields"];

  int nvar = system->get_equation_manager().get_n_dofs();
  for(int nv = 0; nv < nvar; ++nv)
  { 
    system->get_variable_name(nv);
    std::string var_name = system->get_variable_name(nv);
    fields[var_name + "/association"].set("vertex");
    fields[var_name + "/topology"].set("mesh");
    fields[var_name + "/volume_dependent"].set("false");

    fields[var_name + "/values"].set_external(
      system->get_local_solution_array(), mesh.get_n_nodes(), /*offset=*/nv*sizeof(double), /*stride=*/ nvar * sizeof(double));
  }

#if 0 
  fields["velocity/association"].set("vertex");
  fields["velocity/topology"].set("mesh");
  fields["velocity/volume_dependent"].set("false");

  // velocity is stored in non-interlaced form (unlike points).
  fields["velocity/values/x"].set_external(
    attribs.GetVelocityArray(), grid.GetNumberOfPoints(), /*offset=*/0);
  fields["velocity/values/y"].set_external(attribs.GetVelocityArray(), grid.GetNumberOfPoints(),
    /*offset=*/grid.GetNumberOfPoints() * sizeof(double));
  fields["velocity/values/z"].set_external(attribs.GetVelocityArray(), grid.GetNumberOfPoints(),
    /*offset=*/grid.GetNumberOfPoints() * sizeof(double) * 2);

  // pressure is cell-data.
  fields["pressure/association"].set("element");
  fields["pressure/topology"].set("mesh");
  fields["pressure/volume_dependent"].set("false");
  fields["pressure/values"].set_external(attribs.GetPressureArray(), grid.GetNumberOfCells());
#endif 
  catalyst_status err = catalyst_execute(conduit_cpp::c_node(&exec_params));
  if (err != catalyst_status_ok)
  {
    std::cerr << "Failed to execute Catalyst: " << err << std::endl;
  }
}

// Although no arguments are passed for catalyst_finalize  it is required in
// order to release any resources the ParaViewCatalyst implementation has
// allocated.
void Finalize()
{
  conduit_cpp::Node node;
  catalyst_status err = catalyst_finalize(conduit_cpp::c_node(&node));
  if (err != catalyst_status_ok)
  {
    std::cerr << "Failed to finalize Catalyst: " << err << std::endl;
  }
}
}




#endif /* CATALYST_ADAPTOR_H */
