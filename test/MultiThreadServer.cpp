#include <TcpServer.h>
#include <Thread.h>
#include <EventLoop.h>
#include <EventLoopPool.h>

using namespace moxie;
extern bool hookable;

void LoopThreadFunc() {
    EventLoop *loop = new (std::nothrow) EventLoop();
    if (nullptr == loop) {
        return;
    }
    if (!EventLoopPool::AddEventLoop(loop)) {
        return;
    }
    loop->loop();
}

int main() {
    EventLoop *mainloop = new EventLoop;
    EventLoopPool::addMainLoop(mainloop);
    Thread thread0(LoopThreadFunc);

    thread0.start();
    
    NetAddress addr(AF_INET, 6686, "127.0.0.1");
    TcpServer server(addr);

    server.start();

    hookable = true;
    mainloop->loop();
    return 0;
}
