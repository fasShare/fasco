.globl mco_swap
.type  mco_swap, @function
mco_swap:

#if defined(__i386__)
	leal 4(%esp), %eax
	//把参数中的regs地址放到esp里面
	movl 4(%esp), %esp
	leal 36(%esp), %esp

	pushl %eax //esp
	pushl -4(%eax) //ret addr
	pushl %ebp
	pushl %esi
	
	pushl %edi
	pushl %edx
	pushl %ecx
	pushl %ebx
	pushl %eax
	
	movl 4(%eax), %esp

	popl %eax  
	popl %ebx  
	popl %ecx
	popl %edx
	popl %edi
	
	popl %esi
	popl %ebp
	popl %eax
	popl %esp
	
	pushl %eax

	xorl %eax, %eax
	ret
#elif defined(__x86_64__)
	leaq 8(%rsp), %rax
	leaq 136(%rdi),%rsp
	/* 让栈指针始终指向当前协程的私有栈区 */	
	pushq %rax   

	pushq -8(%rax)
	pushq %r15
	pushq %r14
	pushq %r13
	pushq %r12
	pushq %r11
	pushq %r10
	pushq %r9
	pushq %r8
	pushq %rbp
	pushq %rsi
	pushq %rdi
	pushq %rdx
	pushq %rcx
	pushq %rbx
	pushq %rax
	
	movq %rsi, %rsp
	
	popq %rax
	popq %rbx
	popq %rcx
	popq %rdx
	popq %rdi
	popq %rsi
	popq %rbp
	popq %r8
	popq %r9
	popq %r10
	popq %r11
	popq %r12
	popq %r13
	popq %r14
	popq %r15
	popq %rax
	popq %rsp
	pushq %rax
	
	xorl %eax, %eax
	ret
#endif
