
#include "meshtools.h"
#include "xdmf_writer.h"
#include "hdf5_helper.h"
#include "transient_implicit_system.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstdlib>
#include <cstdio>

using namespace std;

#define XDMF_FILE_SIZE 1024


#ifndef HDF5_ENABLE
void BIN_WriteInt(const char *datasetname, void *dataset, long size)
{
    FILE *fbin = fopen(datasetname,"w");
    fwrite(dataset,sizeof(int),size,fbin);
    fclose(fbin);
     
}
void BIN_WriteDouble(const char *datasetname, void *dataset, long size)
{
    FILE *fbin = fopen(datasetname,"w");
    fwrite(dataset,sizeof(double),size,fbin);
    fclose(fbin);
     
}
#endif


//
// XDMF Constructor. 
XDMFWriter::XDMFWriter(std::string filename): 
   						                    n_timestep(0),
                                            basename(filename),
                                            dir(".")
                                            
{
    this->processor_id = MeshTools::processor_id();
    this->n_processors = MeshTools::n_processors();
    this->using_compression = false;

}

XDMFWriter::~XDMFWriter()
{
}


void XDMFWriter::set_dir_path(std::string path)
{
    this->dir = path;
    struct stat st = {0};
    if (stat(dir.c_str(), &st) == -1) {
        mkdir(dir.c_str(), 0700);
    }
}

int XDMFWriter::write(ImplicitSystem * system)
{
    return this->write(system,0.0);
}

int XDMFWriter::write(ImplicitSystem * system, double time)
{
    char filename[XDMF_FILE_SIZE];
    char xdmf_filename[XDMF_FILE_SIZE];
        
    std::vector<double> coords;
    std::vector<int>    conn;
    std::vector<double> solution;

    auto mesh = system->get_mesh();

    this->n_local_nodes = mesh.get_n_nodes();
    this->n_local_elem  = mesh.get_n_elements();

    if(this->n_timestep == 0)
    {
        // write mesh data
        char meshdir[256];
        sprintf(meshdir,"%s/mesh", this->dir.c_str());
        struct stat st2 = {0};
        if (stat(meshdir, &st2) == -1) {
            mkdir(meshdir, 0700);
        }

    
        unsigned int  conn_size    = mesh.getElementConnectivitySize();
        unsigned int *ptr_conn     = mesh.getElementConnectivityData();
        auto coords                = mesh.getCoord();
#ifdef HDF5_ENABLE
        hid_t   cpid = 0;
        hid_t   fid;
        herr_t  status;


        sprintf(filename,"%s/%s_mesh_%d_%03d.h5",meshdir,this->basename.c_str(),n_processors,processor_id);
    
        if (0 > (fid = H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT))) ERROR(H5Fcreate);
            
        hdf5_helper::writeHDF5UIntegerDataSet(fid,"/conn" ,conn_size, ptr_conn);
        hdf5_helper::writeHDF5DoubleDataSet(fid,cpid,"/coords",coords.size(), &coords[0]);

        if (0 > H5Pclose(cpid)) ERROR(H5Pclose);
        if (0 > H5Fclose(fid)) ERROR(H5Fclose);
#else
        sprintf(filename,"%s/%s.coords.%d.%03d.%05d.bin",meshdir,this->basename.c_str(),n_processors,processor_id, this->n_timestep);
        BIN_WriteDouble(filename,&coords[0],coords.size());
        sprintf(filename,"%s/%s.conn.%d.%03d.%05d.bin",meshdir,this->basename.c_str(),n_processors,processor_id, this->n_timestep);
        BIN_WriteInt(filename,&conn[0],conn.size());
#endif

    }

    // Write solution data

    solution.resize(this->n_local_nodes);
    
    char stepdir[256];
    sprintf(stepdir,"%s/step%d", this->dir.c_str(), this->n_timestep);
    struct stat st = {0};
    if (stat(stepdir, &st) == -1) {
        mkdir(stepdir, 0700);
    }

