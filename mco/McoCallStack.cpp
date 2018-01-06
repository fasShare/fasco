#include <unistd.h>
#include <sys/syscall.h>

#include <PoolInThreads.hpp>
#include <CallStack.hpp>
#include <McoRoutine.h>
#include <McoCallStack.h>

boost::shared_ptr<McoCallStack> GetMcoCallStack() {
    return moxie::PoolInThreads<McoCallStack>::Item();
}
