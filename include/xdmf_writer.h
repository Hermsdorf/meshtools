#ifndef XDMF_WRITER_H__
#define XDMF_WRITER_H__

#include "implicit_system.h"
#include "transient_implicit_system.h"


class XDMFWriter
{
    public:
        XDMFWriter(std::string filename);
        void set_dir_path(std::string path);
        void set_file_id(int n_time_file) {n_timestep = n_time_file; } 
        int  get_file_id() { return n_timestep; }
        int write(ImplicitSystem * system, double time);
        int write(ImplicitSystem * system);
        virtual ~XDMFWriter();

    private:
        void write_spatial_collection(ImplicitSystem* system,double time);
        void write_temporal_collection();
        void get_variable_solution(ImplicitSystem* system, int ivar, std::vector<double> &solution);

        std::string          basename;
        std::string          dir;
        int                  n_timestep;
        int                  n_local_nodes;
        int                  n_local_elem;
        int                  processor_id;
        int                  n_processors;
        bool                 using_compression;
};


#endif /* XDMF_WRITER_H__ */
