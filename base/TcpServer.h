#ifndef MOXIE_TCPSERVER_H
#define MOXIE_TCPSERVER_H
#include <memory>
#include <map>
#include <vector>
#include <queue>

#include <Socket.h>
#include <Events.h>
#include <EventLoop.h>
#include <NetAddress.h>
#include <SigIgnore.h>

namespace moxie {

class TcpServer {
public:
    TcpServer(const NetAddress& addr);
    ~TcpServer();

    bool start();
    void accept();
private:
    EventLoop *loop_;
    Socket server_;
    boost::shared_ptr<Events> events_;
    NetAddress addr_;
    const uint listenBacklog_;
};

}
#endif // MOXIE_TCPSERVER_H
