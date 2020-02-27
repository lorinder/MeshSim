#include <cassert>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <unordered_map>

#include "io_utils.h"
#include "ns3_utils.h"
#include "apps_config.h"

using namespace std;

static bool parseSingleAppConfig(AppConfig* ret, const vector<string>& tokens)
{
	if (tokens.size() < 3) {
		cerr << "Error:  Need at least <tag> "
		  "<ip> <app-type> for app description\n";
		return false;
	}

	/* Parse IP addresses */
	ret->tag = tokens[0];
	if (!read_ip_addr(ret->ip, tokens[1])) {
		/* Error already printed */
		return false;
	}

	/* Parse App type */
	static const struct {
		AppConfig::AppType tp;
		string name;
	} appnames[] = {
		{ AppConfig::APP_UDP_ECHO_CLIENT,	"echo_client" },
		{ AppConfig::APP_UDP_ECHO_SERVER,	"echo_server" },
		{ AppConfig::APP_FILE_SERVER,		"file_server" },
		{ AppConfig::APP_STREAMING_SERVER,	"streaming_server" },
		{ AppConfig::APP_CLIENT,		"client" },
		{ AppConfig::APP_WEB_SERVER,		"web_server" },
		{ AppConfig::APP_WEB_CLIENT,		"web_client" },
		{ AppConfig::APP_TIMED_PROXY,		"timed_proxy" },
		{ AppConfig::APP_RQ_ENCODER,		"rq_encoder" },
		{ AppConfig::APP_RQ_DECODER,		"rq_decoder" },
	};
	const string& name = tokens[2];
	ret->type = AppConfig::APP_INVALID;
	for (int i = 0; i < int(sizeof(appnames)/sizeof(*appnames)); ++i) {
		if (name == appnames[i].name) {
			ret->type = appnames[i].tp;
			break;
		}
	}
	if (ret->type == AppConfig::APP_INVALID) {
		cerr << "Error:  Unknown app type \"" << name << "\"\n";
		return false;
	}

	/* Parse and insert attributes */
	for (int i = 3; i < (int)tokens.size(); ++i) {
		pair<string, string> kv;
		if (!ParseAttributeAssignmentSpec(kv, tokens[i]))
			return false;
		ret->attribs.push_back(kv);
	}

	return true;
}

static bool parseSingleConnectionConfig(AppConnectionConfig* ret,
				const vector<string>& tokens)
{
	assert(tokens[0] == "connect");
	if (tokens.size() < 3) {
		cerr << "Error:  Connect needs at least two application "
		  "tags to connect.\n";
		return false;
	}

	ret->senders.clear();
	ret->receivers.clear();
	if (tokens.size() == 3) {
		ret->senders.push_back(tokens[1]);
		ret->receivers.push_back(tokens[2]);
	} else {
		int i;
		for (i = 1; i < int(tokens.size()); ++i) {
			if (tokens[i] == ":") {
				// Separator found
				break;
			}
			ret->senders.push_back(tokens[i]);
		}
		if (i == int(tokens.size())) {
			cerr << "Error:  Connect with multiple senders or "
			  "receivers needs \":\" separator token.\n";
			return false;
		}
		for (++i; i < int(tokens.size()); ++i) {
			ret->receivers.push_back(tokens[i]);
		}
		if (ret->senders.empty() || ret->receivers.empty()) {
			cerr << "Error:  Connect needs at least one sender "
			  "and at least one receiver.\n";
			return false;
		}
	}
	return true;
}

bool loadAppsConfig(AppsCompleteConfig* target,
		    const string& config_file_name)
{
	/* Open */
	fstream fp(config_file_name);
	if (!fp) {
		cerr << "Error:  Could not open apps config file \""
		  << config_file_name << "\"\n";
		return false;
	}

	/* Read line by line */
	unordered_map<string, string> default_attribs;
	vector<string> tokens;
	while (getconfiglinetokenized(fp, tokens)) {

		if (tokens[0] == "defaults") {
			/* Update default attributes */
			for (auto i = 1; i < (int)tokens.size(); ++i) {
				pair<string, string> kv;
				if (!ParseAttributeAssignmentSpec(kv, tokens[i]))
					return false;
				if (kv.second == "") {
					default_attribs.erase(kv.first);
				} else {
					default_attribs[kv.first] = kv.second;
				}
			}
		} else if (tokens[0] == "connect") {
			/* Parse connection config */
			AppConnectionConfig conn;
			if (!parseSingleConnectionConfig(&conn, tokens))
				return false;

			target->conn.push_back(conn);
		} else {
			/* Parse app config */
			AppConfig app;
			if (!parseSingleAppConfig(&app, tokens))
				return false;

			/* Add default attribs */
			for (auto p = default_attribs.begin();
			     p != default_attribs.end();
			     ++p)
			{
				/* Check if value was already in the
				 * supplied list;  in such a case, skip
				 * assigning the default value.
				 */
				bool found = false;
				for (auto q = app.attribs.begin();
				  q != app.attribs.end();
				  ++q)
				{
					if (q->first == p->first) {
						found = true;
						break;
					}
				}
				if (!found)
					app.attribs.push_back( *p );
			}

			/* Add app to list */
			target->app.push_back(app);
		}
	}

	return true;
}
