add_library(ns3_apps		STATIC
	bulk-send-application.cc bulk-send-application.h
	onoff-application.cc	onoff-application.h
	packet-sink.cc		packet-sink.h
	proxy-base.cc		proxy-base.h
	rq-decoder.cc		rq-decoder.h
	rq-encoder.cc		rq-encoder.h
	rq-header.cc		rq-header.h
	timed-proxy.cc		timed-proxy.h
)

target_link_libraries(ns3_apps
	ns3
)
target_include_directories(ns3_apps INTERFACE .)
