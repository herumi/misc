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
void clientProcess(Stream& soc, int ds, int n)
{
	cybozu::save(soc, ds);
	cybozu::save(soc, n);
	std::vector<char> bufW(ds), bufR(ds);

	std::thread reader([&] {
		readProc(soc, bufR, n);
	});
	std::thread writer([&] {
		writeProc(soc, bufW, n);
	});

	reader.join();
	writer.join();
}

template<class Stream>
void serverProcess(Stream& soc)
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
		writeProc(soc, bufW, n);
	});
	reader.join();
	writer.join();
}

int main(int argc, char *argv[])
	try
{
	cybozu::Option opt;
	std::string ip;
	int port;
	int ds;
	int n;
	opt.appendOpt(&ip, "", "ip", ": ip address");
	opt.appendOpt(&port, 10000, "p", ": port");
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
		server.bind(uint16_t(port));
		for (;;) {
			while (!server.queryAccept()) {
			}
			cybozu::Socket client;
			server.accept(client);
			serverProcess(client);
		}
	} else {
		printf("client ip=%s port=%d\n", ip.c_str(), port);
		printf("ds=%d, n=%d\n", ds, n);
		cybozu::SocketAddr sa(ip, uint16_t(port));
		printf("addr=%s\n", sa.toStr().c_str());
		cybozu::Socket client;
		client.connect(sa);
		clientProcess(client, ds, n);
	}
} catch (std::exception& e) {
	printf("err %s\n", e.what());
	return 1;
}
