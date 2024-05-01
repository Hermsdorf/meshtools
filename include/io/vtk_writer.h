#ifndef VTK_WRITER_H
#define VTK_WRITER_H

#include <iostream>
#include <fstream>
#include <typeinfo>

#include "meshtools.h"
#include "parallel_mesh.h"
#include  "encode.h"


class vtkWriter
{
    public:

        vtkWriter();

        ~vtkWriter();

        void open(std::string _filename);

        void write_mesh(Mesh &mesh);
        
        void start_point_data_section();

        void close_point_data_section();

        template <typename T>
        void write_point_data(T* data, size_t size, std::string name, int ncomp = 0);

        template <typename T>
        void write_cell_data(T* data, size_t size, std::string name, int ncomp = 0);

        void start_cell_data_section();

        void close_cell_data_section();

        void close();

    private:
    
        //template <typename T> string get_vtk_type_name();
        
        template <typename T>
        void write_data_item(T* data, size_t size, std::string name, std::string prefix, int ncomp = 0);

        std::ofstream fvtu;
        std::ofstream fpvtu;

        std::string base_name;

        bool is_point_data_open;

        bool is_cell_data_open;

};

template <typename T>
string get_vtk_type_name()
{
    if( typeid(T) == typeid(short)         ) return "Int8";
    if( typeid(T) == typeid(unsigned short)) return "UInt8";
    if( typeid(T) == typeid(int)           ) return "Int32";
    if( typeid(T) == typeid(unsigned int)  ) return "UInt32";
    if( typeid(T) == typeid(long)          ) return "Int64";
    if( typeid(T) == typeid(unsigned long) ) return "UInt64";
    if( typeid(T) == typeid(float)         ) return "Float32";
    if( typeid(T) == typeid(double)        ) return "Float64";
    return "";
}

template <typename T>
void vtkWriter::write_data_item(T* data, size_t size, std::string name,std::string  prefix, int ncomp)
{
    std::stringstream encoded_data; 
    Encoder::encode_base64(data,size, encoded_data);

    this->fvtu << prefix << "<DataArray type=\""               << get_vtk_type_name<T>() <<"\""
                                   << " Name=\""               << name                   << "\"";
    
    if(ncomp > 0 )
        this->fvtu << " NumberOfComponents=\"" << ncomp << "\"";
    
    this->fvtu << " format=\"binary\" />"  << endl;
    this->fvtu << encoded_data.str() ;
    this->fvtu << endl << prefix << "</DataArray>" << std::endl;
                                   
}



#endif /* VTK_WRITER_H */
