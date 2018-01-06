#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <iostream>

#include "Log.h"
#include "McoRoutine.h"
#include "McoCallStack.h"

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
    auto cur_ctx = cur->ctx;
    auto co_ctx = co->ctx;
    if (!cur_ctx || !co_ctx) {
        return;
    }
    mcontext_swap(cur->ctx, co->ctx);
}

bool InitMcoContext(McoRoutine *co, CoCallback run, bool use_private) {
    auto& coctx = co->coctx;
    coctx->ctx = new Mcontext;
    if (!coctx->ctx) {
        return false;
    }
	McontextInit(coctx->ctx);
	if (!use_private) {
		coctx->stack = CreateCommonMcoStack();
	} else {
		coctx->stack = CreatePrivateMcoStack();
	}
	if (!coctx->stack) {
        return false;
    }
    coctx->ctx->ss_sp = coctx->stack->stack;
    coctx->ctx->ss_size = coctx->stack->size;

    coctx->corun = run;
    
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
    if (!InitMcoContext(mco, run, use_private)) {
        return nullptr;
    }
    mco->sink = nullptr;
	mco->is_main = false;
    return mco;
}

void McoSwap(McoRoutine *sink, McoRoutine *co) {
	char c; 
    //LOGGER_TRACE("Begin McoSwap");	
	sink->coctx->stack->stack_sp = &c;
    //LOGGER_TRACE("After set stack_sp");	
	if (!co->coctx->stack->is_private) {
        //LOGGER_TRACE("Before GetCommonOccupy");
		auto occupy_co_tmp = GetCommonOccupy();
        //LOGGER_TRACE("After GetCommonOccupy");
        SetCommonOccupy(co);
        //LOGGER_TRACE("After SetCommonOccupy");
        //LOGGER_TRACE("CommonOccupy:" << (unsigned long)occupy_co_tmp); 
        //LOGGER_TRACE("co:" << (unsigned long)co); 
		if (occupy_co_tmp && occupy_co_tmp != co 
			&& !(occupy_co_tmp->done && occupy_co_tmp->done_yield)) {
            occupy_co_tmp->stack_store = true;
            //LOGGER_TRACE("Before StoreUsedCommonStack");
			StoreUsedCommonStack(occupy_co_tmp->coctx->stack);
            //LOGGER_TRACE("After StoreUsedCommonStack");
		}
	}
    //LOGGER_TRACE("Before mcontext_swap."); 
	mcontext_swap(sink->coctx->ctx, co->coctx->ctx);
    //LOGGER_TRACE("after mcontext_swap."); 
    
    auto callstack = GetMcoCallStack();
    auto cur_co = callstack->getCurMco();
    if (cur_co && !cur_co->coctx->stack->is_private && cur_co->stack_store) {
        cur_co->stack_store = false;
        RecoverUsedCommonStack(cur_co->coctx->stack);
    }
}

McoRoutine *MainMco() {
    auto callstack = GetMcoCallStack();
    auto& cur_index = callstack->getCurrIndex();
    if (cur_index == 0) {
        auto sink = McoCreate(CoEmpty, true);
		McontextMake(sink->coctx->ctx, (cofunc)(CoroutineRun), (void *)sink);
        (*callstack)[cur_index++] = sink;
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
	auto& cur_index = callstack->getCurrIndex();
    if (cur_index == 0) {
         MainMco();
    }
	// 如果当前协程是从co切换过来的，此时应该使用yield，而不是resume，不然会导致栈区的重复保存。 
	auto cur_co = (*callstack)[cur_index-1];
	if (cur_co->sink != co) {
		if (!co->start) {
			co->start = true;
			McontextMake(co->coctx->ctx, (cofunc)(CoroutineRun), (void *)co);
		}
		co->running = true;
		(*callstack)[cur_index++] = co;
		co->sink = (*callstack)[cur_index-2];
		co->sink->running = false;
        co->in_callstack = true;
        //LOGGER_TRACE("will resume co:" << (unsigned long)co);
        //LOGGER_TRACE("will resume sink:" << (unsigned long)(co->sink));
        //LOGGER_TRACE("co stack:" << (unsigned long)(co->coctx->stack->stack));
        //LOGGER_TRACE("sink stack:" << (unsigned long)(co->sink->coctx->stack->stack));
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
    auto& cur_index = callstack->getCurrIndex();
    
    auto sink = co->sink;
    assert(sink == (*callstack)[cur_index-2]);
    cur_index--;
    co->running = false;
    co->in_callstack = true;
    sink->running = true;
    LOGGER_TRACE("will swap co:" << (unsigned long)co << " index:" << cur_index);
    LOGGER_TRACE("will swap sink:" << (unsigned long)sink << " index:" << cur_index);
	
	McoSwap(co, sink);
}
void McoFree(McoRoutine *co) {
    //LOGGER_TRACE("will free co:" << (unsigned long)co);
	if (!co || co->running) {
		co->should_close = true;
		return;
	}
    auto cop = GetCommonOccupy();
    if (cop == co) {
        SetCommonOccupy(nullptr);
    }
    auto up_pending = GetEnvPenging();
    if (up_pending == co) {
        SetEnvPenging(nullptr);
    }
    auto up_occupy = GetEnvOccupy();
    if (up_occupy == co) {
        SetEnvOccupy(nullptr);
    }

	RecycleMcoStack(co->coctx->stack);
	co->done = true;
	co->done_yield = true;
	co->running = false;
	co->start = false;
	
	delete co->coctx->ctx;
	co->coctx->ctx = nullptr;
	co->coctx->args = nullptr;
	
	delete co->coctx;
	co->coctx = nullptr;

    delete co;
    co = nullptr;
}
