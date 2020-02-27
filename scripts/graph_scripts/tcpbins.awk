#!/usr/bin/awk -f
# Converts pcap_eval tcpbins mode output for a single output run
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

        datarecv=0
}

/^DETAILS: Data recv/ {
    datarecv=1
}
/^Data:/{
   if (datarecv){
	print "Time, " $3 " Rate, " $5
   }
}

END {
}
