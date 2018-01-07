#ifndef MOXIE_EVENTSOPS_H
#define MOXIE_EVENTSOPS_H
#include <EventLoop.h>
#include <Events.h>
#include <Handler.h>
#include <EventLoopPool.h>

#include <boost/shared_ptr.hpp>

namespace moxie {

class Eventsops {
public:
    static bool RemoveEventFromLoop(boost::shared_ptr<Events> event) {
        auto loop = EventLoopPool::GetLoop(event->getTid());
        if (!loop) {
            assert(false);
            return false;
        }
        event->setState(Events::state::DEL);
        return loop->updateEvents(event);
    }
	static bool UpdateLoopEvents(boost::shared_ptr<Events> event) {
		auto loop = EventLoopPool::GetLoop(event->getTid());
		if (!loop) {
			return false;
		}
		return loop->updateEvents(event);
	}
    static boost::shared_ptr<Events> GetEvents(int fd) {
        auto loop = EventLoopPool::GetLoop(gettid());
        if (!loop) {
            return nullptr;
        }
        return loop->getEvents(fd);
    }
};

}

#endif //MOXIE_EVENTSOPS_H
