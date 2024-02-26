#!/bin/sh

awk '{ print 0,$4,$5,0,$1,$2,$3 }' $1 \
    | awk -v wx=$2 -v wy=$3 -f $BBMAKE_ROOT/structural/hexassign.awk - normal.txt hexgrid.txt \
    | awk '{ print $5,$6,$7,$2,$3,$8,$9 }'
