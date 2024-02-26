BEGIN {
    split(origin,o," ");
    split(normal,n," ");
}

{
    d = 0;
    for(i=1;i<=3;i++) {
        p[i] = $i;
        v[i] = p[i] - o[i];
        d += v[i] * n[i];
    }
    for(i=1;i<=3;i++) {
        p[i] -= d * n[i];
    }
    print p[1],p[2],p[3]
}
