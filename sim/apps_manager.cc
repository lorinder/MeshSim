#include <cassert>
#include <iomanip>

#include "apps_manager.h"
#include "app_rx_cb.h"
#include "app_rq_dec_cb.h"

#include "bulk-send-application.h"
#include "onoff-application.h"
#include "packet-sink.h"
#include "rq-encoder.h"
#include "rq-decoder.h"
#include "timed-proxy.h"

using namespace std;
using namespace ns3;

/* Helper functions */

static void setAppAttribs(Ptr<Application> app,
                       const AppConfig::AppAttribs& attribs)
{
	for (auto& a: attribs)
		app->SetAttribute(a.first, StringValue(a.second));
}

/* AppsManager implementation */

AppsManager::AppsManager()
{
}

AppsManager::~AppsManager()
{
	for (auto j: rxCbList)
		delete j;
	for (auto j: rqDecCbList)
		delete j;
}

void AppsManager::setOutDir(const std::string& out_dir_)
{
	out_dir = out_dir_;
}

bool AppsManager::createApps(const AppsCompleteConfig& cfg,
				const Addr2NetDevMapping& addr2netdev)
{
	/* Create the apps */
	for (const auto& ca: cfg.app) {
		if (!createApp(ca, addr2netdev))
			return false;
	}

	/* Connect them */
	int c_index = 0;
	for (int i = 0; i < (int)cfg.conn.size(); ++i) {
		if (!createConns(i, &c_index, cfg.conn[i]))
			return false;
	}

	return true;
}

bool AppsManager::createApp(const AppConfig& cfg,
				const Addr2NetDevMapping& addr2netdev)
{
	/* Create the record */
	AppRecord R{};
	R.type = cfg.type;
	R.host_ip = cfg.ip;
	switch (cfg.type) {
	case AppConfig::APP_UDP_ECHO_CLIENT:
		R.app = CreateObject<UdpEchoClient>();
		R.TxNames.address = "RemoteAddress";
		R.TxNames.port = "RemotePort";
		break;
	case AppConfig::APP_UDP_ECHO_SERVER:
		R.app = CreateObject<UdpEchoServer>();
		R.RxNames.port = "Port";
		break;
	case AppConfig::APP_FILE_SERVER:
		R.app = CreateObject<MeshSimBulkSendApplication>();
		R.TxNames.proto = "Protocol";
		R.TxNames.address = "Remote";
		break;
	case AppConfig::APP_STREAMING_SERVER:
		R.app = CreateObject<MeshSimOnOffApplication>();
		R.TxNames.proto = "Protocol";
		R.TxNames.address = "Remote";
		break;
	case AppConfig::APP_CLIENT:
		R.app = CreateObject<MeshSimPacketSink>();
		R.RxNames.proto = "Protocol";
		R.RxNames.address = "Local";
		R.has_rx_trace = true;
		break;
	case AppConfig::APP_WEB_SERVER:
		R.app = CreateObject<ThreeGppHttpServer>();
		R.RxNames.address = "LocalAddress";
		R.RxNames.port = "LocalPort";
		break;
	case AppConfig::APP_WEB_CLIENT:
		R.app = CreateObject<ThreeGppHttpClient>();
		R.TxNames.address = "RemoteServerAddress";
		R.TxNames.port = "RemoteServerPort";
		break;
	case AppConfig::APP_TIMED_PROXY:
		R.app = CreateObject<TimedProxy>();
		R.RxMultiple = false;
		R.RxNames.proto = "ProtocolRx0";
		R.RxNames.address = "RxAddr0";
		R.TxMultiple = false;
		R.TxNames.proto = "ProtocolTx0";
		R.TxNames.address = "TxAddr0";
		R.has_rx_trace = true;
		break;
	case AppConfig::APP_RQ_ENCODER:
		R.app = CreateObject<RqEncoder>();
		R.RxMultiple = true;
		R.RxNames.proto = "ProtocolRx";
		R.RxNames.address = "RxAddr";
		R.TxMultiple = false;
		R.TxNames.proto = "ProtocolTx0";
		R.TxNames.address = "TxAddr0";
		R.has_rx_trace = true;
		break;
	case AppConfig::APP_RQ_DECODER:
		R.app = CreateObject<RqDecoder>();
		R.RxMultiple = false;
		R.RxNames.proto = "ProtocolRx0";
		R.RxNames.address = "RxAddr0";
		R.TxMultiple = true;
		R.TxNames.proto = "ProtocolTx";
		R.TxNames.address = "TxAddr";
		R.has_rx_trace = true;
		R.has_rq_decoder_trace = true;
		break;
	default: {
		cerr << "Error:  Unhandled App type (" << __FILE__
		  << ", " << __LINE__ << ")\n";
		return false;
	}
	};

	/* Set the attributes on the app */
	setAppAttribs(R.app, cfg.attribs);

	/* Insert the record into the map */
	if (rec.find(cfg.tag) != rec.end()) {
		cerr << "Error:  Application tag name "
		  "`" << cfg.tag << "' used more than once.\n";
		return false;
	}
	rec[cfg.tag] = R;

	/* Install the App on the correct machine */
	Ptr<Node> host = addr2netdev.at(cfg.ip)->GetNode();
	host->AddApplication(R.app);

	return true;
}

