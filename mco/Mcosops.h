#ifndef MOXIE_MCOSOPS_H
#define MXOIE_MCOSOPS_H
#include <Continuation.h>
#include <McoPool.h>

namespace moxie {

class Mcosops {
public:
    static void Resume(boost::shared_ptr<Continuation> mco) {
        auto sink = McoPool::GetCurMco();
        if (sink != mco) {
            mco->setSink(sink);
            McoPool::SetCurMco(mco);
            mco->resume();
        } else {
            std::cout << "sink == mco may be a bug." << std::endl;
        }
    }

    static void Yield() {
        auto mco = McoPool::GetCurMco();
        if (mco) {
            McoPool::SetCurMco(mco->getSink());
            mco->yield();
        }
    }
};

}

#endif
