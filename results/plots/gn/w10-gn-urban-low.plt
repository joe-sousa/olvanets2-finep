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

set xrange [-9.0:99.0]
set yrange [-10.0:80.0]

set xtics ("{/Helvetica-Italic T_1" 0, "{/Helvetica-Italic T_2" 18, "{/Helvetica-Italic T_3" 36, "{/Helvetica-Italic T_1" 54, "{/Helvetica-Italic T_2" 72, "{/Helvetica-Italic T_3" 90)
#set tics font ", 18"
set ytics 10

# Select histogram data
set style data histogram
# Give the bars a plain fill pattern, and draw a solid line around them.
set style fill solid border

#plot "dat/tso-urban-high-random2.dat" using ($1-7.5):2 notitle w boxes ls 1 fs so #0.5 fs pa 0 fs nobo,\
#set style line 1 lc rgb '#0e1111' lt 1 lw 2 pt -1 ps 1.0 
set output 'w10-gn-urban-low.eps'
#set xlabel "Half Coverage                        Full Coverage\nWorkload"
set xlabel "Partial 5G Coverage           Full 5G Coverage"
set xzeroaxis lt -1 lw 1.0
#set ylabel "Success Rate (%)" font "sans, 19"
set ylabel "Reduction in Execution Time (%)" offset 1,0,0
set key on right top horizontal Left noreverse noinvert autotitle outside box
#set key spacing 1.0
set key samplen 3
set key font ",20"
set key width -0.5
#set key rows 5
#set key mincolumn 10
set border 31 lw 0.5
set boxwidth 2

set style histogram errorbars

plot "../../dat/gn-urban-low-random2.dat" using ($1-6.0):($2):($3) notitle w boxerrorbars ls 1 fs so 0.5,\
"../../dat/gn-urban-low-hvc.dat" using ($1-3.0):($2):($3) notitle w boxerrorbars ls 3 fs so 0.5,\
"../../dat/gn-urban-low-mdo.dat" using ($1):($2):($3) notitle w boxerrorbars ls 9 fs so 0.5,\
"../../dat/gn-urban-low-gtt.dat" using ($1+3.0):($2):($3) notitle w boxerrorbars ls 5 fs so 0.5,\
"../../dat/gn-urban-low-abc.dat" using ($1+6.0):($2):($3) notitle w boxerrorbars ls 7 fs so 0.5,\
1 / 0 title "FIFO" w boxes ls 1 fs so 0.5,\
2 / 0 title "HVC" with boxes ls 3 fs so 0.5,\
3 / 0 title "MDO" with boxes ls 9 fs so 0.5,\
4 / 0 title "GTT" with boxes ls 5 fs so 0.5,\
5 / 0 title "BTV" with boxes ls 7 fs so 0.5
