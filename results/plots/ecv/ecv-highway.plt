#!/usr/bin/gnuplot
set terminal postscript eps color "Times" 22
set encoding utf8

set size ratio -1
unset autoscale y
set autoscale x

set grid ytics lt 0 lw 1

set style line 1 lt 1 pt 8 ps 2 lw 3 lc rgbcolor '#636363'
set style line 3 lt 3 pt 4 ps 2 lw 3 lc rgbcolor '#de2d25'
set style line 5 lt 5 pt 6 ps 2 lw 3 lc rgbcolor '#2c7fb8'
set style line 6 lt 6 pt 6 ps 2 lw 3 lc rgbcolor '#6fbef2'
set style line 7 lt 7 pt 12 ps 2 lw 3 lc rgbcolor '#2ca25f'
set style line 8 lt 8 pt 13 ps 2 lw 3 lc rgbcolor '#a1d99b'
set style line 9 lt 9 pt 3 ps 2 lw 3 lc rgbcolor '#d95f0e'
set style line 10 lt 10 pt 3 ps 2 lw 3 lc rgbcolor '#fe9929'

set pointsize 0.5

set xrange [-0.5:5.5]
set yrange [0.0:5.0]
 
set xtics ("{/Helvetica-Italic D_{low}" 0, "{/Helvetica-Italic D_{med.}" 1, "{/Helvetica-Italic D_{high}" 2, "{/Helvetica-Italic D_{low}" 3, "{/Helvetica-Italic D_{med.}" 4, "{/Helvetica-Italic D_{high}" 5)
set xtics scale 0
#set tics font ", 18"
set ytics 0.5

set style data histogram
set style fill solid border

set output 'ecv-highway.eps'
#set xlabel "Density/Coverage"
set xlabel "Partial 5G Coverage          Full 5G Coverage"
set xzeroaxis lt -1 lw 1.0
#set ylabel "Success Rate (%)" font "sans, 19"
set ylabel "Violations of Energy Constraints (%)" offset 1,0,0
set key on right top horizontal Left noreverse noinvert autotitle outside box
#set key spacing 1.0
set key samplen 3
set key width -0.5
#set key rows 5
#set key mincolumn 10
set border 31 lw 0.5
set boxwidth 0.16

set style histogram errorbars

plot "../../dat/ecv/ecv-highway-random2.dat" using ($1-0.315):($2) notitle w boxes ls 1 fs so 0.5,\
"../../dat/ecv/ecv-highway-hvc.dat" using ($1-0.105):($2) notitle w boxes ls 3 fs so 0.5,\
"../../dat/ecv/ecv-highway-gtt.dat" using ($1+0.105):($2) notitle w boxes ls 5 fs so 0.5,\
"../../dat/ecv/ecv-highway-abc.dat" using ($1+0.315):($2) notitle w boxes ls 7 fs so 0.5,\
1 / 0 title "FIFO" w boxes ls 1 fs so 0.5,\
2 / 0 title "HVC" with boxes ls 3 fs so 0.5,\
4 / 0 title "GTT" with boxes ls 5 fs so 0.5,\
5 / 0 title "BCV" with boxes ls 7 fs so 0.5
