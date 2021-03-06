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
        LOGGER_TRACE("Mco will destroyed co="<< (unsigned long)co_);
        McoFree(co_);
        LOGGER_TRACE("Mco will destroyed!");
    }
private:
    McoRoutine *co_;
};

#endif 
