#include <iostream>
#include <memory>
#include <unistd.h>
#include <new>

#include <boost/bind.hpp>

#include <Socket.h>
#include <Log.h>
#include <EventLoop.h>
#include <TcpServer.h>
#include <EventLoopPool.h>
#include <McoPool.h>
#include <Eventsops.h>

moxie::TcpServer::TcpServer(const NetAddress& addr) :
    loop_(nullptr),
    server_(AF_INET, SOCK_STREAM, 0),
    events_(new Events(server_.getSocket(), kReadEvent)),
    addr_(addr),
    listenBacklog_(50) {
    assert(server_.bind(addr_));
    assert(server_.listen(listenBacklog_));
	LOGGER_TRACE("server listen fd = " << server_.getSocket());
}

bool moxie::TcpServer::start() { 
	loop_ = EventLoopPool::GetMainLoop();
	if (!loop_) {
		LOGGER_ERROR("Please check MoxieInit() was called!");
		return false;
	}
    boost::shared_ptr<Continuation> co(new Continuation(boost::bind(&TcpServer::accept, this)));
    McoPool::SetMcoRoutine(server_.getSocket(), co);
    loop_->updateEvents(events_);
    return true;
}

void client_call(int fd) {
    while (true) {
        LOGGER_TRACE("==================Begin read========================");
        char buf[1024];
        int ret = read(fd, buf, 1023);
        if (ret > 0) {
            buf[ret] = 0;
            LOGGER_TRACE("recv:"<< buf);
        } else {
            LOGGER_TRACE("read error:"<< strerror(errno) << ", ret:" << ret);
            close(fd);
            break;
        }
        LOGGER_TRACE("==================After read========================");
    }   
    LOGGER_TRACE("exit from client_call.");
}


void moxie::TcpServer::accept() {
    LOGGER_TRACE("In TcpServer AcceptCall");
    loop_->assertInOwnerThread();
    while (true) {
        //NetAddress addr;
        //int fd = server_.accept(addr);
        int fd = ::accept(server_.getSocket(), nullptr, nullptr);
        if (fd > 0) {
            auto loop = EventLoopPool::GetNextLoop();
            auto event = boost::make_shared<Events>(fd, kReadEvent);
            long tid = loop->getTid();
            boost::shared_ptr<Continuation> co(new Continuation(boost::bind(client_call, fd)));
            McoPool::SetMcoRoutine(fd, co, tid);
            event->setTid(tid);
            loop->updateEvents(event);
        }
    }
}

moxie::TcpServer::~TcpServer() {
    loop_->quit();
    LOGGER_TRACE("TcpServer will be destroyed in process " << getpid());
}
