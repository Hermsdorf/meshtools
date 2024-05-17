#ifndef VTK_WRITER_H__
#define VTK_WRITER_H__

#include <iostream>
#include <fstream>
#include <typeinfo>

#include "meshtools.h"
#include "parallel_mesh.h"
#include  "encode.h"

std::string prefix_level(int level);

class vtkWriter
{
    public:

        vtkWriter();

        ~vtkWriter();

        bool open(std::string _filename, unsigned int file_number=0);

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

        unsigned int file_number;

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
    //std::stringstream encoded_data; 
    //Encoder::encode_base64(data,size, encoded_data);

    this->fvtu << prefix << "<DataArray type=\""               << get_vtk_type_name<T>() <<"\""
                                   << " Name=\""               << name                   << "\"";
    
    if(ncomp > 0 )
        this->fvtu << " NumberOfComponents=\"" << ncomp << "\"";
    
    //this->fvtu << " format=\"binary\" />"  << endl;
    //this->fvtu << encoded_data.str() ;
    this->fvtu << " format=\"ascii\">"  << endl;
    this->fvtu << prefix_level(5);
    for(int i = 0; i < size; i++)
    {
        this->fvtu << data[i] << " ";
        if((i+1)%6 == 0)
            this->fvtu << std::endl << prefix_level(5) ;
    }

    this->fvtu << endl << prefix << "</DataArray>" << std::endl;
                                   
}

template <typename T>
void vtkWriter::write_point_data(T* data, size_t size, std::string name, int ncomp)
{
    if(!is_point_data_open)
    {
        std::cout << "[vtkWriter]: Impossivel write dataset: "<< name<< std::endl;
        return;
    }
    write_data_item<T>(data,size,name,prefix_level(4), ncomp);

    if(MeshTools::processor_id() == 0)
    {
        fpvtu << prefix_level(4) <<"<PDataArray type=\"" << get_vtk_type_name<T>()   <<"\" Name=\""<< name <<"\"/>" << std::endl;
    }
}

template <typename T>
void vtkWriter::write_cell_data(T* data, size_t size, std::string name, int ncomp)
{
    if(!is_cell_data_open)
    {
        std::cout << "[vtkWriter]: Impossivel write dataset: "<< name<< std::endl;
        return;
    }
    write_data_item<T>(data,size,name,prefix_level(4), ncomp);

    if(MeshTools::processor_id() == 0)
    {
        fpvtu << prefix_level(4) <<"<PDataArray type=\"" << get_vtk_type_name<T>()   <<"\" Name=\""<< name <<"\"/>" << std::endl;
    }
}




#endif /* VTK_WRITER_H__ */
