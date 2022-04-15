// g++ -std=c++14 main.cpp -lpthread
#include <thread>
#include <condition_variable>
#include <iostream>

using namespace std::literals::chrono_literals;

/*
	State change diagram
	WaitingBoth : waiting A or AAAA after requesting A and AAAA
	WaitingBoth -> Ipv6 : if AAAA is received
	WaitingBoth -> WaitingIpv6 : if A is received
	WaitingIpv6 -> Ipv6 : if AAAA is received
	WaitingIpv6 -> Ipv4 : if timeout
*/
enum class State {
	WaitingBoth,
	WaitingIpv6,
	Ipv4 = 4,
	Ipv6 = 6,
};

struct Notification {
	std::mutex m;
	State status = State::WaitingBoth;
	int addr = 0;
	std::condition_variable cv;
};

void ipv4proc(Notification& notif, const std::chrono::milliseconds& waitTime)
{
	puts("request A");
	std::this_thread::sleep_for(waitTime);
	{
		std::lock_guard<std::mutex> lk(notif.m);
		if (notif.status == State::WaitingBoth) {
			notif.addr = 1234;
			notif.status = State::WaitingIpv6;
		}
	}
	puts("receive A");
	notif.cv.notify_one();
}

void ipv6proc(Notification& notif, const std::chrono::milliseconds& waitTime)
{
	puts("request AAAA");
	std::this_thread::sleep_for(waitTime);
	{
		std::lock_guard<std::mutex> lk(notif.m);
		if (notif.status == State::WaitingBoth || notif.status == State::WaitingIpv6) {
			notif.addr = 5678;
			notif.status = State::Ipv6;
		}
	}
	puts("receive AAAA");
	notif.cv.notify_one();
}

void happyEyeball2(const std::chrono::milliseconds waitTime1, const std::chrono::milliseconds waitTime2)
{
	std::cout << "test happyEyeball2 time:" << waitTime1.count() << "ms " << waitTime2.count() << "ms" << std::endl;
	Notification notif;
	// request A and AAAA
	std::thread t1(ipv4proc, std::ref(notif), waitTime1);
	std::thread t2(ipv6proc, std::ref(notif), waitTime2);

	// waiting both
	{
		std::unique_lock<std::mutex> lk(notif.m);
		notif.cv.wait(lk, [&]{ return notif.status != State::WaitingBoth; });
	}
	// A is received so wait 50ms
	if (notif.status == State::WaitingIpv6) {
		const auto waitIpv6Time = 50ms;
		std::cout << "wait ipv6 " << waitIpv6Time.count() << "ms" << std::endl;
		{
			std::unique_lock<std::mutex> lk(notif.m);
			if (notif.cv.wait_for(lk, waitIpv6Time, [&]{ return notif.status == State::Ipv6; })) {
				puts("receive ipv6");
			} else {
				puts("timeout");
				notif.status = State::Ipv4;
			}
		}
	}
	switch (notif.status) {
	case State::Ipv4: puts("ipv4 syn"); break;
	case State::Ipv6: puts("ipv6 syn"); break;
	default: puts("ERR"); break;
	}
	printf("addr=%d\n", notif.addr);
	t1.join();
	t2.join();
	puts("------------");
}

int main()
{
	puts("case 1. AAAA is fast so send ipv6 syn");
	happyEyeball2(2000ms, 1000ms);
	puts("case 2. A is fast so send ipv4 syn");
	happyEyeball2(1000ms, 2000ms);
	puts("case 2. A is a little fast so wait AAAA and send ipv6 syn");
	happyEyeball2(1000ms, 1010ms);
}
