#ifndef GLVIS_WRITER_H
#define GLVIS_WRITER_H


#include <iostream>
#include <iomanip>
#include <memory>

#include "mesh.h"

class glvisWriter 
{

    public:

        glvisWriter();
        ~glvisWriter();
        bool open(std::string name);
        void write_mesh(Mesh& mesh);
        void close();
    private:

        std::ofstream fout;

};

#endif /* GLVIS_WRITER_H */
