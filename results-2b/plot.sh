#!/bin/bash

for csv in *.csv
    do
    name=$(basename $csv .csv)
    gnuplot << EOF
    reset
    set term png size 1920, 1080
    set ylabel "CWIN size (bytes)"
    set xlabel "time (s)"
    set datafile sep ','
    set title "Congestion window monitoring"
    set grid

    set out "$name.png"
    plot "$csv" using 1:2 with linespoints title "$csv"
EOF
done
