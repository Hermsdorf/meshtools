#ifndef GMSH_IO_H__
#define GMSH_IO_H__

#include "mesh.h"

#include <string>


class GmshIO
{
    public:
        GmshIO();
        ~GmshIO();

        static void read(const std::string &filename,        Mesh &mesh);
        static void write(const std::string &filename,       Mesh &mesh);
};

#endif /* GMSH_IO_H__ */
