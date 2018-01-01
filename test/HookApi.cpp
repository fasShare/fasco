#include <EventLoop.h>
#include <EventLoopPool.h>
#include <Continuation.h>
#include <NetAddress.h>
#include <Mcosops.h>
#include <Socket.h>

using namespace moxie;

void accept_call() {
    Socket sock(AF_INET, SOCK_STREAM, 0);
    NetAddress addr(AF_INET, 6686, "127.0.0.1");
    sock.bind(addr);
    sock.listen(10);
    int i = 0;
    while (i++ < 5) {
        NetAddress addr_tmp;
        int fd = sock.accept(addr_tmp, false);
        char buf[1024];
        int ret = read(fd, buf, 1023);
        if (ret > 0) {
            buf[ret] = 0;
            printf("recv:%s\n", buf);
        }
        std::cout << "i = " << i << std::endl;
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

