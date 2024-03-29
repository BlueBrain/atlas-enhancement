#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Search_traits_3.h>
#include <CGAL/Search_traits_adapter.h>
#include <CGAL/Orthogonal_k_neighbor_search.h>
#include <CGAL/Surface_mesh.h>

#include <fstream>

typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
typedef Kernel::Point_3 Point_3;
typedef CGAL::Surface_mesh<Point_3> Mesh;
typedef boost::graph_traits<Mesh>::vertex_descriptor Point;
typedef boost::graph_traits<Mesh>::vertices_size_type size_type;
typedef boost::property_map<Mesh,CGAL::vertex_point_t>::type Vertex_point_pmap;
typedef CGAL::Search_traits_3<Kernel> Traits_base;
typedef CGAL::Search_traits_adapter<Point,Vertex_point_pmap,Traits_base> Traits;
typedef CGAL::Orthogonal_k_neighbor_search<Traits> K_neighbor_search;
typedef K_neighbor_search::Tree Tree;
typedef Tree::Splitter Splitter;
typedef K_neighbor_search::Distance Distance;

int main(int argc, char* argv[]) {
    // load mesh
    std::ifstream inmesh(argv[1]);
    Mesh mesh;
    inmesh >> mesh;

    // load points
    std::ifstream inpts(argv[2]);
    std::vector<Point_3> points;
    std::copy(std::istream_iterator<Point_3>(inpts),
            std::istream_iterator<Point_3>(),
            std::back_inserter(points));
    std::cerr << "Read " << points.size() << " points" << std::endl;

    Vertex_point_pmap vppmap = get(CGAL::vertex_point,mesh);

    Tree tree(
            vertices(mesh).begin(),
            vertices(mesh).end(),
            Splitter(),
            Traits(vppmap)
            );

    Distance tr_dist(vppmap);

    for(std::vector<Point_3>::iterator pit = points.begin(); pit != points.end(); ++pit) {
        K_neighbor_search search(tree,*pit,1,0,true,tr_dist);
        K_neighbor_search::iterator it = search.begin();
        std::cout << 
            it->first << " "
            << vppmap[it->first] << " "
            << tr_dist.inverse_of_transformed_distance(it->second) << std::endl;
    }

    return 0;
}
