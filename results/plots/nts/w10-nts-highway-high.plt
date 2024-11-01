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

set xrange [-10.0:110.0]
set yrange [0.0:100.0]

set xtics ("{/Helvetica-Italic T_1" 0, "{/Helvetica-Italic T_2" 20, "{/Helvetica-Italic T_3" 40, "{/Helvetica-Italic T_1" 60, "{/Helvetica-Italic T_2" 80, "{/Helvetica-Italic T_3" 100)
set xtics scale 0
#set tics font ", 18"
set ytics 10
#set format y ""

set style data histogram
set style fill solid border

set output 'w10-nts-highway-high.eps'
#set ylabel "Number of Tasks by Success Type (%)" font "sans, 19"
#set ylabel "Tasks by Type of Occurrence (%)" offset 1,0,0
#set xlabel "Partial Coverage                                    Full Coverage\nWorkload"
set xlabel "Partial 5G Coverage          Full 5G Coverage"
set key on right top horizontal Left noreverse noinvert autotitle outside box
#set key spacing 1.0
set key samplen 3
set key font ",20"
set key width -0.5
#set key rows 5
#set key mincolumn 10
set border 31 lw 0.5
set boxwidth 2.2

set style histogram clustered

plot "../../dat/nts-highway-high-random2.dat" using ($1-6.3):($2+$3+$4) notitle w boxes ls 1 fs so 0.5,\
"" using ($1-6.3):($2+$3+$4) notitle w boxes lc 0 fs pa 5 trans,\
"" using ($1-6.3):($2+$3) notitle w boxes ls 1 fs so 0.5,\
"" using ($1-6.3):($2) notitle w boxes ls 1 fs so 0.5,\
"" using ($1-6.3):($2) notitle w boxes lc 0 fs pa 2 trans,\
"../../dat/nts-highway-high-hvc.dat" using ($1-3.1):($2+$3+$4) notitle w boxes ls 3 fs so 0.5,\
"" using ($1-3.1):($2+$3+$4) notitle w boxes lc 0 fs pa 5 trans,\
"" using ($1-3.1):($2+$3) notitle w boxes ls 3 fs so 0.5,\
"" using ($1-3.1):($2) notitle w boxes ls 3 fs so 0.5,\
"" using ($1-3.1):($2) notitle w boxes lc 0 fs pa 2 trans,\
"../../dat/nts-highway-high-mdo.dat" using ($1):($2+$3+$4) notitle w boxes ls 9 fs so 0.5,\
"" using ($1):($2+$3+$4) notitle w boxes lc 0 fs pa 5 trans,\
"" using ($1):($2+$3) notitle w boxes ls 9 fs so 0.5,\
"" using ($1):($2) notitle w boxes ls 9 fs so 0.5,\
"" using ($1):($2) notitle w boxes lc 0 fs pa 2 trans,\
"../../dat/nts-highway-high-gtt.dat" using ($1+3.1):($2+$3+$4) notitle w boxes ls 5 fs so 0.5,\
"" using ($1+3.1):($2+$3+$4) notitle w boxes lc 0 fs pa 5 trans,\
"" using ($1+3.1):($2+$3) notitle w boxes ls 5 fs so 0.5,\
"" using ($1+3.1):($2) notitle w boxes ls 5 fs so 0.5,\
"" using ($1+3.1):($2) notitle w boxes lc 0 fs pa 2 trans,\
"../../dat/nts-highway-high-abc.dat" using ($1+6.3):($2+$3+$4) notitle w boxes ls 7 fs so 0.5,\
"" using ($1+6.3):($2+$3+$4) notitle w boxes lc 0 fs pa 5 trans,\
"" using ($1+6.3):($2+$3) notitle w boxes ls 7 fs so 0.5,\
"" using ($1+6.3):($2) notitle w boxes ls 7 fs so 0.5,\
"" using ($1+6.3):($2) notitle w boxes lc 0 fs pa 2 trans,\
1 / 0 title "FIFO" w boxes ls 1 fs so 0.5,\
2 / 0 title "TL" w boxes lc 0 fs pa 2 trans,\
3 / 0 title "HVC" with boxes ls 3 fs so 0.5,\
4 / 0 title "TS" w boxes lc 0 fs pa 0 trans,\
5 / 0 title "MDO" with boxes ls 9 fs so 0.5,\
6 / 0 title "TR" with boxes lc 0 fs pa 5 trans,\
7 / 0 title "GTT" with boxes linestyle 5 fs so 0.5,\
8 / 0 title "BTV" with boxes linestyle 7 fs so 0.5
