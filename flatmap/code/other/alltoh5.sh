#!/bin/bash

if [ -z "$1" ]; then
    exit
fi

parallel -C' ' -kj1 "\
    m4 -P -D _num={1} -D _len={2} h5config.m4 > /tmp/config.tmp; \
    gunzip -c streamlines/streamline_{1}.txt.gz > /tmp/data.tmp; \
    h5import /tmp/data.tmp -c /tmp/config.tmp -o streamlines.h5" :::: streamlines/index.txt
