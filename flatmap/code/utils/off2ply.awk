BEGIN { outfile = ARGV[2]; ARGC-- }
NR == 2 { nv = $1; nt = $2; printf "%s\n%s\n",nv,nt > outfile; }
NR > 2 && NR <= 2 + nv { print $1,$2,$3 >> outfile }
NR > 2 + nv { print $1,$2,$3,$4 >> outfile }
