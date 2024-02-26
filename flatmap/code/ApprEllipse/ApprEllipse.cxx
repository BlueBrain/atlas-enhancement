#include <iostream>
#include <iomanip>
#include <fstream>
#include <random>

#include <Mathematics/Matrix3x3.h>
#include <Mathematics/Rotation.h>
#include <Mathematics/ContOrientedBox3.h>
#include <Mathematics/ApprOrthogonalPlane3.h>
#include <Mathematics/DistPointHyperellipsoid.h>

#include <nlopt.hpp>

using namespace gte;

Vector3<double> basis[3];

double ellipsoid_cost(const std::vector<double> &x, std::vector<double> &grad, void *data) {
    DCPPoint3Ellipsoid3<double> query;
    std::vector<Vector3<double>> pos = *reinterpret_cast<std::vector<Vector3<double>>*>(data);

    Ellipsoid3<double> el;
    el.center = { x[0], x[1], x[2] };
    el.extent = { x[3], x[4], x[5] };
    el.axis[0] = basis[0];
    el.axis[1] = basis[1];
    el.axis[2] = basis[2];

    // Compute energy
    double d = 0.0;
#pragma omp parallel for reduction(+:d)
    for(size_t i=0;i<pos.size();i++) {
        Vector3<double> point = pos[i];
        auto result = query(point,el);
        d += result.distance; // distance to ellipsoid
    }

#ifdef INSPECT
    std::cerr << "# Ellipsoid cost = " << d << std::endl;
#if INSPECT > 1
    std::cerr << "# Ellipsoid center "
        << std::setprecision(12) << x[0] << " "
        << std::setprecision(12) << x[1] << " "
        << std::setprecision(12) << x[2] << std::endl;
    std::cerr << "# Ellipsoid extents "
        << std::setprecision(12) << x[3] << " "
        << std::setprecision(12) << x[4] << " "
        << std::setprecision(12) << x[5] << std::endl;
#endif
#endif

    return d;
}

