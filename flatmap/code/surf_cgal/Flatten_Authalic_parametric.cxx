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

typedef boost::graph_traits<SurfaceMesh>::halfedge_descriptor halfedge_descriptor;
typedef boost::graph_traits<SurfaceMesh>::vertex_descriptor   vertex_descriptor;
typedef boost::graph_traits<SurfaceMesh>::face_descriptor     face_descriptor;

typedef boost::graph_traits<SurfaceMesh>::vertex_iterator     vertex_iterator;

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


double demi_cardio_x(double t) { // parameter in [0,1[
    const double a = 1.0; // radius
    const double L = 8.0 * a + 4.0 * a; // total length
    if(t < 2.0/3.0) {
        const double s = 1.5 * t; // in [0,1[
        const double phi = 2.0 * std::acos(1 - s); // angle in [0,pi[
        return 2 * a * (1 - std::cos(phi)) * std::cos(phi);
    } else {
        const double s = 3.0 * t - 2.0; // in [0,1[
        return 4 * a * (s - 1.0);
    }
}

double demi_cardio_y(double t) { // parameter in [0,1[
    const double a = 1.0; // radius
    const double L = 8.0 * a + 4.0 * a; // total length
    if(t < 2.0/3.0) {
        const double s = 1.5 * t; // in [0,1[
        const double phi = 2.0 * std::acos(1 - s); // angle in [0,pi[
        return 2 * a * (1 - std::cos(phi)) * std::sin(phi);
    } else return 0.0;
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

    int offset = 0;
    if(argc > 3)
        offset = atoi(argv[3]);

    // The UV property map that holds the parameterized values
    UV_pmap uv_map = sm.add_property_map<vertex_descriptor, Point_2>("v:uv").first;

    typedef SMP::Parametric_curve_border_arc_length_parameterizer_3<SurfaceMesh>      Border_parameterizer;
    typedef SMP::Discrete_authalic_parameterizer_3<SurfaceMesh, Border_parameterizer> Parameterizer;
    Border_parameterizer border_param(
//        [](double t) { return 0.5 + 0.5 * std::cos(-2 * CGAL_PI * t); },
//        [](double t) { return 0.5 + 0.5 * std::sin(-2 * CGAL_PI * t); }
            demi_cardio_x,
            demi_cardio_y
        );
    Parameterizer parameterizer(border_param);

    // shift starting vertex
    halfedge_descriptor start_hd = bhd.first;
    for(int i = 0; i < offset; ++i)
        start_hd = next(start_hd, sm);

    SMP::Error_code err = SMP::parameterize(sm, parameterizer, start_hd, uv_map);
    if(err != SMP::OK) {
        std::cerr << "Error: " << SMP::get_error_message(err) << std::endl;
        return EXIT_FAILURE;
    }

    std::cerr << "Flattening succeded!" << std::endl;

    std::ofstream out((argc > 2) ? argv[2] : "result.off");
    uvmap_off(out,sm,uv_map);

    return EXIT_SUCCESS;
}
