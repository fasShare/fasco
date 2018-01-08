#ifndef MOXIE_MCOROUTINE_H
#define MOXIE_MCOROUTINE_H
#include "Mcontext.h"
#include "McoStack.h"
#include "Log.h"

#include <string.h>
#include <syscall.h>
#include <iostream>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#define gettid() (::syscall(SYS_gettid))

using CoCallback = boost::function<void ()>;

struct McoContext {
    struct Mcontext *ctx;
    McoStack* stack;
    CoCallback corun;
    bool use_private;
    void *args;
    ~McoContext () {
        std::cout << "McoContext will be destroyed!" << std::endl;
    }
};


struct McoRoutine {
    McoContext *coctx;
    struct McoRoutine *sink;
    bool is_main;
    bool in_callstack;
    bool should_close;
    bool start;
    bool running;
    bool done;
    bool done_yield;
    bool stack_store;
    McoRoutine() :
        coctx(nullptr),
        sink(nullptr),
        is_main(false),
        in_callstack(false),
        should_close(false),
        start(false),
        running(false),
        done(false),
        done_yield(false),
        stack_store(false) {
        }
        ~McoRoutine() {
            LOGGER_TRACE("McoRoutine will be destroyed.");
        }
};

struct McoSwapContext {
    struct McoRoutine *occupy;
    struct McoRoutine *pending;
    McoSwapContext() {
        occupy = nullptr;
        pending = nullptr;
    }
};

bool InitMcoContext(McoRoutine *co, CoCallback run, bool use_private = false);
void SwapMcoContext(McoContext *cur, McoContext *sink);
McoRoutine *McoCreate(CoCallback run, bool use_private = false);
void McoResume(McoRoutine *co);
void McoYield(McoRoutine *co);
void McoFree(McoRoutine *co);
McoRoutine *MainMco();
#endif //MOXIE_MCOROUTINE_H
