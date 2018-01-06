#ifndef TOOLS_POOLINTHREADS_H
#define TOOLS_POOLINTHREADS_H
#include <syscall.h>

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <unordered_map>

#include <MutexLocker.h>
#include <Mutex.h>

#define gettid() (::syscall(SYS_gettid))

namespace moxie {

template <class T>
class PoolInThreads {
public:
    static boost::shared_ptr<T> Item() {
        return Instance()->getT();
    }
private:
    boost::shared_ptr<T> getT() {
        MutexLocker lock(mutex_);
        auto tid = gettid();
        auto iter = pools_.find(tid);
        if (iter != pools_.end()) {
            return iter->second;
        }
        auto ret = boost::make_shared<T>();
        pools_[tid] = ret;
        return ret;
    }
    static PoolInThreads *Instance() {
        if (!instance_) {
            instance_ = new PoolInThreads;
        }
        return instance_;
    }
    static PoolInThreads *instance_;
    std::unordered_map<long, boost::shared_ptr<T>> pools_;
    Mutex mutex_;
};

template <class T>
PoolInThreads<T> *PoolInThreads<T>::instance_ = nullptr;

}

#endif
