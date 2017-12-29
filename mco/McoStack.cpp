#include "McoStack.h"
#include "MutexLocker.h"

#include <Log.h>
#include <syscall.h>
#include <assert.h>
#include <string.h>
#include <iostream>
#include <boost/make_shared.hpp>

#define gettid() (::syscall(SYS_gettid))
using moxie::MutexLocker;

boost::shared_ptr<McoStack> McoStackManager::createCommonMcoStack() {
	auto stack  = boost::make_shared<McoStack>();
	stack->stack = commonStack_;
    LOGGER_TRACE("commostack:" << (unsigned long)commonStack_);
	stack->size = commonSize_;
    stack->stack_bp = commonStack_ + commonSize_;
    stack->restore_size = 0;
	stack->stack_tmp = nullptr;
	stack->is_private = false;
	stacks_.insert(stack);
	return stack;
}

boost::shared_ptr<McoStack> McoStackManager::createPrivateMcoStack() {
	auto stack  = boost::make_shared<McoStack>();
	stack->stack = new char[privateSize_];
	stack->size = privateSize_;
	stack->stack_tmp = nullptr;
	stack->is_private = true;
	stacks_.insert(stack);
	return stack;
}

void McoStackManager::recyclePrivateStack(boost::shared_ptr<McoStack> stack) {
	delete stack->stack;
	delete stack->stack_tmp;
    stacks_.erase(stack);
}

void McoStackManager::recycleCommonStack(boost::shared_ptr<McoStack> stack) {
	if (stack->stack_tmp) {
        delete stack->stack_tmp;
    }
    stack->stack = nullptr;
    stacks_.erase(stack);
}

McoStackManager::McoStackManager(const size_t commonSize, size_t privateSize) :
	commonSize_(commonSize),
	privateSize_(privateSize),
	commonStack_(nullptr),
	ready_(true),
    commonOccupy_(nullptr),
    envOccupy_(nullptr),
    envPending_(nullptr) {
	try {
        commonStack_ = new char[commonSize_];
        if(commonSize_ & 0xFFF) {
            commonSize_ &= ~0xFFF;
            commonSize_ += 0x1000;
        }

    } catch (...) {
        ready_ = false;
    }
}

McoStackManagerPool* McoStackManagerPool::instance_ = nullptr;

boost::shared_ptr<McoStackManager> McoStackManagerPool::getMcoStackMgr() {
    MutexLocker locker(mutex_);
    auto tid = gettid();
    auto iter = mgrs_.find(tid);
    if (iter == mgrs_.end()) {
        auto mgr = boost::make_shared<McoStackManager>();
		mgrs_[tid] = mgr;
		return mgr;
	}
	return iter->second;
}

boost::shared_ptr<McoStack> CreateCommonMcoStack() {
	return McoStackManagerPool::GetMcoStackMgr()->createCommonMcoStack();
}

boost::shared_ptr<McoStack> CreatePrivateMcoStack() {
	return McoStackManagerPool::GetMcoStackMgr()->createPrivateMcoStack();
}

void RecycleMcoStack(boost::shared_ptr<McoStack> stack) {
	if (stack->is_private) {
		McoStackManagerPool::GetMcoStackMgr()->recyclePrivateStack(stack);
	} else {
		McoStackManagerPool::GetMcoStackMgr()->recycleCommonStack(stack);
	}
}

void StoreUsedCommonStack(boost::shared_ptr<McoStack> stack) {
    if (!stack || (stack && stack->is_private)) {
		return;
	}
    
    stack->restore_size = 0;
    if (stack->stack_tmp) {
	    delete stack->stack_tmp;
        stack->stack_tmp = nullptr;
    }
    
	stack->restore_size = stack->stack_bp - stack->stack_sp;
	stack->stack_tmp = new char[stack->restore_size];
    
    LOGGER_TRACE("Store->bottom:" << (unsigned long)stack->stack_sp);
    LOGGER_TRACE("Store->top:" << (unsigned long)stack->stack_bp);
    LOGGER_TRACE("Store->restore_size:" << stack->restore_size);

	memcpy(stack->stack_tmp, stack->stack_sp, stack->restore_size);
}

void RecoverUsedCommonStack(boost::shared_ptr<McoStack> stack) {
	if (stack->restore_size == 0) {
		return;
	}
    
    LOGGER_TRACE("Recover->bottom:" << (unsigned long)stack->stack_sp);
    LOGGER_TRACE("Recover->top:" << (unsigned long)stack->stack_bp);
    LOGGER_TRACE("Recover->restore_size:" << stack->restore_size);

	memcpy(stack->stack_sp, stack->stack_tmp, stack->restore_size);
}
McoRoutine *GetCommonOccupy() {
	return McoStackManagerPool::GetMcoStackMgr()->getCommonOccupy();
}
void SetCommonOccupy(McoRoutine *co) {
	McoStackManagerPool::GetMcoStackMgr()->setCommonOccupy(co);
}

McoRoutine *GetEnvOccupy() {
	return McoStackManagerPool::GetMcoStackMgr()->getEnvOccupy();
}
void SetEnvOccupy(McoRoutine *co) {
	McoStackManagerPool::GetMcoStackMgr()->setEnvOccupy(co);
}
McoRoutine *GetEnvPenging() {
	return McoStackManagerPool::GetMcoStackMgr()->getEnvPenging();
}
void SetEnvPenging(McoRoutine *co) {
	McoStackManagerPool::GetMcoStackMgr()->setEnvPenging(co);
}
