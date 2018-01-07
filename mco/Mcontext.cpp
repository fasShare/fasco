#include "Mcontext.h"
#include <string.h>

#define EAX 0
#define EBX 1
#define ECX 2
#define EDX 3
#define EDI 4
#define ESI 5
#define EBP 6
#define ERET 7
#define ESP 8
// -----------
#define RAX 0
#define RBX 1
#define RCX 2
#define RDX 3
#define RDI 4
#define RSI 5
#define RBP 6
#define R8  7
#define R9  8
#define R10 9
#define R11 10
#define R12 11
#define R13 12
#define R14 13
#define R15 14
#define RET 15
#define RSP 16

#if defined(__i386__)
int McontextInit(Mcontext *ctx) {
    memset(ctx, 0, sizeof(*ctx));
    return 0;
}
int McontextMake(Mcontext *ctx, cofunc pfn, const void *s) {
    memset(ctx->regs, 0, sizeof(ctx->regs));

    char *sp = ctx->ss_sp + ctx->ss_size - sizeof(void *);
    sp = (char*) ((unsigned long)sp & -16L);
    unsigned int *p = (unsigned int *)sp;
    *p = (unsigned int)s;
    // popl %esp 完成以下两件事
    // (1) movl (ctx->regs[ESP]), %rsp
    // (2) subl 4, %esp 
    ctx->regs[ESP] = sp - sizeof(p);
    ctx->regs[ERET] = (void *)pfn;
    return 0;
}
#elif defined(__x86_64__)
int McontextMake(Mcontext *ctx, cofunc pfn, const void *conarg) {
    memset(ctx->regs, 0, sizeof(ctx->regs));

    char *sp = ctx->ss_sp + ctx->ss_size - 1;
    sp = (char*) ((unsigned long)sp & -16LL  );

    ctx->regs[RDI] = (void *)conarg;
    // x86_64可以通过寄存器传参，而i386通过堆栈传参
    ctx->regs[RSP] = sp - 8;
    ctx->regs[RET] = (void *)pfn;
    return 0;
}

int McontextInit(Mcontext *ctx) {
    memset(ctx, 0, sizeof(*ctx));
    return 0;
}
#endif

