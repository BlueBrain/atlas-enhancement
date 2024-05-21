#include "Flatten_common.hpp"

#include <CGAL/Surface_mesh_parameterization/Error_code.h>
#include <CGAL/Surface_mesh_parameterization/parameterize.h>
#include <CGAL/Surface_mesh_parameterization/Discrete_authalic_parameterizer_3.h>
#include <CGAL/Surface_mesh_parameterization/Convex_border_parameterizer_3.h>

#include <CGAL/Polygon_mesh_processing/measure.h>

#include <boost/lexical_cast.hpp>

namespace SMP = CGAL::Surface_mesh_parameterization;

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
        offset = boost::lexical_cast<int>(argv[4]);

    Vd_vector vda; // vertices of polygon corners
    if(argc > 5) { // read corners from file
        if(!read_vertices(sm,argv[5],vda))
            return EXIT_FAILURE;
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
        if(offset < 0)
            offset = halfedges_around_face(start_hd, sm).size() + offset;
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
