#include <EventLoop.h>
#include <EventLoopPool.h>
#include <Continuation.h>
#include <NetAddress.h>
#include <Mcosops.h>
#include <Socket.h>
#include <Events.h>
#include <Eventsops.h>
#include <boost/make_shared.hpp>

using namespace moxie;

void client_call(int fd) {
	while (true) {
		char buf[1024];
		int ret = read(fd, buf, 1023);
		if (ret > 0) {
			buf[ret] = 0;
			printf("recv:%s\n", buf);
		} else {
            printf("read error:%s\n", strerror(errno));
            close(fd);
            break;
        }
	}
    printf("exit from client_call.");
}

void accept_call() {
	Socket sock(AF_INET, SOCK_STREAM, 0);
	NetAddress addr(AF_INET, 6686, "127.0.0.1");
	sock.bind(addr);
	sock.listen(10);
	while (true) {
		NetAddress addr_tmp;
		int fd = sock.accept(addr_tmp, false);
		if (fd > 0) {
			boost::shared_ptr<Continuation> co(new Continuation(boost::bind(client_call, fd)));
			McoPool::SetMcoRoutine(fd, co);
			auto event = boost::make_shared<Events>(fd, kReadEvent);
			event->setTid(gettid());
			Eventsops::UpdateLoopEvents(event);
            printf("in accept_call co use_count=%ld\n", co.use_count());
		}
    }
}

int main () {
    EventLoop *loop = new EventLoop;
    EventLoopPool::AddEventLoop(loop);
    
    boost::shared_ptr<Continuation> co(new Continuation(accept_call));
    
    Mcosops::Resume(co);

    loop->loop();
    return 0;
}

