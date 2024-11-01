#!/usr/bin/gnuplot
#set terminal postscript eps color "Times" 20
set terminal postscript eps color "Times" 25
#set encoding iso_8859_1
set encoding utf8

#set key right top vertical font "sans, 17"
#set key outside below
#set key outside above horizontal font "sans, 15"
set grid ytics lt 0 lw 1

set style line 1 lt 1 pt 8 ps 2 lw 3 lc rgbcolor '#636363'
set style line 3 lt 3 pt 4 ps 2 lw 3 lc rgbcolor '#de2d25'
set style line 5 lt 5 pt 6 ps 2 lw 3 lc rgbcolor '#2c7fb8'
set style line 6 lt 6 pt 6 ps 2 lw 3 lc rgbcolor '#6fbef2'
set style line 7 lt 7 pt 12 ps 2 lw 3 lc rgbcolor '#2ca25f'
set style line 8 lt 8 pt 13 ps 2 lw 3 lc rgbcolor '#a1d99b'
set style line 9 lt 9 pt 3 ps 2 lw 3 lc rgbcolor '#d95f0e'
set style line 10 lt 10 pt 3 ps 2 lw 3 lc rgbcolor '#fe9929'

set style line 11 lt 11 pt 5 ps 2.0 lw 5 lc rgbcolor '#90E65F'
set style line 12 lt 12 pt 7 ps 2.0 lw 5 lc rgbcolor '#10B521'
set style line 13 lt 12 pt 13 ps 2.0 lw 5 lc rgbcolor '#11731B'

set pointsize 0.5

set xrange [-6.5:36.5]
set yrange [0.0:30.0]
 
set xtics ("250" 0, "500" 15, "750" 30)
#set tics font ", 18"
set ytics 5 scale 0
set tics nomirror

# Select histogram data
set style data histogram
# Give the bars a plain fill pattern, and draw a solid line around them.
set style fill solid border

#plot "dat/tso-urban-high-random2.dat" using ($1-7.5):2 notitle w boxes ls 1 fs so #0.5 fs pa 0 fs nobo,\
#set style line 1 lc rgb '#0e1111' lt 1 lw 2 pt -1 ps 1.0 
set output 'range-urban-w10-gtt-cl.eps'
set xlabel "Range (meters)"
set xzeroaxis lt -1 lw 1.0
#set ylabel "Number of Replies" font "sans, 19"
set ylabel "Number of Replies"
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

plot "../../dat/range/range-urban-low-w10-gtt.dat" using ($1-2.5):($2):($3) notitle w boxerrorbars ls 11 fs so 0.5 bo 0,\
"../../dat/range/range-urban-medium-w10-gtt.dat" using ($1):($2):($3) notitle w boxerrorbars ls 12 fs so 0.5 bo 0,\
"../../dat/range/range-urban-high-w10-gtt.dat" using ($1+2.5):($2):($3) notitle w boxerrorbars ls 13 fs so 0.5 bo 0,\
1 / 0 title "Low" w boxes ls 11 fs so 0.5 bo 0,\
3 / 0 title "Medium" with boxes ls 12 fs so 0.5 bo 0,\
5 / 0 title "High" with boxes ls 13 fs so 0.5 bo 0
