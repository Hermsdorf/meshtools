

#include "vtk_writer.h"

#include <iostream>
#include <iomanip>


std::string prefix_level(int level)
{
    std::string p = "";
    for(int i =0; i < level; i++) p+=" ";
    return p;
}

std::string BinaryBigEndian(void) 
{
    long _v = 1; 
    return ((char*)&_v)[0] ? "LittleEndian" : "BigEndian";
}


vtkWriter::vtkWriter():is_point_data_open(false), is_cell_data_open(false)
{

}


bool vtkWriter::open(std::string base_file_name, unsigned int file_number)
{   
    this->base_name   = base_file_name;
    this->file_number = file_number;

    stringstream base_name_sufix;
    base_name_sufix << base_file_name << "_"
                    << std::setw(4) << std::setfill('0') << MeshTools::n_processors() << "_"
                    << std::setw(4) << std::setfill('0') << MeshTools::processor_id() << "_"
                    << std::setw(4) << std::setfill('0') << file_number;
    std::string vtu_file  = base_name_sufix.str() +".vtu";
    std::string pvtu_file = base_name_sufix.str() +".pvtu";

    this->fvtu.open(vtu_file);
    if(!fvtu.is_open())
    {
        std::cout << "[vtkWriter]: Error opening file: "<< vtu_file << std::endl;
        return false;
    }
    fvtu << "<VTKFile type=\"UnstructuredGrid\" version=\"1.0\" byte_order=\""<< BinaryBigEndian() <<"\" header_type=\"UInt64\">" << std::endl;
    fvtu << prefix_level(1) <<"<UnstructuredGrid>" << std::endl ;

    fvtu << std::flush;

    if(MeshTools::processor_id() == 0)
    {
        this->fpvtu.open(pvtu_file);
        fpvtu << "<VTKFile type=\"PUnstructuredGrid\" version=\"1.0\" byte_order=\""<< BinaryBigEndian() <<"\" header_type=\"UInt64\">" << std::endl;
        fpvtu << prefix_level(1) <<"<PUnstructuredGrid>" << std::endl;
        fpvtu << std::flush;
    }



    return true;
}

vtkWriter::~vtkWriter()
{
    if(fvtu.is_open()) fvtu.close();
    if(fpvtu.is_open()) fpvtu.close();
}

void vtkWriter::write_mesh(Mesh & mesh)
{

    fvtu << std::setprecision(16);
    fvtu << prefix_level(2) << "<Piece NumberOfPoints=\""<<mesh.get_n_nodes()<<"\" NumberOfCells=\"" << mesh.get_n_elements() + mesh.get_n_surface_elements() <<"\">" << std::endl;
    fvtu << prefix_level(3) << "<Points>" << std::endl;
    write_data_item<double>(mesh.get_coordinate_vector().data(),mesh.get_coordinate_vector().size(),"Points", prefix_level(4),3);
    fvtu << prefix_level(3) << "</Points>" << std::endl;
    fvtu << prefix_level(3) << "<Cells>" << std::endl;
    write_data_item<unsigned int>(mesh.get_connectivity_vector().data(),mesh.get_connectivity_vector().size(),"connectivity", prefix_level(4));
    write_data_item<unsigned int>(mesh.get_offset_vector().data(),mesh.get_offset_vector().size(),"offsets", prefix_level(4));
    write_data_item<unsigned short>(mesh.get_element_type_vector().data(),mesh.get_element_type_vector().size(),"types", prefix_level(4));
    fvtu << prefix_level(3) << "</Cells>"    << std::endl;

    if(MeshTools::processor_id()==0)
    {
        fpvtu << prefix_level(2) <<"<PPoints>" << std::endl;
        fpvtu << prefix_level(3) <<"<PDataArray type=\"Float64\" Name = \"Points\" NumberOfComponents=\"3\"/>" << std::endl;
        fpvtu << prefix_level(2) <<"</PPoints>" << std::endl;
        fpvtu << prefix_level(2) <<"<PCells>" <<std::endl;
        fpvtu << prefix_level(3) <<"<PDataArray type=\"" << get_vtk_type_name<unsigned int>()   <<"\" Name=\"connectivity\"/>" << std::endl;
        fpvtu << prefix_level(3) <<"<PDataArray type=\"" << get_vtk_type_name<unsigned int>()   <<"\" Name=\"offsets\"/>" << std::endl;
        fpvtu << prefix_level(3) <<"<PDataArray type=\"" << get_vtk_type_name<unsigned short>() <<"\" Name=\"types\"/>" << std::endl;
        fpvtu << prefix_level(2) <<"</PCells>" << std::endl;
    }

}

void vtkWriter::start_point_data_section()
{
    this->is_point_data_open = true;
    fvtu <<  prefix_level(3) <<"<PointData>"<< std::endl ;
    if(MeshTools::processor_id()==0)
       fpvtu <<  prefix_level(3) <<"<PPointData>"<< std::endl;
}

// template <typename T>
// void vtkWriter::write_point_data(T* data, size_t size, std::string name, int ncomp)
// {
//     if(!is_point_data_open)
//     {
//         std::cout << "[vtkWriter]: Impossivel write dataset: "<< name<< std::endl;
//         return;
//     }
//     write_data_item<T>(data,size,name,prefix_level(4), ncomp);

//     if(MeshTools::processor_id() == 0)
//     {
//         fpvtu << prefix_level(4) <<"<PDataArray type=\"" << get_vtk_type_name<T>()   <<"\" Name=\""<< name <<"\"/>" << std::endl;
//     }

// }

void vtkWriter::close_point_data_section()
{
    this->is_point_data_open = false;
    fvtu <<  prefix_level(3) <<"</PointData>"<<std::endl;
    if(MeshTools::processor_id()==0)
       fpvtu <<  prefix_level(3) <<"</PPointData>"<<std::endl;
}

void vtkWriter::start_cell_data_section()
{
    this->is_cell_data_open = true;
    fvtu <<  prefix_level(3)    <<"<CellData>"   <<std::endl;
    if(MeshTools::processor_id()==0)
       fpvtu <<  prefix_level(3) <<"<PCellData>"  <<std::endl;
}


void vtkWriter::close_cell_data_section()
{
    this->is_cell_data_open = false;
    fvtu <<  prefix_level(3) <<"</CellData>"<< std::endl;
    if(MeshTools::processor_id()==0)
       fpvtu <<  prefix_level(3) <<"</PCellData>"<<std::endl;
}

void vtkWriter::close()
{
    
    fvtu << prefix_level(1)<<"</UnstructuredGrid>" << std::endl;
    fvtu << "</VTKFile>" << std::endl;
    fvtu.close();
    
    if(MeshTools::processor_id() == 0) 
    {
        for(int p = 0; p < MeshTools::n_processors(); ++p)
        {
            
            stringstream base_name_sufix;
            base_name_sufix << base_name << "_"
                            << std::setw(4) << std::setfill('0') << MeshTools::n_processors() << "_"
                            << std::setw(4) << std::setfill('0') << MeshTools::processor_id() << "_"
                            << std::setw(4) << std::setfill('0') << file_number;
            std::string vtu_file  = base_name_sufix.str() +".vtu";

            fpvtu <<  prefix_level(2) << "<Piece Source=\""<< vtu_file << "\" \\> " << std::endl;
            fpvtu  << prefix_level(1) << "</PUnstructuredGrid>" << std::endl;
            fpvtu << "</VTKFile>"     << endl;
        }
        fpvtu.close();
    }

}


