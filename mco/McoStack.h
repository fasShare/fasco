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

//    McoRoutine *getEnvOccupy() { return envOccupy_; }
//    void setEnvOccupy(McoRoutine *co) { envOccupy_ = co; }
//    McoRoutine *getEnvPenging() { return envPending_; }
//    void setEnvPenging(McoRoutine *co) { envPending_ = co; }
private:
    std::set<McoStack *> stacks_;
    size_t commonSize_;
    size_t privateSize_;
    char *commonStack_;
    bool ready_;
    McoRoutine *commonOccupy_;
//    McoRoutine *envOccupy_;
//    McoRoutine *envPending_;
};

McoStackManager* GetMcoStackMgr();

McoStack* CreatePrivateMcoStack();
McoStack* CreateCommonMcoStack();
void RecycleMcoStack(McoStack*& stack);

void StoreUsedCommonStack(McoStack* stack);
void RecoverUsedCommonStack(McoStack* stack);

McoRoutine *GetCommonOccupy();
void SetCommonOccupy(McoRoutine *co);

McoRoutine *GetEnvOccupy();
void SetEnvOccupy(McoRoutine *co);
McoRoutine *GetEnvPenging();
void SetEnvPenging(McoRoutine *co);
#endif //MOXIE_MCOSTACK_H
