#ifndef MOXIE_MCOSTACK_H
#define MOXIE_MCOSTACK_H
#include "Mutex.h"

#include <set>
#include <map>
#include <boost/shared_ptr.hpp>

using moxie::Mutex;

class McoRoutine;

struct McoStack {
    char *stack;
    size_t size;
    char *stack_bp;
    char *stack_sp;
    char *stack_tmp;
    bool is_private;
    size_t restore_size;

    McoStack() :
        stack(nullptr),
        size(0),
        stack_bp(nullptr),
        stack_sp(nullptr),
        stack_tmp(nullptr),
        is_private(false),
        restore_size(0) {
    }
	~McoStack() {
		restore_size = 0;
		delete stack_tmp;
		stack_tmp = nullptr;
		stack_sp = nullptr;
		stack_bp = nullptr;
		size = 0;
		if (is_private) {
			delete stack;
		}
		stack = nullptr;
	}
};

class McoStackManager {
public:
    McoStack* createCommonMcoStack();
    McoStack* createPrivateMcoStack();
    void recyclePrivateStack(McoStack*& stack);
    void recycleCommonStack(McoStack*& stack);
    McoStackManager(const size_t commonSize = 1024 * 1024 * 5,
                    size_t privateSize = 1024 * 1024 * 5);	

    McoRoutine *getCommonOccupy() { return commonOccupy_; }
    void setCommonOccupy(McoRoutine *co) { commonOccupy_ = co; }
private:
    size_t commonSize_;
    size_t privateSize_;
    char *commonStack_;
    bool ready_;
    McoRoutine *commonOccupy_;
};

McoStackManager* GetMcoStackMgr();

McoStack* CreatePrivateMcoStack();
McoStack* CreateCommonMcoStack();
void RecycleMcoStack(McoStack*& stack);

void StoreUsedCommonStack(McoStack* stack);
void RecoverUsedCommonStack(McoStack* stack);

McoRoutine *GetCommonOccupy();
void SetCommonOccupy(McoRoutine *co);
#endif //MOXIE_MCOSTACK_H
