#ifndef ENCODE_H
#define ENCODE_H

#ifdef HAS_ZLIB
#include "zlib.h"
#endif

#include "base64.h"


/// This class provides tools for encoding and compressing streams
/// for use in output files
/// We cheating in some functions by relying on std::vector data being
/// contiguous in memory. This will be part of the upcoming C++ standard.

namespace Encoder
{
    template<typename T>
    static void encode_base64(const T* data, std::size_t length,
                              std::stringstream& encoded_data)
    {
      encoded_data << base64_encode((const unsigned char*) &data[0],
                                    length*sizeof(T));
    }

#ifdef HAS_ZLIB
    template<typename T>
    static std::vector<unsigned char> compress_data(const std::vector<T>& data)
    {
      // Compute length of uncompressed data
      const unsigned long uncompressed_size = data.size()*sizeof(T);

      // Compute maximum length of compressed data
      unsigned long compressed_size = (uncompressed_size + (((uncompressed_size)/1000)+1)+12);;

      // Allocate space for compressed data
      std::vector<unsigned char> compressed_data(compressed_size);

      // Compress data
      if (compress((Bytef*) compressed_data.data(), &compressed_size,
                   (const Bytef*) data.data(), uncompressed_size) != Z_OK)
      {
        runtime_error("Encoder.h: Zlib error while compressing data");
      }

      // Return data
      return compressed_data;
    }
#endif

}

#endif /* ENCODE_H */
