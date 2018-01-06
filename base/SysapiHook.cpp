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
using read_hook = ssize_t (*)(int fildes, void *buf, size_t nbyte);
using accept_hook = int (*)(int fd, struct sockaddr *addr, socklen_t *len);
using close_hook = int (*)(int fd);

static read_hook  uni_read = (read_hook)dlsym(RTLD_NEXT, "read");
static accept_hook uni_accept = (accept_hook)dlsym(RTLD_NEXT, "accept");
static close_hook uni_close = (close_hook)dlsym(RTLD_NEXT, "close");

ssize_t read(int fd, void *buf, size_t nbyte) {
    printf("In hook read.");
    int flag = fcntl(fd, F_GETFL);
    if (O_NONBLOCK & flag) {
        ssize_t ret = uni_read(fd, buf, nbyte);
        return ret;
    }
    auto newflag = flag | O_NONBLOCK;
    fcntl(fd, F_SETFL, newflag);
    
    ssize_t ret = uni_read(fd, buf, nbyte);
    if ((ret < 0) && (errno == EAGAIN || errno == EWOULDBLOCK)) {
        auto mco = McoPool::GetMcoRoutine(fd);
        printf("mco use_count == 4 real_count=%d\n", mco.use_count());
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
                ret = uni_read(fd, buf, nbyte);
                fcntl(fd, F_SETFL, flag);
                return ret;
            }
            McoPool::SetMcoRoutine(fd, mco);
            auto event = boost::make_shared<Events>(fd, kReadEvent);
            event->setTid(gettid());
			Eventsops::UpdateLoopEvents(event);
			Mcosops::Yield();    
			event->deleteEvent(kReadEvent);
			Eventsops::UpdateLoopEvents(event);
        }
        printf("mco end of read use_count == 4 real_count=%d\n", mco.use_count());
        
		ret = uni_read(fd, buf, nbyte);
    }
    fcntl(fd, F_SETFL, flag);

    return ret;
}

int accept(int fd, struct sockaddr *addr, socklen_t *len) {
    std::cout << "In accept hook api." << std::endl;
    int flag = fcntl(fd, F_GETFL);
    if (O_NONBLOCK & flag) {
        int ret = uni_accept(fd, addr, len);
        return ret;
    }

    auto newflag = flag | O_NONBLOCK;
    fcntl(fd, F_SETFL, newflag);
    int ret = uni_accept(fd, addr, len);
    if ((ret < 0) && (errno == EAGAIN || errno == EWOULDBLOCK)) {
        // do sth
        auto mco = McoPool::GetMcoRoutine(fd);
        if (mco != nullptr) {
            std::cout << "find fd mco!" << std::endl;
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
            std::cout << "fd after yield" << std::endl;
        } else {
            std::cout << "no fd mco!" << std::endl;
            mco = McoPool::GetCurMco();
            if (mco == nullptr) {
                int ret = uni_accept(fd, addr, len);
                fcntl(fd, F_SETFL, flag);
                return ret;
            }
            McoPool::SetMcoRoutine(fd, mco);
            auto event = boost::make_shared<Events>(fd, kReadEvent);
            event->setTid(gettid());
            Eventsops::UpdateLoopEvents(event);
            
			Mcosops::Yield();
			event->deleteEvent(kReadEvent);
			Eventsops::UpdateLoopEvents(event);
            std::cout << "after yield!" << std::endl;
		}
        
		ret = uni_accept(fd, addr, len);
    }
    fcntl(fd, F_SETFL, flag);
    return ret;
}

int close(int fd) {
	auto mcopool = McoPool::GetMcoPool();
    auto cur = mcopool->getCurMco();
    auto fdmco = mcopool->getMcoRoutine(fd);
    if (cur == fdmco) {
        mcopool->setCurMco(nullptr);
    }
    mcopool->removeMcoRoutine(fd);
	auto event = Eventsops::GetEvents(fd);
	if (event) {
		Eventsops::RemoveEventFromLoop(event);
	} else {
		return uni_close(fd);
	}
	return 0;
}
