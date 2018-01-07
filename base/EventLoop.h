#ifndef MOXIE_EVENTLOOP_H
#define MOXIE_EVENTLOOP_H
#include <vector>
#include <map>
#include <iostream>
#include <memory>
#include <atomic>
#include <unordered_map>

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/function.hpp>

#include <Epoll.h>
#include <Mutex.h>
#include <Timestamp.h>

namespace moxie {

class Events;
class MutexLocker;

class EventLoop : boost::noncopyable {
public:
    EventLoop();
    ~EventLoop();

    long getTid() const;
	void resetOwnerTid();

    boost::shared_ptr<moxie::Events> getEvents(int fd);
    bool updateEvents(boost::shared_ptr<Events> event);

    int getEventLoopNum() const;
    bool isInLoopOwnerThread();
    void assertInOwnerThread();  

    void quit();
    bool loop();

    void wakeupLoop();
private:
    bool eventHandleAble(boost::shared_ptr<Events> origin);
    bool pollUpdate(boost::shared_ptr<Events> event);

    static const int kInitMaxEvents_ = 10;
    static std::atomic<int> count_;

    Epoll *poll_;
    int pollDelta_;

    std::unordered_map<int, boost::shared_ptr<Events>> events_;
    
    Mutex mutex_;
    long tid_;

    bool quit_;
    int wakeFd_;
    boost::shared_ptr<Events> wakeEvent_;
};

int CreateEventfd();

}
#endif // MOXIE_EVENTLOOP_H
