BEGIN { outfile = ARGV[2]; ARGC--; offset = 2 }
NR == 1 && $1 != "OFF" { print "Not an OFF file" >> "/dev/stderr"; exit }
$1 == "#" || $1 == "" { ++offset } # ignore comments and empty lines
NR == offset { nv = $1; nt = $2; printf "%s\n%s\n",nv,nt > outfile; }
NR > offset && NR <= offset + nv { print $1,$2,$3 >> outfile }
NR > offset + nv { print $1,$2,$3,$4 >> outfile }
