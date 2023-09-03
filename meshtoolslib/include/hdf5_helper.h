#ifndef HDF5_HELPER_H
#define HDF5_HELPER_H

#include <errno.h>
#include <fcntl.h>
#include <string.h>

#include "meshtools_config.h"

#ifdef HDF5_ENABLE
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


hid_t setupHDF5Compressor(hsize_t n, hsize_t *chunk, uint prec);

void writeHDF5DoubleDataSet(hid_t file, hid_t cpid, const char* datasetname, hsize_t dimsf, double* buffer);

void writeHDF5UIntegerDataSet(hid_t file, const char* datasetname, hsize_t dimsf, unsigned int* buffer);

void writeHDF5IntegerDataSet(hid_t file, const char* datasetname, hsize_t dimsf, int* buffer);

void writeHDF5UshortDataSet(hid_t file, const char* datasetname, hsize_t dimsf, unsigned short* buffer);

}

#endif /* HDF5_ENABLE */
#endif /* HDF5_HELPER_H */
