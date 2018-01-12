#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <iostream>

#include <Log.h>
#include <McoRoutine.h>
#include <McoCallStack.h>

extern "C" 
{
    extern void mcontext_swap(Mcontext *, Mcontext *) asm("mco_swap");
};

static void CoEmpty() {}

static void CoroutineRun(McoRoutine *co) {
    if (co && co->coctx && co->coctx->corun) {
        try {
            co->coctx->corun();    
        } catch (...) {
            LOGGER_TRACE("An exception occur.");
        }
    }
    co->done = true;
    LOGGER_TRACE("Before run yield");
    McoYield(co);
}

void SwapMcoContext(McoContext *cur, McoContext *co) {
    if (!cur || !co) {
        return;
    }
    if (!cur->ctx || !co->ctx) {
        return;
    }
    mcontext_swap(cur->ctx, co->ctx);
}

bool InitMcoContext(McoRoutine *co) {
    auto& coctx = co->coctx;
    coctx->ctx = new Mcontext;
    if (!coctx->ctx) {
        return false;
    }
    McontextInit(coctx->ctx);
    if (!co->use_private) {
        co->stack = co->stackMgr->createCommonMcoStack();
    } else {
        co->stack = co->stackMgr->createPrivateMcoStack();
    }
    if (!co->stack) {
        return false;
    }
    coctx->ctx->ss_sp = co->stack->stack;
    coctx->ctx->ss_size = co->stack->size;

    return true;
}

McoRoutine *McoCreate(CoCallback run, bool use_private) {
    McoRoutine *mco = new McoRoutine;
    if (!mco) {
        return nullptr;
    }
    mco->coctx = new McoContext;
    if (!mco->coctx) {
        return nullptr;
    }
    mco->coctx->corun = run;
    mco->use_private = use_private;

    mco->sink = nullptr;
    mco->is_main = false;
    return mco;
}

void McoSwap(McoRoutine *sink, McoRoutine *co) {
    char c; 
    sink->stack->stack_sp = &c;
    if (!co->stack->is_private) {
        auto occupy_co_tmp = GetCommonOccupy();
        SetCommonOccupy(co);
        if (occupy_co_tmp
			&& occupy_co_tmp != co 
            && !(occupy_co_tmp->done && occupy_co_tmp->done_yield)) {
            occupy_co_tmp->stack_store = true;
			LOGGER_TRACE("occupy_stack_item:" << (unsigned long)(occupy_co_tmp->stack));
			LOGGER_TRACE("occupy_stack:" << (unsigned long)(occupy_co_tmp->stack->stack));
            StoreUsedCommonStack(occupy_co_tmp->stack);
        }
    }

    mcontext_swap(sink->coctx->ctx, co->coctx->ctx);

    auto callstack = GetMcoCallStack();
    auto cur_co = callstack->getCurMco();
    if (cur_co && !cur_co->stack->is_private && cur_co->stack_store) {
        cur_co->stack_store = false;
        RecoverUsedCommonStack(cur_co->stack);
    }
}

McoRoutine *MainMco() {
    auto callstack = GetMcoCallStack();
    auto& index = callstack->getCurrIndex();
    if (index == 0) {
        auto sink = McoCreate(CoEmpty, true);
        if (!InitMcoContext(sink)) {
            return nullptr;
        }
        McontextMake(sink->coctx->ctx, (cofunc)(CoroutineRun), (void *)sink);
        (*callstack)[index++] = sink;
        sink->is_main = true;
        sink->start = true;
        sink->running = true;
        sink->in_callstack = true;
    }
    return (*callstack)[0];
}

void McoResume(McoRoutine *co) {
    if (!co || co->running || (co->done && co->done_yield)) {
        return;
    }
    auto callstack = GetMcoCallStack();
    auto& index = callstack->getCurrIndex();
    if (index == 0) {
        MainMco();
    }
    // 如果当前协程是从co切换过来的，此时应该使用yield，而不是resume，不然会导致栈区的重复保存。 
    auto cur_co = (*callstack)[index-1];
    if (cur_co->sink != co) {
        if (!co->start) {
            if (!InitMcoContext(co)) {
                return;
            }
            co->start = true;
            McontextMake(co->coctx->ctx, (cofunc)(CoroutineRun), (void *)co);
        }
        co->running = true;
        (*callstack)[index++] = co;
        co->sink = (*callstack)[index-2];
        co->sink->running = false;
        co->in_callstack = true;
        LOGGER_TRACE("will resume co:" << (unsigned long)co);
        LOGGER_TRACE("will resume sink:" << (unsigned long)(co->sink));
        LOGGER_TRACE("co stack:" << (unsigned long)(co->stack->stack));
        LOGGER_TRACE("sink stack:" << (unsigned long)(co->sink->stack->stack));
        McoSwap(co->sink, co);
    } else {
        McoYield(cur_co);
    }
}

void McoYield(McoRoutine *co) {
    if (!co || !co->running || (co->done && co->done_yield)) {
        return;
    }
    if (co->done) {
        co->done_yield = true;
        co->start = false;
    }
    auto callstack = GetMcoCallStack();
    auto& index = callstack->getCurrIndex();

    auto sink = co->sink;
    assert(sink == (*callstack)[index-2]);
    index--;
    co->running = false;
    co->in_callstack = true;
    sink->running = true;
    LOGGER_TRACE("will swap co:" << (unsigned long)co << " index:" << index);
    LOGGER_TRACE("will swap sink:" << (unsigned long)sink << " index:" << index);

    McoSwap(co, sink);
}
void McoFree(McoRoutine *co) {
    LOGGER_TRACE("before !co=" << (unsigned long)co);
    if (!co) {
        LOGGER_TRACE("co with nullptr will be free. may be a bug");
        return;
    }
    LOGGER_TRACE("before co->running.");
    if (co->running) {
        co->should_close = true;
        return;
    }
    LOGGER_TRACE("before GetCommonOccupy");
    auto cop = GetCommonOccupy();
    if (cop == co) {
        SetCommonOccupy(nullptr);
    }
    LOGGER_TRACE("Begin RecycleMcoStack.");
    RecycleMcoStack(co->stack);
    LOGGER_TRACE("after RecycleMcoStack.");
    co->done = true;
    co->done_yield = true;
    co->running = false;
    co->start = false;

    LOGGER_TRACE("Begin delete co");
    delete co;
    co = nullptr;
}