#ifdef HDF5_ENABLE
    hid_t   cpid = H5P_DEFAULT;
    hid_t   fid;
    herr_t  status;
    hsize_t chunk = 512;

    if(this->using_compression)
        cpid = hdf5_helper::setupHDF5Compressor(this->n_local_nodes,&chunk,0);

    sprintf(filename,"%s/%s_%d_%03d_%05d.h5",stepdir,this->basename.c_str(),n_processors,processor_id, this->n_timestep);
    if (0 > (fid = H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT))) ERROR(H5Fcreate);

    for(int nv=0; nv < system->get_equation_manager().get_n_dofs(); ++nv)
    {
       this->get_variable_solution(system,nv,solution);
       std::string dataset_name = "/"+system->get_variable_name(nv);
       hdf5_helper::writeHDF5DoubleDataSet(fid,cpid,dataset_name.c_str(),solution.size(),&solution[0]);
        
    }
    if (0 > H5Pclose(cpid)) ERROR(H5Pclose);
    if (0 > H5Fclose(fid)) ERROR(H5Fclose);
#else


    //LOOP over all system
     for(int nv=0; nv < system->get_equation_manager().get_n_dofs(); ++nv)
    {
        
        this->get_variable_solution(system,nv,solution);
        std::string v = system->get_variable_name(nv);
        

        sprintf(filename,"%s/%s.%s.%d.%03d.%05d.bin",stepdir,this->basename.c_str(),v.c_str(),
                                                      n_processors,processor_id, this->n_timestep);
        BIN_WriteDouble(filename,&solution[0],solution.size());
        
    }
    
#endif
    
    write_spatial_collection(system, time);
    write_temporal_collection();

    this->n_timestep++;
    return 0;
}

