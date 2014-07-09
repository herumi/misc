/*
	packet repeater
*/
#include <cybozu/option.hpp>
#include <cybozu/socket.hpp>
#include <thread>
#include <atomic>
#include <chrono>
#include <memory>
#include "sma.hpp"

std::atomic<int> g_quit;
std::atomic<bool> g_stop;

struct Option {
	std::string serverAddr;
	uint16_t serverPort;
	uint16_t recvPort;
	uint16_t cmdPort;
	uint32_t delaySec;
	uint32_t rateMbps;
	size_t threadNum;
	bool verbose;
	Option(int argc, char *argv[])
		: serverPort(0)
		, cmdPort(0)
		, delaySec(0)
		, rateMbps(0)
		, threadNum(0)
		, verbose(false)
	{
		cybozu::Option opt;
		opt.appendParam(&serverAddr, "server", ": server address");
		opt.appendParam(&serverPort, "port", ": server port");
		opt.appendParam(&recvPort, "recvPort", ": port to receive");
		opt.appendParam(&cmdPort, "cmdPort", ": port for command");
		opt.appendOpt(&delaySec, 0, "d", ": delay second");
		opt.appendOpt(&rateMbps, 0, "r", ": data rate(mega bit per second)");
		opt.appendOpt(&threadNum, 5, "t", ": num of thread");
		opt.appendBoolOpt(&verbose, "v", ": verbose message");
		opt.appendHelp("h");
		if (!opt.parse(argc, argv)) {
			opt.usage();
			exit(1);
		}
		opt.put();
	}
};

void cmdThread(const Option& opt)
	try
{
	if (opt.verbose) puts("cmdThread start");
	cybozu::Socket server;
	server.bind(opt.cmdPort);
	while (!g_quit) {
		while (!server.queryAccept()) {
			if (g_quit) break;
		}
		if (g_quit) break;
		cybozu::Socket client;
		server.accept(client);
		char buf[128];
		const size_t readSize = server.readSome(buf, sizeof(buf));
		if (readSize > 0) {
			const std::string cmd(buf, readSize);
			if (opt.verbose) printf("cmd=%s\n", cmd.c_str());
			if (cmd == "quit") {
				g_quit = true;
			} else
			if (cmd == "stop") {
				g_stop = true;
			} else
			if (cmd == "start") {
				g_stop = false;
			}
		}
	}
	if (opt.verbose) puts("cmdThread stop");
} catch (std::exception& e) {
	printf("ERR cmdThread %s\n", e.what());
}

void waitMsec(int msec)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(msec));
}

struct Repeater {
	cybozu::Socket s_[2]; // s_[0] : client, s_[1] : server
	enum {
		Sleep = 0,
		Ready = 1,
		Open1sock = 2,
		Open2sock = 3,
	};
	const Option& opt_;
	std::atomic<int> state_;
	std::thread c2s_;
	std::thread s2c_;
	std::exception_ptr ep_;
	void loop(int dir)
	{
		if (opt_.verbose) printf("thread loop %d start\n", dir);
		assert(dir == 0 || dir == 1);
		cybozu::Socket &from = s_[dir];
		cybozu::Socket &to = s_[1 - dir];
		const bool needShutdown = dir == 1;
		while (!g_quit) {
			if (state_ >= Open1sock && from.isValid()) {
				try {
					while (!from.queryAccept()) {
					}
					if (g_quit) break;
					char buf[4096];
					const size_t readSize = from.readSome(buf, sizeof(buf));
					if (opt_.verbose) printf("readSize %d [%d] state=%d\n", (int)readSize, dir, (int)state_);
					if (readSize > 0) {
						if (to.isValid()) to.write(buf, readSize);
						continue;
					}
				} catch (std::exception& e) {
					printf("ERR Repeater %s\n", e.what());
				}
				if (needShutdown) to.waitForClose();
				from.close();
				state_--;
				assert(state_ == Ready || state_ == Open1sock);
				if (opt_.verbose) printf("close [%d] state=%d\n", dir, (int)state_);
				if (state_ == Ready) state_ = Sleep;
			} else {
				waitMsec(100);
			}
		}
		if (opt_.verbose) printf("thread loop %d end\n", dir);
	}
	int getState() const { return state_; }
public:
	Repeater(const Option& opt)
		: opt_(opt)
		, state_(Sleep)
		, c2s_(&Repeater::loop, this, 0)
		, s2c_(&Repeater::loop, this, 1)
	{
	}
	bool tryAndRun(cybozu::Socket& client)
	{
		int expected = Sleep;
		if (!state_.compare_exchange_strong(expected, Ready)) return false;
		if (opt_.verbose) puts("set socket");
		s_[0].moveFrom(client);
		s_[1].connect(opt_.serverAddr, opt_.serverPort);
		state_ = Open2sock;
		return true;
	}
	void join()
	{
		c2s_.join();
		s2c_.join();
		if (ep_) {
			std::rethrow_exception(ep_);
		}
	}
};

int main(int argc, char *argv[])
{
	const Option opt(argc, argv);
	cybozu::Socket server;
	server.bind(opt.recvPort);
	std::thread cmdWorker(cmdThread, opt);
	std::vector<std::unique_ptr<Repeater>> worker;
	try {
		for (size_t i = 0; i < opt.threadNum; i++) {
			worker.emplace_back(new Repeater(opt));
		}
		while (!g_quit) {
	RETRY:
			while (!server.queryAccept()) {
#if 0
				if (opt.verbose) {
					printf("worker state ");
					for (size_t i = 0; i < opt.threadNum; i++) {
						printf("%d ", worker[i]->getState());
					}
					printf("\n");
				}
#endif
			}
			if (g_quit) break;
			cybozu::SocketAddr addr;
			cybozu::Socket client;
			server.accept(client, &addr);
			if (opt.verbose) printf("accept addr %s\n", addr.toStr().c_str());
			while (!g_quit) {
				for (size_t i = 0; i < opt.threadNum; i++) {
					if (opt.verbose) printf("worker[%d] state=%d\n", (int)i, worker[i]->getState());
					if (worker[i]->tryAndRun(client)) {
						if (opt.verbose) printf("start %d repeater\n", (int)i);
						goto RETRY;
					}
				}
				waitMsec(100);
			}
			waitMsec(100);
		}
	} catch (std::exception& e) {
		printf("ERR %s\n", e.what());
	}
	g_quit = true;
	cmdWorker.join();
	for (std::unique_ptr<Repeater>& p : worker) {
		p->join();
	}
	if (opt.verbose) puts("main end");
}
