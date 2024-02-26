BEGIN {
    OFS=" "
    iv = it = 0
}

NR == 2 {
    nv = $1; nt = $2; ne = $3;
}

NR > 2 && NR <= 2 + nv {
    vx[iv] = $1
    vy[iv] = $2
    vz[iv] = $3
    iv++
}

NR > 2 + nv {
    ta[it] = $2 + 1
    tb[it] = $3 + 1
    tc[it] = $4 + 1
    it++
}

END {
    print " MeshVersionFormatted 2"
    print " Dimension"
    print " 3"
    print " Vertices"
    printf " %d\n",nv
    for(i=0;i<nv;i++) printf " %20g%6s%20g%6s%20g%6s%d\n",vx[i],"",vy[i],"",vz[i],"",1
    print " Triangles"
    printf " %d\n",nt
    for(i=0;i<nt;i++) printf " %d %d %d %d\n",ta[i],tb[i],tc[i],1
    print " End"
}
