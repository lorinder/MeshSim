#ifndef APPS_CONFIG_H
#define APPS_CONFIG_H

#include <cstdint>
#include <vector>
#include <string>

struct AppConfig {
	enum AppType {
		APP_INVALID = -1,
		APP_UDP_ECHO_CLIENT,
		APP_UDP_ECHO_SERVER,
		APP_FILE_SERVER,		// BulkSender Application
		APP_STREAMING_SERVER,		// OnoffApplication
		APP_CLIENT,			// PacketSink
		APP_WEB_SERVER,
		APP_WEB_CLIENT,
		APP_TIMED_PROXY,
		APP_RQ_ENCODER,
		APP_RQ_DECODER
	};
	typedef std::vector< std::pair<std::string, std::string> > AppAttribs;

	std::string tag;			// Unique name of the app
	AppType type;				// The App type
	uint32_t ip;				// An IP address of host
						// containing the App

	AppAttribs attribs;
};

struct AppConnectionConfig {
	std::vector<std::string> senders;
	std::vector<std::string> receivers;
};

struct AppsCompleteConfig {
	std::vector<AppConfig> app;
	std::vector<AppConnectionConfig> conn;
};

bool loadAppsConfig(AppsCompleteConfig* target,
		    const std::string& config_file_name);

#endif /* APPS_CONFIG_H */
