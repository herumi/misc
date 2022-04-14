#include <thread>
#include <condition_variable>

using namespace std::literals::chrono_literals;

enum class Status {
	Start,
	Waiting,
	Ipv4 = 4,
	Ipv6 = 6,
};

struct Notification {
	std::mutex m;
	Status status = Status::Start;
	int addr = 0;
	std::condition_variable cv;
};

int ipv4proc(Notification& notif, const std::chrono::milliseconds& waitTime)
{
	puts("request A");
	std::this_thread::sleep_for(waitTime);
	puts("receive A");
	notif.cv.notify_one();
	{
		std::lock_guard<std::mutex> lk(notif.m);
		if (notif.status == Status::Start) {
			notif.addr = 1234;
			notif.status = Status::Waiting;
		}
	}
	notif.cv.notify_one();
	return 4;
}

int ipv6proc(Notification& notif, const std::chrono::milliseconds& waitTime)
{
	puts("request AAAA");
	std::this_thread::sleep_for(waitTime);
	puts("receive AAAA");
	notif.cv.notify_one();
	{
		std::lock_guard<std::mutex> lk(notif.m);
		if (notif.status == Status::Start || notif.status == Status::Waiting) {
			notif.addr = 5678;
			notif.status = Status::Ipv6;
		}
	}
	notif.cv.notify_one();
	return 6;
}

void happyEyeball2(const std::chrono::milliseconds waitTime1, const std::chrono::milliseconds waitTime2)
{
	Notification notif;
	std::thread t1(ipv4proc, std::ref(notif), waitTime1);
	std::thread t2(ipv6proc, std::ref(notif), waitTime2);

	{
		std::unique_lock<std::mutex> lk(notif.m);
		notif.cv.wait(lk, [&]{ return notif.status != Status::Start; });
	}
	if (notif.status == Status::Waiting) {
		puts("wait ipv6");
		{
			std::unique_lock<std::mutex> lk(notif.m);
			notif.cv.wait_for(lk, 1000ms, [&]{ return notif.status != Status::Ipv6; });
		}
		{
			std::unique_lock<std::mutex> lk(notif.m);
			if (notif.status == Status::Waiting) {
				puts("timeout");
				notif.status = Status::Ipv4;
			}
		}
	}
	printf("ipv%d addr=%d\n", (int)notif.status, notif.addr);
	if (notif.status == Status::Ipv4) {
		puts("send ipv4 syn");
	} else {
		puts("send ipv6 syn");
	}
	t1.join();
	t2.join();
	puts("------------");
}

int main()
{
	puts("case 1. AAAA is fast");
	happyEyeball2(2000ms, 1000ms);
	puts("case 2. A is fast");
	happyEyeball2(1000ms, 2000ms);
	puts("case 2. A is a little fast");
	happyEyeball2(1000ms, 1500ms);
}
