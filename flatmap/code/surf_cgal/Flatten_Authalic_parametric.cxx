#include "Flatten_common.hpp"

#include <CGAL/Surface_mesh_parameterization/Error_code.h>
#include <CGAL/Surface_mesh_parameterization/parameterize.h>
#include <CGAL/Surface_mesh_parameterization/Discrete_authalic_parameterizer_3.h>
#include <CGAL/Surface_mesh_parameterization/Parametric_curve_border_parameterizer_3.h>

#include <CGAL/Polygon_mesh_processing/measure.h>

#include <boost/lexical_cast.hpp>

namespace SMP = CGAL::Surface_mesh_parameterization;

double demi_cardioid_x(double t) { // parameter in [0,1[
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

double demi_cardioid_y(double t) { // parameter in [0,1[
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
        offset = boost::lexical_cast<int>(argv[3]);

    // The UV property map that holds the parameterized values
    UV_pmap uv_map = sm.add_property_map<vertex_descriptor, Point_2>("v:uv").first;

    typedef SMP::Parametric_curve_border_arc_length_parameterizer_3<SurfaceMesh>      Border_parameterizer;
    typedef SMP::Discrete_authalic_parameterizer_3<SurfaceMesh, Border_parameterizer> Parameterizer;
    Border_parameterizer *border_param;

    halfedge_descriptor start_hd = bhd.first;

    // shift starting vertex
    if(offset < 0)
        offset = halfedges_around_face(start_hd, sm).size() + offset;
    for(int i = 0; i < offset; ++i)
        start_hd = next(start_hd, sm);

    if(1) { // demi-cardioid
        border_param = new Border_parameterizer(
            demi_cardioid_x,
            demi_cardioid_y
        );
    } else if(0) { // circle
        border_param = new Border_parameterizer(
            [](double t) { return 0.5 + 0.5 * std::cos(-2 * CGAL_PI * t); },
            [](double t) { return 0.5 + 0.5 * std::sin(-2 * CGAL_PI * t); }
        );
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
