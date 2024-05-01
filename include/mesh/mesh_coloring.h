#ifndef MESH_COLORING_H
#define MESH_COLORING_H

class Mesh;

class MeshColoring{
    friend class Mesh;

    public:
        MeshColoring() = delete ;

        static void apply_coloring(Mesh &mesh);

        static void apply_blocked_coloring(Mesh &mesh, unsigned int block_size = 4096);

    // private:

    //     void apply_coloring_aux(Mesh& mesh, std::vector<unsigned int> &elements_color,std::vector<unsigned int>& sort);

    //     void apply_coloring_blocked_aux(Mesh& mesh, std::vector<unsigned int> &elements_color,std::vector<unsigned int>& sort);

    //     void update_mesh_arrays(Mesh& mesh, 
    //                     std::vector<unsigned int>& elememts_color,
    //                     std::vector<unsigned int>& new_conn  , 
    //                     std::vector<unsigned int>& new_offset, 
    //                     std::vector<unsigned short>& new_type, 
    //                     std::vector<int>& new_tag);

    //     void reorder_elements(Mesh& mesh, std::vector<unsigned int>& sort, 
    //                     std::vector<unsigned int>& new_conn, 
    //                     std::vector<unsigned int>& new_offset, 
    //                     std::vector<unsigned short>& new_type,
    //                     std::vector<int>& new_tag);
};


#endif /* MESH_COLORING_H */