int main(int argc, char **argv) {
    std::vector<Vector3<double>> pos;
    std::vector<Vector3<double>> pos2;

    { // Read points to fit ellipse from
        std::ifstream infile(argv[1]);
        double a,b,c;
        while(infile >> a >> b >> c)
        {
            Vector3<double> data{ a, b, c };
            pos.push_back(data);
        }
        infile.close();
        std::cerr << "# Read " << pos.size() << " points from " << argv[1] << std::endl;
    }

    { // Read additional points to project to ellipse
        std::ifstream infile(argv[2]);
        double a,b,c;
        while(infile >> a >> b >> c)
        {
            Vector3<double> data{ a, b, c };
            pos2.push_back(data);
        }
        infile.close();
        std::cerr << "# Read " << pos2.size() << " points from " << argv[2] << std::endl;
    }

    // Fit plane
    ApprOrthogonalPlane3<double> fitter;
    fitter.Fit(pos);

    std::pair<Vector3<double>,Vector3<double>> par = fitter.GetParameters();
    Vector3<double> origin = par.first;
    Vector3<double> normal = par.second;

    double error = 0.0;
    for(size_t i=0;i<pos.size();i++) {
        Vector3<double> point = pos[i];
        Vector3<double> diff = point - origin;
        error += std::fabs(Dot(diff,normal)); // distance to plane
    }
    std::cerr << "# Plane error = " << std::setprecision(12) << error << std::endl;

    std::cerr << "# Plane origin "
        << std::setprecision(12) << origin[0] << " "
        << std::setprecision(12) << origin[1] << " "
        << std::setprecision(12) << origin[2] << std::endl;

    std::cerr << "# Plane normal "
        << std::setprecision(12) << normal[0] << " "
        << std::setprecision(12) << normal[1] << " "
        << std::setprecision(12) << normal[2] << std::endl;

    basis[0] = normal;
    ComputeOrthogonalComplement(1,basis);

    Vector3<double> planeX = basis[1];
    Vector3<double> planeY = basis[2];

    std::cerr << "# Plane X "
        << std::setprecision(12) << planeX[0] << " "
        << std::setprecision(12) << planeX[1] << " "
        << std::setprecision(12) << planeX[2] << std::endl;

    std::cerr << "# Plane Y "
        << std::setprecision(12) << planeY[0] << " "
        << std::setprecision(12) << planeY[1] << " "
        << std::setprecision(12) << planeY[2] << std::endl;

    double dmin = 1E200; // very large initial value
    DCPPoint3Ellipsoid3<double> query;

    double rmin = 0.0;
    { // Fit sphere with linear search
        double r0 = (argc > 3) ? atoi(argv[3]) : 7000; // initial radius
        size_t kiter = 1000;
        size_t kmin = 0;
        for(size_t k=0;k<kiter;k++) {
            // Test object
            double r = r0 + k * 1.0;
            Ellipsoid3<double> el;
            el.center = origin + r * normal;
            el.extent = { r, r, r };
            el.axis[0] = basis[0];
            el.axis[1] = basis[1];
            el.axis[2] = basis[2];

            // Compute energy
            double d = 0;
#pragma omp parallel for reduction(+:d)
            for(size_t i=0;i<pos.size();i++) {
                Vector3<double> point = pos[i];
                auto result = query(point,el);
                d += result.distance; // distance to ellipsoid
            }

            // Find "unique" optimum
            if(d <= dmin) {
                dmin = d;
                rmin = r;
                kmin = k;
#ifdef INSPECT
                std::cerr << "# Sphere updated with r = " << rmin << " at cost = " << dmin << std::endl;
#endif
            } else break; // gone too far
        }
        if(kmin == kiter - 1) {
            std::cerr << "# Sphere search may exceed " << kiter << " iterations" << std::endl;
        }
    }

    Vector3<double> cmin = origin + rmin * normal;
    Vector3<double> emin = { rmin, rmin, rmin };
    { // Fit ellipsoid with non-linear optimization
        nlopt::opt opt("LN_COBYLA", 6);
        std::vector<double> lb {
            cmin[0] - 100, cmin[1] - 100, cmin[2] - 100,
            rmin * 0.9, rmin * 0.9, rmin * 0.9
        };
        std::vector<double> ub {
            cmin[0] + 100, cmin[1] + 100, cmin[2] + 100,
            rmin * 1.1, rmin * 1.1, rmin * 1.1
        };
        opt.set_lower_bounds(lb);
        opt.set_upper_bounds(ub);
        opt.set_min_objective(ellipsoid_cost, &pos);
        opt.set_xtol_rel(1e-4);
        std::vector<double> x {
            cmin[0], cmin[1], cmin[2],
            emin[0], emin[1], emin[2]
        };
        opt.optimize(x, dmin);

        cmin = { x[0], x[1], x[2] };
        emin = { x[3], x[4], x[5] };
    }

    std::cerr << "# Ellipsoid cost = " << std::setprecision(12) << dmin << std::endl;
    std::cerr << "# Ellipsoid center "
        << std::setprecision(12) << cmin[0] << " "
        << std::setprecision(12) << cmin[1] << " "
        << std::setprecision(12) << cmin[2] << std::endl;
    std::cerr << "# Ellipsoid extents "
        << std::setprecision(12) << emin[0] << " "
        << std::setprecision(12) << emin[1] << " "
        << std::setprecision(12) << emin[2] << std::endl;

    // Final object
    Ellipsoid3<double> el;
    el.center = cmin;
    el.extent = emin;
    el.axis[0] = basis[0];
    el.axis[1] = basis[1];
    el.axis[2] = basis[2];

    // Get axis-angle rotation matrix for ellipsoid
    Matrix3x3<double> mat_el;
    mat_el.SetCol(0,el.axis[0]);
    mat_el.SetCol(1,el.axis[1]);
    mat_el.SetCol(2,el.axis[2]);

    Rotation<3,double> rot_el(mat_el);
    AxisAngle<3,double> aa_el(rot_el);

    std::cerr << "# Ellipsoid angle " << std::setprecision(12) << aa_el.angle << std::endl;
    std::cerr << "# Ellipsoid axis "
        << std::setprecision(12) << aa_el.axis[0] << " "
        << std::setprecision(12) << aa_el.axis[1] << " "
        << std::setprecision(12) << aa_el.axis[2] << std::endl;

    std::vector<Vector3<double>> projPts;
    // Project points orthogonally to ellipsoid
    for(size_t i=0;i<pos.size();i++) {
        Vector3<double> point = pos[i];
        auto result = query(point,el);
        Vector3<double> proj = result.closest;
        projPts.push_back(proj);
        // signed distance
        double d = result.distance;
        double r_point = Length(point - el.center);
        double r_proj = Length(proj - el.center);
        if(r_point < r_proj) d = -d; // inside ellipsoid
        // print
        std::cerr
            << std::setprecision(12) << proj[0] << " "
            << std::setprecision(12) << proj[1] << " "
            << std::setprecision(12) << proj[2] << " "
            << 0 << " "
            << d
            << std::endl;
    }

    std::vector<Vector3<double>> projPts2;
    for(size_t i=0;i<pos2.size();i++) {
        Vector3<double> point = pos2[i];
        auto result = query(point,el);
        Vector3<double> proj = result.closest;
        projPts.push_back(proj);
        // signed distance
        double d = result.distance;
        double r_point = Length(point - el.center);
        double r_proj = Length(proj - el.center);
        if(r_point < r_proj) d = -d; // inside ellipsoid
        // print
        std::cerr
            << std::setprecision(12) << proj[0] << " "
            << std::setprecision(12) << proj[1] << " "
            << std::setprecision(12) << proj[2] << " "
            << 1 << " "
            << d
            << std::endl;
    }

    // Bounding box of all projected points
    OrientedBox3<double> box;
    Vector3<double> *pts = &projPts[0]; 
    GetContainer(projPts.size(),pts,box);

    std::cerr << "# Box center "
        << std::setprecision(12) << box.center[0] << " "
        << std::setprecision(12) << box.center[1] << " "
        << std::setprecision(12) << box.center[2] << std::endl;

    std::cerr << "# Box extents "
        << std::setprecision(12) << box.extent[0] << " "
        << std::setprecision(12) << box.extent[1] << " "
        << std::setprecision(12) << box.extent[2] << std::endl;

    // Get axis-angle rotation matrix for box
    Matrix3x3<double> mat_box;
    mat_box.SetCol(0,box.axis[0]);
    mat_box.SetCol(1,box.axis[1]);
    mat_box.SetCol(2,box.axis[2]);

    Rotation<3,double> rot_box(mat_box);
    AxisAngle<3,double> aa_box(rot_box);

    std::cerr << "# Box angle " << std::setprecision(12) << aa_box.angle << std::endl;
    std::cerr << "# Box axis "
        << std::setprecision(12) << aa_box.axis[0] << " "
        << std::setprecision(12) << aa_box.axis[1] << " "
        << std::setprecision(12) << aa_box.axis[2] << std::endl;

    // Print gmsh geometry file
    std::cout << "SetFactory(\"OpenCASCADE\");" << std::endl; // set kernel
    // Ellipsoid
    std::cout << "Sphere(1) = {0, 0, 0, 1, -Pi/2, Pi/2, 2*Pi};" << std::endl; // unit sphere
    std::cout << "Dilate {{0, 0, 0}, {"
        << std::setprecision(12) << el.extent[0] << ", "
        << std::setprecision(12) << el.extent[1] << ", "
        << std::setprecision(12) << el.extent[2] << "}} {" << std::endl
        << "  Volume{1};" << std::endl
        << "}" << std::endl; // transform into ellipsoid
    std::cout << "Rotate {{"
        << std::setprecision(12) << aa_el.axis[0] << ", "
        << std::setprecision(12) << aa_el.axis[1] << ", "
        << std::setprecision(12) << aa_el.axis[2] << "}, {0, 0, 0}, "
        << std::setprecision(12) << aa_el.angle << "} {" << std::endl
        << "  Volume{1};" << std::endl
        << "}" << std::endl; // rotate ellipsoid
    std::cout << "Translate {"
        << std::setprecision(12) << el.center[0] << ", "
        << std::setprecision(12) << el.center[1] << ", "
        << std::setprecision(12) << el.center[2] << "} {" << std::endl
        << "  Volume{1};" << std::endl
        << "}" << std::endl; // translate ellipsoid
    // Bounding box
    std::cout << "Box(2) = {-1, -1, -1, 2, 2, 2};" << std::endl; // unit cube
    std::cout << "Dilate {{0, 0, 0}, {"
        << std::setprecision(12) << box.extent[0] << ", "
        << std::setprecision(12) << box.extent[1] << ", "
        << std::setprecision(12) << box.extent[2] << "}} {" << std::endl
        << "  Volume{2};" << std::endl
        << "}" << std::endl; // scale bounding box
    std::cout << "Rotate {{"
        << std::setprecision(12) << aa_box.axis[0] << ", "
        << std::setprecision(12) << aa_box.axis[1] << ", "
        << std::setprecision(12) << aa_box.axis[2] << "}, {0, 0, 0}, "
        << std::setprecision(12) << aa_box.angle << "} {" << std::endl
        << "  Volume{2};" << std::endl
        << "}" << std::endl; // rotate bounding box
    std::cout << "Translate {"
        << std::setprecision(12) << box.center[0] << ", "
        << std::setprecision(12) << box.center[1] << ", "
        << std::setprecision(12) << box.center[2] << "} {" << std::endl
        << "  Volume{2};" << std::endl
        << "}" << std::endl; // translate bounding box
    // Intersect ellipsoid with bounding box
    std::cout << "BooleanIntersection { Surface{1}; Delete; }{ Volume{2}; Delete; }" << std::endl;
    std::cout << "Delete {" << std::endl
        << "  Volume{1}; Curve{1}; Curve{2}; Curve{3};" << std::endl
        << "}" << std::endl; // delete old objects
    std::cout << "Delete {" << std::endl
        << "  Point{1}; Point{2};" << std::endl
        << "}" << std::endl; // delete old objects

    return 0;
}
