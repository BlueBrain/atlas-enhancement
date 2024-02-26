BEGIN { outfile = ARGV[2]; ARGC-- }
BEGIN { print "OFF" > outfile }
NR == 1 { nv = $1; printf "%s ",nv >> outfile }
NR == 2 { nt = $1; printf "%s 0\n",nt >> outfile }
NR > 2 && NR <= 2 + nv { print $1,$2,$3 >> outfile }
NR > 2 + nv { print $1,$2,$3,$4 >> outfile }
