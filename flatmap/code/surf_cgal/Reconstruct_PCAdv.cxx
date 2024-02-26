#include <fstream>
#include <iostream>

#include <CGAL/Scale_space_surface_reconstruction_3.h>
#include <CGAL/Scale_space_reconstruction_3/Advancing_front_mesher.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/IO/read_xyz_points.h>
#include <CGAL/Timer.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;

typedef CGAL::Scale_space_surface_reconstruction_3<Kernel>                 Reconstruction;
typedef CGAL::Scale_space_reconstruction_3::Weighted_PCA_smoother<Kernel>  Smoother;
typedef CGAL::Scale_space_reconstruction_3::Advancing_front_mesher<Kernel> Mesher;

typedef Kernel::Point_3                                     Point;

typedef Reconstruction::Facet_const_iterator                Facet_iterator;

typedef CGAL::Timer Timer;

Reconstruction pts2surf(std::vector<Point> points, unsigned int sm_neigh, unsigned int sm_samp, double ms_max_facet_len, double ms_radius_ratio_bound, double ms_beta, unsigned int niter) {
    std::cerr << "Reconstruction ";
    Timer t;
    t.start();
    // Construct the mesh in a scale space.
    Reconstruction reconstruct (points.begin(), points.end());

    Smoother smoother(sm_neigh,sm_samp); // smoothing parameters
    reconstruct.increase_scale(niter,smoother);
    reconstruct.reconstruct_surface(Mesher(ms_max_facet_len,ms_radius_ratio_bound,ms_beta));

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
    double max_facet_len = 0.;
    double radius_ratio_bound = 5;
    double beta = 0.52;
    unsigned int niter = 4;

    if(argc > 3) neigh = atoi(argv[3]);
    if(argc > 4) samp = atoi(argv[4]);
    if(argc > 5) max_facet_len = atof(argv[5]);
    if(argc > 6) radius_ratio_bound = atof(argv[6]);
    if(argc > 7) beta = atof(argv[7]);
    if(argc > 8) niter = atoi(argv[8]);

    Reconstruction reconstruct = pts2surf(points,neigh,samp,max_facet_len,radius_ratio_bound,beta,niter);

    // Write the mesh
    std::ofstream out ((argc > 2) ? argv[2] : "out.off");
    out << reconstruct;
    std::cerr << "Done." << std::endl;

    return EXIT_SUCCESS;
}
