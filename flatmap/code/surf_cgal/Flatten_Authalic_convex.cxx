#include <CGAL/Simple_cartesian.h>
#include <CGAL/Surface_mesh.h>

#include <CGAL/Surface_mesh_parameterization/IO/File_off.h>
#include <CGAL/Surface_mesh_parameterization/Error_code.h>
#include <CGAL/Surface_mesh_parameterization/parameterize.h>
#include <CGAL/Surface_mesh_parameterization/Discrete_authalic_parameterizer_3.h>
#include <CGAL/Surface_mesh_parameterization/Convex_border_parameterizer_3.h>
#include <CGAL/Surface_mesh_parameterization/Parametric_curve_border_parameterizer_3.h>

#include <CGAL/Polygon_mesh_processing/measure.h>

#include <cassert>
#include <iostream>
#include <fstream>

typedef CGAL::Simple_cartesian<double>      Kernel;
typedef Kernel::Point_2                     Point_2;
typedef Kernel::Point_3                     Point_3;
typedef CGAL::Surface_mesh<Kernel::Point_3> SurfaceMesh;

typedef boost::graph_traits<SurfaceMesh>::halfedge_descriptor  halfedge_descriptor;
typedef boost::graph_traits<SurfaceMesh>::vertex_descriptor    vertex_descriptor;
typedef boost::graph_traits<SurfaceMesh>::face_descriptor      face_descriptor;

typedef boost::graph_traits<SurfaceMesh>::vertex_iterator      vertex_iterator;
typedef std::vector<vertex_descriptor>                         Vd_vector;
typedef std::vector<std::pair<double,double>>                  Pt_vector;

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

bool read_border_points(const char* filename, Pt_vector& points) {
    std::ifstream in(filename);
    if(!in.is_open()) {
        std::cerr << "Error: could not read input file: " << filename << std::endl;
        return false;
    }

    std::pair<double,double> pt;
    while(in >> pt.first >> pt.second)
        points.push_back(pt);

    return true;
}

bool read_vertices(const SurfaceMesh& mesh, const char* filename, Vd_vector& fixed_vertices) {
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
    Vd_vector vds;
    vds.reserve(num_vertices(mesh));
    vertex_iterator vi = vertices(mesh).begin(), vi_end = vertices(mesh).end();
    CGAL_For_all(vi, vi_end) {
        vds.push_back(*vi);
    }

    // Get the first line and read the fixed vertex indices
    std::size_t counter = 0;
    std::istringstream point_line(line);
    std::size_t s;

    while(point_line >> s) {
        if(s >= vds.size())
        {
            std::cerr << "Error: Vertex index too large" << std::endl;
            return false;
        }
        vertex_descriptor vd = vds[s];
        if(!is_border(vd, mesh)) { // must be on the border
            std::cerr << "Error: Vertex is not on the mesh border" << std::endl;
            return false;
        }
        fixed_vertices.push_back(vd);
    }

    if(fixed_vertices.size() < 3) {
        std::cerr << "Error: At least 3 unique vertices must be provided" << std::endl;
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

    // read border points
    Pt_vector points;
    if(!read_border_points(argv[3], points)) {
        std::cerr << "Error: failed reading border points" << std::endl;
        return EXIT_FAILURE;
    }
    std::cerr << "Have " << points.size() << " border points" << std::endl;

    int offset = 0;
    if(argc > 4)
        offset = atoi(argv[4]);

    Vd_vector vda; // vertices of polygon corners
    if(argc > 5) { // read corners from file
        if(!read_vertices(sm,argv[5],vda)) {
            std::cerr << "Error: failed reading fixed corners" << std::endl;
            return EXIT_FAILURE;
        }
    }

    // The UV property map that holds the parameterized values
    UV_pmap uv_map = sm.add_property_map<vertex_descriptor, Point_2>("v:uv").first;

    typedef SMP::Convex_border_arc_length_parameterizer_3<SurfaceMesh>                Border_parameterizer;
    typedef SMP::Discrete_authalic_parameterizer_3<SurfaceMesh, Border_parameterizer> Parameterizer;
    Border_parameterizer *border_param;

    halfedge_descriptor start_hd = bhd.first;

    if(!vda.empty()) { // explicit corners
        border_param = new Border_parameterizer(points, vda);
    } else { // automatic corners
        border_param = new Border_parameterizer(points);

        // shift starting vertex
        for(int i = 0; i < offset; ++i)
            start_hd = next(start_hd, sm);
    }

    Parameterizer parameterizer(*border_param);

    SMP::Error_code err = SMP::parameterize(sm, parameterizer, start_hd, uv_map);
    if(err != SMP::OK) {
        std::cerr << "Error: " << SMP::get_error_message(err) << std::endl;
        return EXIT_FAILURE;
    }

    std::cerr << "Flattening succeded!" << std::endl;

    delete border_param;

    std::ofstream out((argc > 2) ? argv[2] : "result.off");
    uvmap_off(out,sm,uv_map);

    return EXIT_SUCCESS;
}
