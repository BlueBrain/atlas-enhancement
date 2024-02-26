from math import pi, sqrt, atan

# cached constants
sqrt_2 = sqrt(2)
square_circle_const = (pi - 10./(3. * sqrt_2))


# volume of cylinder of height `h` and radius `r`
def VC(r, h):
    return pi * h * r * r


# volume of cylinder with unit height and squared radius `r_sq`
def VC_h1(r_sq):
    return pi * r_sq


# volume of cylinder fully contained in bottom of pyramid = VC(r, H - (r - r0) / K), for H = 1
def VCp_h1(r, r_sq, r0, r1):
    return pi * r_sq * ((r1 - r) / (r1 - r0))


# volume of truncated square pyramid with height `h` and radii `r0` and `r1`
def Vf(r0, r1, h):
    return 4./3. * h * (r0 * (r0 + r1) + r1 * r1)


# volume of truncated square pyramid with unit height and radii `r0` and `r1`
def Vf_h1(r0, r1):
    return 4./3. * (r0 * (r0 + r1) + r1 * r1)


# volume of transition between inscribed and excribed squares to circle of cubed radius `r_cb`
def Vt(r_cb, one_over_K):
    return r_cb * one_over_K * square_circle_const


# volume of transition between inscribed and excribed circles of the square with radius `r0`
def Vq(r_sq, r_cb, r0, one_over_K):
    r0_sq = r0 * r0
    rz = sqrt(r_sq - r0_sq)
    return one_over_K * (pi * r_cb
                         + r_sq * (4 * r0 * atan(rz / r0) - 8./3. * rz - pi * r0)
                         - 4./3. * r0_sq * rz)


# volume of wedges in transition from inscribed to excribed circle to square with radius `r1`
def Vr(r_sq, r_cb, r1, one_over_K):
    r1_sq = r1 * r1
    r1_cb = r1_sq * r1
    rz = sqrt(r_sq - r1_sq)
    return 4./3. * one_over_K * (sqrt_2 * r_cb
                                 - r_sq * (2 * rz +
                                           3 * r1 * atan((r1 - rz)/(r1 + rz)))
                                 + (r1_cb - r1_sq * rz))


# first regime
def V1(r, r0, r1, h, one_over_K):
    r_sq = r * r
    r_cb = r_sq * r
    return Vq(r_sq, r_cb, r0, one_over_K) + VC(r, h - (r - r0) * one_over_K)


# first regime, unit height
def V1_h1(r, r_sq, r_cb, r0, r1, one_over_delta):
    return Vq(r_sq, r_cb, r0, one_over_delta) + VCp_h1(r, r_sq, r0, r1)


# second regime
def V2(r, r0, r1, h, one_over_K):
    r_cb = r * r * r
    u = r / sqrt_2
    return Vf(r0, u, (u - r0) * one_over_K) \
         + VC(r, h - (r - r0) * one_over_K) \
         + Vt(r_cb, one_over_K)


# second regime, unit height
def V2_h1(r, r_sq, r_cb, r0, r1, one_over_delta):
    u = r / sqrt_2
    return Vf(r0, u, (u - r0) * one_over_delta) \
         + VCp_h1(r, r_sq, r0, r1) \
         + Vt(r_cb, one_over_delta)


# third regime
def V3(r, r0, r1, h, one_over_K):
    r_sq = r * r
    r_cb = r_sq * r
    return Vf(r0, r1, h) - Vr(r_sq, r_cb, r1, one_over_K)


# third regime, unit height
def V3_h1(r_sq, r_cb, r0, r1, one_over_delta):
    return Vf_h1(r0, r1) - Vr(r_sq, r_cb, r1, one_over_delta)


# full radial volume function
def F(r, r0, r1, h):
    one_over_K = h / (r1 - r0)
    sqrt2_by_r0 = sqrt_2 * r0
    sqrt2_by_r1 = sqrt_2 * r1
    r_sq = r * r
    r_cb = r_sq * r

    if (r <= r0):                         # always happens
        return VC(r, h)
    elif (r <= sqrt2_by_r0 and r <= r1):  # always happens, up to r = min(sqrt2_by_r0, r1)
        return V1(r, r0, r1, h, one_over_K)
    elif (r <= r1):                       # only happens when r1 > sqrt2_by_r0
        return V2(r, r0, r1, h, one_over_K)
    elif (r <= sqrt2_by_r0):              # only happens when r1 < sqrt2_by_r0
        return V3(r, r0, r1, h, one_over_K) + Vr(r_sq, r_cb, r0, one_over_K)
    elif (r <= sqrt2_by_r1):              # always happens, from r = max(sqrt2_by_r0, r1)
        return V3(r, r0, r1, h, one_over_K)
    else:                                 # total volume
        return Vf(r0, r1, h)


# full radial volume function, unit height
def F_h1(r, r0, r1, one_over_delta, sqrt2_by_r0, sqrt2_by_r1):
    r_sq = r * r
    r_cb = r_sq * r

    if (r <= r0):
        return VC_h1(r_sq)
    elif (r <= sqrt2_by_r0 and r <= r1):
        return V1_h1(r, r_sq, r_cb, r0, r1, one_over_delta)
    elif (r <= r1):
        return V2_h1(r, r_sq, r_cb, r0, r1, one_over_delta)
    elif (r <= sqrt2_by_r0):
        return V3_h1(r_sq, r_cb, r0, r1, one_over_delta) + Vr(r_sq, r_cb, r0, one_over_delta)
    elif (r <= sqrt2_by_r1):
        return V3_h1(r_sq, r_cb, r0, r1, one_over_delta)
    else:
        return Vf_h1(r0, r1)


# normalized radial volume
def normalized_volume(r, r0, r1):
    full = Vf_h1(r0, r1)
    one_over_delta = 1.0 / (r1 - r0)
    sqrt2_by_r0 = sqrt_2 * r0
    sqrt2_by_r1 = sqrt_2 * r1
    return F_h1(r, r0, r1, one_over_delta, sqrt2_by_r0, sqrt2_by_r1) / full


# normalized radial volume, cached parameters
def normalized_volume_cached(r, r0, r1, full, one_over_delta, sqrt2_by_r0, sqrt2_by_r1):
    return F_h1(r, r0, r1, one_over_delta, sqrt2_by_r0, sqrt2_by_r1) / full


# normalized radial volume for an array of radius values
def normalized_volume_f(r, r0, r1):
    import numpy
    arr = numpy.asarray(r)
    full = Vf_h1(r0, r1)
    one_over_delta = 1.0 / (r1 - r0)
    sqrt2_by_r0 = sqrt_2 * r0
    sqrt2_by_r1 = sqrt_2 * r1
    return numpy.array([normalized_volume_cached(x, r0, r1, full, one_over_delta, sqrt2_by_r0, sqrt2_by_r1)
                       for x in arr])


