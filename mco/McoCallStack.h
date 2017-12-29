#ifndef MOXIE_MCOCALLSTACK_H
#define MOXIE_MCOCALLSTACK_H
#include <map>

#include "Mutex.h"

using moxie::Mutex;
class McoRoutine;

class McoCallStack {
public:
	size_t& getCurrIndex() { return curIndex_; }
	
	McoRoutine*& operator[](const size_t index) {
		return callStack_[index];
	}

	McoRoutine* getCurMco() {
		if (callStack_.size() > 0) {
			return callStack_[curIndex_ - 1];
		}
		return nullptr;
	}

	bool empty() const {
		return callStack_.size() == 0;
	}

	McoCallStack() :
		callStack_(),
		curIndex_(0) {
	}
private:
	std::map<size_t, McoRoutine*> callStack_;
	size_t curIndex_;
};

class McoCallStackManager {
public:
	static McoCallStack *GetMcoCallStack() {
		return Instance()->getMcoCallStack();
	}
private:
	McoCallStack *getMcoCallStack();
	McoCallStackManager() :
		callStacks_(),
		mutex_() {
	}
	static McoCallStackManager *Instance() {
		if (!instance_) {
			instance_ = new McoCallStackManager();
		}
		return instance_;
	}

	static McoCallStackManager *instance_;
	std::map<long, McoCallStack *> callStacks_;
	Mutex mutex_;
};
McoCallStack *GetMcoCallStack();
#endif //MOXIE_MCOCALLSTACK_H
