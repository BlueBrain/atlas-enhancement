NR == 2 { nv=$1; nt=$2 }
NR > 2 && NR <= 2 + nv {
    n = NR - 3
    vn[n] = 0
}
NR > 2 + nv {
    n = NR - nv - 3
    t1[n] = $2
    t2[n] = $3
    t3[n] = $4
    vn[t1[n]]++
    vn[t2[n]]++
    vn[t3[n]]++
}
END {
    nbad = 0
    for(i=0;i<nv;++i) nbad += (vn[i] == 0);
    print nbad
}
