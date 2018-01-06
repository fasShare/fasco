#include <EventLoopPool.h>
#include <MutexLocker.h>
#include <Log.h>

moxie::EventLoopPool* moxie::EventLoopPool::instance_ = nullptr; 
moxie::EventLoopPool::EventLoopPool() :
	mutex_(),
    mainLoop_(nullptr),
	loops_(),
    nextLoops_(),
    next_(0) {
}

bool moxie::EventLoopPool::addEventLoop(long tid, moxie::EventLoop* loop, bool ismain) {
    MutexLocker locker(mutex_);
    if ((loop == NULL) || (loops_.find(tid) != loops_.end())) {
        return false;
    }
    
	loops_[tid] = loop;
    if (!ismain) {
        nextLoops_.push_back(loop);
    } else {
		mainLoop_ = loop;
	}
    LOGGER_TRACE("Num of EventLoop in Bucket is " << nextLoops_.size());
    return true;
}

moxie::EventLoop *moxie::EventLoopPool::getNextLoop() {
    MutexLocker locker(mutex_);
    if (nextLoops_.size() == 0) {
		return mainLoop_;
    }
    if (nextLoops_.size() <= next_) {
        next_ = 0;
    }
    return nextLoops_[next_++];
}

moxie::EventLoop *moxie::EventLoopPool::getLoop(long tid) {
    MutexLocker locker(mutex_);
    auto iter = loops_.find(tid);
    if (iter == loops_.end()) {
        return nullptr;
    }
    return iter->second;
}

moxie::EventLoop* moxie::EventLoopPool::getMainLoop() {
	MutexLocker locker(mutex_);
	if (mainLoop_) {
		return mainLoop_;
	}
	return nullptr;
}

moxie::EventLoopPool *moxie::EventLoopPool::Instance() {
    if (nullptr == instance_) {
        instance_ = new (std::nothrow) EventLoopPool();
    }
    return instance_;
}
