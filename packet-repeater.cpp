/*
	packet repeater
*/
#include <cybozu/option.hpp>
#include <cybozu/socket.hpp>
#include <cybozu/log.hpp>
#include <thread>
#include <atomic>
#include <chrono>
#include <memory>
#include <time.h>
#include "sma.hpp"

std::atomic<int> g_quit;
std::atomic<bool> g_stop;

struct Option {
	std::string serverAddr;
	uint16_t serverPort;
	uint16_t recvPort;
	uint16_t cmdPort;
	uint32_t delaySec;
	double rateMbps;
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
	if (opt.verbose) cybozu::PutLog(cybozu::LogInfo, "cmdThread start port=%d", opt.cmdPort);
	cybozu::Socket server;
	server.bind(opt.cmdPort);
	while (!g_quit) {
		while (!server.queryAccept()) {
			if (g_quit) break;
		}
		if (g_quit) break;
		try {
			cybozu::SocketAddr addr;
			cybozu::Socket client;
			server.accept(client, &addr);
			if (opt.verbose) cybozu::PutLog(cybozu::LogInfo, "cmdThread accept addr %s", addr.toStr().c_str());
			char buf[128];
			size_t readSize = client.readSome(buf, sizeof(buf));
			if (readSize > 0) {
				if (buf[readSize - 1] == '\n') readSize--;
				if (readSize > 0 && buf[readSize - 1] == '\r') readSize--;
				const std::string cmd(buf, readSize);
				if (cmd == "quit") {
					if (opt.verbose) cybozu::PutLog(cybozu::LogInfo, "cmdThread quit");
					g_quit = true;
				} else
				if (cmd == "stop") {
					if (opt.verbose) cybozu::PutLog(cybozu::LogInfo, "cmdThread stop");
					g_stop = true;
				} else
				if (cmd == "start") {
					if (opt.verbose) cybozu::PutLog(cybozu::LogInfo, "cmdThread start");
					g_stop = false;
				} else
				{
					if (opt.verbose) cybozu::PutLog(cybozu::LogInfo, "bad command `%s'", cmd.c_str());
				}
			}
		} catch (std::exception& e) {
			cybozu::PutLog(cybozu::LogInfo, "cmdThread ERR %s (continue)", e.what());
		}
	}
	if (opt.verbose) cybozu::PutLog(cybozu::LogInfo, "cmdThread stop");
} catch (std::exception& e) {
	cybozu::PutLog(cybozu::LogInfo, "cmdThread ERR %s", e.what());
}

void waitMsec(int msec)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(msec));
}

class Repeater {
	cybozu::Socket s_[2]; // s_[0] : client, s_[1] : server
	enum {
		Sleep,
		Ready,
		Running,
		Error
	};
	const Option& opt_;
	std::atomic<int> state_;
	std::thread c2s_;
	std::thread s2c_;
	std::exception_ptr ep_;
	uint64_t getCurTimeSec() const
	{
		return (uint64_t)::time(0);
	}
	void loop(int dir)
		try
	{
		if (opt_.verbose) cybozu::PutLog(cybozu::LogInfo, "loop %d start", dir);
		assert(dir == 0 || dir == 1);
		cybozu::Socket &from = s_[dir];
		cybozu::Socket &to = s_[1 - dir];
		const bool needShutdown = dir == 1;
		const int intervalSec = 3;
		SMAverage sma(intervalSec);
		std::vector<char> buf(12 * 1024);
		while (!g_quit) {
			switch ((int)state_) {
			case Sleep:
				waitMsec(10);
				continue;
			case Ready:
				waitMsec(1);
				continue;
			case Error:
				if (!to.isValid()) {
					if (opt_.verbose) cybozu::PutLog(cybozu::LogInfo, "loop %d %d toSleep", dir, (int)state_);
					state_ = Sleep;
				} else {
					waitMsec(1);
				}
				break;
			case Running:
				if (!from.isValid()) {
					cybozu::PutLog(cybozu::LogInfo, "loop %d %d from is not valid", dir, (int)state_);
					state_ = Error;
					continue;
				}
				try {
					while (!from.queryAccept()) {
					}
					if (g_quit) break;
					const size_t readSize = from.readSome(buf.data(), buf.size());
					if (opt_.verbose) cybozu::PutLog(cybozu::LogInfo, "loop %d %d readSize %d", dir, (int)state_, (int)readSize);
					if (opt_.rateMbps > 0) {
						sma.append(readSize, getCurTimeSec());
						while (sma.getBps(getCurTimeSec()) * 1e-6 > opt_.rateMbps) {
							waitMsec(10);
						}
					}
					if (readSize > 0) {
						if (!g_stop && to.isValid()) to.write(buf.data(), readSize);
						continue;
					}
				} catch (std::exception& e) {
					cybozu::PutLog(cybozu::LogInfo, "loop %d %d ERR %s", dir, (int)state_, e.what());
					state_ = Error;
					from.close();
					continue;
				}
				if (needShutdown) {
					if (opt_.verbose) cybozu::PutLog(cybozu::LogInfo,"loop %d %d shutdown", dir, (int)state_);
					const bool dontThrow = true;
					to.shutdown(1, dontThrow); // write disallow
				}
				if (opt_.verbose) cybozu::PutLog(cybozu::LogInfo, "loop %d %d close", dir, (int)state_);
				from.close();
				if (opt_.verbose) cybozu::PutLog(cybozu::LogInfo, "loop %d %d isValid %d", dir, (int)state_, from.isValid());
				break;
			}
		}
		if (opt_.verbose) cybozu::PutLog(cybozu::LogInfo, "loop %d end", dir);
	} catch (...) {
		ep_ = std::current_exception();
		s_[0].close();
		s_[1].close();
		state_ = Sleep;
	}
public:
	int getState() const { return state_; }
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
		if (opt_.verbose) cybozu::PutLog(cybozu::LogInfo, "tryAndRun:in");
		try {
			s_[0].moveFrom(client);
			s_[1].connect(opt_.serverAddr, opt_.serverPort);
			state_ = Running;
			return true;
		} catch (std::exception& e) {
			cybozu::PutLog(cybozu::LogInfo, "tryAndRun::connect err %s", e.what());
			s_[0].close();
			s_[1].close();
			state_ = Sleep;
			waitMsec(100);
			return true;
		}
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
			if (opt.verbose) fprintf(stderr, "accept addr %s\n", addr.toStr().c_str());
			while (!g_quit) {
				for (size_t i = 0; i < opt.threadNum; i++) {
//					if (opt.verbose) fprintf(stderr, "worker[%d] state=%d", (int)i, worker[i]->getState());
					if (worker[i]->tryAndRun(client)) {
						if (opt.verbose) fprintf(stderr, "start %d repeater", (int)i);
						goto RETRY;
					}
				}
				waitMsec(100);
			}
			waitMsec(100);
		}
	} catch (std::exception& e) {
		fprintf(stderr, "ERR %s\n", e.what());
	}
	g_quit = true;
	for (std::unique_ptr<Repeater>& p : worker) {
		p->join();
	}
	cmdWorker.join();
	if (opt.verbose) puts("main end");
}
