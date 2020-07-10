#include "FEAdaptor.h"
#include <iostream>

#include <vtkCPDataDescription.h>
#include <vtkCPInputDataDescription.h>
#include <vtkCPProcessor.h>
#include <vtkCPPythonScriptPipeline.h>
#include <vtkCellData.h>
#include <vtkCellType.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkUnstructuredGrid.h>

namespace
{
  vtkCPProcessor *Processor = NULL;
  vtkUnstructuredGrid *VTKGrid;

  void BuildVTKGrid(mesh_t *mesh)
  {
    // create the points information
    int size_coord    = mesh->coord.size();
    double* pointData = &mesh->coord[0];

    vtkNew<vtkDoubleArray> pointArray;
    pointArray->SetNumberOfComponents(3);
    pointArray->SetArray(pointData, static_cast<vtkIdType>(mesh->n_nodes * 3), 1); 
    vtkNew<vtkPoints> points;
    points->SetData(pointArray.GetPointer());
    VTKGrid->SetPoints(points.GetPointer());

    // create the cells
    int ne = mesh->n_elements;
    int nfe = mesh->n_face_elements;
    unsigned int* cellsData = (unsigned int*)&mesh->conn[mesh->offset[nfe]];

    VTKGrid->Allocate(static_cast<vtkIdType>(mesh->offset.back() - mesh->offset[nfe]));
    for (unsigned int cell = 0; cell < ne; cell++)
    {
      unsigned int cell_skip = cell + nfe;
      int n_conn = mesh->offset[cell_skip + 1] - mesh->offset[cell_skip];
      unsigned int *cellPoints = cellsData + (n_conn * cell);

      vtkIdType* tmp = new vtkIdType[n_conn];
      for (int i = 0; i < n_conn; i++)
        tmp[i] = cellPoints[i]; // conectividade

      VTKGrid->InsertNextCell(mesh->type[cell_skip], n_conn, tmp); // (tipo do elemento, numero de nós, vetor com as conn)
      delete[] tmp;
    }

    //delete [] pointData; // devo deletar o pointData pois o pointArray aponta pra ele, não sei se o pointArray é deletado automaticamente
  }

  void UpdateVTKAttributes(vtkCPInputDataDescription *idd, mesh_t *mesh,
                           double *velocityData, float *pressureData)
  {
    int nn = mesh->n_nodes;
    int ne = mesh->n_elements;
    if (idd->IsFieldNeeded("velocity", vtkDataObject::POINT) == true)
    {
      if (VTKGrid->GetPointData()->GetNumberOfArrays() == 0)
      {
        // velocity array
        vtkNew<vtkDoubleArray> velocity;
        velocity->SetName("velocity");
        velocity->SetNumberOfComponents(3);
        velocity->SetNumberOfTuples(static_cast<vtkIdType>(nn));
        VTKGrid->GetPointData()->AddArray(velocity.GetPointer());
      }
      vtkDoubleArray *velocity =
          vtkDoubleArray::SafeDownCast(VTKGrid->GetPointData()->GetArray("velocity"));
      // The velocity array is ordered as vx0,vx1,vx2,..,vy0,vy1,vy2,..,vz0,vz1,vz2,..
      // so we need to create a full copy of it with VTK's ordering of
      // vx0,vy0,vz0,vx1,vy1,vz1,..
      for (unsigned int i = 0; i < nn; i++)
      {
        double values[3] = {velocityData[i], velocityData[i + nn],
                            velocityData[i + 2 * nn]};
        velocity->SetTypedTuple(i, values);
      }
    }
    //
    if (idd->IsFieldNeeded("pressure", vtkDataObject::POINT) == true)
    {
      if (VTKGrid->GetCellData()->GetNumberOfArrays() == 0)
      {
        // pressure array
        vtkNew<vtkFloatArray> pressure;
        pressure->SetName("pressure");
        pressure->SetNumberOfComponents(1);
        VTKGrid->GetCellData()->AddArray(pressure.GetPointer());
      }
      vtkFloatArray *pressure =
          vtkFloatArray::SafeDownCast(VTKGrid->GetCellData()->GetArray("pressure"));
      // The pressure array is a scalar array so we can reuse
      // memory as long as we ordered the points properly.
      pressure->SetArray(pressureData, static_cast<vtkIdType>(ne), 1);
    }
  }

  void BuildVTKDataStructures(vtkCPInputDataDescription *idd, mesh_t *mesh,
                              double *velocity, float *pressure)
  {
    if (VTKGrid == NULL)
    {
      // The grid structure isn't changing so we only build it
      // the first time it's needed. If we needed the memory
      // we could delete it and rebuild as necessary.
      VTKGrid = vtkUnstructuredGrid::New();
      BuildVTKGrid(mesh);
    }
    UpdateVTKAttributes(idd, mesh, velocity, pressure);
  }
} // namespace

void CatalystInitialize(int numScripts, char *scripts[])
{
  if (Processor == NULL)
  {
    Processor = vtkCPProcessor::New();
    Processor->Initialize();
  }
  else
  {
    Processor->RemoveAllPipelines();
  }
  for (int i = 0; i < numScripts; i++)
  {
    vtkNew<vtkCPPythonScriptPipeline> pipeline;
    pipeline->Initialize(scripts[i]);
    Processor->AddPipeline(pipeline.GetPointer());
  }
}

void CatalystFinalize()
{
  if (Processor)
  {
    Processor->Delete();
    Processor = NULL;
  }
  if (VTKGrid)
  {
    VTKGrid->Delete();
    VTKGrid = NULL;
  }
}

void CatalystCoProcess(mesh_t *mesh, double *velocityData, float *pressureData, double time,
                       unsigned int timeStep, int lastTimeStep)
{
  vtkNew<vtkCPDataDescription> dataDescription;
  dataDescription->AddInput("input");
  dataDescription->SetTimeData(time, timeStep);
  if (lastTimeStep == true)
  {
    // assume that we want to all the pipelines to execute if it
    // is the last time step.
    dataDescription->ForceOutputOn();
  }
  if (Processor->RequestDataDescription(dataDescription.GetPointer()) != 0)
  {
    vtkCPInputDataDescription *idd = dataDescription->GetInputDescriptionByName("input");
    BuildVTKDataStructures(idd, mesh, velocityData, pressureData);
    idd->SetGrid(VTKGrid);
    Processor->CoProcess(dataDescription.GetPointer());
  }
}
