#!/usr/bin/gnuplot
#set terminal postscript eps color "Times" 20
set terminal postscript eps color "Times" 25
#set encoding iso_8859_1
set encoding utf8

#set key right top vertical font "sans, 17"
#set key outside below
#set key outside above horizontal font "sans, 15"
set grid ytics lt 0 lw 1
#set grid xtics lt 0 lw 1

#alisson
#set style line 1 lt 1 pt 8 ps 2 lw 3 lc 0
#set style line 2 lt 2 pt 9 ps 1.7 lw 2 lc 0
#set style line 3 lt 3 pt 4 ps 2 lw 3 lc 1
#set style line 4 lt 4 pt 5 ps 1 lw 2 lc 1
#set style line 5 lt 5 pt 6 ps 2 lw 3 lc 3
#set style line 6 lt 6 pt 7 ps 1 lw 2 lc 3
#set style line 7 lt 7 pt 12 ps 2 lw 3 lc 2
#set style line 8 lt 8 pt 13 ps 1 lw 2 lc 2
#set style line 9 lt 9 pt 14 ps 1 lw 2 lc 2

set style line 1 lt 1 pt 8 ps 2 lw 3 lc rgbcolor '#636363'
set style line 3 lt 3 pt 4 ps 2 lw 3 lc rgbcolor '#de2d25'
set style line 5 lt 5 pt 6 ps 2 lw 3 lc rgbcolor '#2c7fb8'
set style line 6 lt 6 pt 6 ps 2 lw 3 lc rgbcolor '#6fbef2'
set style line 7 lt 7 pt 12 ps 2 lw 3 lc rgbcolor '#2ca25f'
set style line 8 lt 8 pt 13 ps 2 lw 3 lc rgbcolor '#a1d99b'
set style line 9 lt 9 pt 3 ps 2 lw 3 lc rgbcolor '#d95f0e'
set style line 10 lt 10 pt 3 ps 2 lw 3 lc rgbcolor '#fe9929'

set pointsize 0.5

set xrange [-10.0:90.0]
set yrange [-10.0:80.0]
 
set xtics ("{/Helvetica-Italic T_6" 0, "{/Helvetica-Italic T_7" 20, "{/Helvetica-Italic T_8" 40, "{/Helvetica-Italic T_9" 60, "{/Helvetica-Italic T_{10}" 80)
#set tics font ", 18"
set ytics 10

# Select histogram data
set style data histogram
# Give the bars a plain fill pattern, and draw a solid line around them.
set style fill solid border

#plot "dat/tso-urban-high-random2.dat" using ($1-7.5):2 notitle w boxes ls 1 fs so #0.5 fs pa 0 fs nobo,\
#set style line 1 lc rgb '#0e1111' lt 1 lw 2 pt -1 ps 1.0 
set output 'w10-gn-urban-medium.eps'
set xlabel "Workload"
set xzeroaxis lt -1 lw 1.0
#set ylabel "Success Rate (%)" font "sans, 19"
set ylabel "Reduction in Execution Time (%)"
set key on right top horizontal Left noreverse noinvert autotitle outside box
#set key spacing 1.0
set key samplen 3
set key width -0.5
#set key rows 5
#set key mincolumn 10
set border 31 lw 0.5
set boxwidth 2
#set style fill (fs) solid (so) 0.5 pattern (pa) 1 border (bo) noborder(nobo) transparent (trans)
#set style pattern 1

#"" using ($1-7.5):($3) notitle w boxerrorbars ls 1,\

#set style histogram clustered
set style histogram errorbars

plot "../../dat/gn-urban-medium-random2.dat" using ($1-6.0):($2):($3) notitle w boxerrorbars ls 1 fs so 0.5,\
"../../dat/gn-urban-medium-hvc.dat" using ($1-3.0):($2):($3) notitle w boxerrorbars ls 3 fs so 0.5,\
"../../dat/gn-urban-medium-gcf2.dat" using ($1+0.0):($2):($3) notitle w boxerrorbars ls 5 fs so 0.5,\
"../../dat/gn-urban-medium-gtt.dat" using ($1+3.0):($2):($3) notitle w boxerrorbars ls 7 fs so 0.5,\
"../../dat/gn-urban-medium-abc.dat" using ($1+6.0):($2):($3) notitle w boxerrorbars ls 9 fs so 0.5,\
1 / 0 title "FIFO" w boxes ls 1 fs so 0.5,\
2 / 0 title "HVC" with boxes ls 3 fs so 0.5,\
3 / 0 title "GCF" with boxes ls 5 fs so 0.5,\
4 / 0 title "GTT" with boxes ls 7 fs so 0.5,\
5 / 0 title "BCV" with boxes ls 9 fs so 0.5
