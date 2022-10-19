#ifndef AA1E2F72_C987_46B6_84C1_4766F456EAEB
#define AA1E2F72_C987_46B6_84C1_4766F456EAEB


#include <errno.h>
#include <fcntl.h>
#include <string.h>

#include "hdf5.h" 

#ifdef H5Z_ZFP_USE_PLUGIN
#include "H5Zzfp_plugin.h"
#endif




namespace hdf5_helper{



/* convenience macro to handle errors */
#define ERROR(FNAME)                                              \
do {                                                              \
    int _errno = errno;                                           \
    fprintf(stderr, #FNAME " failed at line %d, errno=%d (%s)\n", \
        __LINE__, _errno, _errno?strerror(_errno):"ok");          \
    return 1;                                                     \
} while(0)



void writeHDF5DoubleDataSet(hid_t file, const char* datasetname, hsize_t dimsf, double* buffer);

void writeHDF5UIntegerDataSet(hid_t file, const char* datasetname, hsize_t dimsf, unsigned int* buffer);

void writeHDF5IntegerDataSet(hid_t file, const char* datasetname, hsize_t dimsf, int* buffer);

void writeHDF5UshortDataSet(hid_t file, const char* datasetname, hsize_t dimsf, unsigned short* buffer);

}

#endif /* AA1E2F72_C987_46B6_84C1_4766F456EAEB */