void XDMFWriter::write_spatial_collection(ImplicitSystem* system, double time)
{
    unsigned int data[2];
    unsigned int *local_data = new unsigned int [2*this->n_processors];
    
    auto mesh = system->get_mesh();
    
    data[0] = this->n_local_nodes;
    data[1] = this->n_local_elem;
    MPI_Gather(data, 2, MPI_UNSIGNED, local_data, 2, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
    
    if(processor_id == 0)
    {
        char filename[255];
        string elem_name;
        int nnoel = 0;
        if(mesh.get_mesh_element_type() == HEX8) {
             elem_name= "Hexahedron";
             nnoel = 8;
        }
        else if(mesh.get_mesh_element_type() == TET4) {
            elem_name = "Tetrahedron";
            nnoel = 4;
        } else if(mesh.get_mesh_element_type() == TRI3) {
             elem_name= "Triangle";
             nnoel = 3;
        }
        else if(mesh.get_mesh_element_type() == QUAD4) {
            elem_name = "Quadrilateral";
            nnoel = 4;
        } 
        
        char stepdir[XDMF_FILE_SIZE];
        sprintf(stepdir,"%s/step%d", this->dir.c_str(), this->n_timestep);

        char meshfile[XDMF_FILE_SIZE];
        sprintf(meshfile,"mesh/%s_mesh",this->basename.c_str());

        char stepfile[XDMF_FILE_SIZE];
        sprintf(stepfile,"step%d/%s",this->n_timestep ,this->basename.c_str());
        
       
        sprintf(filename,"%s/%s_%d_%05d.xmf", this->dir.c_str(),this->basename.c_str(), n_processors,this->n_timestep);
        FILE * fxml = fopen(filename, "w");
        fprintf(fxml,"<?xml version=\"1.0\" ?>\n");
        fprintf(fxml,"<!-- DOCTYPE Xdmf SYSTEM \"Xdmf.dtd\" [] --> \n");
        fprintf(fxml,"<Xdmf xmlns:xi=\"http://www.w3.org/2001/XInclude\" Version=\"2.0\"> \n");
        fprintf(fxml,"<Domain Name=\"MeshTools\">\n");
        fprintf(fxml," <Grid Name=\"%s_%d_%05d\" GridType=\"Collection\" CollectionType=\"Spatial\">\n", stepfile, n_processors,this->n_timestep );
        fprintf(fxml,"  <Time Type=\"Single\" Value=\"%f\" />\n", time);
        for(int p=0; p < n_processors; p++)
        {
            int lnodes = local_data[p*2];
            int lelem  = local_data[p*2+1];
            fprintf(fxml," <Grid Name=\"%s_%d_%03d\" Type=\"Uniform\"> \n",this->basename.c_str(), n_processors, p);  
            fprintf(fxml,"  <Topology Type=\"%s\" NumberOfElements=\"%d\"  BaseOffset=\"0\">\n",elem_name.c_str(),lelem);
#ifdef HDF5_ENABLE
            fprintf(fxml,"    <DataItem Dimensions=\"%d\" NumberType=\"UInt\" Format=\"HDF\"> %s_%d_%03d.h5:/conn</DataItem>\n",lelem*nnoel,meshfile,n_processors,p);
#else
           fprintf(fxml,"    <DataItem Dimensions=\"%d\" NumberType=\"UInt\" Format=\"Binary\" Endian=\"Little\"> %s.con.%d.%03d.bin </DataItem>\n",lelem*nnoel,meshfile,n_processors,p);
#endif

            fprintf(fxml,"  </Topology>\n");
            fprintf(fxml,"  <Geometry Type=\"XYZ\">\n");
#ifdef HDF5_ENABLE
            fprintf(fxml,"    <DataItem Dimensions=\"%d\" NumberType=\"Float\" Precision=\"8\" Format=\"HDF\">%s_%d_%03d.h5:/coords</DataItem>\n",lnodes*3,meshfile,n_processors,p);
#else
            fprintf(fxml,"    <DataItem Dimensions=\"%d\" NumberType=\"Float\" Precision=\"8\" Format=\"Binary\" Endian=\"Little\">%s.coords.%d.%03d.bin </DataItem>\n",lnodes*3,meshfile,n_processors,p);
#endif
            fprintf(fxml,"  </Geometry>\n");
            
            //LOOP over all system
            for(int nv=0; nv < system->get_equation_manager().get_n_dofs(); ++nv)
            {
                std::string dataset_name = system->get_variable_name(nv);

                fprintf(fxml,"  <Attribute Name=\"%s\" AttributeType=\"Scalar\" Center=\"Node\">\n", dataset_name.c_str());
#ifdef HDF5_ENABLE
                fprintf(fxml,"    <DataItem Dimensions=\"%d\" NumberType=\"Float\" Precision=\"8\" Format=\"HDF\"> %s_%d_%03d_%05d.h5:/%s</DataItem>\n",lnodes,stepfile,n_processors,p,this->n_timestep, dataset_name.c_str());
#else
		        fprintf(fxml,"    <DataItem Dimensions=\"%d\" NumberType=\"Float\" Precision=\"8\" Format=\"Binary\" Endian=\"Little\">%s.%s.%d.%03d.%05d.bin </DataItem>\n",lnodes,stepfile,dataset_name.c_str(),n_processors,p,this->n_timestep);
#endif
                fprintf(fxml,"  </Attribute>\n");
        
           }
           fprintf(fxml,"  </Grid>\n");
        }
        fprintf(fxml," </Grid>\n");
        fprintf(fxml,"</Domain>\n"); 
        fprintf(fxml,"</Xdmf>\n");
        fclose(fxml);
        
        delete [] local_data;
    }
}

void XDMFWriter::write_temporal_collection()
{
    if(processor_id == 0)
    {
        char filename[XDMF_FILE_SIZE];   
        sprintf(filename,"%s/%s_%d.xmf", this->dir.c_str(),this->basename.c_str(), n_processors);
        FILE * fxml = fopen(filename, "w");
        fprintf(fxml,"<?xml version=\"1.0\" ?>\n");
        fprintf(fxml,"<!-- DOCTYPE Xdmf SYSTEM \"Xdmf.dtd\" [] --> \n");
        fprintf(fxml,"<Xdmf xmlns:xi=\"http://www.w3.org/2001/XInclude\" Version=\"2.0\"> \n");
        fprintf(fxml,"<Domain Name=\"MeshTools\">\n");
        fprintf(fxml," <Grid GridType=\"Collection\" CollectionType=\"Temporal\">\n");
        for(int t=0; t <= this->n_timestep; t++)
            fprintf(fxml,"  <xi:include href=\"%s_%d_%05d.xmf\" xpointer=\"xpointer(//Xdmf/Domain/Grid)\" />\n", this->basename.c_str(), n_processors,t);
         fprintf(fxml," </Grid>\n");
        fprintf(fxml,"</Domain>\n"); 
        fprintf(fxml,"</Xdmf>\n");
        
        fclose(fxml);
    }
}


void XDMFWriter::get_variable_solution(ImplicitSystem* system, int ivar, std::vector<double> &solution)
{
    auto mesh        = system->get_mesh();
    auto dof_manager = system->get_equation_manager();
    auto n_dof       = dof_manager.get_n_dofs();

    double *solution_ptr = system->get_local_solution_array();

    for(int ino = 0; ino < mesh.get_n_nodes(); ino++)
        solution[ino] = solution_ptr[ino*n_dof+ivar];
 
    system->restore_local_solution_array(&solution_ptr);
    
}

