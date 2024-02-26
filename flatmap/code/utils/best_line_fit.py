from skspatial.objects import Line, Points, Vector
from scipy.sparse import csr_matrix
from scipy.sparse.linalg import svds

# faster and leaner Line.best_fit
def best_line_fit(points, tol=None, **kwargs) -> Line:
    points_spatial = Points(points)

    if points_spatial.are_concurrent(tol=tol):
        raise ValueError("The points must not be concurrent.")

    points_centered, centroid = points_spatial.mean_center(return_centroid=True)

    _, _, vh = svds(csr_matrix(points_centered), 1, return_singular_vectors="vh")
    direction = vh[0, :]

    return Line(centroid, direction)


def position_along_line(line, position):
    x = line.project_point(position)
    v = Vector.from_points(line.point, x)
    return line.direction.scalar_projection(v)
