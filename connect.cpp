#include <cybozu/option.hpp>
#include <cybozu/socket.hpp>

int main(int argc, char *argv[])
	try
{
	bool isServer;
	std::string ip;
	uint16_t port;
	std::string cmd;
	bool verbose = false;

	cybozu::Option opt;
	opt.appendOpt(&isServer, false, "s", "server");
	opt.appendOpt(&ip, "", "ip", "ip address");
	opt.appendBoolOpt(&verbose, "v", "verbose");
	opt.appendMust(&port, "p", "port");
	opt.appendParamOpt(&cmd, "cmd", "string to send");

	if (!opt.parse(argc, argv)) {
		opt.usage();
		return 1;
	}
	if (isServer) {
		printf("server port=%d\n", port);
		cybozu::Socket server;
		server.bind(port);
		for (;;) {
			while (!server.queryAccept()) {
			}
			cybozu::Socket client;
			if (verbose) {
				cybozu::SocketAddr addr;
				server.accept(client, &addr);
				printf("addr=%s\n", addr.toStr().c_str());
			} else {
				server.accept(client);
			}
			{
				char buf[128];
				size_t readSize = client.readSome(buf, sizeof(buf));
				printf("rec=%s\n", std::string(buf, readSize).c_str());
			}
		}
	} else {
		printf("client ip=%s port=%d\n", ip.c_str(), port);
		cybozu::Socket client;
		client.connect(ip, port);
		client.write(cmd.c_str(), cmd.size());
	}
} catch (std::exception& e) {
	printf("ERR %s\n", e.what());
}
