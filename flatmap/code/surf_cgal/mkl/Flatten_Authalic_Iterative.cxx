#define EIGEN_USE_MKL_ALL 1
#define EIGEN_USE_MKL 1
#define EIGEN_USING_STD(x) EIGEN_USING_STD_MATH(x)
#include <Eigen/PardisoSupport>

#include "Flatten_common.hpp"

#include <CGAL/Surface_mesh_parameterization/Error_code.h>
#include <CGAL/Surface_mesh_parameterization/parameterize.h>
#include <CGAL/Surface_mesh_parameterization/Iterative_authalic_parameterizer_3.h>
#include <CGAL/Surface_mesh_parameterization/Square_border_parameterizer_3.h>

#include <CGAL/Polygon_mesh_processing/measure.h>

#include <cassert>
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

    unsigned int iter = 10;
    if(argc > 3)
        iter = boost::lexical_cast<unsigned int>(argv[3]);
    if(iter == 0) {
        std::cerr << "Error: zero iterations requested" << std::endl;
        return EXIT_FAILURE;
    }

    int offset = 0;
    if(argc > 4)
        offset = boost::lexical_cast<int>(argv[4]);

    Vd_vector vda; // vertices of square corners
    if(argc > 5) { // read corners from file
        if(!read_vertices(sm,argv[5],vda))
            return EXIT_FAILURE;
        if(vda.size() != 4) {
            std::cerr << "Error: expected exactly 4 fixed vertices" << std::endl;
            return EXIT_FAILURE;
        }
    } else { // automatic corner selection
        // collect halfedges on the longest border
        std::vector<halfedge_descriptor> hfs;
        for(halfedge_descriptor haf : CGAL::halfedges_around_face(bhd.first, sm))
            hfs.push_back(haf);

        std::cerr << "Have " << hfs.size()
                  << " vertices along longest border (offset = "
                  << offset << ")" << std::endl;

        // choose square corners equidistant along longest border,
        // starting from point at @offset and cycling around
        vda.reserve(4);
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

    typedef SMP::Square_border_arc_length_parameterizer_3<SurfaceMesh>                    Border_parameterizer;
    typedef SMP::Iterative_authalic_parameterizer_3<SurfaceMesh, Border_parameterizer,
                CGAL::Eigen_solver_traits<Eigen::PardisoLU<Eigen::SparseMatrix<double>>>> Parameterizer;
    Border_parameterizer border_param(vda[0], vda[1], vda[2], vda[3]); // square corners
    Parameterizer parameterizer(border_param);

    std::cerr << "Running for " << iter << " iterations" << std::endl;
    SMP::Error_code err = parameterizer.parameterize(sm, bhd.first, uv_map, iter);
    if(err != SMP::OK) {
        std::cerr << "Error: " << SMP::get_error_message(err) << std::endl;
        return EXIT_FAILURE;
    }

    std::cerr << "Flattening succeded!" << std::endl;

    std::ofstream out((argc > 2) ? argv[2] : "result.off");
    uvmap_off(out,sm,uv_map);

    return EXIT_SUCCESS;
}
