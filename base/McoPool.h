#ifndef MOXIE_MCOPOOL_H
#define MOXIE_MCOPOOL_H
#include <set>
#include <unordered_map>

#include <Continuation.h>
#include <PoolInThreads.hpp>
#include <CallStack.hpp>
#include <Mutex.h>

namespace moxie {

class McoPool {
public:
    static boost::shared_ptr<Continuation> GetMcoRoutine(int fd) {
        return GetMcoPool()->getMcoRoutine(fd);
    }
	static bool RemoveMcoRoutine(int fd) {
		return GetMcoPool()->removeMcoRoutine(fd);
	}
    static void SetMcoRoutine(int fd, boost::shared_ptr<Continuation> co) {
        GetMcoPool()->setMcoRoutine(fd, co);
    }
    static void SetMcoRoutine(int fd, boost::shared_ptr<Continuation> co, long tid) {
        GetMcoPool(tid)->setMcoRoutine(fd, co);
    }
    static boost::shared_ptr<Continuation> GetCurMco() {
        return GetMcoPool()->getCurMco();
    }
    static void SetCurMco(boost::shared_ptr<Continuation> mco) {
        GetMcoPool()->setCurMco(mco);
    }
    static boost::shared_ptr<McoPool> GetMcoPool() {
        return PoolInThreads<McoPool>::Item();
    }
    static boost::shared_ptr<McoPool> GetMcoPool(long tid) {
        return PoolInThreads<McoPool>::Item(tid);
    }
    boost::shared_ptr<Continuation> getCurMco() { return cur_; }
    void setCurMco(boost::shared_ptr<Continuation> mco) { cur_ = mco; }
    boost::shared_ptr<Continuation> getMcoRoutine(int fd);
	bool removeMcoRoutine(int fd);
    void setMcoRoutine(int fd, boost::shared_ptr<Continuation> co);
private:
    boost::shared_ptr<Continuation> cur_;
    std::unordered_map<int, boost::shared_ptr<Continuation>> fdMcos_;
};

}
#endif
