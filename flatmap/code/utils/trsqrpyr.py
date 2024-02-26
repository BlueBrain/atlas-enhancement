from math import pi, sqrt, atan

# cached constants
sqrt_2 = sqrt(2)
four_thirds = 4./3.
five_thirds_sqrt2 = 5./3. * sqrt_2


# volume of cylinder of height `(b - a) / K` and radius `r`
def Vc(r, a, b, one_over_K):
    return pi * one_over_K * (b - a) * r*r


# volume of truncated square pyramid with radii `a` and `b` and height `(b - a) / K`
def Vf(a, b, one_over_K):
    return four_thirds * one_over_K * (b*b*b - a*a*a)


# volume of intersection between cylinder of radius `r` and
# truncated square pyramid of radii `a` and `b` and height `(b - a) / K`
def Vq(r, a, b, one_over_K):
    return Vp(r, b, one_over_K) - Vp(r, a, one_over_K)


# indefinite integral used above
def Vp(r, u, one_over_K):
    rsq = r*r
    usq = u*u
    sqrt_ru = sqrt(rsq - usq)
    return pi * one_over_K * u * rsq + \
           four_thirds * one_over_K * (sqrt_ru * (usq + 2 * rsq) - 3 * rsq * atan(sqrt_ru / u) * u)


# special cases (for numerical stability)
def Vp_r(r, one_over_K):
    return pi * one_over_K * r*r*r


def Vp_rs2(r, one_over_K):
    return five_thirds_sqrt2 * one_over_K * r*r*r


def Vq_r0_r(r, r0, one_over_K):
    return Vp_r(r, one_over_K) - Vp(r, r0, one_over_K)


def Vq_rs2_r(r, one_over_K):
    return Vp_r(r, one_over_K) - Vp_rs2(r, one_over_K)


def Vq_rs2_r1(r, r1, one_over_K):
    return Vp(r, r1, one_over_K) - Vp_rs2(r, one_over_K)


# first case
def V1(r, r0, r1, one_over_K):
    return Vc(r, r0, r1, one_over_K)


# second case
def V2(r, r0, r1, one_over_K):
    return Vq_r0_r(r, r0, one_over_K) + Vc(r, r, r1, one_over_K)


# third case, r1 > sqrt_2 * r0
def V3_a(r, r0, r1, one_over_K):
    return Vf(r0, r/sqrt_2, one_over_K) + Vq_rs2_r(r, one_over_K) + Vc(r, r, r1, one_over_K)


# third case, r1 <= sqrt_2 * r0
def V3_b(r, r0, r1, one_over_K):
    return Vq(r, r0, r1, one_over_K)


# fourth case
def V4(r, r0, r1, one_over_K):
    return Vf(r0, r/sqrt_2, one_over_K) + Vq_rs2_r1(r, r1, one_over_K)


# full radial volume function, cached parameters
def V(r, r0, r1, one_over_K, sqrt2_by_r0, sqrt2_by_r1):
    if (r <= r0):                         # always happens
        return V1(r, r0, r1, one_over_K)
    elif (r <= sqrt2_by_r0 and r <= r1):  # always happens, up to r = min(sqrt2_by_r0, r1)
        return V2(r, r0, r1, one_over_K)
    elif (r <= r1):                       # only happens when r1 > sqrt2_by_r0
        return V3_a(r, r0, r1, one_over_K)
    elif (r <= sqrt2_by_r0):              # only happens when r1 < sqrt2_by_r0
        return V3_b(r, r0, r1, one_over_K)
    elif (r <= sqrt2_by_r1):              # always happens, from r = max(sqrt2_by_r0, r1)
        return V4(r, r0, r1, one_over_K)
    else:                                 # total volume
        return Vf(r0, r1, one_over_K)


# normalized radial volume, cached parameters
def normalized_volume_r(r, r0, r1, full, one_over_delta, sqrt2_by_r0, sqrt2_by_r1):
    return V(r, r0, r1, one_over_delta, sqrt2_by_r0, sqrt2_by_r1) / full


# normalized radial volume for an array of radius values
def normalized_volume_r_fun(r, r0, r1):
    import numpy
    arr = numpy.asarray(r)
    one_over_delta = 1. / (r1 - r0)
    full = Vf(r0, r1, one_over_delta)
    sqrt2_by_r0 = sqrt_2 * r0
    sqrt2_by_r1 = sqrt_2 * r1
    return numpy.array([normalized_volume_r(x, r0, r1, full, one_over_delta, sqrt2_by_r0, sqrt2_by_r1)
        for x in arr])


## Depthwise volume distribution
def Vf_h(h, r0, one_over_K):
    return four_thirds * one_over_K * ((r0 + h / one_over_K)**3 - r0*r0*r0)


# normalized depthwise volume, cached parameters
def normalized_volume_h(h, r0, delta, H, r0_cb):
    return ((r0 + (delta/H * h))**3 - r0_cb) / ((r0 + delta)**3 - r0_cb)


# linear density assumption
def linear_volume_increase(h, a, b, H):
    return a * h + b * h*h if h <= H else 1.0


# normalized depthwise volume for an array of depth values
def normalized_volume_h_fun(h, r0, delta, H):
    import numpy
    arr = numpy.asarray(h)
    r0_cb = r0*r0*r0
    return numpy.array([normalized_volume_h(x, r0, delta, H, r0_cb) for x in arr])


def linear_volume_increase_fun(h, a, b, H):
    import numpy
    arr = numpy.asarray(h)
    return numpy.array([linear_volume_increase(x, a, b, H) for x in arr])


