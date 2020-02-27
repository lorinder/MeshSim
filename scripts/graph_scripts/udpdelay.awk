#!/usr/bin/awk -f
# Converts pcap_eval udpdelay mode output for a single output run
# to csv data for plotting. 
/^file / {
	# Compute size
	size=$2
	gsub(/^.*size/, "", size)
	gsub(/_.*$/, "", size)

	# Compute seed
	seed=$2
	gsub(/^.*seed/, "", seed)
	gsub(/\/.*$/, "", seed)

}

/^UDP Pkt Recv:/{
	delay=$6
	gsub(/\ms/, "", delay)
	print "Time, " $4 ", Delay, " delay
}

END {
}
