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
typedef std::vector<vertex_descriptor>                        Vd_vector;

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
        if(counter >= 4) { // too many border vertices
            std::cerr << "Error: Too many fixed vertices" << std::endl;
            return false;
        }
        fixed_vertices.push_back(vd);
    }

    if(fixed_vertices.size() < 4) {
        std::cerr << "Error: At least four unique vertices must be provided" << std::endl;
        return false;
    }

    return true;
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

    Vd_vector vda; // vertices as square corners
    { // set square corners
        if(argc > 4) { // read corners from file
            read_vertices(sm,argv[4],vda);
        }
#if 1
        else { // automatic corner selection
            vda.reserve(4);
            // collect halfedges on the longest border
            std::vector<halfedge_descriptor> hfs;
            for(halfedge_descriptor haf : CGAL::halfedges_around_face(bhd.first, sm))
                hfs.push_back(haf);

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
#endif
    }

    if(!vda.empty()) {
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
    }

    // The UV property map that holds the parameterized values
    UV_pmap uv_map = sm.add_property_map<vertex_descriptor, Point_2>("v:uv").first;

#if 0
    typedef SMP::Parametric_curve_border_arc_length_parameterizer_3<SurfaceMesh>      Border_parameterizer;
    typedef SMP::Discrete_authalic_parameterizer_3<SurfaceMesh, Border_parameterizer> Parameterizer;
    Border_parameterizer *border_param = new Border_parameterizer(
//        [](double t) { return 0.5 + 0.5 * std::cos(-2 * CGAL_PI * t); },
//        [](double t) { return 0.5 + 0.5 * std::sin(-2 * CGAL_PI * t); }
//        [](double t) { return 2 * std::cos(-2 * CGAL_PI * t); },
//        [](double t) { return 1 * std::sin(-2 * CGAL_PI * t); }
            demi_cardio_x,
            demi_cardio_y
        );
#else
    typedef SMP::Convex_border_arc_length_parameterizer_3<SurfaceMesh>                Border_parameterizer;
    typedef SMP::Discrete_authalic_parameterizer_3<SurfaceMesh, Border_parameterizer> Parameterizer;
    Border_parameterizer *border_param;

    std::vector<std::pair<double,double>> cpoly = {
{0.0518519,0.659259},
{0.0518519,0.607407},
{0.0592593,0.585185},
{0.0962963,0.488889},
{0.125926,0.422222},
{0.133333,0.407407},
{0.17037,0.340741},
{0.281481,0.222222},
{0.355556,0.148148},
{0.377778,0.133333},
{0.392593,0.125926},
{0.414815,0.118519},
{0.6,0.0962963},
{0.622222,0.0962963},
{0.644444,0.103704},
{0.681481,0.118519},
{0.696296,0.125926},
{0.718519,0.140741},
{0.822222,0.222222},
{0.82963,0.22963},
{0.874074,0.288889},
{0.888889,0.311111},
{0.948148,0.407407},
{0.955556,0.422222},
{0.97037,0.459259},
{0.977778,0.496296},
{0.992593,0.607407},
{0.992593,0.644444},
{0.903704,0.740741},
{0.807407,0.844444},
{0.785185,0.866667},
{0.355556,0.918519},
{0.333333,0.918519},
{0.311111,0.903704},
{0.207407,0.814815},
//        {0,0}, {1,0}, {1,1}, {0,1}
//        {0,0}, {2,0}, {2,1}, {0,1}
//        {0,0}, {1,0}, {0.5,1}, {0.7,0.5}
//        {sin(0 * 2 * M_PI / 5),cos(0 * 2 * M_PI / 5)},
//        {sin(1 * 2 * M_PI / 5),cos(1 * 2 * M_PI / 5)},
//        {sin(2 * 2 * M_PI / 5),cos(2 * 2 * M_PI / 5)},
//        {sin(3 * 2 * M_PI / 5),cos(3 * 2 * M_PI / 5)},
//        {sin(4 * 2 * M_PI / 5),cos(4 * 2 * M_PI / 5)},
    };

    if(!vda.empty())
        border_param = new Border_parameterizer(cpoly, vda);
    else
        border_param = new Border_parameterizer(cpoly);

#endif
    Parameterizer parameterizer(*border_param);

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

    delete border_param;

    std::ofstream out((argc > 2) ? argv[2] : "result.off");
    uvmap_off(out,sm,uv_map);
    //SMP::IO::output_uvmap_to_off(sm, bhd.first, uv_map, out);

    return EXIT_SUCCESS;
}
