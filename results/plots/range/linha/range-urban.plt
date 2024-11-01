#!/usr/bin/gnuplot
#set terminal postscript eps color "Times" 20
set terminal postscript eps color "Times" 25
#set encoding iso_8859_1
set encoding utf8

set grid ytics lt 0 lw 1

set style line 1 lt 1 pt 8 ps 2 lw 5 lc rgbcolor '#636363'
set style line 3 lt 3 pt 4 ps 2 lw 5 lc rgbcolor '#de2d25'
set style line 5 lt 5 pt 6 ps 2 lw 5 lc rgbcolor '#2c7fb8'
set style line 7 lt 7 pt 12 ps 2 lw 5 lc rgbcolor '#2ca25f'
set style line 9 lt 9 pt 3 ps 2 lw 3 lc rgbcolor '#d95f0e'

set style line 11 lt 11 pt 5 ps 2.0 lw 5 lc rgbcolor '#90E65F'
set style line 12 lt 12 pt 7 ps 2.0 lw 5 lc rgbcolor '#10B521'
set style line 13 lt 12 pt 13 ps 2.0 lw 5 lc rgbcolor '#11731B'

set style line 14 lt 11 dt 1 pt 5 ps 2.0 lw 5 lc rgbcolor '#FFAB4A'
set style line 15 lt 12 dt 1 pt 7 ps 2.0 lw 5 lc rgbcolor '#FF720D'
set style line 16 lt 12 dt 1 pt 13 ps 2.0 lw 5 lc rgbcolor '#CC4C02'

set pointsize 0.5

set xrange [-6.5:36.5]
set yrange [0.0:30.0]
set xtics ("250" 0, "500" 15, "750" 30)
set ytics 5
set output 'range-urban.eps'
set xlabel "Range (m)"
set ylabel "Number of Replies"
set key on right top horizontal Right outside box width -1 maxrows 1
set key samplen 3
set key width -1
set key spacing 1.2
set border 31 lw 0.5

plot "../../dat/range/range-urban-low-w10-gtt.dat" using 1:2 notitle w linespoints ls 11,\
"../../dat/range/range-urban-low-w10-gtt.dat" using 1:2:3 notitle w errorbars ls 11,\
"../../dat/range/range-urban-medium-w10-gtt.dat" using 1:2 notitle w linespoints ls 12,\
"../../dat/range/range-urban-medium-w10-gtt.dat" using 1:2:3 notitle w errorbars ls 12,\
"../../dat/range/range-urban-high-w10-gtt.dat" using 1:2 notitle w linespoints ls 13,\
"../../dat/range/range-urban-high-w10-gtt.dat" using 1:2:3 notitle w errorbars ls 13,\
1 / 0 title "Low" w linespoints ls 11,\
2 / 0 title "Medium" w linespoints ls 12,\
3 / 0 title "High" w linespoints ls 13
