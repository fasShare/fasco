#include <unistd.h>
#include <sys/syscall.h>

#include "McoCallStack.h"
#include "MutexLocker.h"

using moxie::MutexLocker;

#define gettid() (::syscall(SYS_gettid))

McoCallStackManager* McoCallStackManager::instance_ = nullptr;

McoCallStack *McoCallStackManager::getMcoCallStack() {
	MutexLocker locker(mutex_);
	auto tid = gettid();
	auto iter = callStacks_.find(tid);
	if (iter == callStacks_.end()) {
		callStacks_[tid] = new McoCallStack;
	}
	return callStacks_[tid];
}

McoCallStack *GetMcoCallStack() {
	return McoCallStackManager::GetMcoCallStack();
}
