#include <fstream>
#include <iostream>

#include <CGAL/Scale_space_surface_reconstruction_3.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/IO/read_xyz_points.h>
#include <CGAL/Timer.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;

typedef CGAL::Scale_space_surface_reconstruction_3<Kernel>                 Reconstruction;
typedef CGAL::Scale_space_reconstruction_3::Weighted_PCA_smoother<Kernel>  Smoother;
typedef CGAL::Scale_space_reconstruction_3::Alpha_shape_mesher<Kernel>     Mesher;

typedef Kernel::Point_3                                     Point;

typedef Reconstruction::Facet_const_iterator                Facet_iterator;
typedef Mesher::Facet_const_iterator                        Mesher_iterator;

typedef CGAL::Timer Timer;

Reconstruction pts2surf(std::vector<Point> points, unsigned int neigh, unsigned int samp, unsigned int niter) {
    std::cerr << "Reconstruction ";
    Timer t;
    t.start();
    // Construct the mesh in a scale space.
    Reconstruction reconstruct (points.begin(), points.end());

    Smoother smoother(neigh,samp); // smoothing parameters
    reconstruct.increase_scale(niter,smoother);
    Mesher mesher(smoother.squared_radius(),
            false, // do not separate shells
            true // force manifold output
            );
    reconstruct.reconstruct_surface(mesher);

    std::cerr << "done in " << t.time() << " sec." << std::endl;

    return reconstruct;
}

int main(int argc, char** argv)
{
    if (argc < 2){
        std::cerr << "Error, no input file provided\n";
        return 1;
    }

    std::vector<Point> points;

    // Read the data.
    std::ifstream in(argv[1]);
    std::cerr << "Reading " << std::flush;
    if( !in || !CGAL::IO::read_XYZ( in, std::back_inserter( points ) ) ) {
        std::cerr << "Error: cannot read file" << std::endl;
        return EXIT_FAILURE;
    }
    std::cerr << "done: " << points.size() << " points." << std::endl;

    unsigned int neigh = 12;
    unsigned int samp = 300;
    unsigned int niter = 4;

    if(argc > 3) neigh = boost::lexical_cast<unsigned int>(argv[3]);
    if(argc > 4) samp = boost::lexical_cast<unsigned int>(argv[4]);
    if(argc > 5) niter = boost::lexical_cast<unsigned int>(argv[5]);

    Reconstruction reconstruct = pts2surf(points,neigh,samp,niter);

    // Write the mesh
    const char *outfile = (argc > 2) ? argv[2] : "out.off";
    std::ofstream out (outfile);
    out << reconstruct;
    std::cerr << "Done saving as " << outfile << std::endl;

    return EXIT_SUCCESS;
}
