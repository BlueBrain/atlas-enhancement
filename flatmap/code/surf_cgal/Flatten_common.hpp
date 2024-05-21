#include <CGAL/Simple_cartesian.h>
#include <CGAL/Surface_mesh.h>

typedef CGAL::Simple_cartesian<double>                         Kernel;
typedef Kernel::Point_2                                        Point_2;
typedef Kernel::Point_3                                        Point_3;
typedef CGAL::Surface_mesh<Kernel::Point_3>                    SurfaceMesh;

typedef boost::graph_traits<SurfaceMesh>::halfedge_descriptor  halfedge_descriptor;
typedef boost::graph_traits<SurfaceMesh>::vertex_descriptor    vertex_descriptor;
typedef boost::graph_traits<SurfaceMesh>::face_descriptor      face_descriptor;
typedef boost::graph_traits<SurfaceMesh>::vertex_iterator      vertex_iterator;

typedef std::vector<vertex_descriptor>                         Vd_vector;
typedef std::vector<std::pair<double,double>>                  Pt_vector;

typedef SurfaceMesh::Property_map<vertex_descriptor, Point_2>  UV_pmap;

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
        if(s >= vds.size()) {
            std::cerr << "Error: vertex index " << s << " out of bounds" << std::endl;
            return false;
        }
        vertex_descriptor vd = vds[s];
        if(!is_border(vd, mesh)) { // must be on the border
            std::cerr << "Error: vertex[" << s << "] " << vd << " is not on the mesh border" << std::endl;
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
