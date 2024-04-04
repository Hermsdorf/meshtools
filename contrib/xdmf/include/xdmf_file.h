#ifndef XDMF_FILE_H__
#define XDMF_FILE_H__

// https://github.com/FEniCS/dolfinx/tree/main/cpp/dolfinx/io

#include <filesystem>
#include <memory>
#include <mpi.h>
#include <pugixml.hpp>

#include "meshtools.h"
#include "implicit_system.h"


class XDMFSpatialFile
{
public:
  /// File encoding type
  enum class Encoding
  {
    HDF5,
    ASCII
  };

  enun class Compression
  {
    NONE,
    GZIP,
    SZIP,
    SZ,
    ZFP
  };

  /// Default encoding type
  static const Encoding default_encoding = Encoding::HDF5;

  /// Default compression type
  static const EncodiCompression default_compression = Compression::NONE;

  /// Constructor
  XDMFFile(MPI_Comm comm, const std::filesystem::path& filename,
           Encoding encoding       = default_encoding, 
           Compression compression = default_compression);

  /// Move constructor
  XDMFFile(XDMFFile&&) = default;

  /// Destructor
  ~XDMFFile();

  /// Close the file
  ///
  /// This closes open underlying HDF5 file. In ASCII mode the XML file
  /// is closed each time it is written to or read from, so close() has
  /// no effect.
  void close();

  /// Save Mesh
  /// @param[in] mesh
  /// @param[in] xpath XPath where Mesh Grid will be written
  void write_mesh(const Mesh& mesh, std::string xpath = "/Xdmf/Domain");


  /// @brief Write a fem::Function to file.
  ///
  /// @pre The fem::Function `u` must be (i) a lowest-order (P0)
  /// discontinuous Lagrange element or (ii) a continuous Lagrange
  /// element where the element 'nodes' are the same as the nodes of its
  /// mesh::Mesh. Otherwise an exception is raised.
  ///
  /// @note User interpolation to a suitable Lagrange space may be
  /// required to satisfy the precondition on `u`. The VTX output
  /// (io::VTXWriter) format is recommended over XDMF for discontinuous
  /// and/or high-order spaces.
  ///
  /// @param[in] u Function to write to file.
  /// @param[in] t Time stamp to associate with `u`.
  /// @param[in] mesh_xpath XPath for a Grid under which `u` will be
  /// inserted.
  void write_system(ImplicitSystem& sys, double t,
                      std::string mesh_xpath
                      = "/Xdmf/Domain/Grid[@GridType='Uniform'][1]");



  /// Write Information
  /// @param[in] name
  /// @param[in] value String to store into Information tag
  /// @param[in] xpath XPath where Information will be inserted
  void write_information(std::string name, std::string value,
                         std::string xpath = "/Xdmf/Domain/");

  /// Get the MPI communicator
  /// @return The MPI communicator for the file object
  MPI_Comm comm() const;

private:
  // MPI communicator
  MPI_Comm _comm;

  // Filename
  std::filesystem::path _filename;

  // File mode
  std::string _file_mode;

  // HDF5 file handle
  hid_t _h5_id;

  // The XML document currently representing the XDMF which needs to be
  // kept open for time series etc.
  std::unique_ptr<pugi::xml_document> _xml_spatial_doc;
  std::unique_ptr<pugi::xml_document> _xml_doc;

  Encoding _encoding;

  Compression _compression;

};


#endif /* XDMF_FILE_H__ */
