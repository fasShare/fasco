#ifndef MOXIE_CONTINUATION_H
#define MOXIE_CONTINUATION_H
#include <iostream>

#include <McoRoutine.h>

class Continuation {
public:
    Continuation (CoCallback call) :
        co_(nullptr) {
        co_ = McoCreate(call);
    }
    
    Continuation (McoRoutine *co) :
        co_(co) {
    }

    void resume() {
        McoResume(co_);
    }

    void yield() {

        McoYield(co_);
    }

    ~Continuation() {
        McoFree(co_);
        std::cout << "Mco will destroyed!" << std::endl;
    }
    boost::shared_ptr<Continuation> getSink() {
        return sink_;
    }
    void setSink(boost::shared_ptr<Continuation> sink) {
        sink_ = sink;
    }
private:
    McoRoutine *co_;
    boost::shared_ptr<Continuation> sink_;
};

#endif 
