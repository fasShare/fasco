#ifndef MOXIE_MCONTEXT_H
#define MOXIE_MCONTEXT_H
#include <stdlib.h>

typedef void (*cofunc) (void *);

#if defined(__i386__)
struct Mcontext
{
    void *regs[9];
    size_t ss_size;
    char *ss_sp;
};
#elif defined(__x86_64__)
struct Mcontext
{
    void *regs[17];
    size_t ss_size;
    char *ss_sp;
};
#endif 
int McontextInit(Mcontext *ctx);
int McontextMake(Mcontext *ctx, cofunc pfn, const void *con_args);
#endif 
