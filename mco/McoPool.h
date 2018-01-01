#ifndef MOXIE_MCOPOOL_H
#define MOXIE_MCOPOOL_H
#include <set>
#include <unordered_map>

#include <Continuation.h>
#include <McoRoutine.h>
#include <Mutex.h>

namespace moxie {

class McoPool {
public:
    static boost::shared_ptr<Continuation> GetCurMco() {
        return Instance()->getCurMco();
    }
    static void SetCurMco(boost::shared_ptr<Continuation> mco) {
        Instance()->setCurMco(mco);
    }
    static boost::shared_ptr<Continuation> GetMcoRoutine(int fd) {
        return Instance()->getMcoRoutine(fd);
    }
    static bool SetMcoRoutine(int fd, boost::shared_ptr<Continuation> co) {
        return Instance()->setMcoRoutine(fd, co);
    }
private:
    boost::shared_ptr<Continuation> getCurMco();
    void setCurMco(boost::shared_ptr<Continuation> mco);
    boost::shared_ptr<Continuation> getMcoRoutine(int fd);
    boost::shared_ptr<Continuation> getMainMco();
    bool setMcoRoutine(int fd, boost::shared_ptr<Continuation> co);
    static McoPool *Instance() {
        if (instance_ == nullptr) {
            instance_ = new McoPool;
        }
        return instance_;
    }
    Mutex mutex_;
    static McoPool *instance_;
    std::unordered_map<long, std::unordered_map<int, boost::shared_ptr<Continuation>>> fdMcos_;
    std::unordered_map<long, boost::shared_ptr<Continuation>> curMcos_;
    std::unordered_map<long, boost::shared_ptr<Continuation>> mainMco_;
};

}
#endif
