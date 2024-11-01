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
set style line 9 lt 9 pt 3 ps 2 lw 5 lc rgbcolor '#d95f0e'

#set style line 11 lt 11 pt 1 ps 2.0 lw 5 lc rgbcolor '#78c679'
#set style line 12 lt 12 pt 2 ps 2.0 lw 5 lc rgbcolor '#238443'
#set style line 13 lt 12 pt 4 ps 2.0 lw 5 lc rgbcolor '#004529'

#set style line 1 lt 11 pt 5 ps 2.3 lw 6 lc rgbcolor 'black'
#set style line 1 lt 11 pt 64 ps 2.3 lw 2 lc rgbcolor 'black'
#set style line 11 lt 11 pt 1 ps 2.0 lw 5 lc rgbcolor '#63ED71'
set style line 11 lt 11 pt 5 ps 2.0 lw 5 lc rgbcolor '#90E65F'
set style line 12 lt 12 pt 7 ps 2.0 lw 5 lc rgbcolor '#10B521'
set style line 13 lt 12 pt 13 ps 2.0 lw 5 lc rgbcolor '#11731B'

#set style line 14 lt 11 pt 7 ps 2.0 lw 5 lc rgbcolor '#fe9929'
#set style line 15 lt 12 pt 9 ps 2.0 lw 5 lc rgbcolor '#ec7014'
#set style line 16 lt 12 pt 11 ps 2.0 lw 5 lc rgbcolor '#cc4c02'

set style line 14 lt 11 dt 1 pt 5 ps 2.0 lw 5 lc rgbcolor '#FFAB4A'
set style line 15 lt 12 dt 1 pt 7 ps 2.0 lw 5 lc rgbcolor '#FF720D'
set style line 16 lt 12 dt 1 pt 13 ps 2.0 lw 5 lc rgbcolor '#CC4C02'

#set pointsize 0.5

set xrange [0.0:100.0]
set yrange [-20.0:90.0]
set xtics ("0" 0, "25" 25, "50" 50, "75" 75, "100" 100)
set ytics 10
#set title  'Workload 1'
#set term 'pdf'
set output 'kr-urban-tasks-cl.eps'
set xlabel "Vehicles with Known Routes (%)"
set xzeroaxis lt -1 lw 1.0
set ylabel "Reduction of Recovered Tasks (%)"
set key on right top horizontal Right outside box width -1 maxrows 1
set key samplen 3
set key width -1
set key spacing 1.2
set border 31 lw 0.5

plot "../../dat/kr/kr-urban-low-gtt.dat" using 1:5 notitle w linespoints ls 11,\
"../../dat/kr/kr-urban-medium-gtt.dat" using 1:5 notitle w linespoints ls 12,\
"../../dat/kr/kr-urban-high-gtt.dat" using 1:5 notitle w linespoints ls 13,\
"../../dat/kr/kr-urban-low-abc.dat" using 1:5 notitle w linespoints ls 14,\
"../../dat/kr/kr-urban-medium-abc.dat" using 1:5 notitle w linespoints ls 15,\
"../../dat/kr/kr-urban-high-abc.dat" using 1:5 notitle w linespoints ls 16,\
0 / 0 title "GTT Low" w linespoints ls 11,\
0 / 0 title "GTT Medium" w linespoints ls 12,\
0 / 0 title "GTT High" w linespoints ls 13,\
0 / 0 title "BCV Low" w linespoints ls 14,\
0 / 0 title "BCV Medium" w linespoints ls 15,\
0 / 0 title "BCV High" w linespoints ls 16
