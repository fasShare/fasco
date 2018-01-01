#include <assert.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/eventfd.h>
#include <new>

#include <Log.h>
#include <Epoll.h>
#include <EventLoop.h>
#include <Timestamp.h>
#include <Events.h>
#include <Thread.h>
#include <MutexLocker.h>
#include <McoPool.h>
#include <Mcosops.h>

#include <boost/bind.hpp>
#include <boost/core/ignore_unused.hpp>

std::atomic<int> moxie::EventLoop::count_(0);

moxie::EventLoop::EventLoop() :
    poll_(new (std::nothrow) Epoll),
    pollDelta_(200),
    events_(),
    mutable_(),
    mutex_(),
    tid_(gettid()),
    quit_(false),
    wakeFd_(CreateEventfd()),
    wakeEvent_(new Events(wakeFd_, kReadEvent)) {
    count_++;
    wakeEvent_->setType(EVENT_TYPE_EVENT);
	updateEvents(wakeEvent_);
}

int moxie::CreateEventfd() {
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0) {
        LOGGER_SYSERR("Failed in eventfd!");
        ::abort();
    }   
    return evtfd;
}

long moxie::EventLoop::getTid() const{
    return tid_;
}

int moxie::EventLoop::getEventLoopNum() const {
    return count_;
}

bool moxie::EventLoop::updateEvents(boost::shared_ptr<Events> event) {
    LOGGER_TRACE("updateEvents start");
    if (event->invaild()) {
        return false;
    }
    if (!(tid_ == gettid())) {
        MutexLocker lock(mutex_);
        wakeupLoop();
        mutable_.push_back(event);
        return true;
    }

    return pollUpdate(event);
}

void moxie::EventLoop::wakeupLoop() {
    uint64_t one = 1;
    ssize_t n = ::write(wakeFd_, &one, sizeof one);
    if (n != sizeof one) {
        LOGGER_ERROR("EventLoop::wakeup() writes " << n << " bytes instead of 8");
    }
}

void HandleWake(int wakefd) {
    uint64_t one = 1;
    ssize_t n = ::read(wakefd, &one, sizeof one);
    if (n != sizeof one){
        LOGGER_ERROR("EventLoop::handleRead() reads " << n << " bytes instead of 8");
    }
}

boost::shared_ptr<moxie::Events> moxie::EventLoop::getEvents(int fd) {
    {
        MutexLocker lock(mutex_);
        auto event = events_.find(fd);
        if (event != events_.end()) {
            return event->second;
        }
        return nullptr;
    }
}

bool moxie::EventLoop::pollUpdate(boost::shared_ptr<moxie::Events> event) {
    LOGGER_TRACE("pollUpdate start");
    if (event->invaild()) {
        return false;
    }

    auto iter = events_.find(event->getFd()); 
    if (event->isnew()) {
        assert(iter == events_.end());
        events_[event->getFd()] = event;
        LOGGER_TRACE("pollUpdate new");
        return poll_->EventsAdd(event.get());
    } else if (event->ismod()) {
        LOGGER_TRACE("pollUpdate mod");
        if (iter == events_.end()) {
            events_[event->getFd()] = event;
            return poll_->EventsAdd(event.get());
        }
        return poll_->EventsMod(event.get());
    } else if (event->isdel()) {
        LOGGER_TRACE("pollUpdate del");
        poll_->EventsDel(event.get());
        assert(events_.erase(event->getFd()));
    }
    LOGGER_TRACE("pollUpdate end");
    return true;
}

void moxie::EventLoop::resetOwnerTid() {
	tid_ = gettid();
}

void moxie::EventLoop::assertInOwnerThread() {
    assert(gettid() == tid_);
}

void moxie::EventLoop::quit() {
    quit_ = true;
    if(!(tid_ == gettid())) {
        MutexLocker lock(mutex_);
        wakeupLoop();
    }
}

bool moxie::EventLoop::eventHandleAble(boost::shared_ptr<Events> origin) {
    if (origin->isdel()) {
        return false;
    }
    if (origin->ismod()) {
        if (origin->isRead() && (!origin->originRead())) {
            origin->deleteEventMutable(kReadEvent);
        }
        if (origin->isWrite() && (!origin->originWrite())) {
            origin->deleteEventMutable(kWriteEvent);
        }
        return origin->getMutable() != kNoneEvent;
    }
    return true;
}

bool moxie::EventLoop::loop() {
    LOGGER_TRACE("A EventLoop started.");
    assertInOwnerThread();
    Timestamp looptime;
    std::vector<PollerEvent> occur;
    while (!quit_) {
        LOGGER_TRACE("start new loopping.");
        occur.clear();
        {
            MutexLocker lock(mutex_);
            for (auto iter = mutable_.begin(); iter != mutable_.end(); ++iter) {
                LOGGER_TRACE("update enent.");
                pollUpdate(*iter);
            }
        }

        looptime = poll_->Loop(occur, 2000000);
        for(auto iter = occur.begin(); iter != occur.end(); iter++) {
            auto events = events_.find(iter->fd);
            if (events == events_.end()) {
                continue;
            }
            auto& event = events->second;
            event->setMutable(iter->event);
            if (!eventHandleAble(events->second)) {
                continue;
            }
            // do sth
            if (wakeFd_ == event->getFd()) {
                HandleWake(wakeFd_);
                continue;
            }
            // do timer
            
            // do tcp
            auto mco = McoPool::GetMcoRoutine(event->getFd()); 
            if (mco) {
                Mcosops::Resume(mco);
            }
        }
    } 
    return true;
}

moxie::EventLoop::~EventLoop() {
    delete poll_;
    LOGGER_TRACE("EventLoop will be destroyed in Thread:" << gettid());
}

