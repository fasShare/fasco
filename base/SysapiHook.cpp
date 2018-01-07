#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <boost/make_shared.hpp>

#include <Eventsops.h>
#include <McoPool.h>
#include <Mcosops.h>

#define gettid() (::syscall(SYS_gettid))

using namespace moxie;
bool hookable = false;
using read_hook = ssize_t (*)(int fildes, void *buf, size_t nbyte);
using accept_hook = int (*)(int fd, struct sockaddr *addr, socklen_t *len);
using close_hook = int (*)(int fd);

static read_hook  uni_read = (read_hook)dlsym(RTLD_NEXT, "read");
static accept_hook uni_accept = (accept_hook)dlsym(RTLD_NEXT, "accept");
static close_hook uni_close = (close_hook)dlsym(RTLD_NEXT, "close");

ssize_t read(int fd, void *buf, size_t nbyte) {
    int flag = fcntl(fd, F_GETFL);
    if ((O_NONBLOCK & flag) || (!hookable)) {
        ssize_t ret = uni_read(fd, buf, nbyte);
        return ret;
    }
    auto mco = McoPool::GetMcoRoutine(fd);
    if (mco != nullptr) {
        assert(mco == McoPool::GetCurMco());
        auto event = Eventsops::GetEvents(fd);
        if (!event) {
            event = boost::make_shared<Events>(fd, kReadEvent);
        }
        event->updateEvents(kReadEvent);
        Eventsops::UpdateLoopEvents(event);
        Mcosops::Yield();    
        event->deleteEvent(kReadEvent);
        Eventsops::UpdateLoopEvents(event);
    } else {
        mco = McoPool::GetCurMco();
        if (mco == nullptr) {
            ssize_t ret = uni_read(fd, buf, nbyte);
            return ret;
        }
        McoPool::SetMcoRoutine(fd, mco);
        auto event = boost::make_shared<Events>(fd, kReadEvent);
        Eventsops::UpdateLoopEvents(event);
        Mcosops::Yield();    
        event->deleteEvent(kReadEvent);
        Eventsops::UpdateLoopEvents(event);
    }
        
	return uni_read(fd, buf, nbyte);
}

int accept(int fd, struct sockaddr *addr, socklen_t *len) {
    int flag = fcntl(fd, F_GETFL);
    if ((O_NONBLOCK & flag) || (!hookable)) {
        int ret = uni_accept(fd, addr, len);
        return ret;
    }

    auto mco = McoPool::GetMcoRoutine(fd);
    if (mco != nullptr) {
        assert(mco == McoPool::GetCurMco());
        auto event = Eventsops::GetEvents(fd);
        if (!event) {
            event = boost::make_shared<Events>(fd, kReadEvent);
        }
        event->updateEvents(kReadEvent);
        Eventsops::UpdateLoopEvents(event);

        Mcosops::Yield();
        event->deleteEvent(kReadEvent);
        Eventsops::UpdateLoopEvents(event);
    } else {
        mco = McoPool::GetCurMco();
        if (mco == nullptr) {
            int ret = uni_accept(fd, addr, len);
            fcntl(fd, F_SETFL, flag);
            return ret;
        }
        McoPool::SetMcoRoutine(fd, mco);
        auto event = boost::make_shared<Events>(fd, kReadEvent);
        Eventsops::UpdateLoopEvents(event);
        Mcosops::Yield();
        event->deleteEvent(kReadEvent);
        Eventsops::UpdateLoopEvents(event);
    }

    int ret = uni_accept(fd, addr, len);
    return ret;
}

int close(int fd) {
    if (!hookable) {
        return uni_close(fd);
    }
    auto mcopool = McoPool::GetMcoPool();
    auto cur = mcopool->getCurMco();
    auto fdmco = mcopool->getMcoRoutine(fd);
    if (cur == fdmco) {
        mcopool->setCurMco(nullptr);
    }
    mcopool->removeMcoRoutine(fd);
	auto event = Eventsops::GetEvents(fd);
	if (event) {
        LOGGER_TRACE("RemoveEventFromLoop in close, fd:" << event->getFd());
		Eventsops::RemoveEventFromLoop(event);
	} else {
		return uni_close(fd);
	}
	return 0;
}
