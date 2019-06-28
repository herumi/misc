#include <cybozu/socket.hpp>
#include <cybozu/serializer.hpp>
#include <cybozu/option.hpp>
#include <cybozu/time.hpp>
#include <thread>
#include <vector>
#include <atomic>

std::atomic_uint64_t readS{};
std::atomic_uint64_t writeS{};

struct Timer {
	const char *msg_;
	double begin_;
	void begin(const char *msg)
	{
		msg_ = msg;
		begin_ = cybozu::GetCurrentTimeSec();
	}
	void end() const
	{
		printf("%s %.2f sec\n", msg_, cybozu::GetCurrentTimeSec() - begin_);
	}
};


template<class Stream>
void readProc(Stream& soc, std::vector<char>& buf, int n)
{
	for (int i = 0; i < n; i++) {
		soc.read(buf.data(), buf.size());
		readS += buf.size();
		if ((i % 1000) == 0) {
			printf("readS=%d, writeS=%d\r", (int)readS, (int)writeS);
		}
	}
}

template<class Stream>
void writeProc(Stream& soc, std::vector<char>& buf, int n)
{
	for (int i = 0; i < n; i++) {
		soc.write(buf.data(), buf.size());
		writeS += buf.size();
		if ((i % 1000) == 0) {
			printf("readS=%d, writeS=%d\r", (int)readS, (int)writeS);
		}
	}
}

template<class Stream>
void clientProcess(Stream& soc, int ds, int n, Stream *soc2)
{
	cybozu::save(soc, ds);
	cybozu::save(soc, n);
	std::vector<char> bufW(ds), bufR(ds);

	std::thread reader([&] {
		readProc(*soc2, bufR, n);
	});
	std::thread writer([&] {
		writeProc(soc, bufW, n);
	});

	reader.join();
	writer.join();
	printf("readS=%d, writeS=%d\n", (int)readS, (int)writeS);
}

template<class Stream>
void serverProcess(Stream& soc, Stream *soc2)
{
	int ds, n;
	cybozu::load(ds, soc);
	cybozu::load(n, soc);
	printf("ds=%d, n=%d\n", ds, n);

	std::vector<char> bufW(ds), bufR(ds);

	std::thread reader([&] {
		readProc(soc, bufR, n);
	});
	std::thread writer([&] {
		writeProc(*soc2, bufW, n);
	});
	reader.join();
	writer.join();
}

int main(int argc, char *argv[])
	try
{
	cybozu::Option opt;
	std::string ip;
	uint16_t port;
	uint16_t port2;
	int ds;
	int n;
	opt.appendOpt(&ip, "", "ip", ": ip address");
	opt.appendOpt(&port, 10000, "p", ": port");
	opt.appendOpt(&port2, 0, "p2", ": 2nd port");
	opt.appendOpt(&ds, 64 * 3 * 2 * 4, "ds", ": data size");
	opt.appendOpt(&n, 1024, "n", ": num");
	opt.appendHelp("h", "show this message");
	if (!opt.parse(argc, argv)) {
		opt.usage();
		return 1;
	}

	if (ip.empty()) {
		printf("server port=%d\n", port);
		cybozu::Socket server;
		server.bind(port);
		for (;;) {
			while (!server.queryAccept()) {
			}
			cybozu::Socket client;
			server.accept(client);
#if 1
			cybozu::Socket server2;
			cybozu::Socket client2;
			if (port2) {
				server2.bind(port2);
				while (!server2.queryAccept()) {
				}
				printf("port2=%d\n", port2);
				server2.accept(client2);
			}
#endif
			serverProcess(client, port2 > 0 ? &client2 : &client);
		}
	} else {
		printf("client ip=%s port=%d\n", ip.c_str(), port);
		printf("ds=%d, n=%d\n", ds, n);
		cybozu::SocketAddr sa(ip, port);
		printf("addr=%s\n", sa.toStr().c_str());
		cybozu::Socket client;
		client.connect(sa);
		cybozu::Socket client2;
		if (port2) {
			printf("port2=%d\n", port2);
			client2.connect(ip, port2);
		}
		clientProcess(client, ds, n, port2 > 0 ? &client2 : &client);
	}
} catch (std::exception& e) {
	printf("err %s\n", e.what());
	return 1;
}
