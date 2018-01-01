#ifndef MOXIE_MCOSOPS_H
#define MXOIE_MCOSOPS_H
#include <Continuation.h>
#include <McoPool.h>

namespace moxie {

class Mcosops {
public:
    static void Resume(boost::shared_ptr<Continuation> mco) {
        std::cout << "Before get cur" << std::endl;
        auto sink = McoPool::GetCurMco();
        std::cout << "after get sink" << std::endl;
        mco->setSink(sink);
        std::cout << "after set sink" << std::endl;
        McoPool::SetCurMco(mco);
        std::cout << "before resume" << std::endl;
        mco->resume();
        std::cout << "after resume" << std::endl;
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