bool AppsManager::createConns(int conn_index,
			     int* sindex,
			     const AppConnectionConfig& cfg)
{
	/* Validate inputs */
	if (cfg.senders.size() != 1 && cfg.receivers.size() != 1) {
		// XXX I haven't deeply thought about what it would mean
		// if there are both multiple receivers and senders; not
		// sure it even makes sense in general.  So we don't
		// support it.
		cerr << "Error:  Support for N:M connections not "
		  "implemented.\n";
		return false;
	}

	if (cfg.senders.size() > 1) {
		string tag_rx = cfg.receivers[0];
		const AppRecord& rec_rx = rec[tag_rx];
		if (!rec_rx.RxMultiple) {
			cerr << "Error:  N:1 connection to an "
			  "app that does not support multiple "
			  "RX connections.\n";
			return false;
		}
	}

	if (cfg.receivers.size() > 1) {
		string tag_tx = cfg.senders[0];
		const AppRecord& rec_tx = rec[tag_tx];
		if (!rec_tx.TxMultiple) {
			cerr << "Error:  1:N connection from an "
			  "app that does not support multiple "
			  "TX connections.\n";
			return false;
		}
	}

	/* Create all the connections and install callbacks */
	for (int i = 0; i < int(cfg.receivers.size()); ++i) {
		string tag_rx = cfg.receivers[i];
		const AppRecord& rec_rx = rec[tag_rx];
		for (int j = 0; j < int(cfg.senders.size()); ++j) {
			string tag_tx = cfg.senders[j];
			const AppRecord& rec_tx = rec[tag_tx];

			// XXX: Errcheck
			createConn1(*sindex, i, rec_rx, j, rec_tx);
			++*sindex;
		}

		/* Install the callbacks
		 *
		 *
		 * Note that we install the callbacks outside of the
		 * inner loop over j.  Since they're all installed on
		 * the RX side, that's enough and it avoids installing
		 * the callbacks multiple times.
		 */
		if (rec_rx.has_rx_trace) {
			ostringstream rx_tr;
			rx_tr << out_dir
			  << "/trace-app-rx-"
			  << setfill('0') << setw(3) << *sindex << ".txt";
			FILE* rx_byte_fp = fopen(rx_tr.str().c_str(), "w");
#define WriteHeader(fp) do { \
		fprintf(fp, "# app_connect_id = %d\n", *sindex); \
		fprintf(fp, "# connect_stmt_id = %d\n", conn_index); \
		fprintf(fp, "# tags_tx ="); \
		for (int j = 0; j < int(cfg.senders.size()); ++j) { \
			fprintf(fp, " %s", cfg.senders[j].c_str()); \
		} \
		fputc('\n', fp); \
		fprintf(fp, "# tag_rx = %s\n", tag_rx.c_str()); \
	} while (0)
			WriteHeader(rx_byte_fp);

			ostringstream pl_tr;
			pl_tr << out_dir
			  << "/trace-app-pl-"
			  << setfill('0') << setw(3) << *sindex << ".txt";
			FILE* pl_fp = fopen(pl_tr.str().c_str(), "w");
			WriteHeader(pl_fp);

			AppRxCb* S = new AppRxCb(rx_byte_fp, pl_fp, 128);
			rec_rx.app->TraceConnectWithoutContext("Rx",
				MakeCallback(&AppRxCb::rxCb, S));

			/* Record the callback so we can later remove it */
			rxCbList.push_back(S);
		}
		if (rec_rx.has_rq_decoder_trace) {
			ostringstream rqdec_tr;
			rqdec_tr << out_dir
			  << "/trace-app-rqdec-"
			  << setfill('0') << setw(3) << *sindex << ".txt";
			FILE* fp = fopen(rqdec_tr.str().c_str(), "w");

			AppRqDecCb* S = new AppRqDecCb(fp);
			rec_rx.app->TraceConnectWithoutContext("RqDecodingEvent",
				MakeCallback(&AppRqDecCb::rqDecCb, S));
			rqDecCbList.push_back(S);
		}
	}

	return true;
}

