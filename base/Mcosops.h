#ifndef MOXIE_MCOSOPS_H
#define MXOIE_MCOSOPS_H
#include <Continuation.h>
#include <McoPool.h>

namespace moxie {

class Mcosops {
public:
    static void Resume(boost::shared_ptr<Continuation> mco) {
        McoPool::SetCurMco(mco);
        mco->resume();
    }

    static void Yield() {
        auto mco = McoPool::GetCurMco();
        if (mco) {
            mco->yield();
        }
    }
};

}

#endif
