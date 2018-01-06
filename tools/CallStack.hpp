#ifndef TOOLS_CALLSTACK_HPP
#define TOOLS_CALLSTACK_HPP
#include <unordered_map>

namespace moxie {

template <class T>
class CallStack {
public:
    size_t& getCurrIndex() { 
        return curIndex_;
    }

    T& operator[](const size_t index) {
        return callStack_[index];
    }

    bool empty() const {
        return callStack_.size() == 0;
    }
    
    CallStack() :
        callStack_(),
        curIndex_(0) {
    }
private:
    std::unordered_map<size_t, T> callStack_;
    size_t curIndex_;
};

}

#endif
