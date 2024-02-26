BEGIN {
    iv = it = 0
}
NR == 5 { nv=$1 }
NR > 5 && NR <= 5 + nv {
    vx[iv] = $1
    vy[iv] = $2
    vz[iv] = $3
    iv++
}
NR == 5 + nv + 2 { nt=$1 }
NR > 5 + nv + 2 {
    ta[it] = $1 - 1
    tb[it] = $2 - 1
    tc[it] = $3 - 1
    it++
}
END {
    print "OFF"
    print nv,nt,0
    for(i=0;i<nv;i++) print vx[i],vy[i],vz[i]
    for(i=0;i<nt;i++) print 3,ta[i],tb[i],tc[i]
}
