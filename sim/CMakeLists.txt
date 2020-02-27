add_executable(mesh_sim
	apps_config.cc			apps_config.h
	apps_manager.cc			apps_manager.h
	app_rx_cb.cc			app_rx_cb.h
	app_rq_dec_cb.cc		app_rq_dec_cb.h
	io_utils.cc			io_utils.h
	main.cc
	mesh_sim.cc			mesh_sim.h
	mobility_config.cc		mobility_config.h
	ns3_utils.cc			ns3_utils.h
	ns3object_config.cc		ns3object_config.h
	progress_report.cc		progress_report.h
	routing_config.cc		routing_config.h
	wifi_config.cc			wifi_config.h
)

target_link_libraries(mesh_sim
	ns3
	Boost::boost
	Boost::filesystem
	ns3_apps
)
