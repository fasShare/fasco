#ifndef MOXIE_CONTINUATION_H
#define MOXIE_CONTINUATION_H
#include <iostream>

#include "McoRoutine.h"
#include "McoCallStack.h"

class Continuation {
public:
    Continuation (CoCallback call) :
        co(nullptr) {
        co = McoCreate(call);
    }
    
    void resume() {
        McoResume(co);
    }

    void yield() {
        McoYield(co);
    }

    ~Continuation() {
        McoFree(co);
        std::cout << "Mco will destroyed!" << std::endl;
    }
    static void YieldCurMco() {
        auto callstack = GetMcoCallStack();
        McoYield(callstack->getCurMco());
    }
private:
    McoRoutine *co;
};

#endif 
