#ifndef AA1E2F72_C987_46B6_84C1_4766F456EAEB
#define AA1E2F72_C987_46B6_84C1_4766F456EAEB

#include "hdf5.h" 

void writeHDF5DoubleDataSet(hid_t file, const char* datasetname, hsize_t dimsf, double* buffer)
{
    hid_t      dataspace, dataset;   /* handles */
    herr_t     status;   

    dataspace = H5Screate_simple(1, &dimsf, NULL); 
    dataset   = H5Dcreate(file,datasetname, H5T_NATIVE_DOUBLE, dataspace, H5P_DEFAULT);
    /*
     * Write the data to the dataset using default transfer properties.
     */
    status = H5Dwrite(dataset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer);

    /*
     *  Close/release resources.
     */
    H5Sclose(dataspace);
    H5Dclose(dataset);
}

void writeHDF5UIntegerDataSet(hid_t file, const char* datasetname, hsize_t dimsf, unsigned int* buffer)
{
    hid_t      dataspace, dataset;   /* handles */
    herr_t     status;   

    dataspace = H5Screate_simple(1, &dimsf, NULL); 
    dataset   = H5Dcreate(file,datasetname, H5T_NATIVE_UINT, dataspace, H5P_DEFAULT);
    /*
     * Write the data to the dataset using default transfer properties.
     */
    status = H5Dwrite(dataset, H5T_NATIVE_UINT, H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer);

    /*
     *  Close/release resources.
     */
    H5Sclose(dataspace);
    H5Dclose(dataset);
}


void writeHDF5UshortDataSet(hid_t file, const char* datasetname, hsize_t dimsf, unsigned int* buffer)
{
    hid_t      dataspace, dataset;   /* handles */
    herr_t     status;   

    dataspace = H5Screate_simple(1, &dimsf, NULL); 
    dataset   = H5Dcreate(file,datasetname, H5T_NATIVE_USHORT, dataspace, H5P_DEFAULT);
    /*
     * Write the data to the dataset using default transfer properties.
     */
    status = H5Dwrite(dataset, H5T_NATIVE_USHORT, H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer);

    /*
     *  Close/release resources.
     */
    H5Sclose(dataspace);
    H5Dclose(dataset);
}

#endif /* AA1E2F72_C987_46B6_84C1_4766F456EAEB */
