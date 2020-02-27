#!/bin/sh

# This script takes as input a chainsim output directory and generates analysis
# data and graphs using the pcap_eval and tabulate tool. The paths for both
# pcap_eval and tabulate default to the current working directory. They can be
# changed however using the arguments listed below.
# Similarly the name of the output directory to analyze can be changed.
# By default, the conversation between a wired-sta and backhaul with fixed IP
# addresses is tracked for the purpose of this script. But these can also be
# changed using the environment parameters listed below.

: ${outdir:=./out}
: ${pcapcmd:=./pcap_eval -s "10.1.4.1" -d "10.1.1.1" -n 12}
: ${filenm:=wiredsta-10.1.4.1.pcap.gz}
: ${chainsimdir:=./}
: ${tabulatecmd:=./tabulate -d 40}

# Run pcap_eval on the entire directory
echo "Tabulating tcpavg"
$pcapcmd -m "tcprate" $outdir/*/$filenm > $outdir/tcpavg.txt
echo "Formatting rate data into a table"
$tabulatecmd $outdir/tcpavg.txt > $outdir/tcprate_table.txt
echo "Tabulating tcpbins"
$pcapcmd -m "tcpbins" $outdir/*/$filenm > $outdir/tcpbins.txt
echo "Tabulating udpbins"
$pcapcmd -m "udpbins" $outdir/*/$filenm > $outdir/udpbins.txt
echo "Tabulating udpdelay"
$pcapcmd -m "udpdelay" $outdir/*/$filenm > $outdir/udpdelay.txt

# Prepare the graphs
echo "Generating graphs..."

graphscriptsdir=$chainsimdir/../graph_scripts
graphdir=$outdir/graphs
mkdir -p $graphdir
logfn="${graphdir}/log.txt"
for d in $(find $outdir -maxdepth 1 -type d);
do
  if [ -d "$d" ]; then
      cfg=$(basename $d)
      if [ -f "$d/$filenm" ]
      then
	  echo $cfg
	  echo $cfg >> $logfn
	  $pcapcmd -m "tcpbins" $d/$filenm > ${graphdir}/${cfg}_tcpbins.txt
	  ${graphscriptsdir}/tcpbins.awk ${graphdir}/${cfg}_tcpbins.txt > ${graphdir}/${cfg}_tcpbins.dat
	  $pcapcmd -m "udpdelay" $d/$filenm > ${graphdir}/${cfg}_udpdelay.txt
	  ${graphscriptsdir}/udpdelay.awk ${graphdir}/${cfg}_udpdelay.txt > ${graphdir}/${cfg}_udpdelay.dat
	  gnuplot -e "tcpbinsfn='${graphdir}/${cfg}_tcpbins.dat'; udpdelayfn='${graphdir}/${cfg}_udpdelay.dat' ; outfn='${graphdir}/${cfg}.png'" ${graphscriptsdir}/udpdelay_tcpbins.gnuplot >> $logfn 2>&1
      fi
      
  fi 
done
