#ifndef MOXIE_MCOCALLSTACK_H
#define MOXIE_MCOCALLSTACK_H
#include <map>

#include <Mutex.h>
#include <CallStack.hpp>
#include <PoolInThreads.hpp>

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

boost::shared_ptr<McoCallStack> GetMcoCallStack();
//template <class T>
//boost::shared_ptr<moxie::CallStack<T>> GetMcoCallStack() {
//    return moxie::PoolInThreads<moxie::CallStack<T>>::Item();
//}
#endif //MOXIE_MCOCALLSTACK_H
