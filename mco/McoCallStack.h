#ifndef MOXIE_MCOCALLSTACK_H
#define MOXIE_MCOCALLSTACK_H
#include <map>

class McoRoutine;

class McoCallStack {
public:
    size_t& getCurrIndex() { return index_; }

    McoRoutine*& operator[](const size_t index) {
		assert(callStack_.size() == index_);
        return callStack_[index];
    }

    McoRoutine* getCurMco() {
		assert(callStack_.size() == index_);
        if (callStack_.size() > 0) {
            return callStack_[index_ - 1];
        }
        return nullptr;
    }

    bool empty() const {
		assert(callStack_.size() == index_);
        return callStack_.size() == 0;
    }

    McoCallStack() :
        callStack_(),
        index_(0) {
    }
private:
    std::map<size_t, McoRoutine*> callStack_;
    size_t index_;
};

McoCallStack* GetMcoCallStack();
#endif //MOXIE_MCOCALLSTACK_H
