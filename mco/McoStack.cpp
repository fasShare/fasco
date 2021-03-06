#include <Log.h>
#include <syscall.h>
#include <assert.h>
#include <string.h>
#include <iostream>
#include <boost/make_shared.hpp>

#include <McoStack.h>
#include <MutexLocker.h>
#include <PoolInThreads.hpp>

#define gettid() (::syscall(SYS_gettid))
using moxie::MutexLocker;

McoStack* McoStackManager::createCommonMcoStack() {
	auto stack  = new McoStack();
	stack->stack = commonStack_;
    LOGGER_TRACE("commostack:" << (unsigned long)commonStack_);
	stack->size = commonSize_;
    stack->stack_bp = commonStack_ + commonSize_;
    stack->restore_size = 0;
	stack->stack_tmp = nullptr;
	stack->is_private = false;
	return stack;
}

McoStack* McoStackManager::createPrivateMcoStack() {
	auto stack  = new McoStack();
	stack->stack = new char[privateSize_];
	stack->size = privateSize_;
	stack->stack_tmp = nullptr;
	stack->is_private = true;
	return stack;
}

void McoStackManager::recyclePrivateStack(McoStack*& stack) {
	(void)stack;
}

void McoStackManager::recycleCommonStack(McoStack*& stack) {
	(void)stack;
}

McoStackManager::McoStackManager(const size_t commonSize, size_t privateSize) :
	commonSize_(commonSize),
	privateSize_(privateSize),
	commonStack_(nullptr),
	ready_(true),
    commonOccupy_(nullptr) {
	try {
        if(commonSize_ & 0xFFF) {
            commonSize_ &= ~0xFFF;
            commonSize_ += 0x1000;
        }
        commonStack_ = new char[commonSize_];
    } catch (...) {
        ready_ = false;
    }
}

McoStack* CreateCommonMcoStack() {
	return GetMcoStackMgr()->createCommonMcoStack();
}

McoStack* CreatePrivateMcoStack() {
	return GetMcoStackMgr()->createPrivateMcoStack();
}

void RecycleMcoStack(McoStack*& stack) {
	(void)stack;
}

void StoreUsedCommonStack(McoStack* stack) {
    LOGGER_TRACE("stack_addr:" << (unsigned long)stack);
    if (!stack || stack->is_private) {
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

void RecoverUsedCommonStack(McoStack* stack) {
    LOGGER_TRACE("stack_addr:" << (unsigned long)stack);
	if (!stack || stack->restore_size == 0) {
		return;
	}
    
    LOGGER_TRACE("Recover->bottom:" << (unsigned long)stack->stack_sp);
    LOGGER_TRACE("Recover->top:" << (unsigned long)stack->stack_bp);
    LOGGER_TRACE("Recover->restore_size:" << stack->restore_size);

	memcpy(stack->stack_sp, stack->stack_tmp, stack->restore_size);
}
McoRoutine *GetCommonOccupy() {
	return GetMcoStackMgr()->getCommonOccupy();
}
void SetCommonOccupy(McoRoutine *co) {
    GetMcoStackMgr()->setCommonOccupy(co);
}

McoStackManager* GetMcoStackMgr() {
    return moxie::PoolInThreads<McoStackManager *>::Item();
}
