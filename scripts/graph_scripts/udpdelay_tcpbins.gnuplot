# Script to create a single plot with tcpbins and udpdelay data
# Takes as input:
# tcpbinsfn: csv data file with TCP bins data
# udpdelayfn: csv data file with UDP data
# outfn: name of output png file to store the generated graph
#
# Example: gnuplot -e "tcpbinsfn='tcpbins_size3_seed4.dat'; 
# udpdelayfn='udpdelay_size3_seed4.dat' ; outfn='temp.png'"
# udpdelay_tcpbins.gnuplot

if (!exists("tcpbinsfn")) tcpbinsfn='tcpbins.dat'
if (!exists("udpdelayfn")) udpdelayfn='udpdelay.dat'
if (!exists("outfn")) outfn='output.png'
if (!exists("titlestr")) titlestr='UDP packet delay and TCP data rate'
if (!exists("legend2str")) legend2str='TCP rate'

set term png size 800, 600
set output outfn
set sample 1000

set key left 

set title titlestr


set xlabel 'Time (seconds)'
set xrange [0:60]
set ylabel 'Delay (ms)'
set y2label 'Data rate (kbps)'
set ytics nomirror
set y2tics

plot udpdelayfn u 2:4 w l t 'UDP delay' axes x1y1, tcpbinsfn u 2:4 w lines t legend2str axes x1y2



