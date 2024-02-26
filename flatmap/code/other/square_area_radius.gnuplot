 f(x,a) = (x <= a) ? pi * x**2  : ((x <= sqrt(2) * a) ? pi * a**2 + 4 * (pi / 4 * (x**2 - a**2) + a * sqrt(x**2 - a**2) - x**2 * atan(sqrt(x**2 - a**2) / a)) : 4 * a**2)
df(x,a) = (x <= a) ? 2 * pi * x : ((x <= sqrt(2) * a) ? 2 * x * (pi - 4 * atan((sqrt(x - a) * sqrt(x + a)) / a))                                              : 0       )
fN(x,a) = f(x,a) / (4 * a**2)

K(r0,r1,h) = (r1 - r0) / h
VC(r,h) = pi * h * r**2
Vf(r0,r1,h) = 4./3. * h * (r0**2 + r0*r1 + r1**2)
Vt(r,r0,r1,h) = r**3 / K(r0,r1,h) * (pi - 10./(3. * sqrt(2)))
Vq(r,r0,r1,h) = 1./(3 * K(r0,r1,h)) * (r**2 * (12 * r0 * atan(sqrt(r**2 - r0**2) / r0) - 8 * sqrt(r**2 - r0**2) + 3 * pi * (r - r0)) - 4 * r0**2 * sqrt(r**2 - r0**2))
Vr(r,r0,r1,h) = 4./(3. * K(r0,r1,h)) * (r1**3 - r1**2 * sqrt(r**2 - r1**2) + r**2 * (sqrt(2) * r - 2 * sqrt(r**2 - r1**2)) - 3 * r1 * r**2 * atan((r1 - sqrt(r**2 - r1**2))/(r1 + sqrt(r**2 - r1**2))))
Vs(r,r0,r1,h) = 4./(3. * K(r0,r1,h)) * (sqrt(2) * r**3 - r**2 * (2 * sqrt(r**2 - r0**2) + 3 * r0 * atan((r0 - sqrt(r**2 - r0**2))/(r0 + sqrt(r**2 - r0**2)))) + (r0**3 - r0**2 * sqrt(r**2 - r0**2)))

F(r,r0,r1,h) =            (r <= r0)            ? VC(r,h) : \
               ((r <= sqrt(2) * r0 && r <= r1) ? V1(r,r0,r1,h) : \
                         ((r <= r1)            ? V2(r,r0,r1,h) : \
               ((r <= sqrt(2) * r0)            ? V3(r,r0,r1,h) + Vs(r,r0,r1,h) : \
               ((r <= sqrt(2) * r1)            ? V3(r,r0,r1,h) : \
                                                 Vf(r0,r1,h)))))
V1(r,r0,r1,h) = Vq(r,r0,r1,h) + VC(r,h - (r - r0)/K(r0,r1,h))
V2(r,r0,r1,h) = Vf(r0,r / sqrt(2),(r / sqrt(2) - r0) / K(r0,r1,h)) + VC(r,h - (r - r0)/K(r0,r1,h)) + Vt(r,r0,r1,h)
V3(r,r0,r1,h) = Vf(r0,r1,h) - Vr(r,r0,r1,h)
FN(r,r0,r1,h) = F(r,r0,r1,h) / Vf(r0,r1,h)

V0N(r,r0,r1) = (r <= r0)                                ? 3./4. * pi * r**2 / (r0**2 + r0*r1 + r1**2) : 0
V1N(r,r0,r1) = (r > r0 && r <= sqrt(2) * r0 && r <= r1) ? 1./4. * (3 * pi * r**2 * (r1 - r0) + 12 * r0 * r**2 * atan(sqrt(r**2 - r0**2) / r0) - 4 * sqrt(r**2 - r0**2) * (2 * r**2 + r0**2)) / (r1**3 - r0**3) : 0
V2N(r,r0,r1) = (r > sqrt(2) * r0 && r <= r1)            ? (3./4. * pi * r1 * r**2 - r0**3 - sqrt(2) * r**3) / (r1**3 - r0**3) : 0
V3N(r,r0,r1) = (r > r1 && r <= sqrt(2) * r1)            ? 1. - (r1**2 * (r1 - sqrt(r**2-r1**2)) + 2 * r**2 * (r/sqrt(2) - sqrt(r**2-r1**2)) - 3 * r1 * r**2 * atan((r1 - sqrt(r**2-r1**2))/(r1 + sqrt(r**2-r1**2)))) / (r1**3 - r0**3) : 0
ONE(r,r0,r1) = (r > sqrt(2) * r1)                       ? 1 : 0
G(r,r0,r1) = V0N(r,r0,r1) + V1N(r,r0,r1) + V2N(r,r0,r1) + V3N(r,r0,r1) + ONE(r,r0,r1)
