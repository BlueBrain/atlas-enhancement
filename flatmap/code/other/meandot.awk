NR == 1 { a=$1; b=$2; c=$3; x=$4; y=$5; z=$6; n=1; } # initial condition
{
    if($1==a && $2==b && $3==c) { x=+$4; y+=$5; z+=$6; n++; } # add to mean
    else {
        a=$1; b=$2; c=$3; x=$4; y=$5; z=$6; n=1; # reset
        if(NF == 6)
            print a,b,c,x/n,y/n,z/n; # print mean point
        else
            print a,b,c,x/n,y/n; # print mean point
    }
}
