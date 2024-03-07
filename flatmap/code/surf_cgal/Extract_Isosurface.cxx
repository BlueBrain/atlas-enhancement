#include <CGAL/Surface_mesh_default_triangulation_3.h>
#include <CGAL/Surface_mesh_default_criteria_3.h>
#include <CGAL/Complex_2_in_triangulation_3.h>
#include <CGAL/IO/Complex_2_in_triangulation_3_file_writer.h>
#include <fstream>
#include <CGAL/make_surface_mesh.h>
#include <CGAL/Gray_level_image_3.h>
#include <CGAL/Implicit_surface_3.h>

// default triangulation for Surface_mesher
typedef CGAL::Surface_mesh_default_triangulation_3 Tr;
// c2t3
typedef CGAL::Complex_2_in_triangulation_3<Tr> C2t3;
typedef Tr::Geom_traits GT;
typedef CGAL::Gray_level_image_3<GT::FT, GT::Point_3> Gray_level_image;
typedef CGAL::Implicit_surface_3<GT, Gray_level_image> Surface_3;

int main(int argc, char** argv)
{
  Tr tr;            // 3D-Delaunay triangulation
  C2t3 c2t3 (tr);   // 2D-complex in 3D-Delaunay triangulation

  const char *input_file = argv[1];
  float iso_level = atof(argv[2]);

  float sph_cx = atof(argv[3]);
  float sph_cy = atof(argv[4]);
  float sph_cz = atof(argv[5]);
  float sph_r = atof(argv[6]);

  float rel_prec = atof(argv[7]);

  float angle_bound = atof(argv[8]); // lower bound, degrees
  float radius_bound = atof(argv[9]); // upper bound
  float distance_bound = atof(argv[10]); // upper bound

  const char *output_file = argv[11];

  // the 'function' is a 3D gray level image
  Gray_level_image image(input_file, iso_level);

  // Carefully chosen bounding sphere: the center must be inside the
  // surface defined by 'image' and the radius must be high enough so that
  // the sphere actually bounds the whole image.
  GT::Point_3 bounding_sphere_center(sph_cx, sph_cy, sph_cz);
  GT::FT bounding_sphere_squared_radius = sph_r * sph_r;
  GT::Sphere_3 bounding_sphere(bounding_sphere_center,
                               bounding_sphere_squared_radius);

  // definition of the surface
  Surface_3 surface(image, bounding_sphere, rel_prec);

  // defining meshing criteria
  CGAL::Surface_mesh_default_criteria_3<Tr> criteria(angle_bound, radius_bound, distance_bound);

  // meshing surface, with the "manifold without boundary" algorithm
  CGAL::make_surface_mesh(c2t3, surface, criteria,
                          CGAL::Manifold_with_boundary_tag());

  std::ofstream out(output_file);
  CGAL::output_surface_facets_to_off (out, c2t3);

  std::cout << "Final number of points: " << tr.number_of_vertices() << "\n";
}
