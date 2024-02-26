#define EIGEN_USE_MKL_ALL 1
#define EIGEN_USE_MKL 1
#define EIGEN_USING_STD(x) EIGEN_USING_STD_MATH(x)
#include <Eigen/PardisoSupport>

#include <CGAL/Simple_cartesian.h>
#include <CGAL/Surface_mesh.h>

#include <CGAL/Surface_mesh_parameterization/IO/File_off.h>
#include <CGAL/Surface_mesh_parameterization/Error_code.h>
#include <CGAL/Surface_mesh_parameterization/parameterize.h>
#include <CGAL/Surface_mesh_parameterization/Iterative_authalic_parameterizer_3.h>
#include <CGAL/Surface_mesh_parameterization/Square_border_parameterizer_3.h>

#include <CGAL/Polygon_mesh_processing/measure.h>

#include <cassert>
#include <iostream>
#include <fstream>

typedef CGAL::Simple_cartesian<double>      Kernel;
typedef Kernel::Point_2                     Point_2;
typedef Kernel::Point_3                     Point_3;
typedef CGAL::Surface_mesh<Kernel::Point_3> SurfaceMesh;

typedef boost::graph_traits<SurfaceMesh>::halfedge_descriptor halfedge_descriptor;
typedef boost::graph_traits<SurfaceMesh>::vertex_descriptor   vertex_descriptor;
typedef boost::graph_traits<SurfaceMesh>::face_descriptor     face_descriptor;

typedef boost::graph_traits<SurfaceMesh>::vertex_iterator     vertex_iterator;
typedef boost::array<vertex_descriptor, 4>                    Vd_array;

typedef SurfaceMesh::Property_map<vertex_descriptor, Point_2>  UV_pmap;

namespace SMP = CGAL::Surface_mesh_parameterization;

// https://github.com/CGAL/cgal/issues/2994
bool uvmap_off(std::ofstream &out, SurfaceMesh & sm, UV_pmap uv_map)  {
    std::size_t vertices_counter = 0, faces_counter = 0;
    typedef std::unordered_map<vertex_descriptor, std::size_t> Vertex_index_map;
    Vertex_index_map vium;
    boost::associative_property_map<Vertex_index_map> vimap(vium);

    out << "OFF\n" ;
    out << sm.number_of_vertices() <<" "<< sm.number_of_faces() << " 0\n";
    boost::graph_traits<SurfaceMesh>::vertex_iterator vit, vend;
    boost::tie(vit, vend) = vertices(sm);
    while(vit!=vend)
    {
        vertex_descriptor vd = *vit++;
        out << get(uv_map, vd) << " 0\n";
        put(vimap, vd, vertices_counter++);
    }

    BOOST_FOREACH(face_descriptor fd, faces(sm)){
        halfedge_descriptor hd = halfedge(fd, sm);
        out << "3";
        BOOST_FOREACH(vertex_descriptor vd, vertices_around_face(hd, sm)){
            out << " " << get(vimap, vd);
        }
        out << '\n';
        faces_counter++;
    }
    if(vertices_counter != sm.number_of_vertices())
        return 0;
    else if(faces_counter != sm.number_of_faces())
        return 0;
    else
        return 1;
}

bool read_vertices(const SurfaceMesh& mesh, const char* filename, Vd_array& fixed_vertices) {
    std::string str = filename;
    if( (str.length()) < 14 || (str.substr(str.length() - 14) != ".selection.txt") ) {
        std::cerr << "Error: vertices must be given by a *.selection.txt file" << std::endl;
        return false;
    }

    std::ifstream in(filename);
    std::string line;
    if(!std::getline(in, line)) {
        std::cerr << "Error: could not read input file: " << filename << std::endl;
        return false;
    }

    // The selection file is a list of integers, so we must build a correspondence
    // between vertices and the integers.
    std::vector<vertex_descriptor> vds;
    vds.reserve(num_vertices(mesh));
    vertex_iterator vi = vertices(mesh).begin(), vi_end = vertices(mesh).end();
    CGAL_For_all(vi, vi_end) {
        vds.push_back(*vi);
    }

    // Get the first line and read the fixed vertex indices
    std::size_t counter = 0;
    std::istringstream point_line(line);
    std::size_t s;
    std::unordered_set<std::size_t> indices;

    while(point_line >> s) {
        if(s >= vds.size())
        {
            std::cerr << "Error: vertex index " << s << " out of bounds" << std::endl;
            return false;
        }
        vertex_descriptor vd = vds[s];
        if(!is_border(vd, mesh)) { // must be on the border
            std::cerr << "Error: vertex[" << s << "] " << vd << " is not on the border of the mesh" << std::endl;
            return false;
        }
        if(counter >= 4) { // too many border vertices
            std::cerr << "Error: only four vertices may be provided" << std::endl;
            return false;
        }
        fixed_vertices[counter++] = vd;
        indices.insert(s);
    }

    if(indices.size() < 4) {
        std::cerr << "Error: at least four unique vertices must be provided" << std::endl;
        return false;
    }

    return true;
}

