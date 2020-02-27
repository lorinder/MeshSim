#ifndef APPS_MANAGER_H
#define APPS_MANAGER_H

#include <unordered_map>
#include <string>
#include <vector>

#include "ns3_all.h"

#include "apps_config.h"

class AppRxCb;
class AppRqDecCb;

/**	Utility to manage the applications.
 *
 *	This class is used to create apps, connect them among each
 *	other, and cleanly dispose of them.  Trace extraction from apps
 *	is also handled by AppsManager.
 */
class AppsManager {
public:
	AppsManager();
	~AppsManager();

	typedef std::unordered_map< uint32_t, ns3::Ptr<ns3::NetDevice> >
	  Addr2NetDevMapping;

	void setOutDir(const std::string& out_dir);
	bool createApps(const AppsCompleteConfig& cfg,
			const Addr2NetDevMapping& addr2netdev);

private:
	/* Input Parameters */

	/* Output directory (used for trace creation) */
	std::string out_dir;

	/* Processing data */

	struct AttribNames {
		std::string proto;
		std::string address;
		std::string port;
	};

	struct AppRecord {
		AppConfig::AppType type;
		ns3::Ptr<ns3::Application> app;
		uint32_t host_ip;

		bool TxMultiple;
		AttribNames TxNames;

		bool RxMultiple;
		AttribNames RxNames;

		bool has_rx_trace;
		bool has_rq_decoder_trace;
	};

	/* Map: tag -> AppRecord */
	std::unordered_map< std::string, AppRecord > rec;

	/* Processing methods */
	bool createApp(const AppConfig& cfg,
		const Addr2NetDevMapping& addr2netdev);

	/** Create the connections from a connect statement.
	 *
	 *  @param	conn_index
	 *		The index of the "connect" statement.
	 *
	 *  @param	sindex
	 *		Pointer to the index of the individual
	 *		connection.  For cases where there are just 1:1
	 *		connect statements, this is the same as
	 *		conn_index, but otherwise it's larger.
	 *		createConn updates *sindex.
	 *
	 */
	bool createConns(int conn_index,
			int* sindex,
			const AppConnectionConfig& cfg);

	/** */
	void createConn1(int sindex,
		int rx_index,
		const AppRecord& rec_rx,
		int tx_index,
		const AppRecord& rec_tx);

	static AttribNames getNamesFromNameTemplates(
			const AttribNames& templ,
			int index);

	/* Connected callbacks */
	std::vector< AppRxCb* > rxCbList;
	std::vector< AppRqDecCb* > rqDecCbList;
};

#endif /* APPS_MANAGER_H */
