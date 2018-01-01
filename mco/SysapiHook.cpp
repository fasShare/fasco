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
using getuid_hook = uid_t (*)(void);

static read_hook  __read = (read_hook)dlsym(RTLD_NEXT, "read");
static accept_hook __accept = (accept_hook)dlsym(RTLD_NEXT, "accept");
static getuid_hook __getuid = (getuid_hook)dlsym(RTLD_NEXT, "getuid");

uid_t getuid() {
    printf("In hook getuid\n");
    return __getuid();
}

ssize_t read(int fd, void *buf, size_t nbyte) {
    printf("In hook read.");
    int flag = fcntl(fd, F_GETFL);
    if (O_NONBLOCK & flag) {
        ssize_t ret = __read(fd, buf, nbyte);
        return ret;
    }
    auto newflag = flag | O_NONBLOCK;
    fcntl(fd, F_SETFL, newflag);
    // fd should be nobclock
    ssize_t ret = __read(fd, buf, nbyte);
    if ((ret < 0) && (errno == EAGAIN || errno == EWOULDBLOCK)) {
        auto mco = McoPool::GetMcoRoutine(fd);
        if (mco != nullptr) {
            assert(mco == McoPool::GetCurMco());
            auto event = Eventsops::GetEvents(fd);
            event->updateEvents(kReadEvent);
            Eventsops::UpdateLoopEvents(event);
            mco->yield();
            event->deleteEvent(kReadEvent);
            Eventsops::UpdateLoopEvents(event);
        } else {
            mco = McoPool::GetCurMco();
            if (mco == nullptr) {
                fcntl(fd, F_SETFL, flag);
                ret = __read(fd, buf, nbyte);
                return ret;
            }
            McoPool::SetMcoRoutine(fd, mco);
            boost::shared_ptr<Events> event = boost::make_shared<Events>(fd, kReadEvent);
            event->setTid(gettid());
            Eventsops::UpdateLoopEvents(event);
            mco->yield();
            event->deleteEvent(kReadEvent);
            Eventsops::UpdateLoopEvents(event);
        }
        ret = __read(fd, buf, nbyte);
        fcntl(fd, F_SETFL, flag);
    }

    return ret;
}

int accept(int fd, struct sockaddr *addr, socklen_t *len) {
    std::cout << "In accept hook api." << std::endl;
    int flag = fcntl(fd, F_GETFL);
    if (O_NONBLOCK & flag) {
        int ret = __accept(fd, addr, len);
        return ret;
    }

    auto newflag = flag | O_NONBLOCK;
    fcntl(fd, F_SETFL, newflag);
    int ret = __accept(fd, addr, len);
    if ((ret < 0) && (errno == EAGAIN || errno == EWOULDBLOCK)) {
        // do sth
        auto mco = McoPool::GetMcoRoutine(fd);
        if (mco != nullptr) {
            std::cout << "find fd mco!" << std::endl;
            assert(mco == McoPool::GetCurMco());
            auto event = Eventsops::GetEvents(fd);
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
                fcntl(fd, F_SETFL, flag);
                int ret = __accept(fd, addr, len);
                return ret;
            }
            McoPool::SetMcoRoutine(fd, mco);
            boost::shared_ptr<Events> event = boost::make_shared<Events>(fd, kReadEvent);
            event->setTid(gettid());
            Eventsops::UpdateLoopEvents(event);
            Mcosops::Yield();
            event->deleteEvent(kReadEvent);
            Eventsops::UpdateLoopEvents(event);
            std::cout << "after yield!" << std::endl;
        }
        ret = __accept(fd, addr, len);
        fcntl(fd, F_SETFL, flag);
    }
    return ret;
}

