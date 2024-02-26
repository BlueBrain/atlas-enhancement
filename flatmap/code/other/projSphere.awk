BEGIN {
    split(center,c," ");
    r = radius;
}

{
    norm = 0;
    for(i=1;i<=3;i++) {
        p[i] = $i;
        v[i] = p[i] - c[i];
        norm += v[i] * v[i];
    }
    norm = sqrt(norm);

    for(i=1;i<=3;i++) {
        v[i] /= norm;
        p[i] = c[i] + r * v[i];
    }
    print p[1],p[2],p[3]
}
