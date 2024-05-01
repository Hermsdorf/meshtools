#if 0
#include "hdf5_helper.h"

#ifdef HDF5_ENABLE
namespace hdf5_helper 
{


hid_t setupHDF5Compressor(hsize_t n, hsize_t *chunk, uint prec=0)
{
    
    unsigned int cd_values[10];
    int i, cd_nelmts = 10;
    hid_t cpid = H5P_DEFAULT;
    
    /* setup dataset creation properties */
    if (0 > (cpid = H5Pcreate(H5P_DATASET_CREATE))) ERROR(H5Pcreate);
    if (0 > H5Pset_chunk(cpid, n, chunk)) ERROR(H5Pset_chunk);

#ifdef H5Z_ZFP_USE_PLUGIN
    /* setup zfp filter via generic (cd_values) interface */
    if (prec != 0)
        H5Pset_zfp_precision_cdata(prec, cd_nelmts, cd_values);
    else 
        H5Pset_zfp_reversible_cdata(cd_nelmts, cd_values);
    /* Add filter to the pipeline via generic interface */
    if (0 > H5Pset_filter(cpid, H5Z_FILTER_ZFP, H5Z_FLAG_MANDATORY, cd_nelmts, cd_values)) ERROR(H5Pset_filter);
#else
    if (0 > H5Pset_deflate(cpid, 6) ) ERROR(H5Pset_deflate);
#endif
    return cpid;
}


void writeHDF5DoubleDataSet(hid_t file,hid_t cpid, const char* datasetname, hsize_t dimsf, double* buffer)
{
    hid_t      dataspace, dataset;   /* handles */
    herr_t     status;   

    dataspace = H5Screate_simple(1, &dimsf, NULL); 
    dataset   = H5Dcreate(file,datasetname, H5T_NATIVE_DOUBLE, dataspace,H5P_DEFAULT ,cpid,H5P_DEFAULT);
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
    dataset   = H5Dcreate(file,datasetname, H5T_NATIVE_UINT, dataspace, H5P_DEFAULT ,H5P_DEFAULT,H5P_DEFAULT);
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

void writeHDF5IntegerDataSet(hid_t file, const char* datasetname, hsize_t dimsf, int* buffer)
{
    hid_t      dataspace, dataset;   /* handles */
    herr_t     status;   

    dataspace = H5Screate_simple(1, &dimsf, NULL); 
    dataset   = H5Dcreate(file,datasetname, H5T_NATIVE_INT, dataspace, H5P_DEFAULT ,H5P_DEFAULT,H5P_DEFAULT);
    /*
     * Write the data to the dataset using default transfer properties.
     */
    status = H5Dwrite(dataset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer);

    /*
     *  Close/release resources.
     */
    H5Sclose(dataspace);
    H5Dclose(dataset);
}


void writeHDF5UshortDataSet(hid_t file, const char* datasetname, hsize_t dimsf, unsigned short* buffer)
{
    hid_t      dataspace, dataset;   /* handles */
    herr_t     status;   

    dataspace = H5Screate_simple(1, &dimsf, NULL); 
    dataset   = H5Dcreate(file,datasetname, H5T_NATIVE_USHORT, dataspace, H5P_DEFAULT ,H5P_DEFAULT,H5P_DEFAULT);
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

}

#endif // HDF5_ENABLE

#endif