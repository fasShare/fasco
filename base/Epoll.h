#ifndef MOXIE_EPOLL_H
#define MOXIE_EPOLL_H
#include <sys/epoll.h>
#include <vector>

#include <boost/noncopyable.hpp>

#include <Timestamp.h>
namespace moxie {

class Events;

struct PollerEvent {
    int fd;
    uint32_t event;
};

class Epoll {
public:
    typedef struct epoll_event  EpollEvent;

    Epoll();
    ~Epoll();

    bool EventsAdd(Events* events);
    bool EventsMod(Events* events);
    bool EventsDel(Events* events);
    Timestamp Loop(std::vector<PollerEvent> &events, int timeout);
private:
    bool eventCtl(int op, int fd, EpollEvent* event);
    bool eventAdd(int fd, EpollEvent* event);
    bool eventDel(int fd, EpollEvent* event);
    bool eventMod(int fd, EpollEvent* event);
    int loopWait(EpollEvent* events, int maxevents, int timeout);

    int epoll_fd_;
    std::vector<EpollEvent> revents_;
    int maxNum_;
    const int addStep_;
};

}
#endif // MOXIE_EPOLL_H
