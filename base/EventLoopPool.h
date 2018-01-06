#ifndef MOXIE_EVENTLOOPPOOL_H
#define MOXIE_EVENTLOOPPOOL_H
#include <map>
#include <vector>
#include <unistd.h>
#include <atomic>

#include <EventLoop.h>
#include <Mutex.h>

namespace moxie {

class EventLoop;

class EventLoopPool {
public:
    static bool AddEventLoop(EventLoop* loop) {
        return Instance()->addEventLoop(loop->getTid(), loop);
    }
    static bool addMainLoop(EventLoop* loop) {
		return Instance()->addEventLoop(loop->getTid(), loop, true);	
    }
    static EventLoop *GetNextLoop() {
        return Instance()->getNextLoop();
    }
	static EventLoop *GetMainLoop() {
		return Instance()->getMainLoop();
	}
    static EventLoop *GetLoop(long tid) {
        return Instance()->getLoop(tid);
    }
private:
    bool addEventLoop(long tid, EventLoop* loop, bool ismain = false);
    EventLoop *getNextLoop();
    EventLoop *getLoop(long tid);
	EventLoop *getMainLoop();
    static EventLoopPool *Instance();
    EventLoopPool();
    
    Mutex mutex_;
    EventLoop *mainLoop_;
    std::map<long, EventLoop *> loops_;
    std::vector<EventLoop *> nextLoops_;
    std::atomic<size_t> next_;
    static EventLoopPool *instance_;
};

}

#endif //MOXIE_EVENTLOOPPOOL_H
