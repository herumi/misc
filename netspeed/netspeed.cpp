#include <cybozu/socket.hpp>
#include <cybozu/serializer.hpp>
#include <cybozu/option.hpp>
#include <cybozu/time.hpp>

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
void clientProcess(Stream& soc)
{
	std::string buf;
	const size_t N = 10;
	for (size_t i = 0; i < N; i++) {
		size_t bufSize = size_t(32 * 3) << i;
		size_t n = 1 << 16;
		buf.resize(bufSize);
		printf("bufSize=%zd, n=%zd\n", bufSize, n);
		double begin = cybozu::GetCurrentTimeSec();
		cybozu::save(soc, bufSize);
		cybozu::save(soc, n);
		for (size_t j = 0; j < n; j++) {
			soc.write(buf.data(), bufSize);
		}
		int ok;
		cybozu::load(ok, soc);
		double t = cybozu::GetCurrentTimeSec() - begin;
		printf("send %.2f sec %3.f Mbps\n", t, bufSize * n * 8 / t / 1024 / 1024);
	}
	cybozu::save(soc, 0);
}

template<class Stream>
void serverProcess(Stream& soc)
{
	std::string buf;
	for (;;) {
		size_t bufSize;
		size_t n;
		cybozu::load(bufSize, soc);
		if (bufSize == 0) break;
		buf.resize(bufSize);
		cybozu::load(n, soc);
		for (size_t i = 0; i < n; i++) {
			soc.read(&buf[0], bufSize);
		}
		int ok = 1;
		cybozu::save(soc, ok);
	}
}

int main(int argc, char *argv[])
	try
{
	cybozu::Option opt;
	std::string ip;
	int port;
	opt.appendOpt(&ip, "", "ip", ": ip address");
	opt.appendOpt(&port, 10000, "p", ": port");
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
		cybozu::SocketAddr sa(ip, uint16_t(port));
		printf("addr=%s\n", sa.toStr().c_str());
		cybozu::Socket client;
		client.connect(sa);
		clientProcess(client);
	}
} catch (std::exception& e) {
	printf("err %s\n", e.what());
	return 1;
}