int main(int argc, char** argv)
{
    std::ifstream in(argv[1]);
    if(!in) {
        std::cerr << "Problem loading the input data" << std::endl;
        return EXIT_FAILURE;
    }

    SurfaceMesh sm;
    in >> sm; // load mesh from file

    std::cerr << "Loaded mesh with area: " << CGAL::Polygon_mesh_processing::area(sm) << std::endl;

    // a halfedge on the border
    std::pair<halfedge_descriptor,double> bhd = CGAL::Polygon_mesh_processing::longest_border(sm);
    std::cerr << "Found longest border with length: " << bhd.second << std::endl;

    Vd_array vda; // vertices as square corners
    int offset = 0; if(argc > 4) offset = atoi(argv[4]);
    { // set square corners
        if(argc > 5) { // read corners from file
            if(!read_vertices(sm,argv[5],vda))
                return EXIT_FAILURE;
        } else { // automatic corner selection
            // collect halfedges on the longest border
            std::vector<halfedge_descriptor> hfs;
            for(halfedge_descriptor haf : CGAL::halfedges_around_face(bhd.first, sm))
                hfs.push_back(haf);

            std::cerr << "Have " << hfs.size()
                      << " vertices along longest border (offset = " << offset << ")" << std::endl;

            // choose square corners equidistant along longest border,
            // starting from point at @offset and cycling around
            double len = 0;
            for(int i=0;i<hfs.size();i++) {
                int index = (i + offset) % hfs.size();
                double plen = len / bhd.second; // proportion of total length
                vertex_descriptor src = source(hfs[index], sm);
                if(plen <= 0.00) vda[0] = src;
                if(plen <= 0.25) vda[1] = src;
                if(plen <= 0.50) vda[2] = src;
                if(plen <= 0.75) vda[3] = src;
                len += CGAL::Polygon_mesh_processing::edge_length(hfs[index], sm);
            }
        }
    }

    double peri[4]; // perimeter of square
    { // Compute perimeter length between corners
        // collect halfedges on the longest border
        std::vector<halfedge_descriptor> hfs;
        for(halfedge_descriptor haf : CGAL::halfedges_around_face(bhd.first, sm))
            hfs.push_back(haf);

        double len = 0;
        for(int i=0;i<hfs.size();i++) {
            vertex_descriptor src = source(hfs[i], sm);
            if(src == vda[0]) peri[0] = len;
            else if(src == vda[1]) peri[1] = len - peri[0];
            else if(src == vda[2]) peri[2] = len - peri[1] - peri[0];
            else if(src == vda[3]) peri[3] = len - peri[2] - peri[1] - peri[0];
            len += CGAL::Polygon_mesh_processing::edge_length(hfs[i], sm);
        }
        peri[0] = len - peri[3] - peri[2] - peri[1];
    }
    assert(peri[0] + peri[1] + peri[2] + peri[3] == bhd.second);

    std::cerr << "Using square border with corners: " << std::endl;
    for(int i=0;i<4;i++)
        std::cerr << vda[i] << ": " << sm.point(vda[i]) << " :: length = " << peri[i] << std::endl;

    // The UV property map that holds the parameterized values
    UV_pmap uv_map = sm.add_property_map<vertex_descriptor, Point_2>("v:uv").first;

    //typedef SMP::Square_border_uniform_parameterizer_3<SurfaceMesh>              Border_parameterizer;
    typedef SMP::Square_border_arc_length_parameterizer_3<SurfaceMesh>              Border_parameterizer;
    Border_parameterizer border_param(vda[0], vda[1], vda[2], vda[3]); // square corners
    typedef SMP::Iterative_authalic_parameterizer_3<SurfaceMesh, Border_parameterizer
                ,CGAL::Eigen_solver_traits<
                    Eigen::PardisoLU<Eigen::SparseMatrix<double>>
                    >
                    > Parameterizer;
    Parameterizer parameterizer(border_param);

    int iter = 10; if(argc > 3) iter = atoi(argv[3]);
    std::cerr << "Running for " << iter << " iterations" << std::endl;
    SMP::Error_code err = parameterizer.parameterize(sm, bhd.first, uv_map, iter);
    if(err != SMP::OK) {
        std::cerr << "Error: " << SMP::get_error_message(err) << std::endl;
        return EXIT_FAILURE;
    }

    std::cerr << "Flattening succeded!" << std::endl;

    std::ofstream out((argc > 2) ? argv[2] : "result.off");
    uvmap_off(out,sm,uv_map); // keep order of vertices
    //SMP::IO::output_uvmap_to_off(sm, bhd.first, uv_map, out);

    return EXIT_SUCCESS;
}
