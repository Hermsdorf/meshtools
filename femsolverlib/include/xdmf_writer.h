#ifndef XDMF_WRITER_H__
#define XDMF_WRITER_H__

#include "implicit_system.h"


class XDMFWriter
{
    public:
        XDMFWriter(const ImplicitSystem& system);
        void set_file_name(std::string filename);
        void set_dir_path(std::string path);
        void set_file_id(int n_time_file) {n_timestep = n_time_file; } 
        int  get_file_id() { return n_timestep; }
        int write(double time);
        virtual ~XDMFWriter();

    private:
        void write_spatial_collection(double time);
        void write_temporal_collection();

        std::string          basename;
        std::string          dir;
        int                  n_timestep;
        int                  n_local_nodes;
        int                  n_local_elem;
        const ImplicitSystem & system;
        int                  processor_id;
        int                  n_processors;
        bool                 using_compression;
};


#endif /* XDMF_WRITER_H__ */
