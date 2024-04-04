#ifndef HDF5_INTERFACE_H__
#define HDF5_INTERFACE_H__

#pragma once

#include <array>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <hdf5.h>
#include <mpi.h>
#include <numeric>
#include <string>
#include <vector>

namespace hdf5
{

/// C++ type to HDF5 data type
template <typename T>
hid_t hdf5_type()
{
  if constexpr (std::is_same_v<T, int>)
    return H5T_NATIVE_INT;
  if constexpr (std::is_same_v<T, float>)
    return H5T_NATIVE_FLOAT;
  else if constexpr (std::is_same_v<T, double>)
    return H5T_NATIVE_DOUBLE;
  else if constexpr (std::is_same_v<T, std::int32_t>)
    return H5T_NATIVE_INT32;
  else if constexpr (std::is_same_v<T, std::uint32_t>)
    return H5T_NATIVE_UINT32;
  else if constexpr (std::is_same_v<T, std::int64_t>)
    return H5T_NATIVE_INT64;
  else if constexpr (std::is_same_v<T, std::uint64_t>)
    return H5T_NATIVE_UINT64;
  else if constexpr (std::is_same_v<T, std::size_t>)
  {
    throw std::runtime_error(
        "Cannot determine size of std::size_t. std::size_t is not the same "
        "size as long or int.");
  }
  else
  {
    throw std::runtime_error("Cannot get HDF5 primitive data type. No "
                             "specialised function for this data type.");
  }
}

/// Open HDF5 and return file descriptor
/// @param[in] comm MPI communicator
/// @param[in] filename Name of the HDF5 file to open
/// @param[in] mode Mode in which to open the file (w, r, a)
/// @param[in] use_mpi_io True if MPI-IO should be used
hid_t open_file(const std::filesystem::path& filename);

/// Close HDF5 file
/// @param[in] handle HDF5 file handle
void close_file(hid_t handle);

/// Flush data to file to improve data integrity after interruption
/// @param[in] handle HDF5 file handle
void flush_file(hid_t handle);

/// Get filename
/// @param[in] handle HDF5 file handle
/// return The filename
std::filesystem::path get_filename(hid_t handle);

/// Check for existence of dataset in HDF5 file
/// @param[in] handle HDF5 file handle
/// @param[in] dataset_path Data set path
/// @return True if @p dataset_path is in the file
bool has_dataset(hid_t handle, const std::string& dataset_path);

/// Open dataset
/// @param[in] handle HDF5 file handle.
/// @param[in] path Data set path.
/// @return Data set handle. Should be closed by caller using `H5Dclose`.
hid_t open_dataset(hid_t handle, const std::string& path);


/// Add group to HDF5 file
/// @param[in] handle HDF5 file handle
/// @param[in] dataset_path Data set path to add
void add_group(hid_t handle, const std::string& dataset_path);


/// Write data to existing HDF file as defined by range blocks on each
/// process
/// @param[in] file_handle HDF5 file handle
/// @param[in] dataset_path Path for the dataset in the HDF5 file
/// @param[in] data Data to be written, flattened into 1D vector
///   (row-major storage)
/// @param[in] range The local range on this processor
/// @param[in] global_size The global shape shape of the array
/// @param[in] use_mpi_io True if MPI-IO should be used
/// @param[in] use_chunking True if chunking should be used
template <typename T>
void write_dataset(hid_t file_handle, const std::string& dataset_path,
                   const T* data, std::vector<hsize_t> &dimsf, bool use_chunking)
{
  // Data rank
  const int rank = dimsf.size();

  // Get HDF5 data type
  const hid_t h5type = hdf5::hdf5_type<T>();

  // Create a global data space
  const hid_t filespace0 = H5Screate_simple(rank, dimsf.data(), nullptr);
  if (filespace0 == H5I_INVALID_HID)
    throw std::runtime_error("Failed to create HDF5 data space");

  // Set chunking parameters

  hid_t chunking_properties = H5P_DEFAULT;

  // Check that group exists and recursively create if required
  const std::string group_name(dataset_path, 0, dataset_path.rfind('/'));
  add_group(file_handle, group_name);

  // Create data space
  const hid_t memspace = H5Screate_simple(rank, count.data(), nullptr);
  if (memspace == H5I_INVALID_HID)
    throw std::runtime_error("Failed to create HDF5 local data space.");

  // Create a file dataspace within the global space - a hyperslab
  const hid_t filespace1 = H5Dget_space(dset_id);
    if (filespace1 == H5I_INVALID_HID)
        throw std::runtime_error("Failed to get HDF5 data space.");

  // Write local dataset into selected hyperslab
  if (H5Dwrite(dset_id, h5type, memspace, filespace1, plist_id, data) < 0)
  {
    throw std::runtime_error(
        "Failed to write HDF5 local dataset into hyperslab.");
  }

  // Close dataset collectively
  if (H5Dclose(dset_id) < 0)
    throw std::runtime_error("Failed to close HDF5 dataset.");

  // Close local dataset
  if (H5Sclose(memspace) < 0)
    throw std::runtime_error("Failed to close local HDF5 dataset.");

}
} // namespace hdf5


#endif /* HDF5_INTERFACE_H__ */