void AppsManager::createConn1(int sindex,
	int rx_index,
	const AppRecord& rec_rx,
	int tx_index,
	const AppRecord& rec_tx)
{
	/* Get the attribute names, performing "template" substitution
	 * if applicable.
	 */
	AttribNames txNames = rec_tx.TxNames, rxNames = rec_rx.RxNames;
	if (rec_tx.TxMultiple)
		txNames = getNamesFromNameTemplates(txNames, rx_index);
	if (rec_rx.RxMultiple)
		rxNames = getNamesFromNameTemplates(rxNames, tx_index);

	/* Make the protocols match */
	if (txNames.proto != "") {
		TypeIdValue proto;
		rec_tx.app->GetAttribute(txNames.proto, proto);
		rec_rx.app->SetAttribute(rxNames.proto, proto);
	}

	/* Sender side: set port and addresses */
	Ipv4Address rxAddr(rec_rx.host_ip);
	const int port = sindex + 1001;
	assert(txNames.address != ""); // Need to be able to tell where to send.
	if (txNames.port != "") {
		rec_tx.app->SetAttribute(txNames.port,
		  UintegerValue(port));
		rec_tx.app->SetAttribute(txNames.address,
		  AddressValue(rxAddr));
	} else {
		/* In this case, the address is a sockaddr,
		 * incorporating the port.
		 */
		InetSocketAddress sockaddr(rxAddr, port);
		rec_tx.app->SetAttribute(txNames.address,
		  AddressValue(sockaddr));
	}

	/* Receiver side: set port and addresses */
	if (rxNames.port != "") {
		rec_rx.app->SetAttribute(rxNames.port,
		  UintegerValue(port));
		if (rxNames.address != "") {
			rec_rx.app->SetAttribute(rxNames.address,
			  AddressValue(Ipv4Address::GetAny()));
		}
	} else {
		/* sockaddr case */
		InetSocketAddress sockaddr(Ipv4Address::GetAny(), port);
		rec_rx.app->SetAttribute(rxNames.address,
		  AddressValue(sockaddr));
	}
}

/* Template substitution */

static string tsubst(const string& orig, int index)
{
	if (orig == "")
		return string("");
	ostringstream p;
	p << orig << index;
	return p.str();
}

AppsManager::AttribNames AppsManager::getNamesFromNameTemplates(
			const AttribNames& templ,
			int i)
{
	AttribNames ret;
	ret.proto = tsubst(templ.proto, i);
	ret.address = tsubst(templ.address, i);
	ret.port = tsubst(templ.port, i);
	return ret;
}
