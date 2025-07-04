/**********************************************************************/
/*                    DO NOT MODIFY THIS HEADER                       */
/*             Swift, a Fourier spectral solver for MOOSE             */
/*                                                                    */
/*            Copyright 2024 Battelle Energy Alliance, LLC            */
/*                        ALL RIGHTS RESERVED                         */
/**********************************************************************/

#include "LatticeBoltzmannMesh.h"

// VTK includes
#ifdef LIBMESH_HAVE_VTK

#include "vtkSmartPointer.h"
#include "vtkXMLStructuredGridReader.h"
#include "vtkStructuredGrid.h"
#include "vtkPointData.h"
#include "vtkDataArray.h"

#endif

registerMooseObject("SwiftApp", LatticeBoltzmannMesh);

InputParameters
LatticeBoltzmannMesh::validParams()
{
  InputParameters params = UniformTensorMesh::validParams();

  params.addParam<bool>("load_mesh_from_dat", false, "Load mesh from dat file");
  params.addParam<bool>("load_mesh_from_vtk", false, "Load mesh from VTK file");
  params.addParam<std::string>("mesh_file", "", "Mesh file name");

  params.addClassDescription("Create mesh file for LBM problems.");

  return params;
}

LatticeBoltzmannMesh::LatticeBoltzmannMesh(const InputParameters & parameters)
  : UniformTensorMesh(parameters),
    _load_mesh_from_dat(getParam<bool>("load_mesh_from_dat")),
    _load_mesh_from_vtk(getParam<bool>("load_mesh_from_vtk")),
    _mesh_file(getParam<std::string>("mesh_file"))
{
  mooseDeprecated("LatticeBoltzmannMesh is deprecated, use Domain instead");
}

void
LatticeBoltzmannMesh::buildMesh()
{
  // call base class buildMesh
  UniformTensorMesh::buildMesh();

  if (_load_mesh_from_dat)
  {
    _binary_mesh = torch::ones({_nx, _ny, _nz}, MooseTensor::intTensorOptions());
    loadMeshFromDatFile();
  }
  else if (_load_mesh_from_vtk)
  {
    _binary_mesh = torch::ones({_nx, _ny, _nz}, MooseTensor::intTensorOptions());
    _local_pore_size = torch::ones({_nx, _ny, _nz}, MooseTensor::floatTensorOptions());
    _Knudsen_number = torch::ones({_nx, _ny, _nz}, MooseTensor::floatTensorOptions());
    loadMeshFromVTKFile(_mesh_file, _binary_mesh, _local_pore_size, _Knudsen_number);
  }
}

void
LatticeBoltzmannMesh::loadMeshFromDatFile()
{
  auto dummy = getParam<bool>("dummy_mesh");
  if (dummy)
    mooseError("Cannot load mesh from file when dummy mesh is enabled");

  _console << COLOR_WHITE << "Loading Binary Mesh From Dat File\n";

  std::ifstream file(_mesh_file);
  if (!file.is_open())
    mooseError("Cannot open file " + _mesh_file);

  // read mesh into standart vector
  std::vector<int> fileData(_nx * _ny * _nz);
  for (int i = 0; i < fileData.size(); i++)
  {
    if (!(file >> fileData[i]))
    {
      mooseError("Insufficient data in the mesh file");
    }
  }
  file.close();

  // reshape and write into torch tensor
  for (int64_t k = 0; k < _nz; k++)
    for (int64_t j = 0; j < _ny; j++)
      for (int64_t i = 0; i < _nx; i++)
        _binary_mesh.index_put_({i, j, k}, fileData[k * _ny * _nx + j * _nx + i]);
}

void
LatticeBoltzmannMesh::loadMeshFromVTKFile(const std::string & filePath,
                                          torch::Tensor & binaryMedia,
                                          torch::Tensor & poreSize,
                                          torch::Tensor & knudsenNumber)
{
  if (_dim != 2)
    mooseError("VTK mesh reader is only supported for 2D cases");

  _console << COLOR_WHITE << "Loading Binary Mesh From VTK File\n";

  std::vector<int> dims = {static_cast<int>(_nx), static_cast<int>(_ny), static_cast<int>(_nz)};

#ifdef LIBMESH_HAVE_VTK
  /*
     Function to read binary porous media, local pore size distribution and Knudsen number
     distributions from VTK file. scripts/create_vtk.py MUST be used to generate the vtk file
   */
  vtkSmartPointer<vtkXMLStructuredGridReader> reader =
      vtkSmartPointer<vtkXMLStructuredGridReader>::New();
  reader->SetFileName(filePath.c_str());
  reader->Update();

  vtkSmartPointer<vtkStructuredGrid> structuredGrid = reader->GetOutput();

  vtkSmartPointer<vtkDataArray> binaryMediaArray =
      structuredGrid->GetPointData()->GetArray("BinaryMedia");
  vtkSmartPointer<vtkDataArray> poreSizeArray =
      structuredGrid->GetPointData()->GetArray("PoreSize");
  vtkSmartPointer<vtkDataArray> knudsenNumberArray =
      structuredGrid->GetPointData()->GetArray("KnudsenNumber");

  for (int64_t i = 0; i < dims[0]; i++)
    for (int64_t j = 0; j < dims[1]; j++)
      for (int64_t k = 0; k < dims[2]; k++)
      {
        // 2D only, 3D needs to be tested
        int index = k * dims[1] * dims[2] + i * dims[1] + j;
        binaryMedia.index_put_({i, j, k}, binaryMediaArray->GetTuple1(index));
        poreSize.index_put_({i, j, k}, poreSizeArray->GetTuple1(index));
        knudsenNumber.index_put_({i, j, k}, knudsenNumberArray->GetTuple1(index));
      }
#else
  mooseError("VTK not enabled");
#endif
}
